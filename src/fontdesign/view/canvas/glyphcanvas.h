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
#pragma once

#include <optional>
#include <set>
#include <vector>

#include <QColor>
#include <QImage>

#include "async/asyncable.h"
#include "iinteractive.h"
#include "modularity/ioc.h"
#include "uicomponents/view/quickpaintedview.h"

#include "../../ifontdesignservice.h"
#include "../../ifontdesigneditsurface.h"
#include "../../internal/project/glyphoutline.h"

namespace mu::fontdesign {
//! 字形编辑画布。渲染（sp 网格/五线/坐标轴/advance/轮廓/锚点）+ 缩放平移 +
//! 绘图工具：选择(整体移动)/节点编辑(多选/框选/拖弯曲线)/钢笔/曲线钢笔/矩形/椭圆；
//! 所有轮廓修改经 ReplaceOutlineCommand 可撤销。
//! 键盘：画布 C++ 侧处理工具键/删除/微调等，并通过 ShortcutOverride
//! 阻止全局 QML Shortcut 抢键；Ctrl+C/V/A 级别的语义动作仍走全局 fontdesign-* 动作，
//! 经 IFontDesignEditSurface 桥接回画布选择集。
//! 视图变换：字体单位（y 向上）→ 视图像素（y 向下）。
class GlyphCanvas : public muse::uicomponents::QuickPaintedView, public muse::Injectable, public muse::async::Asyncable,
    public IFontDesignEditSurface
{
    Q_OBJECT

    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor outlineColor READ outlineColor WRITE setOutlineColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor NOTIFY colorsChanged)
    Q_PROPERTY(int tool READ tool WRITE setTool NOTIFY toolChanged)
    Q_PROPERTY(bool penActive READ penActive NOTIFY penActiveChanged)
    Q_PROPERTY(bool snapEnabled READ snapEnabled WRITE setSnapEnabled NOTIFY snapEnabledChanged)
    Q_PROPERTY(bool hasReferenceImage READ hasReferenceImage NOTIFY referenceImageChanged)
    Q_PROPERTY(double referenceImageOpacity READ referenceImageOpacity WRITE setReferenceImageOpacity NOTIFY referenceImageChanged)

    muse::Inject<IFontDesignService> fontDesignService = { this };
    muse::Inject<muse::IInteractive> interactive = { this };

public:
    explicit GlyphCanvas(QQuickItem* parent = nullptr);
    ~GlyphCanvas() override;

    //! 与 QML 的 tool 枚举保持一致
    enum class Tool {
        Select = 0,
        Node,
        Pen,
        Rectangle,
        Ellipse,
        CurvePen        //! 落节点、以自动平滑的三次曲线连接
    };
    Q_ENUM(Tool)

    Q_INVOKABLE void init();
    Q_INVOKABLE void resetView();
    Q_INVOKABLE void deleteSelection();
    Q_INVOKABLE void toggleSmoothSelection();

    //! 布尔运算（Qt WindingFill 路径桥）：无选择时作用于全部轮廓（并集）；
    //! 减/交需要有选择：选中轮廓作为第二操作数，与其余轮廓运算
    Q_INVOKABLE void booleanUnion();
    Q_INVOKABLE void booleanSubtract();
    Q_INVOKABLE void booleanIntersect();

    //! 翻转（选中轮廓；无选择 = 全部），绕包围盒中心；翻转后反转点序保持环绕方向
    Q_INVOKABLE void flipHorizontally();
    Q_INVOKABLE void flipVertically();

    //! 修正路径方向（nonzero winding 镂空规则）：按嵌套深度归一——
    //! 外轮廓逆时针、孔顺时针（PostScript 惯例，y 向上）
    Q_INVOKABLE void correctPathDirections();

    bool snapEnabled() const { return m_snapEnabled; }
    void setSnapEnabled(bool enabled);

    //! 临时参考图（描摹底图）：字体单位空间渲染，随视图缩放平移；
    //! Alt+拖拽移动、Alt+滚轮缩放；不持久化，切换项目时清除
    Q_INVOKABLE void importReferenceImage();
    Q_INVOKABLE void clearReferenceImage();
    Q_INVOKABLE void resetReferenceImagePlacement();

    bool hasReferenceImage() const { return !m_refImage.isNull(); }
    double referenceImageOpacity() const { return m_refOpacity; }
    void setReferenceImageOpacity(double opacity);

    void paint(QPainter* painter) override;

    QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor& color);
    QColor gridColor() const { return m_gridColor; }
    void setGridColor(const QColor& color);
    QColor outlineColor() const { return m_outlineColor; }
    void setOutlineColor(const QColor& color);
    QColor accentColor() const { return m_accentColor; }
    void setAccentColor(const QColor& color);

    int tool() const { return static_cast<int>(m_tool); }
    void setTool(int tool);

    bool penActive() const { return m_penActive; }

    // IFontDesignEditSurface
    bool hasOutlineSelection() const override;
    GlyphOutline selectionAsOutline() const override;
    void pasteOutline(const GlyphOutline& outline) override;
    void selectAllPoints() override;

signals:
    void colorsChanged();
    void toolChanged();
    void penActiveChanged();
    void snapEnabledChanged();
    void referenceImageChanged();

private:
    struct PointRef {
        int contour = -1;
        int index = -1;
        bool valid() const { return contour >= 0 && index >= 0; }
        bool operator==(const PointRef& o) const { return contour == o.contour && index == o.index; }
        bool operator<(const PointRef& o) const { return contour != o.contour ? contour < o.contour : index < o.index; }
    };

    bool event(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void hoverMoveEvent(QHoverEvent* event) override;

    //! 画布自处理的按键（工具切换/删除/微调等）：ShortcutOverride 用同一判定抢回按键
    bool isCanvasHandledKey(const QKeyEvent* event) const;
    bool handleCanvasKey(const QKeyEvent* event);

    FontDesignProjectPtr project() const;
    const GlyphItem* currentGlyph() const;

    void attachToProject();
    void resetInteraction();

    void ensureViewInitialized();
    void zoomAround(double factor, const QPointF& viewPos);
    void applyToolCursor();

    QPointF toView(const muse::PointF& fontPos) const;
    muse::PointF fromView(const QPointF& viewPos) const;
    //! 视图像素长度 → 字体单位长度
    double viewToUnit(double px) const;

    //! 捕捉：整数字体单位 + 谱线/基线/advance 线（开关 m_snapEnabled）
    muse::PointF snapPoint(const muse::PointF& raw) const;

    enum class BooleanOp {
        Union,
        Subtract,
        Intersect
    };
    void applyBoolean(BooleanOp op);
    void applyFlip(bool horizontal);
    //! 新轮廓按嵌套深度定向：落在实心区域内自动反向成孔（作用于 m_workingOutline）
    void orientNewContour(int contourIndex);

    std::optional<AnchorId> anchorAt(const QPointF& viewPos) const;

    //! 当前编辑对象：拖拽/绘制期间用 m_workingOutline，否则用字形轮廓
    const GlyphOutline& activeOutline() const;
    PointRef pointAt(const QPointF& viewPos) const;
    bool segmentAt(const QPointF& viewPos, int& outContour, int& outSegOnCurveIndex, double& outT) const;

    // 选择集
    bool isSelected(const PointRef& ref) const;
    void setSingleSelection(const PointRef& ref);
    void toggleSelected(const PointRef& ref);
    void clearSelection();
    void pruneInvalidSelection();
    //! 刚性移动选择集的实际点集：on-curve 连带其相邻控制柄
    std::set<PointRef> expandSelectionForMove() const;

    // 工具处理
    void beginEdit();
    void commitEdit();       // 推 ReplaceOutlineCommand（无变化时不入栈）
    void cancelEdit();
    //! alignOppositeTangent：强制两侧控制柄共线（Shift 拖拽；smooth 节点默认共线）
    void movePoint(PointRef ref, const muse::PointF& newPos, bool alignOppositeTangent = false);
    void translateSelection(const muse::PointF& delta);
    void nudgeSelection(const muse::PointF& delta);
    void insertPointOnSegment(int contour, int segOnCurveIndex, double t);

    //! 右键（节点工具）：切换命中节点/段的 曲线↔直线 模式
    void toggleCurveModeAt(const QPointF& viewPos);
    void setSegmentCurved(int contour, int segStartOnCurveIndex, bool curved);

    //! 钢笔在建节点：curveFromPrev = 从上一节点到本节点的段是否为曲线
    //! （直线/曲线钢笔可在绘制中途互切，逐段记录模式）
    struct PenNode {
        muse::PointF pos;
        bool curveFromPrev = false;
    };

    GlyphOutline::Contour buildPenContour(const std::vector<PenNode>& nodes, bool closingCurved) const;

    void penPress(const QPointF& viewPos, bool close);
    void penMoveLastNode(const QPointF& viewPos);
    void penFinish();
    void penCancel();
    void setPenActive(bool active);

    void beginSegmentBend(int contour, int segOnCurveIndex, double t, const QPointF& viewPos);
    void updateSegmentBend(const QPointF& viewPos);

    void paintGrid(QPainter* painter);
    void paintStaffLines(QPainter* painter);
    void paintAxes(QPainter* painter);
    void paintReferenceImage(QPainter* painter);
    void paintAdvance(QPainter* painter, const GlyphItem& glyph);
    void paintOutlineFill(QPainter* painter, const GlyphOutline& outline);
    void paintNodes(QPainter* painter, const GlyphOutline& outline);
    void paintPenPreview(QPainter* painter);
    void paintShapePreview(QPainter* painter);
    void paintMarquee(QPainter* painter);
    void paintAnchors(QPainter* painter, const GlyphItem& glyph);

    bool isPenTool() const { return m_tool == Tool::Pen || m_tool == Tool::CurvePen; }

    QColor m_backgroundColor;
    QColor m_gridColor;
    QColor m_outlineColor;
    QColor m_accentColor;

    bool m_viewInitialized = false;
    double m_pxPerUnit = 0.15;
    QPointF m_originPx;

    const FontDesignProject* m_attachedProject = nullptr;

    Tool m_tool = Tool::Node;
    bool m_snapEnabled = true;

    bool m_panning = false;
    QPointF m_lastPanPos;

    std::optional<AnchorId> m_dragAnchor;
    std::optional<muse::PointF> m_dragAnchorOldValue;

    // 轮廓编辑：拖拽/绘制期间的工作副本 + 快照
    bool m_editing = false;
    GlyphOutline m_workingOutline;
    GlyphOutline m_editStartOutline;

    std::vector<PointRef> m_selection;
    bool m_draggingSelection = false;   // 节点工具：拖动选择集
    bool m_draggingWhole = false;       // 选择工具：整体平移
    QPointF m_dragLastView;

    // 框选（节点工具，空白处拖拽）
    bool m_marqueeActive = false;
    bool m_marqueeAdditive = false;
    QPointF m_marqueeStartView;
    QPointF m_marqueeCurView;

    // 曲线段直接拖弯（节点工具，FontLab 式）
    bool m_bendingSegment = false;
    int m_bendContour = -1;
    int m_bendC1 = -1;
    int m_bendC2 = -1;
    double m_bendT = 0.0;
    muse::PointF m_bendStartFont;
    muse::PointF m_bendC1Start;
    muse::PointF m_bendC2Start;

    // 钢笔在建轮廓（开放路径：仅在此阶段存在，闭合后才入模型）
    bool m_penActive = false;
    std::vector<PenNode> m_penNodes;
    bool m_penNodeDrag = false;         // 按住（左/右键）拖动刚落的节点，实时调整曲率
    std::optional<QPointF> m_hoverView; // 钢笔预览随动光标

    // 矩形/椭圆
    bool m_shapeDragging = false;
    QPointF m_shapeStartView;
    QPointF m_shapeCurView;

    // 临时参考图
    QImage m_refImage;
    muse::PointF m_refPosFont;        // 图片左下角（字体单位）
    double m_refScale = 1.0;          // 字体单位 / 图片像素
    double m_refOpacity = 0.5;
    bool m_draggingRefImage = false;
};
}
