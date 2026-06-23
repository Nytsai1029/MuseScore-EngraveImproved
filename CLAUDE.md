# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A fork of **MuseScore Studio** ("MuseScore Engrave Improved") focused on finer engraving control
(spacing, ledger lines, beam/slur/tie geometry, key-signature spacing, text tracking, etc.). It is a
large C++17 / Qt 6 application built with CMake + Ninja. Most fork work lives in the **engraving** and
**notation** modules.

Fork-edited scores are saved as `.msdz` (upstream uses `.mscz`) to keep fork-specific data distinct.

## Build, run, test

Unity builds are **ON by default** (`MUSE_COMPILE_USE_UNITY`), so a one-line source change recompiles a
whole unity chunk. Two build trees are used: `build.debug` (compiled) and `build.install` (installed bundle).

```bash
# Full configure + build (Debug) into build.debug/
./ninja_build.sh -t debug

# Configure + build + install the app bundle into build.install/
./ninja_build.sh -t installdebug

# Fast edit loop once build.debug is configured:
ninja -C build.debug mscore     # build just the app after C++ changes
ninja -C build.debug install    # refresh build.install/ so the bundle is current

# Run the installed app (prefer this over the raw build.debug binary for runtime checks)
build.install/mscore.app/Contents/MacOS/mscore
```

Headless import/export smoke test (use for rendering / import-export / notation changes):

```bash
env HOME=/private/tmp/musescore-home QT_QPA_PLATFORM=offscreen \
  build.install/mscore.app/Contents/MacOS/mscore -F -f -o /tmp/out.pdf input.msdz
```

Tests use **GoogleTest** and are off by default. Enable, build, and run:

```bash
MUSESCORE_BUILD_UNIT_TESTS=ON ./ninja_build.sh -t debug
ctest --test-dir build.debug                              # all suites
build.debug/.../engraving_tests --gtest_filter='Engraving_BeamTests.*'   # one suite/case
```

Each module's tests build into a `<module>_tests` target (e.g. `engraving_tests`, `muse_global_tests`),
live in `src/<module>/tests/`, and load fixtures from adjacent `*_data/` directories.

`clangd` is configured (`.clangd`) but needs a non-unity compile DB: `./ninja_build.sh -t compile_commands`
generates `build.tooldata/compile_commands.json` (unity OFF).

## Architecture

**Two namespace roots.** `muse::*` is the reusable "Muse framework" in `src/framework/` (draw, ui, audio,
global, accessibility, network, diagnostics — no music knowledge). `mu::*` is MuseScore proper (engraving,
notation, project, importexport, …). Don't reach from framework into app code.

**Module + dependency-injection wiring.** Each module has a `*module.cpp` (e.g.
[engravingmodule.cpp](src/engraving/engravingmodule.cpp)) whose `registerExports()` registers interface
implementations into the global IoC container: `ioc()->registerExport<IFoo>(moduleName(), impl)`. Consumers
get them via `ioc()->resolve<IFoo>()`. This is how modules talk across boundaries — follow it instead of
including a sibling module's internals directly.

**Rough layering:** `framework` (shared) → `engraving` (score model + layout, runnable headless) →
`notation` (editing, commands, UI integration) → `appshell` (assembles the app). `importexport/` and
`project/` (open/save/export) sit alongside.

### Inside `src/engraving/` (the fork's main surface)

- **`dom/`** — the score object model (~370 files). `EngravingItem` is the element base class; the tree runs
  Score → Page → System → Measure → Segment → Chord/Rest → Note. **Cast with `item->isType()` / `toType()`
  helpers, not `dynamic_cast`** (see `Code_Guide.md`). The model stores computed geometry in per-element
  `LayoutData`, kept separate from the layout algorithms.
- **`rendering/score/`** — the layout + paint engine, deliberately separated from the model.
  `ScoreRenderer::layoutScore()` is the entry point; `TLayout::layoutItem()` dispatches per element type;
  each element class gets its own layout file (`ChordLayout`, `BeamLayout`, `SlurTieLayout`,
  `HorizontalSpacing`, `SystemLayout`, `PageLayout`, …). Most engraving tweaks land here.
- **`style/`** — `styledef.cpp` defines every style key (the `Sid` enum) and its default. Most engraving
  behavior is **style-driven**, so new layout options usually mean a new `Sid` + default + a read/write hook.
- **`rw/`** — serialization. Current format lives in `write/` and `read460/`; `read114/206/302/400/410/`
  are version-specific readers for backward compatibility, wired up in `rwregister.cpp`. Touch the matching
  reader when changing how something is stored.
- **`types/`** enums/types, **`infrastructure/`** SMuFL + font plumbing, **`compat/`** legacy adapters.

## Conventions and working notes

- **`Code_Guide.md` and `AGENTS.md` are the sources of truth** for style and workflow — read them before
  non-trivial work. Key C++ rules: 4 spaces (no tabs), lines < 120 cols, `#pragma once`, lowercase `mu::*` /
  `muse::*` namespaces, `PascalCase` types, `camelCase` functions/vars, `UPPER_CASE` constants, `m_` member
  prefix, `nullptr`, one declaration per line, no C-style casts. Match the surrounding file; don't fight the
  formatter.
- For C++ changes that affect the app, the expected verification is `ninja -C build.debug mscore` →
  `ninja -C build.debug install`, then check the installed bundle (not the raw `build.debug` binary).
- **In-progress fork features have specs under `.kiro/specs/<feature>/`** (`requirements.md` in EARS format,
  `design.md`, `tasks.md`) — e.g. `fit-music-reflow`, `split-tie`. Read the spec before working on that
  feature. Specs and some commit messages are written in Chinese; the maintainer communicates in Chinese.
- Per the maintainer's note in `Code_Guide.md`: reason from first principles, don't assume the stated ask is
  the real goal — if the motivation is unclear, stop and discuss; if the goal is clear but the proposed path
  isn't the shortest, say so and suggest a better one.
- Keep commits scoped to one change with a short imperative subject; stage only related files; push only when
  explicitly asked.
