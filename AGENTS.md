# AGENTS.md

## Repository purpose

This repository is a fork of MuseScore Studio (desktop music notation and composition software).  
It contains:

- A modular C++/Qt application runtime.
- Notation engraving core logic.
- Import/export pipelines for multiple music formats.
- Playback/audio infrastructure.
- GUI modules (Qt Widgets + QML).
- CI, packaging, translation, visual-regression, and unit-test tooling.

The top-level executable target is `mscore` (or platform-specific variants).

---

## High-level runtime architecture

### Boot flow

1. `src/app/main.cpp` initializes Qt environment variables, parses command-line options, and creates either GUI or console mode.
2. `src/app/appfactory.cpp` constructs an app instance and registers modules in a dependency-aware order.
3. Modules are enabled/disabled by build configuration flags (`app`, `app-web`, `utest`, `vtest`) and module options from CMake.
4. Most modules expose interfaces and implementations; when a module is disabled, a stub module is often linked instead.

### Core layers

- **Framework layer (`src/framework`)**: global infra, UI infra, audio, networking, actions, diagnostics, etc.
- **Domain modules (`src/*`)**: engraving, notation, project handling, inspector, palette, playback, import/export, etc.
- **Resources layer (`share`, `fonts`)**: templates, icons, locale files, sounds, styles, instruments, fonts.

---

## Important source directories and what they do

### Root-level

- `CMakeLists.txt`: global build entry, options, configuration modes, module toggles.
- `SetupConfigure.cmake`: build-mode behavior (`app`, `app-web`, `utest`, `vtest`) and module enable/disable policy.
- `build.cmake`: portable script wrapper for configure/build/install/run.
- `ninja_build.sh`: Linux/macOS-oriented fast build script used heavily in CI.
- `buildscripts/`: CI, packaging, release, and helper scripts.
- `test/`: legacy/manual test assets and scripts.
- `vtest/`: visual/draw-data regression assets and scripts.

### Main product modules (`src/`)

- `app`: executable target and startup wiring.
- `appshell`: shell-level app actions, startup scenarios, splash/loading UI.
- `context`: global/UI/playback context state for module coordination.
- `engraving`: core notation engine (DOM, layout, editing, rendering, RW for multiple format versions).
- `notation`: higher-level notation behaviors over engraving (selection, input, automation, interaction).
- `notationscene`: notation scene actions, MIDI I/O control, notation UI action wiring.
- `project`: open/save/migrate/autosave/export/project metadata and templates.
- `importexport`: format-specific modules (MIDI, MusicXML, GuitarPro, MEI, MNX, etc.).
- `playback`: playback control, sound profile handling, online sounds hooks.
- `palette`: palette models/widgets/actions and symbol/time/key dialogs.
- `inspector`: score element inspection/repository services and inspector UI module.
- `instrumentsscene`: instrument-selection scenario and instrument UI actions.
- `musesounds`: MuseSounds repositories/configuration/update checks.
- `preferences`: preferences module and QML page integration.
- `print`: print provider logic.
- `braille`: braille conversion/input/writer module (liblouis-backed).
- `stubs`: fallback modules when feature modules are disabled.
- `web`: app-web variants (`web/appshell`, `web/appjs`) for web configuration.

### Framework modules (`src/framework`)

Core framework modules (each has its own subdirectory) include:

- `global`: DI, async communication, filesystem/process abstractions, base data types.
- `ui` and `uicomponents`: navigation/dialog infrastructure and reusable UI controls.
- `actions`: action dispatch/subscription infrastructure.
- `audio` and `audioplugins`: audio engine and plugin scan/metadata infra.
- `draw`: drawing APIs, font metrics, low-level drawing support.
- `midi`, `mpe`, `musesampler`, `vst`: music playback/control integrations.
- `network`, `cloud`: networking and cloud service integration.
- `diagnostics`: crash dump and diagnostics support.
- `languages`, `update`, `workspace`, `extensions`, `tours`, `learn`, `shortcuts`, `multiwindows`, `interactive`, `accessibility`, `dockwindow`, `autobot`.

### Assets and data

- `share/`: runtime resources (icons, templates, locale, styles, instruments, plugins, workspaces, manuals).
- `fonts/`: notation/UI fonts and sources.
- `demos/`: sample scores.
- `thirdparty/`: embedded external libraries used by some modules.

---

## Build configurations that matter

Build configuration is controlled by `MUSESCORE_BUILD_CONFIGURATION`:

- `app`: normal desktop app build.
- `app-portable`: portable app packaging variant.
- `app-web`: web build variant with many modules disabled.
- `utest`: unit-test-focused build (enables test infra and disables nonessential modules).
- `vtest`: visual test build profile for draw-data/visual regression workflows.

Common mode for release channel behavior (`MUSE_APP_BUILD_MODE`):

- `dev`, `testing`, `release`.

---

## Testing guide

### 1) Prerequisites

- CMake (project requires CMake >= 3.22; Apple builds use >= 3.26).
- Qt 6.10.x toolchain (CI uses Qt `6.10.2`).
- Ninja (if using `ninja_build.sh` flow).
- A C++17 compiler.

If using presets on Linux, `CMakePresets.json` expects `QT_DIR` to be set.

### 2) Fast smoke build + run

Use the wrapper script:

```bash
cmake -P build.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake -P build.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo run
```

Or use Ninja helper:

```bash
bash ./ninja_build.sh -t installdebug
```

Typical executable locations after install:

- Linux: `build.install/bin/mscore`
- macOS: `build.install/mscore.app/Contents/MacOS/mscore`
- Windows: `build.install/bin/MuseScore5.exe`

### 3) Unit tests (recommended)

### Build test configuration

```bash
MUSESCORE_BUILD_CONFIGURATION=utest \
MUSESCORE_DOWNLOAD_SOUNDFONT=OFF \
MUSESCORE_BUILD_UNIT_TESTS=ON \
bash ./ninja_build.sh -t installdebug
```

### Run all tests

```bash
cd build.debug
QT_QPA_PLATFORM="minimal:enable_fonts" \
ASAN_OPTIONS="detect_leaks=0:new_delete_type_mismatch=0" \
ctest -V
```

### Helpful test commands

```bash
cd build.debug
ctest -N
ctest -R engraving_tests -V
ctest -R iex_musicxml_tests -V
ctest -R project_test -V
```

### Known details

- Tests are mostly defined via `src/framework/testing/gtest.cmake` and `qtest.cmake`.
- Test names are `MODULE_TEST` names in each `*/tests/CMakeLists.txt`.
- Some test targets are placeholders or currently have no active test sources (for example `notation_tests` comments indicate no own tests yet).

### 4) Visual regression tests (`vtest`)

### Build vtest profile

```bash
MUSESCORE_BUILD_CONFIGURATION=vtest \
MUSESCORE_INSTALL_DIR=./build.install.vtest \
MUSESCORE_DOWNLOAD_SOUNDFONT=OFF \
bash ./ninja_build.sh -t installdebug
```

### Compare current draw data with repo reference data

```bash
XDG_RUNTIME_DIR=/tmp/runtime-root \
QT_QPA_PLATFORM=offscreen \
MU_QT_QPA_PLATFORM=offscreen \
./build.install.vtest/bin/mscore \
  --test-case ./vtest/vtest.js \
  --test-case-context ./vtest/vtest_context.json
```

If diffs are found, inspect:

- `vtest/temp_comparison/vtest_compare.html`
- generated diff artifacts under `vtest/temp_comparison/`

### Regenerate reference draw data (when intentionally updating expected output)

```bash
XDG_RUNTIME_DIR=/tmp/runtime-root \
QT_QPA_PLATFORM=offscreen \
MU_QT_QPA_PLATFORM=offscreen \
./build.install.vtest/bin/mscore \
  --test-case ./vtest/vtest.js \
  --test-case-context ./vtest/vtest_context.json \
  --test-case-func generateRefDrawData
```

### PNG comparison utilities

- Generate PNGs from scores: `vtest/vtest-generate-pngs.sh`
- Compare PNG sets: `vtest/vtest-compare-pngs.sh`

These are used in CI to compare two builds (current vs reference commit).

### 5) Conversion/import-export smoke checks

After an app build, run a few conversion paths:

```bash
# Pick the correct binary path for your platform, then:
# Linux:   ./build.install/bin/mscore
# macOS:   ./build.install/mscore.app/Contents/MacOS/mscore
# Windows: ./build.install/bin/MuseScore5.exe
MSCORE_BIN="./build.install/bin/mscore"
"$MSCORE_BIN" -o /tmp/demo.pdf ./demos/Amazing_grace.mscz
"$MSCORE_BIN" -o /tmp/demo.musicxml ./demos/Amazing_grace.mscz
"$MSCORE_BIN" --score-meta ./demos/Amazing_grace.mscz
```

This quickly validates key converter and import/export pipelines.

### 6) Code style checks

Install pre-commit hook:

```bash
./hooks/install.sh
```

This formats staged source files under `src/` using codestyle tools.

To run the same codestyle check style as CI:

```bash
cmake -P ./buildscripts/ci/checkcodestyle/ci_fetch.cmake
cmake -P ./buildscripts/ci/checkcodestyle/_deps/checkcodestyle.cmake ./src/
```

### 7) CI parity tests

The two key CI workflows to mirror locally are:

- Unit tests: `.github/workflows/check_unit_tests.yml`
- Visual tests: `.github/workflows/check_visual_tests.yml`

When in doubt, follow the commands in those workflows.

---

## Practical test matrix for normal feature changes

- **UI-only change**: smoke run app + targeted module unit tests (`muse_ui_tests`, related test target).
- **Engraving/notation change**: `engraving_tests` + related import/export tests + `vtest`.
- **Import/export change**: relevant `iex_*_tests` + converter smoke commands on demo files.
- **Playback/audio change**: `playback_test`, `muse_audio_tests`, and manual playback smoke in app.
- **Project/open-save change**: `project_test`, `engraving_tests`, save/reopen smoke using demos.

---

## Notes for contributors

- Module dependency order matters; see `src/CMakeLists.txt` and `src/framework/CMakeLists.txt`.
- `SetupConfigure.cmake` is the source of truth for what modules are active in each build configuration.
- Stubs are expected behavior in reduced profiles (`utest`, `vtest`, web, or optional module-off builds).
- The `test/` directory is mostly legacy/manual test data and scripts; modern automated coverage is primarily in `src/*/tests` and `vtest/`.
