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
#include "officialmscznotationwriter.h"

#include "io/buffer.h"

#include "engraving/dom/masterscore.h"
#include "engraving/engravingproject.h"
#include "engraving/infrastructure/mscreader.h"
#include "engraving/infrastructure/mscwriter.h"
#include "engraving/officialexport/officiallayoutbaker.h"
#include "engraving/rw/mscloader.h"

#include "log.h"

using namespace muse;
using namespace muse::io;
using namespace mu::engraving;
using namespace mu::notation;
using namespace mu::project;

std::vector<INotationWriter::UnitType> OfficialMsczNotationWriter::supportedUnitTypes() const
{
    return { UnitType::MULTI_PART };
}

bool OfficialMsczNotationWriter::supportsUnitType(UnitType unitType) const
{
    return unitType == UnitType::MULTI_PART;
}

//! A second, independent project with the same content: what is baked must not be the open score
static EngravingProjectPtr makeWorkingCopy(MasterScore* masterScore)
{
    EngravingProjectPtr source = masterScore->project().lock();
    IF_ASSERT_FAILED(source) {
        return nullptr;
    }

    ByteArray data;
    {
        Buffer buffer(&data);
        buffer.open(IODevice::WriteOnly);

        MscWriter::Params params;
        params.device = &buffer;
        params.mode = MscIoMode::Zip;

        MscWriter writer(params);
        if (!writer.open()) {
            return nullptr;
        }
        source->writeMscz(writer, false);
        writer.close();
        if (writer.hasError()) {
            return nullptr;
        }
    }

    Buffer buffer(&data);
    buffer.open(IODevice::ReadOnly);

    MscReader::Params params;
    params.device = &buffer;
    params.mode = MscIoMode::Zip;

    MscReader reader(params);
    if (!reader.open()) {
        return nullptr;
    }

    EngravingProjectPtr copy = EngravingProject::create(masterScore->iocContext());
    copy->setFileInfoProvider(masterScore->fileInfo());

    SettingsCompat settingsCompat;
    if (!copy->loadMscz(reader, settingsCompat, true) || !copy->setupMasterScore(true)) {
        return nullptr;
    }

    for (Score* score : copy->masterScore()->scoreList()) {
        score->setLayoutAll();
        score->doLayout();
    }

    return copy;
}

Ret OfficialMsczNotationWriter::write(INotationPtr notation, io::IODevice& device, const Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    Score* score = notation->elements()->msScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    EngravingProjectPtr copy = makeWorkingCopy(score->masterScore());
    if (!copy) {
        LOGE() << "could not copy the score for export";
        return make_ret(Ret::Code::UnknownError);
    }

    const OfficialExportReport report = OfficialLayoutBaker::bake(copy->masterScore());

    MscWriter::Params params;
    params.device = &device;
    params.filePath = device.meta("file_path");
    params.mode = MscIoMode::Zip;

    MscWriter writer(params);
    if (!writer.open()) {
        LOGE() << "MscWriter is not opened";
        return make_ret(Ret::Code::UnknownError);
    }

    copy->writeMscz(writer, true);
    writer.close();

    if (writer.hasError()) {
        LOGE() << "MscWriter has error";
        return make_ret(Ret::Code::UnknownError);
    }

    for (const auto& [feature, count] : report.dropped) {
        LOGW() << "not in the official format, left out: " << droppedFeatureTitle(feature).str << " (" << count << ")";
    }

    Ret ret = make_ok();
    ret.setData(REPORT_KEY, report);
    return ret;
}

Ret OfficialMsczNotationWriter::writeList(const INotationPtrList& notations, io::IODevice& device, const Options& options)
{
    IF_ASSERT_FAILED(!notations.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    // One file holds the full score and all its parts, whichever of them were ticked
    return write(notations.front(), device, options);
}
