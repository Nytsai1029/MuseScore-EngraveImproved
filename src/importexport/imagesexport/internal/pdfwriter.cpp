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

#include "pdfwriter.h"

#include <algorithm>
#include <iterator>

#include <QPdfWriter>
#include <QBuffer>

#include "engraving/dom/masterscore.h"

#include "log.h"

using namespace mu::iex::imagesexport;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::io;
using namespace muse::draw;
using namespace mu::engraving;

namespace {
struct CreatorPreset {
    const char* creator = nullptr;
    const char* producer = nullptr;
    bool keepXmp = true;
};

//! NOTE Values copied from real PDFs exported by each program; order must match PdfSettingsPage.qml
static const CreatorPreset CREATOR_PRESETS[] = {
    {},                                                                           // MuseScore Studio
    { "Dorico 6.2.20.6183", "Qt 6.10.1", true },                                  // Dorico writes via Qt 6
    { "Finale 2012", "Mac OS X 10.9.5 Quartz PDFContext", true },                 // Finale prints via Quartz
    { "Sibelius version 22.9.0 on Windows 10 Version 2009", "Qt 5.15.8", false }, // Qt 5 writes no XMP
};

const CreatorPreset* creatorPreset(int index)
{
    if (index <= 0 || index >= static_cast<int>(std::size(CREATOR_PRESETS))) {
        return nullptr;
    }

    return &CREATOR_PRESETS[index];
}

//! NOTE Same encoding as QPdfEnginePrivate::printString: UTF-16BE with BOM, escaping ( ) and backslash bytes
QByteArray pdfTextString(const QString& str)
{
    QByteArray result("(\xfe\xff");
    for (const QChar ch : str) {
        const char16_t code = ch.unicode();
        for (const char part : { static_cast<char>(code >> 8), static_cast<char>(code & 0xff) }) {
            if (part == '(' || part == ')' || part == '\\') {
                result.append('\\');
            }
            result.append(part);
        }
    }
    result.append(')');
    return result;
}

//! Returns the index just past the literal string starting at `start` (which must be '('), or -1
qsizetype pdfLiteralEnd(const QByteArray& pdf, qsizetype start)
{
    if (start < 0 || start >= pdf.size() || pdf.at(start) != '(') {
        return -1;
    }

    int depth = 0;
    for (qsizetype i = start; i < pdf.size(); ++i) {
        const char c = pdf.at(i);
        if (c == '\\') {
            ++i;
        } else if (c == '(') {
            ++depth;
        } else if (c == ')' && --depth == 0) {
            return i + 1;
        }
    }

    return -1;
}

//! Parses the unsigned integer at `pos`; `end` receives the index after its last digit
qsizetype parseNumber(const QByteArray& pdf, qsizetype pos, qsizetype& end)
{
    end = pos;
    while (end < pdf.size() && pdf.at(end) >= '0' && pdf.at(end) <= '9') {
        ++end;
    }

    bool ok = false;
    const qsizetype value = pdf.mid(pos, end - pos).toLongLong(&ok);
    return ok ? value : -1;
}

struct ByteEdit {
    qsizetype pos = 0;
    qsizetype length = 0;
    QByteArray bytes;
};

//! QPdfWriter hardcodes "Qt <version>" as Producer in the Info dictionary and the XMP packet, so rewrite both,
//! then fix the stream length and the classic xref table that Qt writes
bool applyProducer(QByteArray& pdf, const CreatorPreset& preset)
{
    const QByteArray producer(preset.producer);
    std::vector<ByteEdit> edits;

    // Info dictionary
    const QByteArray infoKey("/Producer ");
    const qsizetype infoStart = pdf.indexOf(infoKey);
    const qsizetype infoEnd = pdfLiteralEnd(pdf, infoStart < 0 ? -1 : infoStart + infoKey.size());
    if (infoEnd < 0) {
        return false;
    }
    const qsizetype infoValueStart = infoStart + infoKey.size();
    edits.push_back({ infoValueStart, infoEnd - infoValueStart, pdfTextString(QString::fromLatin1(producer)) });

    // XMP packet and its stream length
    const QByteArray xmpKey("pdf:Producer=\"");
    const qsizetype xmpStart = pdf.indexOf(xmpKey);
    const qsizetype xmpValueStart = xmpStart + xmpKey.size();
    const qsizetype xmpEnd = xmpStart < 0 ? -1 : pdf.indexOf('"', xmpValueStart);
    const qsizetype streamStart = xmpStart < 0 ? -1 : pdf.lastIndexOf("stream\n", xmpStart);
    const qsizetype objStart = streamStart < 0 ? -1 : pdf.lastIndexOf(" 0 obj", streamStart);
    const qsizetype lengthKey = streamStart < 0 ? -1 : pdf.lastIndexOf("/Length ", streamStart);
    if (xmpEnd < 0 || objStart < 0 || lengthKey < objStart) {
        return false;
    }
    qsizetype lengthEnd = 0;
    const qsizetype lengthStart = lengthKey + 8;
    const qsizetype length = parseNumber(pdf, lengthStart, lengthEnd);
    if (length < 0) {
        return false;
    }
    const qsizetype xmpDelta = producer.size() - (xmpEnd - xmpValueStart);
    edits.push_back({ lengthStart, lengthEnd - lengthStart, QByteArray::number(length + xmpDelta) });
    edits.push_back({ xmpValueStart, xmpEnd - xmpValueStart, producer });

    // Detach the XMP packet from the catalog without moving any bytes
    if (!preset.keepXmp) {
        const qsizetype metadataStart = pdf.indexOf("/Metadata ", xmpEnd);
        qsizetype refEnd = 0;
        if (metadataStart < 0 || parseNumber(pdf, metadataStart + 10, refEnd) < 0 || pdf.mid(refEnd, 4) != " 0 R") {
            return false;
        }
        const qsizetype metadataLength = refEnd + 4 - metadataStart;
        edits.push_back({ metadataStart, metadataLength, QByteArray(metadataLength, ' ') });
    }

    auto shifted = [&edits](qsizetype offset) {
        qsizetype result = offset;
        for (const ByteEdit& edit : edits) {
            if (edit.pos < offset) {
                result += edit.bytes.size() - edit.length;
            }
        }
        return result;
    };

    // Xref table: "xref\n0 N\n" followed by fixed-width "%010d 00000 n \n" entries
    const qsizetype startxref = pdf.lastIndexOf("startxref\n");
    qsizetype startxrefEnd = 0;
    const qsizetype xrefPos = startxref < 0 ? -1 : parseNumber(pdf, startxref + 10, startxrefEnd);
    if (xrefPos < 0 || pdf.mid(xrefPos, 7) != "xref\n0 ") {
        return false;
    }
    qsizetype countEnd = 0;
    const qsizetype count = parseNumber(pdf, xrefPos + 7, countEnd);
    const qsizetype entriesStart = countEnd + 1;
    if (count <= 0 || entriesStart + count * 20 > startxref) {
        return false;
    }

    std::vector<ByteEdit> xrefEdits;
    for (qsizetype i = 0; i < count; ++i) {
        const qsizetype entry = entriesStart + i * 20;
        if (pdf.at(entry + 17) != 'n') {
            continue;
        }
        qsizetype end = 0;
        const qsizetype offset = parseNumber(pdf, entry, end);
        if (offset < 0 || end != entry + 10) {
            return false;
        }
        xrefEdits.push_back({ entry, 10, QByteArray::number(shifted(offset)).rightJustified(10, '0') });
    }
    xrefEdits.push_back({ startxref + 10, startxrefEnd - startxref - 10, QByteArray::number(shifted(xrefPos)) });
    edits.insert(edits.end(), xrefEdits.begin(), xrefEdits.end());

    std::sort(edits.begin(), edits.end(), [](const ByteEdit& a, const ByteEdit& b) { return a.pos > b.pos; });
    for (const ByteEdit& edit : edits) {
        pdf.replace(edit.pos, edit.length, edit.bytes);
    }

    return true;
}

void applyCreatorPreset(QByteArray& pdf, int presetIndex)
{
    const CreatorPreset* preset = creatorPreset(presetIndex);
    if (!preset) {
        return;
    }

    QByteArray patched = pdf;
    if (!applyProducer(patched, *preset)) {
        LOGW() << "Unexpected PDF layout, producer metadata left unchanged";
        return;
    }

    pdf = patched;
}
}

std::vector<INotationWriter::UnitType> PdfWriter::supportedUnitTypes() const
{
    return { UnitType::PER_PART, UnitType::MULTI_PART };
}

Ret PdfWriter::write(INotationPtr notation, io::IODevice& destinationDevice, const Options& options)
{
    UnitType unitType = unitTypeFromOptions(options);
    IF_ASSERT_FAILED(unitType == UnitType::PER_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    QPdfWriter pdfWriter(&buf);
    preparePdfWriter(pdfWriter, notation->projectWorkTitleAndPartName(), notation->painting()->pageSizeInch().toQSizeF());

    const Painter::TextDrawingMode textDrawingMode = configuration()->exportPdfWithVectorizedText()
                                                     ? Painter::TextDrawingMode::Paths
                                                     : Painter::TextDrawingMode::Native;
    Painter painter(&pdfWriter, "pdfwriter", textDrawingMode);
    if (!painter.isActive()) {
        return false;
    }

    const bool TRANSPARENT_BACKGROUND = muse::value(options, OptionKey::TRANSPARENT_BACKGROUND,
                                                    Val(configuration()->exportPdfWithTransparentBackground())).toBool();

    INotationPainting::Options opt;
    opt.deviceDpi = pdfWriter.logicalDpiX();
    opt.onNewPage = [&pdfWriter]() { pdfWriter.newPage(); };
    opt.printPageBackground = !TRANSPARENT_BACKGROUND;

    auto pageNumIt = options.find(OptionKey::PAGE_NUMBER);
    if (pageNumIt != options.end()) {
        opt.fromPage = pageNumIt->second.toInt();
        opt.toPage = opt.fromPage;
    }

    notation->painting()->paintPdf(&painter, opt);

    painter.endDraw();

    applyCreatorPreset(qdata, configuration()->exportPdfCreatorPreset());

    ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
    destinationDevice.write(data);

    return true;
}

Ret PdfWriter::writeList(const INotationPtrList& notations, io::IODevice& destinationDevice, const Options& options)
{
    IF_ASSERT_FAILED(!notations.empty()) {
        return make_ret(Ret::Code::UnknownError);
    }

    UnitType unitType = unitTypeFromOptions(options);
    IF_ASSERT_FAILED(unitType == UnitType::MULTI_PART) {
        return Ret(Ret::Code::NotSupported);
    }

    INotationPtr firstNotation = notations.front();
    IF_ASSERT_FAILED(firstNotation) {
        return make_ret(Ret::Code::UnknownError);
    }

    QByteArray qdata;
    QBuffer buf(&qdata);
    buf.open(QIODevice::WriteOnly);

    QPdfWriter pdfWriter(&buf);
    preparePdfWriter(pdfWriter, firstNotation->projectWorkTitle(), firstNotation->painting()->pageSizeInch().toQSizeF());

    const Painter::TextDrawingMode textDrawingMode = configuration()->exportPdfWithVectorizedText()
                                                     ? Painter::TextDrawingMode::Paths
                                                     : Painter::TextDrawingMode::Native;
    Painter painter(&pdfWriter, "pdfwriter", textDrawingMode);
    if (!painter.isActive()) {
        return false;
    }

    const bool TRANSPARENT_BACKGROUND = muse::value(options, OptionKey::TRANSPARENT_BACKGROUND,
                                                    Val(configuration()->exportPdfWithTransparentBackground())).toBool();

    INotationPainting::Options opt;
    opt.deviceDpi = pdfWriter.logicalDpiX();
    opt.onNewPage = [&pdfWriter]() { pdfWriter.newPage(); };
    opt.printPageBackground = !TRANSPARENT_BACKGROUND;

    for (const auto& notation : notations) {
        IF_ASSERT_FAILED(notation) {
            return make_ret(Ret::Code::UnknownError);
        }

        if (notation != firstNotation) {
            QSizeF size = notation->painting()->pageSizeInch().toQSizeF();
            pdfWriter.setPageSize(QPageSize(size, QPageSize::Inch));
            pdfWriter.newPage();
        }

        notation->painting()->paintPdf(&painter, opt);
    }

    painter.endDraw();

    applyCreatorPreset(qdata, configuration()->exportPdfCreatorPreset());

    ByteArray data = ByteArray::fromQByteArrayNoCopy(qdata);
    destinationDevice.write(data);

    return true;
}

void PdfWriter::preparePdfWriter(QPdfWriter& pdfWriter, const QString& title, const QSizeF& size) const
{
    pdfWriter.setResolution(configuration()->exportPdfDpiResolution());
    const CreatorPreset* preset = creatorPreset(configuration()->exportPdfCreatorPreset());
    pdfWriter.setCreator(preset
                         ? QString::fromLatin1(preset->creator)
                         : QString("MuseScore Studio Version: ") + application()->version().toString().toQString());
    pdfWriter.setTitle(title);
    pdfWriter.setPageMargins(QMarginsF());
    pdfWriter.setPageLayout(QPageLayout(QPageSize(size, QPageSize::Inch), QPageLayout::Orientation::Portrait, QMarginsF()));
}
