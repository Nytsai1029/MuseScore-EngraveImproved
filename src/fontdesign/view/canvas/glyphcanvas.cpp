/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "glyphcanvas.h"

#include <algorithm>
#include <cmath>

#include <QKeyEvent>
#include <QLineF>
#include <QPainter>
#include <QPainterPath>

#include "translation.h"

#include "internal/fontdesigntypes.h"
#include "internal/project/curvefitter.h"
#include "internal/project/outlinegeometry.h"
#include "internal/project/projectcommands.h"

using namespace mu::fontdesign;
using namespace muse;

//! 定义在渲染区段；snapPoint 也要用（谱线吸附）
static double staffBottomLineSp(const GlyphItem* glyph);

static constexpr double MIN_PX_PER_UNIT = 0.01;
static constexpr double MAX_PX_PER_UNIT = 20.0;
static constexpr double ANCHOR_HIT_RADIUS_PX = 8.0;
static constexpr double POINT_HIT_RADIUS_PX = 7.0;
static constexpr double SEGMENT_HIT_RADIUS_PX = 6.0;
static constexpr double PEN_CLOSE_RADIUS_PX = 10.0;
static constexpr double NUDGE_STEP = 1.0;          // 字体单位
static constexpr double NUDGE_STEP_LARGE = 10.0;

namespace {
using OutPoint = GlyphOutline::Point;
using PointType = GlyphOutline::PointType;

//! contour 中所有 OnCurve 点的下标（按出现顺序）
std::vector<int> onCurveIndices(const GlyphOutline::Contour& c)
{
    std::vector<int> result;
    for (int i = 0; i < static_cast<int>(c.points.size()); ++i) {
        if (c.points[i].type == PointType::OnCurve) {
            result.push_back(i);
        }
    }
    return result;
}

PointF lerp(const PointF& a, const PointF& b, double t)
{
    return PointF(a.x() + (b.x() - a.x()) * t, a.y() + (b.y() - a.y()) * t);
}

PointF cubicEval(const PointF& p0, const PointF& p1, const PointF& p2, const PointF& p3, double t)
{
    const double u = 1.0 - t;
    const double w0 = u * u * u;
    const double w1 = 3 * u * u * t;
    const double w2 = 3 * u * t * t;
    const double w3 = t * t * t;
    return PointF(w0 * p0.x() + w1 * p1.x() + w2 * p2.x() + w3 * p3.x(),
                  w0 * p0.y() + w1 * p1.y() + w2 * p2.y() + w3 * p3.y());
}

PointF scaled(const PointF& p, double f)
{
    return PointF(p.x() * f, p.y() * f);
}

//! 修复 contour 不变式：相邻 OnCurve 之间只允许 0 或 2 个控制点。
//! 删除操作可能留下 1 个（或 >2 个）控制点的段——退化为直线段。
void repairContourInvariant(GlyphOutline::Contour& c)
{
    std::vector<int> onc = onCurveIndices(c);
    if (onc.empty()) {
        c.points.clear();
        return;
    }

    const int n = static_cast<int>(c.points.size());
    std::vector<bool> remove(n, false);

    for (size_t s = 0; s < onc.size(); ++s) {
        int a = onc[s];
        int b = onc[(s + 1) % onc.size()];
        // 统计 a→b 之间的控制点
        std::vector<int> ctrls;
        for (int i = (a + 1) % n; i != b; i = (i + 1) % n) {
            ctrls.push_back(i);
        }
        if (ctrls.size() != 0 && ctrls.size() != 2) {
            for (int i : ctrls) {
                remove[i] = true;
            }
        }
    }

    std::vector<OutPoint> kept;
    kept.reserve(c.points.size());
    for (int i = 0; i < n; ++i) {
        if (!remove[i]) {
            kept.push_back(c.points[i]);
        }
    }
    c.points = std::move(kept);
}

}

GlyphCanvas::GlyphCanvas(QQuickItem* parent)
    : muse::uicomponents::QuickPaintedView(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);
    applyToolCursor();
}

GlyphCanvas::~GlyphCanvas()
{
    if (fontDesignService() && fontDesignService()->activeEditSurface() == this) {
        fontDesignService()->setActiveEditSurface(nullptr);
    }
}

void GlyphCanvas::init()
{
    fontDesignService()->setActiveEditSurface(this);

    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        m_viewInitialized = false;
        resetInteraction();
        //! 参考图是临时的：换项目即清除（换字形保留，便于连续描摹相关字形）
        if (!m_refImage.isNull()) {
            m_refImage = QImage();
            m_draggingRefImage = false;
            emit referenceImageChanged();
        }
        attachToProject();
        update();
    });

    attachToProject();
    update();
}

void GlyphCanvas::attachToProject()
{
    FontDesignProjectPtr proj = project();
    if (proj.get() == m_attachedProject) {
        return;
    }

    m_attachedProject = proj.get();

    if (proj) {
        proj->currentGlyphChanged().onNotify(this, [this]() {
            resetInteraction();
            update();
        }, muse::async::Asyncable::AsyncMode::AsyncSetRepeat);

        proj->changed().onNotify(this, [this]() {
            pruneInvalidSelection();
            update();
        }, muse::async::Asyncable::AsyncMode::AsyncSetRepeat);
    }
}

//! 切字形/切项目时丢弃一切进行中的交互状态（在建钢笔路径、拖拽、框选、选择集）
void GlyphCanvas::resetInteraction()
{
    cancelEdit();
    penCancel();
    m_shapeDragging = false;
    m_marqueeActive = false;
    m_bendingSegment = false;
    m_dragAnchor.reset();
    m_dragAnchorOldValue.reset();
    m_hoverView.reset();
    clearSelection();
}

void GlyphCanvas::resetView()
{
    m_viewInitialized = false;
    update();
}

void GlyphCanvas::setTool(int tool)
{
    if (static_cast<int>(m_tool) == tool) {
        return;
    }

    const Tool newTool = static_cast<Tool>(tool);
    //! 直线钢笔 ↔ 曲线钢笔：在建路径不落定，后续落点按新模式逐段混合
    const bool penContinues = m_penActive && isPenTool()
                              && (newTool == Tool::Pen || newTool == Tool::CurvePen);
    if (!penContinues) {
        penFinish();       // 切换到非钢笔工具时结束（闭合）在建轮廓，而不是静默丢弃
    }

    cancelEdit();
    m_marqueeActive = false;
    m_bendingSegment = false;
    m_tool = newTool;
    clearSelection();
    applyToolCursor();
    emit toolChanged();
    update();
}

void GlyphCanvas::applyToolCursor()
{
    switch (m_tool) {
    case Tool::Select:
    case Tool::Node:
        setCursor(QCursor(Qt::ArrowCursor));
        break;
    case Tool::Pen:
    case Tool::CurvePen:
    case Tool::Rectangle:
    case Tool::Ellipse:
        setCursor(QCursor(Qt::CrossCursor));
        break;
    }
}

void GlyphCanvas::paint(QPainter* painter)
{
    painter->fillRect(QRectF(0, 0, width(), height()), m_backgroundColor);

    FontDesignProjectPtr proj = project();
    if (!proj) {
        return;
    }

    ensureViewInitialized();
    painter->setRenderHint(QPainter::Antialiasing);

    paintGrid(painter);
    paintStaffLines(painter);
    paintAxes(painter);
    paintReferenceImage(painter);

    const GlyphItem* glyph = currentGlyph();
    if (glyph) {
        paintAdvance(painter, *glyph);

        const GlyphOutline& outline = activeOutline();
        paintOutlineFill(painter, outline);

        // 编辑工具下叠加线框与节点
        if (m_tool != Tool::Select) {
            paintNodes(painter, outline);
        }
    }

    //! 工具预览不依赖字形存在：空白字体在空码位上首笔绘制时也要可见
    paintPenPreview(painter);
    paintShapePreview(painter);
    paintMarquee(painter);

    if (glyph) {
        paintAnchors(painter, *glyph);
    }
}

void GlyphCanvas::setBackgroundColor(const QColor& color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit colorsChanged();
        update();
    }
}

void GlyphCanvas::setGridColor(const QColor& color)
{
    if (m_gridColor != color) {
        m_gridColor = color;
        emit colorsChanged();
        update();
    }
}

void GlyphCanvas::setOutlineColor(const QColor& color)
{
    if (m_outlineColor != color) {
        m_outlineColor = color;
        emit colorsChanged();
        update();
    }
}

void GlyphCanvas::setAccentColor(const QColor& color)
{
    if (m_accentColor != color) {
        m_accentColor = color;
        emit colorsChanged();
        update();
    }
}

// ---------------------------------------------------------------------------
// 编辑对象与命中测试
// ---------------------------------------------------------------------------

const GlyphOutline& GlyphCanvas::activeOutline() const
{
    if (m_editing) {
        return m_workingOutline;
    }
    static const GlyphOutline empty;
    const GlyphItem* glyph = currentGlyph();
    return glyph ? glyph->outline : empty;
}

double GlyphCanvas::viewToUnit(double px) const
{
    return px / m_pxPerUnit;
}

GlyphCanvas::PointRef GlyphCanvas::pointAt(const QPointF& viewPos) const
{
    const GlyphOutline& outline = activeOutline();
    PointRef best;
    double bestDist = POINT_HIT_RADIUS_PX;
    for (int ci = 0; ci < static_cast<int>(outline.contours().size()); ++ci) {
        const auto& pts = outline.contours()[ci].points;
        for (int pi = 0; pi < static_cast<int>(pts.size()); ++pi) {
            QPointF pv = toView(pts[pi].pos);
            double d = QLineF(viewPos, pv).length();
            if (d <= bestDist) {
                bestDist = d;
                best = { ci, pi };
            }
        }
    }
    return best;
}

bool GlyphCanvas::segmentAt(const QPointF& viewPos, int& outContour, int& outSegOnCurveIndex, double& outT) const
{
    const GlyphOutline& outline = activeOutline();
    double bestDist = SEGMENT_HIT_RADIUS_PX;
    bool found = false;

    for (int ci = 0; ci < static_cast<int>(outline.contours().size()); ++ci) {
        const auto& pts = outline.contours()[ci].points;
        std::vector<int> onc = onCurveIndices(outline.contours()[ci]);
        if (onc.size() < 2) {
            continue;
        }

        for (size_t s = 0; s < onc.size(); ++s) {
            int aIdx = onc[s];
            int bIdx = onc[(s + 1) % onc.size()];
            int between = (aIdx + 1) % static_cast<int>(pts.size());
            bool cubic = pts[between].type == PointType::Control;

            const PointF a = pts[aIdx].pos;
            const PointF b = pts[bIdx].pos;
            PointF c1, c2;
            if (cubic) {
                c1 = pts[between].pos;
                c2 = pts[(between + 1) % static_cast<int>(pts.size())].pos;
            }

            constexpr int SAMPLES = 24;
            for (int k = 0; k <= SAMPLES; ++k) {
                double t = static_cast<double>(k) / SAMPLES;
                PointF p = cubic ? cubicEval(a, c1, c2, b, t) : lerp(a, b, t);
                double d = QLineF(viewPos, toView(p)).length();
                if (d < bestDist) {
                    bestDist = d;
                    outContour = ci;
                    outSegOnCurveIndex = aIdx;
                    outT = t;
                    found = true;
                }
            }
        }
    }
    return found;
}

// ---------------------------------------------------------------------------
// 选择集
// ---------------------------------------------------------------------------

bool GlyphCanvas::isSelected(const PointRef& ref) const
{
    return std::find(m_selection.begin(), m_selection.end(), ref) != m_selection.end();
}

void GlyphCanvas::setSingleSelection(const PointRef& ref)
{
    m_selection.clear();
    if (ref.valid()) {
        m_selection.push_back(ref);
    }
}

void GlyphCanvas::toggleSelected(const PointRef& ref)
{
    auto it = std::find(m_selection.begin(), m_selection.end(), ref);
    if (it != m_selection.end()) {
        m_selection.erase(it);
    } else {
        m_selection.push_back(ref);
    }
}

void GlyphCanvas::clearSelection()
{
    m_selection.clear();
}

void GlyphCanvas::pruneInvalidSelection()
{
    const GlyphOutline& outline = activeOutline();
    auto valid = [&outline](const PointRef& ref) {
        return ref.contour >= 0 && ref.contour < static_cast<int>(outline.contours().size())
               && ref.index >= 0 && ref.index < static_cast<int>(outline.contours()[ref.contour].points.size());
    };
    m_selection.erase(std::remove_if(m_selection.begin(), m_selection.end(),
                                     [&valid](const PointRef& r) { return !valid(r); }),
                      m_selection.end());
}

std::set<GlyphCanvas::PointRef> GlyphCanvas::expandSelectionForMove() const
{
    std::set<PointRef> result;
    const auto& contours = m_workingOutline.contours();

    for (const PointRef& ref : m_selection) {
        if (ref.contour < 0 || ref.contour >= static_cast<int>(contours.size())) {
            continue;
        }
        const auto& pts = contours[ref.contour].points;
        const int n = static_cast<int>(pts.size());
        if (ref.index < 0 || ref.index >= n) {
            continue;
        }

        result.insert(ref);
        if (pts[ref.index].type == PointType::OnCurve && n > 1) {
            int prev = (ref.index - 1 + n) % n;
            int next = (ref.index + 1) % n;
            if (pts[prev].type == PointType::Control) {
                result.insert({ ref.contour, prev });
            }
            if (pts[next].type == PointType::Control) {
                result.insert({ ref.contour, next });
            }
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// 编辑事务
// ---------------------------------------------------------------------------

void GlyphCanvas::beginEdit()
{
    if (m_editing) {
        return;
    }
    const GlyphItem* glyph = currentGlyph();
    m_editStartOutline = glyph ? glyph->outline : GlyphOutline();
    m_workingOutline = m_editStartOutline;
    m_editing = true;
}

void GlyphCanvas::commitEdit()
{
    if (!m_editing) {
        return;
    }

    //! 无实际变化（如单击仅选中）不进撤销栈
    if (m_workingOutline == m_editStartOutline) {
        m_editing = false;
        update();
        return;
    }

    FontDesignProjectPtr proj = project();
    if (proj) {
        proj->undoStack().push(std::make_unique<ReplaceOutlineCommand>(proj.get(), proj->currentGlyph(), m_workingOutline));
    }

    m_editing = false;
    pruneInvalidSelection();
    update();
}

void GlyphCanvas::cancelEdit()
{
    m_editing = false;
    m_draggingSelection = false;
    m_draggingWhole = false;
    m_bendingSegment = false;
}

void GlyphCanvas::movePoint(PointRef ref, const PointF& newPos, bool alignOppositeTangent)
{
    if (!ref.valid() || ref.contour >= static_cast<int>(m_workingOutline.contours().size())) {
        return;
    }
    auto& pts = m_workingOutline.contours()[ref.contour].points;
    if (ref.index >= static_cast<int>(pts.size())) {
        return;
    }

    const int n = static_cast<int>(pts.size());
    OutPoint& p = pts[ref.index];

    if (p.type == PointType::OnCurve) {
        // 移动 on-curve 点：随之刚性平移其相邻控制柄
        PointF delta = newPos - p.pos;
        p.pos = newPos;
        int prev = (ref.index - 1 + n) % n;
        int next = (ref.index + 1) % n;
        if (pts[prev].type == PointType::Control) {
            pts[prev].pos += delta;
        }
        if (pts[next].type == PointType::Control) {
            pts[next].pos += delta;
        }
    } else {
        // 移动控制柄：确定其属主 on-curve；属主 smooth 或按住 Shift 时
        // 对侧柄保持自身长度、方向随动，使两侧切线保持在同一直线上
        p.pos = newPos;
        int prev = (ref.index - 1 + n) % n;
        int next = (ref.index + 1) % n;

        int ownerIdx = -1;
        int oppositeIdx = -1;
        if (pts[prev].type == PointType::OnCurve) {
            ownerIdx = prev;
            oppositeIdx = (prev - 1 + n) % n;   // 属主的另一侧控制柄
        } else if (pts[next].type == PointType::OnCurve) {
            ownerIdx = next;
            oppositeIdx = (next + 1) % n;
        }

        const bool align = ownerIdx >= 0 && (alignOppositeTangent || pts[ownerIdx].smooth);
        if (align && oppositeIdx >= 0 && pts[oppositeIdx].type == PointType::Control) {
            const PointF o = pts[ownerIdx].pos;
            PointF dir = newPos - o;
            const double len = std::hypot(dir.x(), dir.y());
            if (len > 1e-9) {
                const PointF opp = pts[oppositeIdx].pos - o;
                const double oppLen = std::hypot(opp.x(), opp.y());
                pts[oppositeIdx].pos = PointF(o.x() - dir.x() / len * oppLen,
                                              o.y() - dir.y() / len * oppLen);
            }
        }
    }
}

void GlyphCanvas::translateSelection(const PointF& delta)
{
    std::set<PointRef> toMove = expandSelectionForMove();
    for (const PointRef& ref : toMove) {
        auto& pts = m_workingOutline.contours()[ref.contour].points;
        pts[ref.index].pos += delta;
    }
}

//! 方向键微调：一次按键 = 一条撤销记录
void GlyphCanvas::nudgeSelection(const PointF& delta)
{
    if (m_tool == Tool::Select) {
        const GlyphItem* glyph = currentGlyph();
        if (!glyph || glyph->outline.isEmpty()) {
            return;
        }
        beginEdit();
        m_workingOutline.translate(delta);
        commitEdit();
        return;
    }

    if (m_selection.empty()) {
        return;
    }
    beginEdit();
    translateSelection(delta);
    commitEdit();
}

void GlyphCanvas::insertPointOnSegment(int contour, int segOnCurveIndex, double t)
{
    beginEdit();
    auto& pts = m_workingOutline.contours()[contour].points;
    const int n = static_cast<int>(pts.size());
    const int aIdx = segOnCurveIndex;
    const int between = (aIdx + 1) % n;
    const bool cubic = pts[between].type == PointType::Control;

    // 找该段末端 on-curve
    std::vector<int> onc = onCurveIndices(m_workingOutline.contours()[contour]);
    int bIdx = -1;
    for (size_t s = 0; s < onc.size(); ++s) {
        if (onc[s] == aIdx) {
            bIdx = onc[(s + 1) % onc.size()];
            break;
        }
    }
    if (bIdx < 0) {
        cancelEdit();
        return;
    }

    if (!cubic) {
        // 直线段：在 a 之后插入一个 on-curve 中点
        OutPoint mid(lerp(pts[aIdx].pos, pts[bIdx].pos, t), PointType::OnCurve);
        pts.insert(pts.begin() + aIdx + 1, mid);
    } else {
        const PointF a = pts[aIdx].pos;
        const PointF c1 = pts[between].pos;
        const PointF c2 = pts[(between + 1) % n].pos;
        const PointF b = pts[bIdx].pos;

        // de Casteljau 在 t 处细分
        PointF p01 = lerp(a, c1, t);
        PointF p12 = lerp(c1, c2, t);
        PointF p23 = lerp(c2, b, t);
        PointF p012 = lerp(p01, p12, t);
        PointF p123 = lerp(p12, p23, t);
        PointF mid = lerp(p012, p123, t);

        OutPoint midPt(mid, PointType::OnCurve);
        midPt.smooth = true;

        // 替换 [c1, c2] 为 [p01, p012, mid, p123, p23]
        std::vector<OutPoint> replacement = {
            OutPoint(p01, PointType::Control),
            OutPoint(p012, PointType::Control),
            midPt,
            OutPoint(p123, PointType::Control),
            OutPoint(p23, PointType::Control),
        };
        pts.erase(pts.begin() + between, pts.begin() + between + 2);
        pts.insert(pts.begin() + between, replacement.begin(), replacement.end());
    }

    clearSelection();
    commitEdit();
}

void GlyphCanvas::deleteSelection()
{
    pruneInvalidSelection();
    if (m_selection.empty()) {
        return;
    }
    const GlyphItem* glyph = currentGlyph();
    if (!glyph) {
        return;
    }

    beginEdit();

    // 每条 contour 收集要删除的下标：on-curve 连带两侧控制柄；控制柄连带其配对柄
    const int contourCount = static_cast<int>(m_workingOutline.contours().size());
    std::vector<std::set<int>> toDelete(contourCount);

    for (const PointRef& ref : m_selection) {
        if (ref.contour >= contourCount) {
            continue;
        }
        const auto& pts = m_workingOutline.contours()[ref.contour].points;
        const int n = static_cast<int>(pts.size());
        if (ref.index >= n) {
            continue;
        }

        std::set<int>& del = toDelete[ref.contour];
        if (pts[ref.index].type == PointType::OnCurve) {
            del.insert(ref.index);
            int prev = (ref.index - 1 + n) % n;
            int next = (ref.index + 1) % n;
            if (pts[prev].type == PointType::Control) {
                del.insert(prev);
            }
            if (pts[next].type == PointType::Control) {
                del.insert(next);
            }
        } else {
            del.insert(ref.index);
            int prev = (ref.index - 1 + n) % n;
            int partner = (pts[prev].type == PointType::Control) ? prev : (ref.index + 1) % n;
            if (pts[partner].type == PointType::Control) {
                del.insert(partner);
            }
        }
    }

    std::vector<GlyphOutline::Contour> kept;
    kept.reserve(contourCount);
    for (int ci = 0; ci < contourCount; ++ci) {
        const auto& src = m_workingOutline.contours()[ci].points;
        GlyphOutline::Contour c;
        for (int i = 0; i < static_cast<int>(src.size()); ++i) {
            if (toDelete[ci].find(i) == toDelete[ci].end()) {
                c.points.push_back(src[i]);
            }
        }
        repairContourInvariant(c);
        // 不足以构成轮廓（少于 2 个 on-curve）→ 丢弃整条
        if (onCurveIndices(c).size() >= 2) {
            kept.push_back(c);
        }
    }
    m_workingOutline.contours() = std::move(kept);

    clearSelection();
    commitEdit();
}

void GlyphCanvas::toggleSmoothSelection()
{
    pruneInvalidSelection();
    if (m_selection.empty()) {
        return;
    }
    const GlyphItem* glyph = currentGlyph();
    if (!glyph) {
        return;
    }

    beginEdit();
    for (const PointRef& ref : m_selection) {
        if (ref.contour >= static_cast<int>(m_workingOutline.contours().size())) {
            continue;
        }
        auto& pts = m_workingOutline.contours()[ref.contour].points;
        const int n = static_cast<int>(pts.size());
        if (ref.index >= n || pts[ref.index].type != PointType::OnCurve) {
            continue;
        }

        OutPoint& p = pts[ref.index];
        p.smooth = !p.smooth;

        //! 转为 smooth 时把两侧控制柄对齐到过该点的直线（保持各自长度），
        //! 否则 smooth 标记与实际几何不符
        if (p.smooth && n > 2) {
            int prev = (ref.index - 1 + n) % n;
            int next = (ref.index + 1) % n;
            if (pts[prev].type == PointType::Control && pts[next].type == PointType::Control) {
                PointF a = pts[prev].pos - p.pos;
                PointF b = pts[next].pos - p.pos;
                PointF dir = b - a;
                double len = std::hypot(dir.x(), dir.y());
                if (len > 1e-9) {
                    dir = PointF(dir.x() / len, dir.y() / len);
                    double la = std::hypot(a.x(), a.y());
                    double lb = std::hypot(b.x(), b.y());
                    pts[prev].pos = PointF(p.pos.x() - dir.x() * la, p.pos.y() - dir.y() * la);
                    pts[next].pos = PointF(p.pos.x() + dir.x() * lb, p.pos.y() + dir.y() * lb);
                }
            }
        }
    }
    commitEdit();
}

// ---------------------------------------------------------------------------
// 布尔运算 / 翻转 / 捕捉
// ---------------------------------------------------------------------------

void GlyphCanvas::applyBoolean(BooleanOp op)
{
    const GlyphItem* glyph = currentGlyph();
    if (!glyph || glyph->outline.isEmpty()) {
        return;
    }

    pruneInvalidSelection();

    //! 选中轮廓集合（作为第二操作数）
    std::set<int> selectedContours;
    for (const PointRef& ref : m_selection) {
        selectedContours.insert(ref.contour);
    }

    beginEdit();
    const auto& contours = m_workingOutline.contours();
    const int contourCount = static_cast<int>(contours.size());

    QPainterPath base;
    base.setFillRule(Qt::WindingFill);
    QPainterPath operand;
    operand.setFillRule(Qt::WindingFill);

    const bool hasSelection = !selectedContours.empty()
                              && static_cast<int>(selectedContours.size()) < contourCount;

    for (int ci = 0; ci < contourCount; ++ci) {
        if (hasSelection && selectedContours.count(ci)) {
            outlinegeom::appendContourToQPath(operand, contours[ci]);
        } else {
            outlinegeom::appendContourToQPath(base, contours[ci]);
        }
    }

    //! Qt 布尔内部把曲线压平成折线，误差随坐标尺度缩小：
    //! 放大 16 倍运算再缩回，压平误差 /16，重拟合输入接近精确曲线采样
    constexpr double BOOLEAN_UPSCALE = 16.0;
    QTransform upScale;
    upScale.scale(BOOLEAN_UPSCALE, BOOLEAN_UPSCALE);
    QTransform downScale;
    downScale.scale(1.0 / BOOLEAN_UPSCALE, 1.0 / BOOLEAN_UPSCALE);
    base = upScale.map(base);
    operand = upScale.map(operand);

    QPainterPath resultPath;
    switch (op) {
    case BooleanOp::Union:
        resultPath = hasSelection ? base.united(operand) : base.simplified();
        break;
    case BooleanOp::Subtract:
        if (!hasSelection) {
            cancelEdit();
            return;
        }
        resultPath = base.subtracted(operand);
        break;
    case BooleanOp::Intersect:
        if (!hasSelection) {
            cancelEdit();
            return;
        }
        resultPath = base.intersected(operand);
        break;
    }
    resultPath = downScale.map(resultPath);

    //! Qt 布尔运算把曲线压平成密集直线段：重拟合恢复三次贝塞尔（直线边保持直线）
    FontDesignProjectPtr proj = project();
    const double upem = proj ? proj->upem() : 1000.0;
    m_workingOutline.contours() = CurveFitter::refit(GlyphOutline::fromQPainterPath(resultPath), upem).contours();
    clearSelection();
    commitEdit();
}

void GlyphCanvas::booleanUnion()
{
    applyBoolean(BooleanOp::Union);
}

void GlyphCanvas::booleanSubtract()
{
    applyBoolean(BooleanOp::Subtract);
}

void GlyphCanvas::booleanIntersect()
{
    applyBoolean(BooleanOp::Intersect);
}

void GlyphCanvas::applyFlip(bool horizontal)
{
    const GlyphItem* glyph = currentGlyph();
    if (!glyph || glyph->outline.isEmpty()) {
        return;
    }

    pruneInvalidSelection();

    std::set<int> selectedContours;
    for (const PointRef& ref : m_selection) {
        selectedContours.insert(ref.contour);
    }

    beginEdit();
    auto& contours = m_workingOutline.contours();

    //! 无选择 = 全部
    std::vector<int> targets;
    for (int ci = 0; ci < static_cast<int>(contours.size()); ++ci) {
        if (selectedContours.empty() || selectedContours.count(ci)) {
            targets.push_back(ci);
        }
    }
    if (targets.empty()) {
        cancelEdit();
        return;
    }

    //! 目标轮廓集的联合包围盒中心为镜像轴
    RectF bbox;
    bool first = true;
    for (int ci : targets) {
        for (const OutPoint& p : contours[ci].points) {
            if (first) {
                bbox = RectF(p.pos.x(), p.pos.y(), 0, 0);
                first = false;
            } else {
                bbox = bbox.united(RectF(p.pos.x(), p.pos.y(), 0, 0));
            }
        }
    }

    const double cx = bbox.x() + bbox.width() / 2.0;
    const double cy = bbox.y() + bbox.height() / 2.0;

    for (int ci : targets) {
        for (OutPoint& p : contours[ci].points) {
            if (horizontal) {
                p.pos = PointF(2.0 * cx - p.pos.x(), p.pos.y());
            } else {
                p.pos = PointF(p.pos.x(), 2.0 * cy - p.pos.y());
            }
        }
        outlinegeom::reverseContour(contours[ci]);
    }

    clearSelection();
    commitEdit();
}

void GlyphCanvas::flipHorizontally()
{
    applyFlip(true);
}

void GlyphCanvas::flipVertically()
{
    applyFlip(false);
}

void GlyphCanvas::correctPathDirections()
{
    const GlyphItem* glyph = currentGlyph();
    if (!glyph || glyph->outline.isEmpty()) {
        return;
    }

    beginEdit();
    auto& contours = m_workingOutline.contours();
    for (int ci = 0; ci < static_cast<int>(contours.size()); ++ci) {
        outlinegeom::orientContourByDepth(contours, ci);
    }
    clearSelection();
    commitEdit();
}

void GlyphCanvas::orientNewContour(int contourIndex)
{
    auto& contours = m_workingOutline.contours();
    if (contourIndex < 0 || contourIndex >= static_cast<int>(contours.size())) {
        return;
    }
    outlinegeom::orientContourByDepth(contours, contourIndex);
}

void GlyphCanvas::setSnapEnabled(bool enabled)
{
    if (m_snapEnabled != enabled) {
        m_snapEnabled = enabled;
        emit snapEnabledChanged();
    }
}

// ---------------------------------------------------------------------------
// 临时参考图
// ---------------------------------------------------------------------------

void GlyphCanvas::importReferenceImage()
{
    std::vector<std::string> filter = {
        muse::trc("fontdesign", "Images") + " (*.png *.jpg *.jpeg *.bmp *.gif *.tif *.tiff *.webp)"
    };
    io::path_t path = interactive()->selectOpeningFileSync(
        muse::trc("fontdesign", "Import reference image"), io::path_t(), filter);
    if (path.empty()) {
        return;
    }

    QImage image(path.toQString());
    if (image.isNull()) {
        interactive()->error(muse::trc("fontdesign", "Unable to import image"),
                             muse::trc("fontdesign", "The file could not be read as an image."));
        return;
    }

    m_refImage = std::move(image);
    resetReferenceImagePlacement();
    emit referenceImageChanged();
    update();
}

void GlyphCanvas::clearReferenceImage()
{
    if (m_refImage.isNull()) {
        return;
    }
    m_refImage = QImage();
    m_draggingRefImage = false;
    emit referenceImageChanged();
    update();
}

//! 默认摆放：图高 = 1 em（谱表高），左下角在 x=0、谱表最低线上
void GlyphCanvas::resetReferenceImagePlacement()
{
    FontDesignProjectPtr proj = project();
    if (m_refImage.isNull() || !proj || m_refImage.height() <= 0) {
        return;
    }

    const double spatium = proj->spatium();
    m_refScale = (4.0 * spatium) / m_refImage.height();
    m_refPosFont = PointF(0.0, staffBottomLineSp(currentGlyph()) * spatium);
    update();
}

void GlyphCanvas::setReferenceImageOpacity(double opacity)
{
    const double clamped = std::clamp(opacity, 0.05, 1.0);
    if (std::abs(m_refOpacity - clamped) > 1e-6) {
        m_refOpacity = clamped;
        emit referenceImageChanged();
        update();
    }
}

void GlyphCanvas::paintReferenceImage(QPainter* painter)
{
    if (m_refImage.isNull()) {
        return;
    }

    const double wUnits = m_refImage.width() * m_refScale;
    const double hUnits = m_refImage.height() * m_refScale;
    const QPointF topLeft = toView(PointF(m_refPosFont.x(), m_refPosFont.y() + hUnits));
    const QSizeF size(wUnits * m_pxPerUnit, hUnits * m_pxPerUnit);

    painter->setOpacity(m_refOpacity);
    painter->drawImage(QRectF(topLeft, size), m_refImage);
    painter->setOpacity(1.0);
}

muse::PointF GlyphCanvas::snapPoint(const PointF& raw) const
{
    if (!m_snapEnabled) {
        return raw;
    }

    //! 基础捕捉：整数字体单位（CFF 坐标友好）
    PointF snapped(std::round(raw.x()), std::round(raw.y()));

    FontDesignProjectPtr proj = project();
    if (!proj) {
        return snapped;
    }

    const double threshold = viewToUnit(6.0);
    const double spatium = proj->spatium();

    //! 谱线 y 吸附
    const double bottomSp = staffBottomLineSp(currentGlyph());
    for (int i = 0; i <= 4; ++i) {
        const double lineY = (bottomSp + i) * spatium;
        if (std::abs(raw.y() - lineY) < threshold) {
            snapped = PointF(snapped.x(), lineY);
            break;
        }
    }

    //! x=0（注册原点）与 advance 线吸附
    if (std::abs(raw.x()) < threshold) {
        snapped = PointF(0.0, snapped.y());
    } else if (const GlyphItem* glyph = currentGlyph()) {
        if (glyph->advance > 0 && std::abs(raw.x() - glyph->advance) < threshold) {
            snapped = PointF(glyph->advance, snapped.y());
        }
    }

    return snapped;
}

// ---------------------------------------------------------------------------
// IFontDesignEditSurface（页面级复制/粘贴/全选经此桥接画布选择集）
// ---------------------------------------------------------------------------

bool GlyphCanvas::hasOutlineSelection() const
{
    const GlyphOutline& outline = activeOutline();
    for (const PointRef& ref : m_selection) {
        if (ref.contour >= 0 && ref.contour < static_cast<int>(outline.contours().size())
            && ref.index >= 0 && ref.index < static_cast<int>(outline.contours()[ref.contour].points.size())) {
            return true;
        }
    }
    return false;
}

GlyphOutline GlyphCanvas::selectionAsOutline() const
{
    GlyphOutline result;
    const GlyphOutline& outline = activeOutline();

    std::set<int> contourIndices;
    for (const PointRef& ref : m_selection) {
        if (ref.contour >= 0 && ref.contour < static_cast<int>(outline.contours().size())
            && ref.index >= 0 && ref.index < static_cast<int>(outline.contours()[ref.contour].points.size())) {
            contourIndices.insert(ref.contour);
        }
    }
    for (int ci : contourIndices) {
        result.contours().push_back(outline.contours()[ci]);
    }
    return result;
}

void GlyphCanvas::pasteOutline(const GlyphOutline& outline)
{
    if (outline.isEmpty()) {
        return;
    }
    FontDesignProjectPtr proj = project();
    if (!proj || !proj->glyph(proj->currentGlyph())) {
        return;
    }

    penFinish();      // 粘贴前落定在建路径

    beginEdit();
    const int firstNew = static_cast<int>(m_workingOutline.contours().size());
    for (const GlyphOutline::Contour& c : outline.contours()) {
        m_workingOutline.contours().push_back(c);
    }
    commitEdit();

    //! 选中粘贴内容（on-curve），可立即整体移动；选择需要节点工具可见
    if (m_tool != Tool::Node) {
        setTool(static_cast<int>(Tool::Node));
    }
    clearSelection();
    const GlyphItem* glyph = currentGlyph();
    if (glyph) {
        for (int ci = firstNew; ci < static_cast<int>(glyph->outline.contours().size()); ++ci) {
            const auto& pts = glyph->outline.contours()[ci].points;
            for (int pi = 0; pi < static_cast<int>(pts.size()); ++pi) {
                if (pts[pi].type == PointType::OnCurve) {
                    m_selection.push_back({ ci, pi });
                }
            }
        }
    }
    update();
}

void GlyphCanvas::selectAllPoints()
{
    const GlyphItem* glyph = currentGlyph();
    if (!glyph || glyph->outline.isEmpty()) {
        return;
    }

    penFinish();

    //! 选择集只在节点工具下可见/可操作
    if (m_tool != Tool::Node) {
        setTool(static_cast<int>(Tool::Node));
    }

    clearSelection();
    for (int ci = 0; ci < static_cast<int>(glyph->outline.contours().size()); ++ci) {
        const auto& pts = glyph->outline.contours()[ci].points;
        for (int pi = 0; pi < static_cast<int>(pts.size()); ++pi) {
            if (pts[pi].type == PointType::OnCurve) {
                m_selection.push_back({ ci, pi });
            }
        }
    }
    update();
}

// ---------------------------------------------------------------------------
// 段类型切换（右键，节点工具）：曲线 ↔ 直线
// ---------------------------------------------------------------------------

//! 段（从 segStartOnCurveIndex 出发到下一个 on-curve）设为曲线或直线。
//! 直线→曲线：控制点放在三分点（形状不变，可继续拖弯）；曲线→直线：删除控制点。
//! contour 以 on-curve 开头（模型不变式），段的控制点总在起点之后，编辑不影响更靠前的下标。
void GlyphCanvas::setSegmentCurved(int contour, int segStartOnCurveIndex, bool curved)
{
    auto& pts = m_workingOutline.contours()[contour].points;
    const int n = static_cast<int>(pts.size());
    const int aIdx = segStartOnCurveIndex;
    const bool isCurved = pts[(aIdx + 1) % n].type == PointType::Control;
    if (isCurved == curved) {
        return;
    }

    if (curved) {
        // 找该段末端 on-curve
        std::vector<int> onc = onCurveIndices(m_workingOutline.contours()[contour]);
        int bIdx = -1;
        for (size_t s = 0; s < onc.size(); ++s) {
            if (onc[s] == aIdx) {
                bIdx = onc[(s + 1) % onc.size()];
                break;
            }
        }
        if (bIdx < 0) {
            return;
        }
        const PointF a = pts[aIdx].pos;
        const PointF b = pts[bIdx].pos;
        const PointF third((b.x() - a.x()) / 3.0, (b.y() - a.y()) / 3.0);
        std::vector<OutPoint> controls = {
            OutPoint(a + third, PointType::Control),
            OutPoint(PointF(a.x() + 2.0 * third.x(), a.y() + 2.0 * third.y()), PointType::Control),
        };
        pts.insert(pts.begin() + aIdx + 1, controls.begin(), controls.end());
    } else {
        pts.erase(pts.begin() + aIdx + 1, pts.begin() + aIdx + 3);
    }
}

void GlyphCanvas::toggleCurveModeAt(const QPointF& viewPos)
{
    const GlyphItem* glyph = currentGlyph();
    if (!glyph || glyph->outline.isEmpty()) {
        return;
    }

    //! 命中节点：切换该节点两侧段（都为曲线→全转直线；否则全转曲线）
    PointRef ref = pointAt(viewPos);
    if (ref.valid()) {
        const auto& cpts = activeOutline().contours()[ref.contour].points;
        if (cpts[ref.index].type == PointType::OnCurve) {
            beginEdit();
            auto& pts = m_workingOutline.contours()[ref.contour].points;
            const int n = static_cast<int>(pts.size());

            const bool outCurved = pts[(ref.index + 1) % n].type == PointType::Control;
            int prevOn = -1;
            for (int k = 1; k < n; ++k) {
                int idx = (ref.index - k + n) % n;
                if (pts[idx].type == PointType::OnCurve) {
                    prevOn = idx;
                    break;
                }
            }
            const bool inCurved = prevOn >= 0
                                  && pts[(prevOn + 1) % n].type == PointType::Control;
            const bool target = !(inCurved && outCurved);

            //! 先出段（编辑位置在节点之后），再入段（重新扫描起点下标）
            setSegmentCurved(ref.contour, ref.index, target);
            if (prevOn >= 0) {
                setSegmentCurved(ref.contour, prevOn, target);
            }
            commitEdit();
            return;
        }
    }

    //! 命中段：切换单段
    int contour = -1;
    int segIdx = -1;
    double t = 0.0;
    if (segmentAt(viewPos, contour, segIdx, t)) {
        beginEdit();
        const auto& pts = m_workingOutline.contours()[contour].points;
        const int n = static_cast<int>(pts.size());
        const bool curved = pts[(segIdx + 1) % n].type == PointType::Control;
        setSegmentCurved(contour, segIdx, !curved);
        commitEdit();
    }
}

// ---------------------------------------------------------------------------
// 钢笔工具（直线/曲线可中途互切，逐段混合）
// ---------------------------------------------------------------------------

//! 由在建节点构造闭合轮廓：曲线段用 Catmull-Rom 切线生成三次贝塞尔；
//! 两侧均为曲线的节点为 smooth（对称切线），直线/曲线过渡节点为角点（单侧弦向切线）。
GlyphOutline::Contour GlyphCanvas::buildPenContour(const std::vector<PenNode>& nodes, bool closingCurved) const
{
    GlyphOutline::Contour c;
    const int n = static_cast<int>(nodes.size());
    if (n < 2) {
        for (const PenNode& node : nodes) {
            c.points.emplace_back(node.pos, PointType::OnCurve);
        }
        return c;
    }

    //! 段 i：节点 i → 节点 (i+1)%n；末段（闭合段）类型由 closingCurved 决定
    auto segCurved = [&](int i) {
        i = ((i % n) + n) % n;
        return i < n - 1 ? nodes[i + 1].curveFromPrev : closingCurved;
    };
    auto pos = [&](int i) {
        return nodes[((i % n) + n) % n].pos;
    };
    auto smoothAt = [&](int i) {
        return segCurved(i - 1) && segCurved(i);
    };

    for (int i = 0; i < n; ++i) {
        OutPoint on(pos(i), PointType::OnCurve);
        on.smooth = smoothAt(i);
        c.points.push_back(on);

        if (!segCurved(i)) {
            continue;
        }

        const int j = i + 1;
        const PointF chord = pos(j) - pos(i);
        const PointF tOut = smoothAt(i) ? scaled(pos(i + 1) - pos(i - 1), 0.5) : chord;
        const PointF tIn = smoothAt(j) ? scaled(pos(j + 1) - pos(j - 1), 0.5) : chord;

        c.points.emplace_back(pos(i) + scaled(tOut, 1.0 / 3.0), PointType::Control);
        c.points.emplace_back(pos(j) - scaled(tIn, 1.0 / 3.0), PointType::Control);
    }
    return c;
}

void GlyphCanvas::penPress(const QPointF& viewPos, bool close)
{
    if (close && m_penNodes.size() >= 2) {
        penFinish();
        return;
    }
    PenNode node;
    node.pos = snapPoint(fromView(viewPos));
    node.curveFromPrev = (m_tool == Tool::CurvePen);
    m_penNodes.push_back(node);
    m_penNodeDrag = true;      // 按住即可拖动刚落的节点，实时调整位置/曲率
    setPenActive(true);
    update();
}

void GlyphCanvas::penMoveLastNode(const QPointF& viewPos)
{
    if (m_penNodes.empty()) {
        return;
    }
    m_penNodes.back().pos = snapPoint(fromView(viewPos));
    update();
}

void GlyphCanvas::penFinish()
{
    if (!m_penActive) {
        return;
    }
    if (m_penNodes.size() >= 2) {
        beginEdit();
        //! 闭合段类型按当前钢笔模式（点击起点/Enter/切工具时的模式）
        m_workingOutline.contours().push_back(buildPenContour(m_penNodes, m_tool == Tool::CurvePen));
        //! 画在实心区域内自动反向成孔（nonzero winding）
        orientNewContour(static_cast<int>(m_workingOutline.contours().size()) - 1);
        commitEdit();
    }
    m_penNodes.clear();
    m_penNodeDrag = false;
    setPenActive(false);
    update();
}

void GlyphCanvas::penCancel()
{
    m_penNodes.clear();
    m_penNodeDrag = false;
    setPenActive(false);
    update();
}

void GlyphCanvas::setPenActive(bool active)
{
    if (m_penActive != active) {
        m_penActive = active;
        emit penActiveChanged();
    }
}

// ---------------------------------------------------------------------------
// 曲线段直接拖弯（FontLab 式）：最小改动两个控制柄，使曲线在 t 处经过拖拽点
// ---------------------------------------------------------------------------

void GlyphCanvas::beginSegmentBend(int contour, int segOnCurveIndex, double t, const QPointF& viewPos)
{
    beginEdit();
    const auto& pts = m_workingOutline.contours()[contour].points;
    const int n = static_cast<int>(pts.size());
    const int c1 = (segOnCurveIndex + 1) % n;
    const int c2 = (c1 + 1) % n;

    m_bendingSegment = true;
    m_bendContour = contour;
    m_bendC1 = c1;
    m_bendC2 = c2;
    m_bendT = t;
    m_bendStartFont = fromView(viewPos);
    m_bendC1Start = pts[c1].pos;
    m_bendC2Start = pts[c2].pos;
}

void GlyphCanvas::updateSegmentBend(const QPointF& viewPos)
{
    if (!m_bendingSegment || !m_editing) {
        return;
    }

    const double t = m_bendT;
    const double u = 1.0 - t;
    const double w1 = 3 * u * u * t;
    const double w2 = 3 * u * t * t;
    const double denom = w1 * w1 + w2 * w2;
    if (denom < 1e-9) {
        return;
    }

    const PointF delta = fromView(viewPos) - m_bendStartFont;
    // 最小范数解：曲线在参数 t 处恰好随光标位移 delta
    const PointF e1(delta.x() * w1 / denom, delta.y() * w1 / denom);
    const PointF e2(delta.x() * w2 / denom, delta.y() * w2 / denom);

    auto& pts = m_workingOutline.contours()[m_bendContour].points;
    pts[m_bendC1].pos = m_bendC1Start + e1;
    pts[m_bendC2].pos = m_bendC2Start + e2;
    update();
}

// ---------------------------------------------------------------------------
// 键盘：画布自处理键 + ShortcutOverride 抢回（防全局 QML Shortcut 吞键）
// ---------------------------------------------------------------------------

bool GlyphCanvas::isCanvasHandledKey(const QKeyEvent* event) const
{
    if (!project()) {
        return false;
    }

    const Qt::KeyboardModifiers mods = event->modifiers()
                                       & (Qt::ControlModifier | Qt::AltModifier | Qt::ShiftModifier | Qt::MetaModifier);

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        return mods == Qt::NoModifier || mods == Qt::ShiftModifier;
    case Qt::Key_V:
    case Qt::Key_A:
    case Qt::Key_P:
    case Qt::Key_C:
    case Qt::Key_R:
    case Qt::Key_E:
    case Qt::Key_S:
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
    case Qt::Key_Escape:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        return mods == Qt::NoModifier;
    default:
        return false;
    }
}

bool GlyphCanvas::handleCanvasKey(const QKeyEvent* event)
{
    if (!isCanvasHandledKey(event)) {
        return false;
    }

    const bool shift = event->modifiers() & Qt::ShiftModifier;
    const double step = shift ? NUDGE_STEP_LARGE : NUDGE_STEP;

    switch (event->key()) {
    case Qt::Key_V: setTool(static_cast<int>(Tool::Select)); return true;
    case Qt::Key_A: setTool(static_cast<int>(Tool::Node)); return true;
    case Qt::Key_P: setTool(static_cast<int>(Tool::Pen)); return true;
    case Qt::Key_C: setTool(static_cast<int>(Tool::CurvePen)); return true;
    case Qt::Key_R: setTool(static_cast<int>(Tool::Rectangle)); return true;
    case Qt::Key_E: setTool(static_cast<int>(Tool::Ellipse)); return true;
    case Qt::Key_S:
        toggleSmoothSelection();
        return true;
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        if (m_penActive) {
            // 在建路径：撤掉最后一个落点
            if (!m_penNodes.empty()) {
                m_penNodes.pop_back();
            }
            if (m_penNodes.empty()) {
                penCancel();
            }
            update();
        } else {
            deleteSelection();
        }
        return true;
    case Qt::Key_Escape:
        if (m_penActive) {
            penCancel();
        } else if (!m_selection.empty()) {
            clearSelection();
            update();
        }
        return true;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (m_penActive) {
            penFinish();
        }
        return true;
    case Qt::Key_Left: nudgeSelection(PointF(-step, 0.0)); return true;
    case Qt::Key_Right: nudgeSelection(PointF(step, 0.0)); return true;
    case Qt::Key_Up: nudgeSelection(PointF(0.0, step)); return true;
    case Qt::Key_Down: nudgeSelection(PointF(0.0, -step)); return true;
    default:
        return false;
    }
}

bool GlyphCanvas::event(QEvent* event)
{
    //! 接受 ShortcutOverride：这些按键作为普通 KeyPress 送达画布，
    //! 而不是被全局 QML Shortcut（乐谱页注册的 S/V/A/… 序列）拦截吞掉
    if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (isCanvasHandledKey(keyEvent)) {
            event->accept();
            return true;
        }
    }
    return QuickPaintedView::event(event);
}

void GlyphCanvas::keyPressEvent(QKeyEvent* event)
{
    if (handleCanvasKey(event)) {
        event->accept();
        return;
    }
    QuickPaintedView::keyPressEvent(event);
}

// ---------------------------------------------------------------------------
// 鼠标事件
// ---------------------------------------------------------------------------

void GlyphCanvas::wheelEvent(QWheelEvent* event)
{
    QPoint delta = event->pixelDelta();
    if (delta.isNull()) {
        delta = event->angleDelta() / 4;
    }

    //! Alt+滚轮：缩放参考图（绕光标位置）
    if (!m_refImage.isNull() && (event->modifiers() & Qt::AltModifier)) {
        constexpr double zoomSpeed = 1.002;
        double magnitude = std::abs(delta.x()) > std::abs(delta.y()) ? delta.x() : delta.y();
        const double factor = std::pow(zoomSpeed, magnitude);
        const PointF anchor = fromView(event->position());
        m_refScale *= factor;
        m_refPosFont = anchor + scaled(m_refPosFont - anchor, factor);
        update();
        return;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        constexpr double zoomSpeed = 1.002;
        double magnitude = std::abs(delta.x()) > std::abs(delta.y()) ? delta.x() : delta.y();
        zoomAround(std::pow(zoomSpeed, magnitude), event->position());
    } else {
        m_originPx += QPointF(delta.x(), delta.y());
        update();
    }
}

void GlyphCanvas::mousePressEvent(QMouseEvent* event)
{
    forceActiveFocus();

    if (event->button() == Qt::MiddleButton) {
        m_panning = true;
        m_lastPanPos = event->position();
        return;
    }

    //! Alt+左键拖拽：移动参考图（任何工具下；也不要求已选字形位）
    if (!m_refImage.isNull() && event->button() == Qt::LeftButton
        && (event->modifiers() & Qt::AltModifier)) {
        m_draggingRefImage = true;
        m_dragLastView = event->position();
        return;
    }

    //! 未选中字形位（空白字体初始状态）：不允许绘制/编辑，需先在字形浏览器中选择码位
    FontDesignProjectPtr proj = project();
    if (!proj || proj->currentGlyph() == 0) {
        return;
    }

    const QPointF pos = event->position();

    // 钢笔：左/右键都落节点（右键按住拖动可调整曲率，参考 FontForge）
    if (isPenTool()) {
        if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
            bool nearStart = false;
            if (m_penNodes.size() >= 2) {
                nearStart = QLineF(pos, toView(m_penNodes.front().pos)).length() <= PEN_CLOSE_RADIUS_PX;
            }
            penPress(pos, nearStart);
        }
        return;
    }

    // 节点工具右键：切换命中节点/段的 曲线↔直线 模式
    if (event->button() == Qt::RightButton) {
        if (m_tool == Tool::Node) {
            toggleCurveModeAt(pos);
        }
        return;
    }

    if (event->button() != Qt::LeftButton) {
        return;
    }

    // 锚点拖拽（仅选择/节点工具，避免与绘制类工具抢点击）
    if (m_tool == Tool::Select || m_tool == Tool::Node) {
        if (std::optional<AnchorId> anchor = anchorAt(pos)) {
            const GlyphItem* glyph = currentGlyph();
            m_dragAnchor = anchor;
            m_dragAnchorOldValue = glyph->anchors.at(anchor.value());
            return;
        }
    }

    switch (m_tool) {
    case Tool::Select: {
        beginEdit();
        m_draggingWhole = true;
        m_dragLastView = pos;
        break;
    }
    case Tool::Node: {
        PointRef ref = pointAt(pos);
        if (ref.valid()) {
            const auto& hitPts = activeOutline().contours()[ref.contour].points;
            const bool isControl = hitPts[ref.index].type == PointType::Control;

            //! Shift+单击 on-curve：增删选择集；控制柄不参与多选，
            //! Shift+拖控制柄 = 两侧切线共线约束（见 movePoint）
            if (!isControl && (event->modifiers() & Qt::ShiftModifier)) {
                toggleSelected(ref);
                update();
                break;
            }
            if (isControl) {
                setSingleSelection(ref);
            } else if (!isSelected(ref)) {
                setSingleSelection(ref);
            }
            beginEdit();
            m_draggingSelection = true;
            m_dragLastView = pos;
            update();
            break;
        }

        // 命中曲线段：直接拖弯（直线段不弯，落入框选）
        int contour = -1;
        int segIdx = -1;
        double t = 0.0;
        if (segmentAt(pos, contour, segIdx, t)) {
            const auto& pts = activeOutline().contours()[contour].points;
            const int n = static_cast<int>(pts.size());
            const bool cubic = pts[(segIdx + 1) % n].type == PointType::Control;
            if (cubic && t > 0.05 && t < 0.95) {
                clearSelection();
                beginSegmentBend(contour, segIdx, t, pos);
                update();
                break;
            }
        }

        // 空白处：框选（Shift = 追加）
        m_marqueeActive = true;
        m_marqueeAdditive = event->modifiers() & Qt::ShiftModifier;
        if (!m_marqueeAdditive) {
            clearSelection();
        }
        m_marqueeStartView = pos;
        m_marqueeCurView = pos;
        update();
        break;
    }
    case Tool::Pen:
    case Tool::CurvePen:
        break;      // 已在上面处理
    case Tool::Rectangle:
    case Tool::Ellipse: {
        m_shapeDragging = true;
        m_shapeStartView = pos;
        m_shapeCurView = pos;
        break;
    }
    }
}

void GlyphCanvas::mouseMoveEvent(QMouseEvent* event)
{
    const QPointF pos = event->position();

    if (m_panning) {
        m_originPx += pos - m_lastPanPos;
        m_lastPanPos = pos;
        update();
        return;
    }

    if (m_draggingRefImage) {
        m_refPosFont += fromView(pos) - fromView(m_dragLastView);
        m_dragLastView = pos;
        update();
        return;
    }

    if (m_penNodeDrag && m_penActive) {
        penMoveLastNode(pos);
        return;
    }

    if (m_dragAnchor.has_value()) {
        FontDesignProjectPtr proj = project();
        GlyphItem* glyph = proj ? proj->glyphMut(proj->currentGlyph()) : nullptr;
        if (glyph) {
            PointF fontPos = snapPoint(fromView(pos));
            glyph->anchors[m_dragAnchor.value()] = PointF(fontPos.x() / proj->spatium(), fontPos.y() / proj->spatium());
            proj->notifyChanged();
            update();
        }
        return;
    }

    if (m_draggingSelection && m_editing) {
        if (m_selection.size() == 1) {
            // 单点拖拽：绝对跟随光标（捕捉后）；Shift = 两侧切线保持共线（临时平滑）
            const bool alignTangent = event->modifiers() & Qt::ShiftModifier;
            movePoint({ m_selection.front().contour, m_selection.front().index }, snapPoint(fromView(pos)), alignTangent);
        } else {
            translateSelection(fromView(pos) - fromView(m_dragLastView));
        }
        m_dragLastView = pos;
        update();
        return;
    }

    if (m_bendingSegment && m_editing) {
        updateSegmentBend(pos);
        return;
    }

    if (m_draggingWhole && m_editing) {
        PointF delta = fromView(pos) - fromView(m_dragLastView);
        m_workingOutline.translate(delta);
        m_dragLastView = pos;
        update();
        return;
    }

    if (m_marqueeActive) {
        m_marqueeCurView = pos;
        update();
        return;
    }

    if (m_shapeDragging) {
        m_shapeCurView = pos;
        update();
        return;
    }
}

void GlyphCanvas::mouseReleaseEvent(QMouseEvent* event)
{
    m_panning = false;

    if (m_draggingRefImage) {
        m_draggingRefImage = false;
        return;
    }

    if (m_penNodeDrag) {
        m_penNodeDrag = false;
        return;
    }

    if (m_dragAnchor.has_value()) {
        FontDesignProjectPtr proj = project();
        const GlyphItem* glyph = currentGlyph();
        if (proj && glyph) {
            auto it = glyph->anchors.find(m_dragAnchor.value());
            if (it != glyph->anchors.end()) {
                proj->undoStack().push(std::make_unique<SetAnchorCommand>(proj.get(), proj->currentGlyph(),
                                                                          m_dragAnchor.value(),
                                                                          m_dragAnchorOldValue, it->second));
            }
        }
        m_dragAnchor.reset();
        m_dragAnchorOldValue.reset();
        return;
    }

    if (m_draggingSelection) {
        m_draggingSelection = false;
        commitEdit();
        return;
    }

    if (m_bendingSegment) {
        m_bendingSegment = false;
        commitEdit();
        return;
    }

    if (m_draggingWhole) {
        m_draggingWhole = false;
        commitEdit();
        return;
    }

    if (m_marqueeActive) {
        m_marqueeActive = false;
        QRectF rect = QRectF(m_marqueeStartView, m_marqueeCurView).normalized();
        // 单击空白（无拖动）时仅清除选择；有面积才做框选
        if (rect.width() > 2.0 || rect.height() > 2.0) {
            const GlyphOutline& outline = activeOutline();
            for (int ci = 0; ci < static_cast<int>(outline.contours().size()); ++ci) {
                const auto& pts = outline.contours()[ci].points;
                for (int pi = 0; pi < static_cast<int>(pts.size()); ++pi) {
                    if (pts[pi].type != PointType::OnCurve) {
                        continue;
                    }
                    if (rect.contains(toView(pts[pi].pos))) {
                        PointRef ref { ci, pi };
                        if (!isSelected(ref)) {
                            m_selection.push_back(ref);
                        }
                    }
                }
            }
        }
        update();
        return;
    }

    if (m_shapeDragging) {
        m_shapeDragging = false;
        PointF a = snapPoint(fromView(m_shapeStartView));
        PointF b = snapPoint(fromView(event->position()));
        RectF rect(std::min(a.x(), b.x()), std::min(a.y(), b.y()),
                   std::abs(b.x() - a.x()), std::abs(b.y() - a.y()));
        if (rect.width() > 1.0 && rect.height() > 1.0) {
            beginEdit();
            GlyphOutline::Contour c = (m_tool == Tool::Rectangle)
                                      ? GlyphOutline::rectContour(rect)
                                      : GlyphOutline::ellipseContour(rect);
            m_workingOutline.contours().push_back(c);
            //! 画在实心区域内自动反向成孔（nonzero winding）
            orientNewContour(static_cast<int>(m_workingOutline.contours().size()) - 1);
            commitEdit();
        } else {
            cancelEdit();
        }
        update();
        return;
    }
}

void GlyphCanvas::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    // 钢笔：双击 = 落定最后一点并闭合
    if (isPenTool()) {
        if (m_penActive) {
            penFinish();
        }
        return;
    }

    if (m_tool != Tool::Node) {
        return;
    }

    // 双击 on-curve 点：切换 smooth/corner（FontLab 习惯）
    PointRef ref = pointAt(event->position());
    if (ref.valid()) {
        const GlyphOutline& outline = activeOutline();
        const auto& pts = outline.contours()[ref.contour].points;
        if (pts[ref.index].type == PointType::OnCurve) {
            setSingleSelection(ref);
            toggleSmoothSelection();
        }
        return;
    }

    // 双击线段：插入节点
    int contour = -1;
    int segIdx = -1;
    double t = 0.0;
    if (segmentAt(event->position(), contour, segIdx, t)) {
        insertPointOnSegment(contour, segIdx, t);
    }
}

void GlyphCanvas::hoverMoveEvent(QHoverEvent* event)
{
    //! 钢笔在建路径：光标随动预览（下一段 + 闭合虚线）
    if (m_penActive) {
        m_hoverView = event->position();
        update();
    } else if (m_hoverView.has_value()) {
        m_hoverView.reset();
        update();
    }
    QuickPaintedView::hoverMoveEvent(event);
}

// ---------------------------------------------------------------------------
// 视图与坐标
// ---------------------------------------------------------------------------

FontDesignProjectPtr GlyphCanvas::project() const
{
    return fontDesignService()->currentProject();
}

const GlyphItem* GlyphCanvas::currentGlyph() const
{
    FontDesignProjectPtr proj = project();
    return proj ? proj->glyph(proj->currentGlyph()) : nullptr;
}

void GlyphCanvas::ensureViewInitialized()
{
    if (m_viewInitialized || width() <= 0 || height() <= 0) {
        return;
    }

    FontDesignProjectPtr proj = project();
    if (!proj) {
        return;
    }

    const double em = proj->upem();
    m_pxPerUnit = std::min(width() / (2.0 * em), height() / (2.5 * em));
    m_originPx = QPointF(width() * 0.25, height() * 0.7);
    m_viewInitialized = true;
}

void GlyphCanvas::zoomAround(double factor, const QPointF& viewPos)
{
    const PointF fontBefore = fromView(viewPos);
    m_pxPerUnit = std::clamp(m_pxPerUnit * factor, MIN_PX_PER_UNIT, MAX_PX_PER_UNIT);
    const QPointF viewAfter = toView(fontBefore);
    m_originPx += viewPos - viewAfter;
    update();
}

QPointF GlyphCanvas::toView(const PointF& fontPos) const
{
    return QPointF(m_originPx.x() + fontPos.x() * m_pxPerUnit,
                   m_originPx.y() - fontPos.y() * m_pxPerUnit);
}

PointF GlyphCanvas::fromView(const QPointF& viewPos) const
{
    return PointF((viewPos.x() - m_originPx.x()) / m_pxPerUnit,
                  (m_originPx.y() - viewPos.y()) / m_pxPerUnit);
}

std::optional<AnchorId> GlyphCanvas::anchorAt(const QPointF& viewPos) const
{
    const GlyphItem* glyph = currentGlyph();
    FontDesignProjectPtr proj = project();
    if (!glyph || !proj) {
        return std::nullopt;
    }

    const double spatium = proj->spatium();
    for (const auto& pair : glyph->anchors) {
        QPointF anchorView = toView(PointF(pair.second.x() * spatium, pair.second.y() * spatium));
        if (QLineF(viewPos, anchorView).length() <= ANCHOR_HIT_RADIUS_PX) {
            return pair.first;
        }
    }
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// 渲染
// ---------------------------------------------------------------------------

void GlyphCanvas::paintGrid(QPainter* painter)
{
    FontDesignProjectPtr proj = project();
    const double sp = proj->spatium() * m_pxPerUnit;

    QColor minorColor = m_gridColor;
    minorColor.setAlphaF(0.35);
    QColor majorColor = m_gridColor;
    majorColor.setAlphaF(0.7);

    auto drawGridLines = [this, painter](double stepPx, const QColor& color) {
        if (stepPx < 6.0) {
            return;
        }
        painter->setPen(QPen(color, 1.0));
        double x = std::fmod(m_originPx.x(), stepPx);
        if (x < 0) {
            x += stepPx;
        }
        for (; x < width(); x += stepPx) {
            painter->drawLine(QPointF(x, 0), QPointF(x, height()));
        }
        double y = std::fmod(m_originPx.y(), stepPx);
        if (y < 0) {
            y += stepPx;
        }
        for (; y < height(); y += stepPx) {
            painter->drawLine(QPointF(0, y), QPointF(width(), y));
        }
    };

    drawGridLines(sp / 4.0, minorColor);
    drawGridLines(sp, majorColor);
}

static double staffBottomLineSp(const GlyphItem* glyph)
{
    if (!glyph) {
        return -2.5;
    }

    auto startsWith = [&glyph](const char* prefix) {
        return glyph->smuflName.rfind(prefix, 0) == 0;
    };
    auto contains = [&glyph](const char* part) {
        return glyph->smuflName.find(part) != std::string::npos;
    };

    if (startsWith("gClef")) {
        return -1.0;
    }
    if (startsWith("fClef")) {
        return -3.0;
    }
    if (startsWith("cClef")) {
        return -2.0;
    }
    if (startsWith("unpitchedPercussionClef") || startsWith("semipitchedPercussionClef")
        || startsWith("6stringTabClef") || startsWith("4stringTabClef")) {
        return -2.0;
    }
    if (startsWith("notehead") || startsWith("rest") || startsWith("stem")
        || startsWith("flag") || startsWith("augmentationDot")
        || contains("Notehead") || startsWith("accidental")) {
        return -2.5;
    }
    return -2.5;
}

void GlyphCanvas::paintStaffLines(QPainter* painter)
{
    FontDesignProjectPtr proj = project();
    const double spatium = proj->spatium();
    const double bottomSp = staffBottomLineSp(currentGlyph());

    QColor staffColor = m_outlineColor;
    staffColor.setAlphaF(0.5);
    painter->setPen(QPen(staffColor, 1.5));

    for (int i = 0; i <= 4; ++i) {
        double y = toView(PointF(0.0, (bottomSp + i) * spatium)).y();
        painter->drawLine(QPointF(0, y), QPointF(width(), y));
    }
}

void GlyphCanvas::paintAxes(QPainter* painter)
{
    QColor axisColor = m_outlineColor;
    axisColor.setAlphaF(0.8);
    painter->setPen(QPen(axisColor, 1.5));
    double x0 = toView(PointF(0.0, 0.0)).x();
    double y0 = toView(PointF(0.0, 0.0)).y();
    painter->drawLine(QPointF(x0, 0), QPointF(x0, height()));
    painter->drawLine(QPointF(0, y0), QPointF(width(), y0));
}

void GlyphCanvas::paintAdvance(QPainter* painter, const GlyphItem& glyph)
{
    if (glyph.advance <= 0) {
        return;
    }

    QColor advanceColor = m_accentColor;
    advanceColor.setAlphaF(0.7);
    QPen pen(advanceColor, 1.5);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    double x = toView(PointF(glyph.advance, 0.0)).x();
    painter->drawLine(QPointF(x, 0), QPointF(x, height()));
}

void GlyphCanvas::paintOutlineFill(QPainter* painter, const GlyphOutline& outline)
{
    if (outline.isEmpty()) {
        return;
    }

    QPainterPath path = outline.toPainterPath().toQPainterPath();
    QTransform transform;
    transform.translate(m_originPx.x(), m_originPx.y());
    transform.scale(m_pxPerUnit, -m_pxPerUnit);
    QPainterPath viewPath = transform.map(path);

    QColor fillColor = m_outlineColor;
    fillColor.setAlphaF(m_tool == Tool::Select ? 0.75 : 0.25);
    painter->fillPath(viewPath, fillColor);
    painter->setPen(QPen(m_outlineColor, 1.0));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(viewPath);
}

void GlyphCanvas::paintNodes(QPainter* painter, const GlyphOutline& outline)
{
    QColor handleColor = m_accentColor;
    handleColor.setAlphaF(0.6);

    for (int ci = 0; ci < static_cast<int>(outline.contours().size()); ++ci) {
        const auto& pts = outline.contours()[ci].points;
        const int n = static_cast<int>(pts.size());

        // 控制柄连线
        painter->setPen(QPen(handleColor, 1.0));
        for (int i = 0; i < n; ++i) {
            if (pts[i].type != PointType::Control) {
                continue;
            }
            int prev = (i - 1 + n) % n;
            int next = (i + 1) % n;
            int anchorIdx = pts[prev].type == PointType::OnCurve ? prev
                            : (pts[next].type == PointType::OnCurve ? next : -1);
            if (anchorIdx >= 0) {
                painter->drawLine(toView(pts[i].pos), toView(pts[anchorIdx].pos));
            }
        }

        // 节点标记
        for (int i = 0; i < n; ++i) {
            QPointF v = toView(pts[i].pos);
            bool sel = isSelected({ ci, i });
            if (pts[i].type == PointType::OnCurve) {
                double r = 3.5;
                painter->setPen(QPen(m_accentColor, sel ? 2.0 : 1.5));
                painter->setBrush(sel ? QBrush(m_accentColor)
                                  : (pts[i].smooth ? QBrush(m_backgroundColor) : QBrush(Qt::NoBrush)));
                QRectF rc(v.x() - r, v.y() - r, 2 * r, 2 * r);
                if (pts[i].smooth) {
                    painter->drawEllipse(rc);
                } else {
                    painter->drawRect(rc);
                }
            } else {
                double r = 2.5;
                painter->setPen(QPen(handleColor, sel ? 2.0 : 1.0));
                painter->setBrush(sel ? QBrush(m_accentColor) : QBrush(Qt::NoBrush));
                painter->drawEllipse(QRectF(v.x() - r, v.y() - r, 2 * r, 2 * r));
            }
        }
    }
}

void GlyphCanvas::paintPenPreview(QPainter* painter)
{
    if (!m_penActive || m_penNodes.empty()) {
        return;
    }

    painter->setPen(QPen(m_accentColor, 1.5));
    painter->setBrush(Qt::NoBrush);

    //! 光标随动：把 hover 位置视作“下一个节点”参与预览（模式按当前钢笔）
    std::vector<PenNode> nodes = m_penNodes;
    const bool hoverExtends = m_hoverView.has_value() && !m_penNodeDrag;
    if (hoverExtends) {
        PenNode hoverNode;
        hoverNode.pos = fromView(m_hoverView.value());
        hoverNode.curveFromPrev = (m_tool == Tool::CurvePen);
        nodes.push_back(hoverNode);
    }

    const int k = static_cast<int>(nodes.size());
    QPainterPath path;
    path.moveTo(toView(nodes.front().pos));

    if (k >= 2) {
        //! 开放式混合预览：逐段直线/Catmull-Rom（端点与直线过渡处用单侧弦向切线）
        auto segCurved = [&](int i) {                   // 段 i：节点 i → i+1
            return nodes[i + 1].curveFromPrev;
        };
        auto smoothAt = [&](int i) {                    // 两侧均曲线才平滑
            if (i <= 0 || i >= k - 1) {
                return false;
            }
            return segCurved(i - 1) && segCurved(i);
        };
        for (int i = 0; i < k - 1; ++i) {
            const PointF p0 = nodes[i].pos;
            const PointF p1 = nodes[i + 1].pos;
            if (!segCurved(i)) {
                path.lineTo(toView(p1));
                continue;
            }
            const PointF chord = p1 - p0;
            const PointF tOut = smoothAt(i) ? scaled(nodes[i + 1].pos - nodes[i - 1].pos, 0.5) : chord;
            const PointF tIn = smoothAt(i + 1) ? scaled(nodes[i + 2].pos - nodes[i].pos, 0.5) : chord;
            path.cubicTo(toView(p0 + scaled(tOut, 1.0 / 3.0)),
                         toView(p1 - scaled(tIn, 1.0 / 3.0)),
                         toView(p1));
        }
    }
    painter->drawPath(path);

    //! 开放路径提示：末端回到起点的“隐式闭合”虚线
    if (k >= 2) {
        QColor closeColor = m_accentColor;
        closeColor.setAlphaF(0.45);
        QPen closePen(closeColor, 1.2);
        closePen.setStyle(Qt::DashLine);
        painter->setPen(closePen);
        painter->drawLine(toView(nodes.back().pos), toView(nodes.front().pos));
    }

    // 已落节点
    painter->setPen(QPen(m_accentColor, 1.5));
    for (const PenNode& node : m_penNodes) {
        QPointF v = toView(node.pos);
        painter->drawRect(QRectF(v.x() - 3, v.y() - 3, 6, 6));
    }

    // 起点高亮（可闭合提示）；光标靠近时放大提示“点击闭合”
    QPointF start = toView(m_penNodes.front().pos);
    bool nearStart = m_penNodes.size() >= 2 && m_hoverView.has_value()
                     && QLineF(m_hoverView.value(), start).length() <= PEN_CLOSE_RADIUS_PX;
    painter->setBrush(QBrush(m_accentColor));
    double sr = nearStart ? 6.0 : 4.0;
    painter->drawEllipse(QRectF(start.x() - sr, start.y() - sr, 2 * sr, 2 * sr));
    if (nearStart) {
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(QRectF(start.x() - sr - 3, start.y() - sr - 3, 2 * (sr + 3), 2 * (sr + 3)));
    }
    painter->setBrush(Qt::NoBrush);
}

void GlyphCanvas::paintShapePreview(QPainter* painter)
{
    if (!m_shapeDragging) {
        return;
    }

    QRectF r(m_shapeStartView, m_shapeCurView);
    r = r.normalized();

    QColor c = m_accentColor;
    QPen pen(c, 1.5);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    QColor fill = c;
    fill.setAlphaF(0.12);
    painter->setBrush(QBrush(fill));

    if (m_tool == Tool::Ellipse) {
        painter->drawEllipse(r);
    } else {
        painter->drawRect(r);
    }
    painter->setBrush(Qt::NoBrush);
}

void GlyphCanvas::paintMarquee(QPainter* painter)
{
    if (!m_marqueeActive) {
        return;
    }

    QRectF r = QRectF(m_marqueeStartView, m_marqueeCurView).normalized();
    QColor c = m_accentColor;
    QPen pen(c, 1.0);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    QColor fill = c;
    fill.setAlphaF(0.08);
    painter->setBrush(QBrush(fill));
    painter->drawRect(r);
    painter->setBrush(Qt::NoBrush);
}

void GlyphCanvas::paintAnchors(QPainter* painter, const GlyphItem& glyph)
{
    if (glyph.anchors.empty()) {
        return;
    }

    FontDesignProjectPtr proj = project();
    const double spatium = proj->spatium();
    const double markerRadius = 5.0;

    QFont labelFont = painter->font();
    labelFont.setPixelSize(10);
    painter->setFont(labelFont);

    for (const auto& pair : glyph.anchors) {
        PointF fontPos(pair.second.x() * spatium, pair.second.y() * spatium);
        QPointF viewPos = toView(fontPos);

        painter->setPen(QPen(m_accentColor, 2.0));
        painter->drawLine(viewPos + QPointF(-markerRadius, -markerRadius), viewPos + QPointF(markerRadius, markerRadius));
        painter->drawLine(viewPos + QPointF(-markerRadius, markerRadius), viewPos + QPointF(markerRadius, -markerRadius));

        painter->setPen(QPen(m_accentColor, 1.0));
        painter->drawText(viewPos + QPointF(markerRadius + 2.0, -2.0),
                          QString::fromStdString(anchorNameById(pair.first)));
    }
}
