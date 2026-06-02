# MuseScore Engrave Improved

MuseScore Engrave Improved is a MuseScore Studio fork focused on practical engraving control: spacing,
notation geometry, line behavior, and score presentation details that matter when preparing polished sheet
music.

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.en.html)

## Project Status

This project is in beta. It is suitable for engraving experiments, score preparation, and export testing, but
keep backups of important scores and verify exported files before publication.

MuseScore Engrave Improved uses its own score suffix, `.msdz`, so edited files can be kept separate from
regular MuseScore Studio projects.

## Highlights

- More flexible note positioning when automatic placement is not required.
- Editable ledger line behavior and per-note visual adjustments.
- Improved horizontal spacing rules that better account for engraving context.
- Custom beam and beam-slant handling for finer rhythmic grouping control.
- Adjustable key signature spacing.
- Editable text letter spacing for score text styles.
- Longer and draggable laissez vibrer lines.
- Broken and shaped ottava/slur-style line workflows.
- Editable hand-symbol appearance.
- PDF export keeps italic and bold-italic text styling when fallback fonts are used.

## Supported Platforms

The project follows MuseScore Studio's desktop platform support model and currently carries CI definitions for:

- Linux x64
- macOS universal builds
- Windows x64

## Build From Source

For a local debug build:

```bash
./ninja_build.sh -t debug
```

Install the debug app bundle after building:

```bash
ninja -C build.debug install
```

Run the installed macOS debug app:

```bash
build.install/mscore.app/Contents/MacOS/mscore
```

## Export Smoke Test

Use the installed app bundle for command-line import/export checks:

```bash
env HOME=/private/tmp/musescore-home QT_QPA_PLATFORM=offscreen \
  build.install/mscore.app/Contents/MacOS/mscore \
  -F -f -o /tmp/out.pdf input.msdz
```

## Release Notes

The current beta announcement is available in [docs/releases/beta.md](docs/releases/beta.md).

## Contributing

Keep changes focused on engraving behavior, score compatibility, or export correctness. When possible, include
a small score, regression test, or headless export command that demonstrates the change.

## License

This project is licensed under GPL-3.0-only, following MuseScore Studio's licensing model.
