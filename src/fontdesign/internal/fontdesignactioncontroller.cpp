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
#include "fontdesignactioncontroller.h"

#include <cmath>

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>

#include "translation.h"

#include "../ifontdesigneditsurface.h"
#include "project/projectcommands.h"

using namespace mu::fontdesign;
using namespace muse;
using namespace muse::actions;

//! 跨实例（多窗口多字体）复制粘贴走系统剪贴板：轮廓序列化为 JSON，
//! 携带源字体 upem，粘贴时按目标字体 upem 缩放。
static const char* OUTLINE_MIME_TYPE = "application/x-musescore-fontdesign-outline";

static QByteArray serializeOutline(const GlyphOutline& outline, double upem)
{
    QJsonArray contoursArr;
    for (const GlyphOutline::Contour& contour : outline.contours()) {
        QJsonArray pointsArr;
        for (const GlyphOutline::Point& p : contour.points) {
            QJsonObject po;
            po["x"] = p.pos.x();
            po["y"] = p.pos.y();
            po["c"] = p.type == GlyphOutline::PointType::Control;
            po["s"] = p.smooth;
            pointsArr.append(po);
        }
        contoursArr.append(pointsArr);
    }

    QJsonObject root;
    root["upem"] = upem;
    root["contours"] = contoursArr;
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

static bool deserializeOutline(const QByteArray& data, GlyphOutline& outline, double& upem)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }
    QJsonObject root = doc.object();
    upem = root.value("upem").toDouble(0.0);

    outline = GlyphOutline();
    for (const QJsonValue& contourVal : root.value("contours").toArray()) {
        GlyphOutline::Contour contour;
        for (const QJsonValue& pointVal : contourVal.toArray()) {
            QJsonObject po = pointVal.toObject();
            GlyphOutline::Point p(muse::PointF(po.value("x").toDouble(), po.value("y").toDouble()),
                                  po.value("c").toBool() ? GlyphOutline::PointType::Control
                                  : GlyphOutline::PointType::OnCurve);
            p.smooth = po.value("s").toBool();
            contour.points.push_back(p);
        }
        if (!contour.points.empty()) {
            outline.contours().push_back(contour);
        }
    }
    return !outline.isEmpty();
}

void FontDesignActionController::init()
{
    dispatcher()->reg(this, "fontdesign-undo", this, &FontDesignActionController::undo);
    dispatcher()->reg(this, "fontdesign-redo", this, &FontDesignActionController::redo);
    dispatcher()->reg(this, "fontdesign-save", this, &FontDesignActionController::save);
    dispatcher()->reg(this, "fontdesign-close", this, &FontDesignActionController::close);
    dispatcher()->reg(this, "fontdesign-copy-glyph", this, &FontDesignActionController::copyGlyph);
    dispatcher()->reg(this, "fontdesign-paste-glyph", this, &FontDesignActionController::pasteGlyph);
    dispatcher()->reg(this, "fontdesign-select-all", this, &FontDesignActionController::selectAll);

    //! enabled 状态依赖是否有项目：项目开/关时刷新，否则快捷键解析
    //! （ShortcutsController::resolveAction 检查 actionState().enabled）会用到过期值
    fontDesignService()->currentProjectChanged().onNotify(this, [this]() {
        notifyActionEnabledChanged();
    });
}

bool FontDesignActionController::canReceiveAction(const ActionCode& code) const
{
    //! paste 不做剪贴板有无的门控：系统剪贴板可能随时被另一窗口写入，
    //! 而 enabled 状态只在上下文变化时重算，会得到过期值；空剪贴板粘贴为无操作。
    return fontDesignService()->hasCurrentProject();
}

muse::async::Channel<ActionCodeList> FontDesignActionController::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

void FontDesignActionController::notifyActionEnabledChanged()
{
    static const ActionCodeList codes {
        "fontdesign-undo",
        "fontdesign-redo",
        "fontdesign-save",
        "fontdesign-close",
        "fontdesign-copy-glyph",
        "fontdesign-paste-glyph",
        "fontdesign-select-all",
    };
    m_actionEnabledChanged.send(codes);
}

FontDesignProjectPtr FontDesignActionController::project() const
{
    return fontDesignService()->currentProject();
}

void FontDesignActionController::undo()
{
    if (FontDesignProjectPtr p = project()) {
        p->undoStack().undo();
    }
}

void FontDesignActionController::redo()
{
    if (FontDesignProjectPtr p = project()) {
        p->undoStack().redo();
    }
}

void FontDesignActionController::save()
{
    if (!fontDesignService()->hasCurrentProject()) {
        return;
    }

    std::vector<std::string> warnings;
    Ret ret = fontDesignService()->saveProject(warnings);
    if (!ret) {
        interactive()->error(trc("fontdesign", "Unable to save font"), ret.text());
        return;
    }

    if (!warnings.empty()) {
        std::string detail;
        for (const std::string& w : warnings) {
            if (!detail.empty()) {
                detail += "\n";
            }
            detail += "• ";
            detail += w;
        }
        interactive()->warning(trc("fontdesign", "Font saved with warnings"), detail);
    }
}

void FontDesignActionController::close()
{
    FontDesignProjectPtr p = project();
    if (!p) {
        return;
    }

    if (p->isDirty()) {
        IInteractive::Result res = interactive()->questionSync(
            trc("fontdesign", "Close font"),
            trc("fontdesign", "Do you want to save changes before closing?"),
            { interactive()->buttonData(IInteractive::Button::Save),
              interactive()->buttonData(IInteractive::Button::DontSave),
              interactive()->buttonData(IInteractive::Button::Cancel) });

        if (res.standardButton() == IInteractive::Button::Cancel) {
            return;
        }

        if (res.standardButton() == IInteractive::Button::Save) {
            std::vector<std::string> warnings;
            Ret ret = fontDesignService()->saveProject(warnings);
            if (!ret) {
                interactive()->error(trc("fontdesign", "Unable to save font"), ret.text());
                return;
            }
        }
    }

    fontDesignService()->closeProject();
    interactive()->open("musescore://home?section=fontdesign");
}

void FontDesignActionController::copyGlyph()
{
    FontDesignProjectPtr p = project();
    if (!p) {
        return;
    }

    IFontDesignEditSurface* surface = fontDesignService()->activeEditSurface();
    if (surface && surface->hasOutlineSelection()) {
        m_glyphClipboard = surface->selectionAsOutline();
    } else {
        const GlyphItem* glyph = p->glyph(p->currentGlyph());
        if (!glyph || glyph->outline.isEmpty()) {
            return;
        }
        m_glyphClipboard = glyph->outline;
    }

    m_hasClipboard = true;

    //! 同步写入系统剪贴板：另一实例（另一字体窗口）可直接粘贴
    QMimeData* mime = new QMimeData();
    mime->setData(OUTLINE_MIME_TYPE, serializeOutline(m_glyphClipboard, p->upem()));
    QGuiApplication::clipboard()->setMimeData(mime);
}

void FontDesignActionController::pasteGlyph()
{
    FontDesignProjectPtr p = project();
    if (!p) {
        return;
    }

    //! 优先取系统剪贴板（跨窗口/跨字体），upem 不同按比例缩放；退回进程内剪贴板
    GlyphOutline outline;
    bool hasOutline = false;

    const QMimeData* mime = QGuiApplication::clipboard()->mimeData();
    if (mime && mime->hasFormat(OUTLINE_MIME_TYPE)) {
        double sourceUpem = 0.0;
        if (deserializeOutline(mime->data(OUTLINE_MIME_TYPE), outline, sourceUpem)) {
            if (sourceUpem > 0 && std::abs(sourceUpem - p->upem()) > 0.5) {
                outline.scale(p->upem() / sourceUpem);
            }
            hasOutline = true;
        }
    }

    if (!hasOutline) {
        if (!m_hasClipboard) {
            return;
        }
        outline = m_glyphClipboard;
    }

    //! 画布在场：追加轮廓并选中粘贴内容（可继续移动）；否则整字形替换
    IFontDesignEditSurface* surface = fontDesignService()->activeEditSurface();
    if (surface) {
        surface->pasteOutline(outline);
        return;
    }

    p->undoStack().push(std::make_unique<ReplaceOutlineCommand>(p.get(), p->currentGlyph(), outline));
}

void FontDesignActionController::selectAll()
{
    IFontDesignEditSurface* surface = fontDesignService()->activeEditSurface();
    if (surface) {
        surface->selectAllPoints();
    }
}
