<p align="center">
  <img src="share/icons/musescore_logo_full.png" alt="MuseScore Engrave Improved logo" width="220">
</p>

<h1 align="center">MuseScore Engrave Improved</h1>

<p align="center">
  A MuseScore Studio fork for finer engraving control, cleaner score spacing, and more predictable PDF output.
</p>

<p align="center">
  <a href="README.zh-CN.md">简体中文</a>
  ·
  <a href="docs/releases/beta.md">Beta announcement</a>
  ·
  <a href="LICENSE.txt">License</a>
</p>

<p align="center">
  <img alt="C++" src="https://img.shields.io/badge/C%2B%2B-17-00599c">
  <img alt="Qt" src="https://img.shields.io/badge/Qt-6-41cd52">
  <img alt="Platforms" src="https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux-blue">
  <img alt="Status" src="https://img.shields.io/badge/status-beta-f59e0b">
  <img alt="License" src="https://img.shields.io/badge/license-GPL--3.0--only-16a34a">
</p>

## Why This Fork

MuseScore Engrave Improved exists for scores that need more hands-on engraving control than the default editing
surface exposes. It keeps the MuseScore Studio foundation, then adds practical tools for spacing, lines, note
geometry, text styling, and export consistency.

The goal is not to turn MuseScore into a different notation program. It is a focused fork for arrangers,
copyists, and engravers who want to keep working in a familiar MuseScore environment while gaining more control
over the final page.

## Highlights

- Manual-friendly note positioning: fewer restrictions when automatic placement is not the right choice.
- Ledger line control: adjust ledger line behavior for cleaner dense passages.
- Better horizontal spacing: spacing calculations account for more engraving context.
- Beam shaping controls: customize beam and beam-slant behavior for complex rhythmic notation.
- Key signature spacing: tune accidental spacing around key signatures.
- Text letter spacing: adjust score text tracking directly from style and text settings.
- Laissez vibrer editing: drag and extend laissez vibrer lines more freely.
- Broken and shaped line workflows: more flexible ottava and slur-style presentation.
- Ottava broken-line editing: Shift-drag endpoints align to the visible line anchor without small vertical drift.
- Editable hand symbols: adjust hand-symbol appearance where the score requires it.

## How It Compares

| Need | Standard MuseScore Studio | MuseScore Engrave Improved |
| --- | --- | --- |
| General notation editing | Full-featured and stable | Keeps the same foundation |
| Fine manual engraving | Available, but some controls are limited | Adds more direct layout controls |
| Custom spacing decisions | Mostly automatic with style-level controls | Adds targeted spacing and geometry overrides |
| Export matching editor view | Good for common cases | Improves edge cases such as fallback italic text in PDF |
| File separation | Uses standard MuseScore formats | Uses `.msdz` to keep fork-edited scores distinct |

## File Format

Scores edited in this fork use the `.msdz` suffix. This keeps fork-specific engraving data separate from regular
MuseScore Studio projects and makes it easier to identify which scores were prepared with the improved engraving
toolchain.

Keep backups of production scores before moving them into a beta workflow.

## Requirements

- CMake and Ninja
- Qt 6 development environment
- A platform toolchain supported by MuseScore Studio
- macOS, Windows, or Linux

For platform-specific dependency setup, use the scripts under `buildscripts/ci/` as the source of truth.

## Build

Configure and build a local debug tree:

```bash
./ninja_build.sh -t debug
```

Install the debug app bundle:

```bash
ninja -C build.debug install
```

Run the installed macOS debug app:

```bash
build.install/mscore.app/Contents/MacOS/mscore
```

## Export Check

Use the installed app bundle for headless export checks:

```bash
env HOME=/private/tmp/musescore-home QT_QPA_PLATFORM=offscreen \
  build.install/mscore.app/Contents/MacOS/mscore \
  -F -f -o /tmp/out.pdf input.msdz
```

## Local Development

Common edit-build-test loop:

```bash
ninja -C build.debug mscore
ninja -C build.debug install
```

When unit tests are enabled in the build tree, run the relevant Ninja target or use `ctest` from the build
directory.

## Project Structure

```text
src/engraving/      Engraving model, layout, rendering, read/write logic
src/notation/       Notation UI integration and editing workflows
src/importexport/   PDF, image, MusicXML, and other import/export paths
src/project/        Project open/save/export behavior
src/framework/      Shared drawing, UI, audio, diagnostics, and app framework
share/              Application resources
fonts/              Bundled music and text fonts
buildscripts/       CI and platform build scripts
docs/               Documentation and release notes
```

## Status

This is beta software. The engraving improvements are useful in real scores, but every exported score should
still be reviewed before publication. The most important validation path is simple: open the score, inspect the
layout, export PDF, and compare the PDF against the editor view.

## License

MuseScore Engrave Improved follows MuseScore Studio's GPL-3.0-only licensing model. See `LICENSE.txt`.
