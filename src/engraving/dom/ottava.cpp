/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "ottava.h"

#include <algorithm>
#include <cmath>

#include "types/translatablestring.h"

#include "chordrest.h"
#include "editdata.h"
#include "score.h"
#include "staff.h"
#include "system.h"
#include "text.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {

static inline bool isOttavaTurningPointPid(Pid pid)
{
    return pid == Pid::OTTAVA_BREAK_POINT_1_OFFSET
           || pid == Pid::OTTAVA_BREAK_POINT_2_OFFSET
           || pid == Pid::OTTAVA_BREAK_POINT_3_OFFSET
           || pid == Pid::OTTAVA_BREAK_POINT_4_OFFSET;
}

//---------------------------------------------------------
//   ottavaStyle
//---------------------------------------------------------

static const ElementStyle ottavaStyle {
    { Sid::ottavaNumbersOnly,                  Pid::NUMBERS_ONLY },
    { Sid::ottava8VAPlacement,                 Pid::PLACEMENT },
    { Sid::ottava8VAText,                      Pid::BEGIN_TEXT },
    { Sid::ottava8VAContinueText,              Pid::CONTINUE_TEXT },
    { Sid::ottavaHookAbove,                    Pid::END_HOOK_HEIGHT },
    { Sid::ottavaFontFace,                     Pid::BEGIN_FONT_FACE },
    { Sid::ottavaFontFace,                     Pid::CONTINUE_FONT_FACE },
    { Sid::ottavaFontFace,                     Pid::END_FONT_FACE },
    { Sid::ottavaFontSize,                     Pid::BEGIN_FONT_SIZE },
    { Sid::ottavaFontSize,                     Pid::CONTINUE_FONT_SIZE },
    { Sid::ottavaFontSize,                     Pid::END_FONT_SIZE },
    { Sid::ottavaFontStyle,                    Pid::BEGIN_FONT_STYLE },
    { Sid::ottavaFontStyle,                    Pid::CONTINUE_FONT_STYLE },
    { Sid::ottavaFontStyle,                    Pid::END_FONT_STYLE },
    { Sid::ottavaTextAlignAbove,               Pid::BEGIN_TEXT_ALIGN },
    { Sid::ottavaTextAlignAbove,               Pid::CONTINUE_TEXT_ALIGN },
    { Sid::ottavaTextAlignAbove,               Pid::END_TEXT_ALIGN },
    { Sid::ottavaLineWidth,                    Pid::LINE_WIDTH },
    { Sid::ottavaLineStyle,                    Pid::LINE_STYLE },
    { Sid::ottavaDashLineLen,                  Pid::DASH_LINE_LEN },
    { Sid::ottavaDashGapLen,                   Pid::DASH_GAP_LEN },
    { Sid::ottavaPosAbove,                     Pid::OFFSET },
    { Sid::ottavaFontSpatiumDependent,         Pid::TEXT_SIZE_SPATIUM_DEPENDENT },
};

OttavaSegment::OttavaSegment(Ottava* sp, System* parent)
    : TextLineBaseSegment(ElementType::OTTAVA_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    m_text->setTextStyleType(TextStyleType::OTTAVA);
    m_endText->setTextStyleType(TextStyleType::OTTAVA);
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* OttavaSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::OTTAVA_TYPE
        || pid == Pid::NUMBERS_ONLY
        || pid == Pid::OTTAVA_ALLOW_BROKEN_LINE
        || pid == Pid::OTTAVA_BREAK_POINTS_COUNT) {
        return spanner();
    }
    return TextLineBaseSegment::propertyDelegate(pid);
}

PropertyValue OttavaSegment::getProperty(Pid propertyId) const
{
    if (isOttavaTurningPointPid(propertyId)) {
        switch (propertyId) {
        case Pid::OTTAVA_BREAK_POINT_1_OFFSET:
            return turningPointOffset(0);
        case Pid::OTTAVA_BREAK_POINT_2_OFFSET:
            return turningPointOffset(1);
        case Pid::OTTAVA_BREAK_POINT_3_OFFSET:
            return turningPointOffset(2);
        case Pid::OTTAVA_BREAK_POINT_4_OFFSET:
            return turningPointOffset(3);
        default:
            break;
        }
    }

    return TextLineBaseSegment::getProperty(propertyId);
}

bool OttavaSegment::setProperty(Pid propertyId, const PropertyValue& value)
{
    if (isOttavaTurningPointPid(propertyId)) {
        switch (propertyId) {
        case Pid::OTTAVA_BREAK_POINT_1_OFFSET:
            setTurningPointOffset(0, value.value<PointF>());
            break;
        case Pid::OTTAVA_BREAK_POINT_2_OFFSET:
            setTurningPointOffset(1, value.value<PointF>());
            break;
        case Pid::OTTAVA_BREAK_POINT_3_OFFSET:
            setTurningPointOffset(2, value.value<PointF>());
            break;
        case Pid::OTTAVA_BREAK_POINT_4_OFFSET:
            setTurningPointOffset(3, value.value<PointF>());
            break;
        default:
            break;
        }

        triggerLayout();
        return true;
    }

    return TextLineBaseSegment::setProperty(propertyId, value);
}

PropertyValue OttavaSegment::propertyDefault(Pid propertyId) const
{
    if (isOttavaTurningPointPid(propertyId)) {
        return PropertyValue::fromValue(PointF());
    }

    return TextLineBaseSegment::propertyDefault(propertyId);
}

bool OttavaSegment::isUserModified() const
{
    for (const PointF& offset : m_turningPointOffsets) {
        if (!offset.isNull()) {
            return true;
        }
    }

    return TextLineBaseSegment::isUserModified();
}

int OttavaSegment::gripsCount() const
{
    return LineSegment::gripsCount() + turningPointsCount();
}

std::vector<PointF> OttavaSegment::gripsPositions(const EditData& editData) const
{
    std::vector<PointF> grips = LineSegment::gripsPositions(editData);
    grips.resize(gripsCount());

    PointF pagePoint(pagePos());
    for (int pointIndex = 0; pointIndex < turningPointsCount(); ++pointIndex) {
        grips[3 + pointIndex] = turningPointBasePosition(pointIndex) + turningPointOffset(pointIndex) + pagePoint;
    }

    return grips;
}

void OttavaSegment::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    if (!eed) {
        eed = std::make_shared<ElementEditData>();
        eed->e = this;
        ed.addData(eed);
    }

    for (int pointIndex = 0; pointIndex < MAX_TURNING_POINTS; ++pointIndex) {
        eed->pushProperty(turningPointOffsetPid(pointIndex));
        m_turningPointUnsnappedOffsets[size_t(pointIndex)] = turningPointOffset(pointIndex);
    }

    m_shiftSnapAxis = SnapAxis::NONE;
    m_shiftSnapPointIndex = -1;

    LineSegment::startEditDrag(ed);
}

void OttavaSegment::editDrag(EditData& ed)
{
    if (!isTurningPointGrip(ed.curGrip)) {
        m_shiftSnapAxis = SnapAxis::NONE;
        m_shiftSnapPointIndex = -1;
        const bool isStartEndGrip = (ed.curGrip == Grip::START || ed.curGrip == Grip::END);
        const bool isBrokenOttava = ottava()->allowBrokenLine();
        const bool snapEndToPreviousHorizontal = isBrokenOttava
                                                 && ed.curGrip == Grip::END
                                                 && (ed.modifiers & ShiftModifier);
        const bool preserveTurningPoints = isBrokenOttava && isStartEndGrip && turningPointsCount() > 0;
        const int preservedPointsCount = preserveTurningPoints ? turningPointsCount() : 0;

        EditData adjustedEd = ed;
        PointF gripDelta = ed.evtDelta;
        double previousPointY = 0.0;
        PointF oldStartOffset;
        PointF oldEndPos;

        if (preserveTurningPoints) {
            oldStartOffset = offset();
            oldEndPos = pos2();
        }

        // For ottava endpoint drag, Shift aligns endpoint to previous point horizontally.
        if (snapEndToPreviousHorizontal) {
            int startPointIndex = 0;
            int endPointIndex = 0;
            if (mainLineRange(startPointIndex, endPointIndex) && endPointIndex - startPointIndex >= 2) {
                previousPointY = points()[endPointIndex - 2].y();
            } else {
                previousPointY = pos().y();
            }
            gripDelta.setY(0.0);
        }
        adjustedEd.evtDelta = gripDelta;

        const bool shouldAllowDiagonalResize = isBrokenOttava && isStartEndGrip;
        const bool wasDiagonal = line()->diagonal();
        if (shouldAllowDiagonalResize && !wasDiagonal) {
            line()->setDiagonal(true);
        }

        LineSegment::editDrag(adjustedEd);

        if (snapEndToPreviousHorizontal) {
            setUserYoffset2(userOff2().y() + (previousPointY - pos2().y()));
        }

        if (preserveTurningPoints) {
            PointF anchorDelta;
            if (ed.curGrip == Grip::START) {
                anchorDelta = offset() - oldStartOffset;
            } else {
                anchorDelta = pos2() - oldEndPos;
            }

            if (!anchorDelta.isNull()) {
                for (int pointIndex = 0; pointIndex < preservedPointsCount; ++pointIndex) {
                    const double ratio = double(pointIndex + 1) / double(preservedPointsCount + 1);
                    const double influence = (ed.curGrip == Grip::START) ? (1.0 - ratio) : ratio;
                    setTurningPointOffset(pointIndex, turningPointOffset(pointIndex) - anchorDelta * influence);
                }
            }
        }

        if (shouldAllowDiagonalResize && !wasDiagonal) {
            line()->setDiagonal(false);
        }

        if (isBrokenOttava && isStartEndGrip) {
            triggerLayout();
        }
        return;
    }

    int startPointIndex = 0;
    int endPointIndex = 0;
    if (!mainLineRange(startPointIndex, endPointIndex)) {
        return;
    }

    const int pointIndex = int(ed.curGrip) - 3;
    PointF basePoint = turningPointBasePosition(pointIndex);
    PointF freeOffset = m_turningPointUnsnappedOffsets[size_t(pointIndex)] + ed.evtDelta;
    m_turningPointUnsnappedOffsets[size_t(pointIndex)] = freeOffset;
    PointF targetPoint = basePoint + freeOffset;

    if (ed.modifiers & ShiftModifier) {
        PointF previousPoint;
        if (pointIndex == 0) {
            previousPoint = points()[startPointIndex];
        } else {
            previousPoint = turningPointBasePosition(pointIndex - 1) + turningPointOffset(pointIndex - 1);
        }

        if (m_shiftSnapAxis == SnapAxis::NONE || m_shiftSnapPointIndex != pointIndex) {
            PointF segmentVector = targetPoint - previousPoint;
            m_shiftSnapAxis = std::abs(segmentVector.x()) >= std::abs(segmentVector.y()) ? SnapAxis::HORIZONTAL : SnapAxis::VERTICAL;
            m_shiftSnapPointIndex = pointIndex;
        }

        if (m_shiftSnapAxis == SnapAxis::HORIZONTAL) {
            targetPoint.setY(previousPoint.y());
        } else if (m_shiftSnapAxis == SnapAxis::VERTICAL) {
            targetPoint.setX(previousPoint.x());
        }
    } else {
        m_shiftSnapAxis = SnapAxis::NONE;
        m_shiftSnapPointIndex = -1;
    }

    setTurningPointOffset(pointIndex, targetPoint - basePoint);
    triggerLayout();
}

void OttavaSegment::endEditDrag(EditData& ed)
{
    m_shiftSnapAxis = SnapAxis::NONE;
    m_shiftSnapPointIndex = -1;
    m_turningPointUnsnappedOffsets.fill(PointF());
    LineSegment::endEditDrag(ed);
}

int OttavaSegment::turningPointsCount() const
{
    int requestedPoints = ottava()->effectiveBreakPointsCount();

    int startPointIndex = 0;
    int endPointIndex = 0;
    if (!mainLineRange(startPointIndex, endPointIndex)) {
        return requestedPoints;
    }

    const int prefixCount = startPointIndex;
    const int suffixCount = npoints() - endPointIndex;
    const int maxLinePoints = int(std::size(m_points));
    const int maxTurningPoints = std::max(0, maxLinePoints - prefixCount - suffixCount - 2);
    return std::clamp(requestedPoints, 0, maxTurningPoints);
}

PointF OttavaSegment::turningPointOffset(int pointIndex) const
{
    if (pointIndex < 0 || pointIndex >= MAX_TURNING_POINTS) {
        return PointF();
    }

    return m_turningPointOffsets[size_t(pointIndex)];
}

void OttavaSegment::setTurningPointOffset(int pointIndex, const PointF& offset)
{
    if (pointIndex < 0 || pointIndex >= MAX_TURNING_POINTS) {
        return;
    }

    m_turningPointOffsets[size_t(pointIndex)] = offset;
}

Pid OttavaSegment::turningPointOffsetPid(int pointIndex)
{
    switch (pointIndex) {
    case 0:
        return Pid::OTTAVA_BREAK_POINT_1_OFFSET;
    case 1:
        return Pid::OTTAVA_BREAK_POINT_2_OFFSET;
    case 2:
        return Pid::OTTAVA_BREAK_POINT_3_OFFSET;
    case 3:
        return Pid::OTTAVA_BREAK_POINT_4_OFFSET;
    default:
        break;
    }

    return Pid::END;
}

bool OttavaSegment::mainLineRange(int& startPointIndex, int& endPointIndex) const
{
    startPointIndex = 0;
    endPointIndex = npoints();

    if (npoints() < 2) {
        return false;
    }

    const TextLineBase* line = textLineBase();
    const bool isNonSolid = line->lineStyle() != LineType::SOLID;

    if (isSingleBeginType() && line->beginHookType() != HookType::NONE) {
        const bool drawSeparately = isNonSolid || line->beginHookType() == HookType::HOOK_90T;
        if (drawSeparately) {
            startPointIndex += 2;
        } else {
            startPointIndex += 1;
        }
    }

    if (isSingleEndType() && line->endHookType() != HookType::NONE) {
        const bool drawSeparately = isNonSolid || line->endHookType() == HookType::HOOK_90T;
        if (drawSeparately) {
            endPointIndex -= 2;
        } else {
            endPointIndex -= 1;
        }
    }

    return endPointIndex - startPointIndex >= 2;
}

PointF OttavaSegment::turningPointBasePosition(int pointIndex) const
{
    const int pointsCount = turningPointsCount();
    if (pointIndex < 0 || pointIndex >= pointsCount) {
        return PointF();
    }

    int startPointIndex = 0;
    int endPointIndex = 0;
    PointF startPoint;
    PointF endPoint;
    if (mainLineRange(startPointIndex, endPointIndex)) {
        startPoint = points()[startPointIndex];
        endPoint = points()[endPointIndex - 1];
    } else {
        startPoint = PointF();
        endPoint = pos2();
    }

    const double ratio = double(pointIndex + 1) / double(pointsCount + 1);
    return startPoint + (endPoint - startPoint) * ratio;
}

bool OttavaSegment::isTurningPointGrip(Grip grip) const
{
    const int pointIndex = int(grip) - 3;
    return pointIndex >= 0 && pointIndex < turningPointsCount();
}

void OttavaSegment::applyBrokenLineToPoints()
{
    int pointsCount = turningPointsCount();
    if (pointsCount <= 0) {
        return;
    }

    int startPointIndex = 0;
    int endPointIndex = 0;
    if (!mainLineRange(startPointIndex, endPointIndex)) {
        return;
    }

    const int prefixCount = startPointIndex;
    const int suffixCount = npoints() - endPointIndex;

    const PointF mainStart = points()[startPointIndex];
    const PointF mainEnd = points()[endPointIndex - 1];

    std::vector<PointF> newPoints;
    newPoints.reserve(size_t(prefixCount + suffixCount + pointsCount + 2));

    for (int i = 0; i < prefixCount; ++i) {
        newPoints.push_back(points()[i]);
    }

    newPoints.push_back(mainStart);
    for (int pointIndex = 0; pointIndex < pointsCount; ++pointIndex) {
        const double ratio = double(pointIndex + 1) / double(pointsCount + 1);
        PointF basePoint = mainStart + (mainEnd - mainStart) * ratio;
        newPoints.push_back(basePoint + turningPointOffset(pointIndex));
    }
    newPoints.push_back(mainEnd);

    for (int i = endPointIndex; i < npoints(); ++i) {
        newPoints.push_back(points()[i]);
    }

    npointsRef() = int(newPoints.size());
    for (int i = 0; i < npoints(); ++i) {
        pointsRef()[i] = newPoints[size_t(i)];
    }

    const int mainLineStart = prefixCount;
    const int mainLineEnd = mainLineStart + pointsCount + 2;
    double newLineLength = 0.0;
    for (int i = mainLineStart; i < mainLineEnd - 1; ++i) {
        const PointF segmentVector = points()[i + 1] - points()[i];
        newLineLength += std::hypot(segmentVector.x(), segmentVector.y());
    }
    setLineLength(newLineLength);

    RectF bbox = ldata()->bbox();
    const double lineWidth = 0.5 * absoluteFromSpatium(textLineBase()->lineWidth());
    const bool isDottedLine = textLineBase()->lineStyle() == LineType::DOTTED;
    for (int i = 0; i < npoints() - 1; ++i) {
        bbox = bbox.united(boundingBoxOfLine(points()[i], points()[i + 1], lineWidth, isDottedLine));
    }
    mutldata()->setBbox(bbox);
}

//---------------------------------------------------------
//   setOttavaType
//---------------------------------------------------------

void Ottava::setOttavaType(OttavaType val)
{
    if (m_ottavaType == val) {
        return;
    }
    m_ottavaType = val;
    styleChanged();
}

//---------------------------------------------------------
//   setNumbersOnly
//---------------------------------------------------------

void Ottava::setNumbersOnly(bool val)
{
    m_numbersOnly = val;
}

void Ottava::setAllowBrokenLine(bool val)
{
    m_allowBrokenLine = val;
}

void Ottava::setBreakPointsCount(int val)
{
    m_breakPointsCount = std::clamp(val, 0, OttavaSegment::MAX_TURNING_POINTS);
}

int Ottava::effectiveBreakPointsCount() const
{
    if (!m_allowBrokenLine) {
        return 0;
    }

    return std::clamp(m_breakPointsCount, 0, OttavaSegment::MAX_TURNING_POINTS);
}

//---------------------------------------------------------
//   setPlacement
//---------------------------------------------------------

void Ottava::setPlacement(PlacementV p)
{
    TextLineBase::setPlacement(p);
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void OttavaSegment::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::OTTAVA_TYPE
        || id == Pid::NUMBERS_ONLY
        || id == Pid::OTTAVA_ALLOW_BROKEN_LINE
        || id == Pid::OTTAVA_BREAK_POINTS_COUNT) {
        EngravingObject::undoChangeProperty(id, v, ps);
    } else {
        EngravingObject::undoChangeProperty(id, v, ps);
    }
}

void Ottava::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::OTTAVA_TYPE || id == Pid::NUMBERS_ONLY) {
        TextLineBase::undoChangeProperty(id, v, ps);
        styleChanged();       // these properties may change style settings
    } else {
        TextLineBase::undoChangeProperty(id, v, ps);
    }
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid OttavaSegment::getPropertyStyle(Pid pid) const
{
    switch (pid) {
    case Pid::OFFSET:
        return spanner()->placeAbove() ? Sid::ottavaPosAbove : Sid::ottavaPosBelow;
    default:
        return TextLineBaseSegment::getPropertyStyle(pid);
    }
}

Sid Ottava::getPropertyStyle(Pid pid) const
{
    static_assert(int(OttavaType::OTTAVA_22MB) - int(OttavaType::OTTAVA_8VA) == 5);

    static const std::vector<Sid> ss = {
        Sid::ottava8VAPlacement,
        Sid::ottava8VAnoText,
        Sid::ottava8VAnoContinueText,
        Sid::ottava8VBPlacement,
        Sid::ottava8VBnoText,
        Sid::ottava8VBnoContinueText,
        Sid::ottava15MAPlacement,
        Sid::ottava15MAnoText,
        Sid::ottava15MAnoContinueText,
        Sid::ottava15MBPlacement,
        Sid::ottava15MBnoText,
        Sid::ottava15MBnoContinueText,
        Sid::ottava22MAPlacement,
        Sid::ottava22MAnoText,
        Sid::ottava22MAnoContinueText,
        Sid::ottava22MBPlacement,
        Sid::ottava22MBnoText,
        Sid::ottava22MBnoContinueText,

        Sid::ottava8VAPlacement,
        Sid::ottava8VAText,
        Sid::ottava8VAContinueText,
        Sid::ottava8VBPlacement,
        Sid::ottava8VBText,
        Sid::ottava8VBContinueText,
        Sid::ottava15MAPlacement,
        Sid::ottava15MAText,
        Sid::ottava15MAContinueText,
        Sid::ottava15MBPlacement,
        Sid::ottava15MBText,
        Sid::ottava15MBContinueText,
        Sid::ottava22MAPlacement,
        Sid::ottava22MAText,
        Sid::ottava22MAContinueText,
        Sid::ottava22MBPlacement,
        Sid::ottava22MBText,
        Sid::ottava22MBContinueText,
    };

    size_t idx = size_t(m_ottavaType) * 3 + (m_numbersOnly ? 0 : ss.size() / 2);
    switch (pid) {
    case Pid::OFFSET:
        return placeAbove() ? Sid::ottavaPosAbove : Sid::ottavaPosBelow;
    case Pid::PLACEMENT:
        return ss[idx];
    case Pid::BEGIN_TEXT:
        return ss[idx + 1];               // BEGIN_TEXT
    case Pid::CONTINUE_TEXT:
        return ss[idx + 2];               // CONTINUE_TEXT
    case Pid::END_HOOK_HEIGHT:
        if (isStyled(Pid::PLACEMENT)) {
            return style().styleI(ss[idx]) == int(PlacementV::ABOVE) ? Sid::ottavaHookAbove : Sid::ottavaHookBelow;
        } else {
            return placeAbove() ? Sid::ottavaHookAbove : Sid::ottavaHookBelow;
        }
    case Pid::BEGIN_TEXT_ALIGN:
    case Pid::CONTINUE_TEXT_ALIGN:
    case Pid::END_TEXT_ALIGN:
        return placeAbove() ? Sid::ottavaTextAlignAbove : Sid::ottavaTextAlignBelow;
    default:
        return TextLineBase::getPropertyStyle(pid);
    }
}

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(EngravingItem* parent)
    : TextLineBase(ElementType::OTTAVA, parent, ElementFlag::ON_STAFF | ElementFlag::MOVABLE)
{
    m_ottavaType  = OttavaType::OTTAVA_8VA;
    m_numbersOnly = false;
    setBeginTextPlace(TextPlace::LEFT);
    setContinueTextPlace(TextPlace::LEFT);
    setEndHookType(HookType::HOOK_90);
    setLineVisible(true);
    setBeginHookHeight(Spatium(.0));
    setEndText(u"");

    initElementStyle(&ottavaStyle);
}

Ottava::Ottava(const Ottava& o)
    : TextLineBase(o)
{
    setOttavaType(o.m_ottavaType);
    m_numbersOnly = o.m_numbersOnly;
    m_allowBrokenLine = o.m_allowBrokenLine;
    m_breakPointsCount = o.m_breakPointsCount;
}

//---------------------------------------------------------
//   pitchShift
//---------------------------------------------------------

int Ottava::pitchShift() const
{
    return ottavaDefault[int(m_ottavaType)].shift;
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

static const ElementStyle ottavaSegmentStyle {
    { Sid::ottavaPosAbove, Pid::OFFSET },
    { Sid::ottavaMinDistance, Pid::MIN_DISTANCE },
};

LineSegment* Ottava::createLineSegment(System* parent)
{
    OttavaSegment* os = new OttavaSegment(this, parent);
    os->setTrack(track());
    os->initElementStyle(&ottavaSegmentStyle);
    return os;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Ottava::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::OTTAVA_TYPE:
        return int(ottavaType());

    case Pid::NUMBERS_ONLY:
        return m_numbersOnly;

    case Pid::OTTAVA_ALLOW_BROKEN_LINE:
        return m_allowBrokenLine;

    case Pid::OTTAVA_BREAK_POINTS_COUNT:
        return m_breakPointsCount;

    case Pid::END_TEXT_PLACE:                         // HACK
        return TextPlace::LEFT;

    default:
        break;
    }
    return TextLineBase::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ottava::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::PLAY:
        setPlaySpanner(val.toBool());
        staff()->updateOttava();
        break;

    case Pid::OTTAVA_TYPE:
        setOttavaType(OttavaType(val.toInt()));
        break;

    case Pid::NUMBERS_ONLY:
        m_numbersOnly = val.toBool();
        break;

    case Pid::OTTAVA_ALLOW_BROKEN_LINE:
        setAllowBrokenLine(val.toBool());
        break;

    case Pid::OTTAVA_BREAK_POINTS_COUNT:
        setBreakPointsCount(val.toInt());
        break;

    case Pid::SPANNER_TICKS:
        setTicks(val.value<Fraction>());
        staff()->updateOttava();
        break;

    case Pid::SPANNER_TICK:
        setTick(val.value<Fraction>());
        staff()->updateOttava();
        break;

    default:
        if (!TextLineBase::setProperty(propertyId, val)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Ottava::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::OTTAVA_TYPE:
        return PropertyValue();
    case Pid::OTTAVA_ALLOW_BROKEN_LINE:
        return false;
    case Pid::OTTAVA_BREAK_POINTS_COUNT:
        return 1;
    case Pid::END_HOOK_TYPE:
        return HookType::HOOK_90;
    case Pid::LINE_VISIBLE:
        return true;
    case Pid::BEGIN_TEXT_OFFSET:
    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF());
    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::LEFT;
    case Pid::BEGIN_HOOK_TYPE:
        return HookType::NONE;
    case Pid::BEGIN_HOOK_HEIGHT:
        return Spatium(.0);
    case Pid::END_TEXT:
        return String();
    case Pid::PLACEMENT:
        return styleValue(Pid::PLACEMENT, getPropertyStyle(Pid::PLACEMENT));

    default:
        return TextLineBase::propertyDefault(pid);
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Ottava::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), String::fromUtf8(ottavaDefault[static_cast<int>(ottavaType())].name));
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Ottava::subtypeUserName() const
{
    return ottavaDefault[int(ottavaType())].userName;
}

void OttavaSegment::rebaseOffsetsOnAnchorChanged(Grip grip, const PointF& oldPos, System* sys)
{
    if (grip == Grip::MIDDLE || grip == Grip::END) {
        ottava()->computeEndElement();
    }
    LineSegment::rebaseOffsetsOnAnchorChanged(grip, oldPos, sys);
}

//---------------------------------------------------------
//   ottavaTypeName
//---------------------------------------------------------

const char* Ottava::ottavaTypeName(OttavaType type)
{
    return ottavaDefault[int(type)].name;
}

PointF Ottava::linePos(Grip grip, System** system) const
{
    if (grip == Grip::START) {
        return TextLineBase::linePos(grip, system);
    }

    bool extendToEndOfDuration = false; // TODO: style
    if (extendToEndOfDuration) {
        return SLine::linePos(grip, system);
    }

    ChordRest* endCr = endElement() && endElement()->isChordRest() ? toChordRest(endElement()) : nullptr;
    if (!endCr) {
        return PointF();
    }

    Segment* seg = endCr->segment();

    *system = seg->measure()->system();

    // End 1sp after the right edge of the end chord, but don't overlap followig segments
    Shape staffShape = seg->staffShape(endCr->vStaffIdx());
    staffShape.remove_if([](ShapeElement& el) { return el.height() == 0; });
    double x = staffShape.right() + seg->x() + seg->measure()->x() + spatium();
    Segment* followingCRseg = score()->tick2segment(endCr->tick() + endCr->actualTicks(), true, SegmentType::ChordRest);
    if (followingCRseg && followingCRseg->system() == seg->system()) {
        x = std::min(x, followingCRseg->x() + followingCRseg->measure()->x());
    }

    x -= 0.5 * absoluteFromSpatium(lineWidth());

    return PointF(x, 0.0);
}

void Ottava::doComputeEndElement()
{
    setEndElement(score()->findChordRestEndingBeforeTickInStaff(tick2(), track2staff(track())));
}
}
