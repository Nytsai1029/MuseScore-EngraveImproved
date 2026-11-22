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

#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "global/async/notification.h"
#include "global/iapplication.h"
#include "global/types/bytearray.h"
#include "global/types/string.h"
#include "modularity/ioc.h"

#include "../rendering/layoutoptions.h"

//! Layout is computed anew whenever a score is opened, and is meant to come out as it was when the score
//! was saved. To notice when it does not, a file holds a compact description of the layout it was saved
//! with, the layout fingerprint (`layoutfingerprint.json` in the container, see MscSaver), and after
//! loading the layout is compared with it (LayoutFingerprintCheck). Nothing is shown and nothing in the
//! score is changed: the outcome is a report on the master score.

namespace mu::engraving {
class MasterScore;
class Score;

//! The hash of the geometry of a group of items. Geometry is rounded to steps of
//! LayoutFingerprint::CONTENT_STEP before it is hashed, and a value next to the edge of a step lands
//! on the other side of it by the slightest change. So there are two hashes, of steps half a step apart:
//! what is next to an edge of one is in the middle of the other, and two layouts hold the same when
//! either hash is the same, which it is only when every value is less than a step apart.
struct LayoutContentHash
{
    uint64_t a = 0;
    uint64_t b = 0;

    bool matches(const LayoutContentHash& other) const { return a == other.a || b == other.b; }
    bool operator==(const LayoutContentHash& other) const { return a == other.a && b == other.b; }
    bool operator!=(const LayoutContentHash& other) const { return !operator==(other); }
};

//! The layout of one score, the master score or a part, in page view.
//! Lengths are in staff spaces (the spatium of the score's style) and whole numbers of POSITION_STEP.
struct LayoutFingerprint
{
    //! What stored lengths count: a thousandth of a staff space
    static constexpr double POSITION_STEP = 0.001;
    //! What geometry is rounded to before it is hashed, in staff spaces
    static constexpr double CONTENT_STEP = 0.01;
    //! In place of the position of a staff that is not shown in its system
    static constexpr int HIDDEN_STAFF = std::numeric_limits<int>::min();

    struct MeasureData
    {
        int x = 0;      // in its system
        int width = 0;
        //! Of everything in the measure (what Measure::scanElements finds: notes, stems, beams and tuplets
        //! that begin here, lyrics, …), each relative to the measure and to the staff it stands on,
        //! so neither a measure that moved nor a staff that moved changes it
        LayoutContentHash content;
    };

    struct SystemData
    {
        int page = 0;           // index of its page
        int firstMeasure = 0;   // index of its first measure, see below
        int y = 0;              // on its page
        int height = 0;
        std::vector<int> staffY;    // in the system, or HIDDEN_STAFF
        //! Of the items of the system itself (what System::scanElements finds: the segments of slurs,
        //! ties, hairpins, voltas and other lines, brackets, instrument names, dividers), each relative
        //! to the system and to the staff it stands on
        LayoutContentHash content;
        //! Measures and frames, as they are laid out: a multimeasure rest is one. Their place in the row
        //! of all of them is what a measure index means here.
        std::vector<MeasureData> measures;
    };

    LayoutMode layoutMode = LayoutMode::PAGE;
    int spatium = 0;    // of the score's style, in thousandths of the unit lengths are held in (1/360 inch)
    std::vector<int> pageSystems;   // the number of systems on each page
    std::vector<SystemData> systems;

    int measureCount() const;

    //! Of the layout the score has now. One pass over the items of its pages.
    static LayoutFingerprint of(Score* score);

    //! Whether the layout of the score is one to describe: page view, and laid out as the score is now
    //! (a part that is not open is left out of layout, see Score::isLayoutUpToDate)
    static bool isAvailableFor(const Score* score);
};

//! The fingerprints of the scores of a file, as stored in it
struct LayoutFingerprintFile
{
    //! Of the stored form. A file of another number is not compared.
    static constexpr int FORMAT = 1;

    struct Entry
    {
        muse::String part;    // the file name of the part's excerpt in the container; empty: the master score
        LayoutFingerprint fingerprint;
    };

    int format = FORMAT;
    //! The program that laid the scores out, as the score file has them in programVersion and
    //! programRevision
    muse::String programVersion;
    muse::String programRevision;
    std::vector<Entry> scores;

    const LayoutFingerprint* find(const muse::String& part) const;

    //! Compact JSON, see the schema at the definition
    muse::ByteArray toJson() const;

    enum class ReadResult {
        Ok,
        UnknownFormat,  // readable, but of a format number this build does not know
        Malformed
    };
    static ReadResult fromJson(const muse::ByteArray& json, LayoutFingerprintFile& file);
};

//! One place where the layout is not the one the file was saved with
struct LayoutDifference
{
    enum class Kind {
        Spatium,            // the size of a staff space; values in the unit lengths are held in (1/360 inch)
        PageCount,          // values: number of pages
        MeasureCount,       // measures and frames as they are laid out; values: their number
        SystemBreak,        // at `measure`: a system began there in one layout only; values: 1 began, 0 did not
        PageBreak,          // `system` stands on another page; values: page index
        SystemPosition,     // values: y of the system on its page
        SystemHeight,
        StaffPosition,      // of `staff`; values: y in the system, NaN for a staff that is not shown
        MeasurePosition,    // values: x of the measure in its system
        MeasureWidth,
        MeasureContent,     // something in the measure stands or is shaped otherwise; no values
        SystemContent       // the same for the items of the system: slurs, ties, lines, brackets, …; no values
    };

    Kind kind = Kind::PageCount;
    //! The file name of the part's excerpt, as Excerpt::fileName has it after loading; empty: the master score
    muse::String part;

    //! Where, in the layout the score has now, from 0; -1: does not apply
    int page = -1;
    int system = -1;        // counted through the score
    int measure = -1;       // see LayoutFingerprint::SystemData::measures
    int measureNumber = 0;  // of that measure as the user sees it; 0: none (a frame)
    int staff = -1;

    //! Lengths in staff spaces
    double saved = 0.0;
    double current = 0.0;

    static const char* kindName(Kind kind);
};

//! What became of comparing the layout after loading with the fingerprints in the file.
//! Positions are reported where they differ by more than TOLERANCE and only where the difference begins:
//! not for the systems below one that grew, the staves below one that moved, the measures after one that
//! got wider. Measures of a system that holds other measures than it did are not compared.
struct LayoutFingerprintReport
{
    //! For positions and sizes, in staff spaces
    static constexpr double TOLERANCE = 0.02;

    enum class Reason {
        None,                   // compared
        NotChecked,             // nothing was compared yet: the score was not loaded from a file, or not set up
        NoFingerprint,          // the file has none for this score: an older or official file, another view than
                                // page view when it was saved, a part that was not laid out then
        Malformed,              // the file has one that cannot be read
        UnknownFormat,          // the file has one in a format this build does not know
        LayoutModeDiffers,      // the score is not laid out in the view its fingerprint is of
        NotLaidOut,             // not laid out yet, a part that is not open: compared when it first is
        ChangedBeforeLayout     // a part that was first laid out after the score had been changed
    };

    struct ScoreState
    {
        muse::String part;        // see LayoutDifference::part
        muse::String partName;    // of the part as the user named it; empty for the master score
        bool comparable = false;
        Reason reason = Reason::NotChecked;
        size_t differenceCount = 0;
    };

    //! Of the master score
    bool comparable = false;
    Reason reason = Reason::NotChecked;

    //! The program that saved the file is this one, version and revision: then a difference is a defect,
    //! and otherwise to be expected after an update
    bool sameProgram = false;
    muse::String savedProgramVersion;
    muse::String savedProgramRevision;

    //! The master score first, then the parts
    std::vector<ScoreState> scores;
    //! Of all scores that were compared, score by score and in the order of the layout
    std::vector<LayoutDifference> differences;

    bool hasDifferences() const { return !differences.empty(); }

    static const char* reasonName(Reason reason);
};

//! Compares, once after loading, the layout of the scores of a file with the fingerprints in it.
//! Owned by the master score (MasterScore::layoutFingerprintCheck).
class LayoutFingerprintCheck : public muse::Injectable
{
    muse::Inject<muse::IApplication> application = { this };

public:
    LayoutFingerprintCheck(const muse::modularity::ContextPtr& iocCtx);

    //! What the file holds for layoutfingerprint.json; empty when it has none. While loading (MscLoader).
    void setSaved(const muse::ByteArray& json);

    //! When the scores are set up after loading (EngravingProject::setupMasterScore): compares those that
    //! have their layout, and leaves the others for the first complete layout they get. Those are all of
    //! them while updates are locked, as they are when the application loads a project, and later the
    //! parts that are not open.
    void start(MasterScore* masterScore);

    //! From Score::doLayoutRange, for a score left for later: it has a complete layout now
    void scoreLaidOut(Score* score);

    const LayoutFingerprintReport& report() const { return m_report; }
    //! After each score that was compared or given up
    muse::async::Notification reportChanged() const { return m_reportChanged; }

    //! The differences between two layouts of `score`, of which `current` is the one it has now
    static std::vector<LayoutDifference> compare(const LayoutFingerprint& saved, const LayoutFingerprint& current,
                                                 const Score* score, const muse::String& part = muse::String());

private:
    void compareScore(Score* score, size_t stateIndex);

    std::optional<LayoutFingerprintFile> m_saved;
    LayoutFingerprintReport::Reason m_noSavedReason = LayoutFingerprintReport::Reason::NoFingerprint;
    std::vector<std::pair<const Score*, size_t> > m_pending;    // with the index of the score's state in the report

    LayoutFingerprintReport m_report;
    muse::async::Notification m_reportChanged;
};
}
