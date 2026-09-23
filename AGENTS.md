# AGENTS.md

Canonical instructions for every coding agent in this repo (Claude Code loads this via `CLAUDE.md`).

## What this is

A fork of **MuseScore Studio** ("MuseScore Engrave Improved") focused on finer engraving control
(spacing, ledger lines, beam/slur/tie geometry, key-signature spacing, text tracking, etc.). Large C++17 / Qt 6
app built with CMake + Ninja. Most fork work lives in the **engraving** and **notation** modules.

- Fork-edited scores are saved as `.msdz` (upstream uses `.mscz`) to keep fork-specific data distinct.
- The maintainer writes specs and commit messages in Chinese. Reply in the language the user writes in.

## How to approach a task

From the maintainer (`Code_Guide.md`): reason from first principles; don't assume the stated ask is the real
goal. If the motivation is unclear, stop and discuss. If the goal is clear but the proposed path isn't the
shortest, say so and suggest a better one.

Before editing, read the relevant module and nearby patterns (`rg`); keep edits scoped to the requested
behavior; never revert unrelated working-tree changes.

## Build, run, test

Unity builds are **ON by default** (`MUSE_COMPILE_USE_UNITY`), so a one-line change recompiles a whole unity
chunk. Build trees: `build.debug` (compiled), `build.install` (installed bundle). If `build.debug/` does not
exist, run `./ninja_build.sh -t installdebug` first (a full build — long).

```bash
./ninja_build.sh -t debug          # configure + build Debug into build.debug/
./ninja_build.sh -t installdebug   # ...and install the app bundle into build.install/
ninja -C build.debug mscore        # fast loop: rebuild the app after C++ changes
ninja -C build.debug install       # refresh build.install/ so the bundle is current
build.install/mscore.app/Contents/MacOS/mscore   # run (use this, not the raw build.debug binary)
```

Headless smoke test (rendering / import-export / notation changes); write output outside the repo:

```bash
env HOME=/private/tmp/musescore-home QT_QPA_PLATFORM=offscreen \
  build.install/mscore.app/Contents/MacOS/mscore -F -f -o /tmp/out.pdf input.msdz
```

Unit tests (GoogleTest) are off in `ninja_build.sh` by default. The script's env var
`MUSESCORE_BUILD_UNIT_TESTS` maps to the CMake option `MUSE_ENABLE_UNIT_TESTS`:

```bash
MUSESCORE_BUILD_UNIT_TESTS=ON ./ninja_build.sh -t debug
ctest --test-dir build.debug                                            # all suites
build.debug/.../engraving_tests --gtest_filter='Engraving_BeamTests.*'  # one suite/case
```

- Each module's tests build into a `<module>_tests` target (e.g. `engraving_tests`), live in
  `src/<module>/tests/`, and load fixtures from adjacent `*_data/` dirs. Name fixtures `<Module>_<Behavior>Tests`.
  Add focused regression data for importer, playback, rendering, or export bugs.
- Visual layout regressions: `vtest/` (see `vtest/README.md`).
- `./ninja_build.sh -t compile_commands` writes a non-unity compile DB under `build.tooldata/` (for clangd).

### Verification ladder

Run the narrowest step that proves the change, then climb as far as the change warrants:

1. `git diff --check`
2. The relevant `<module>_tests` target, if tests are built.
3. `ninja -C build.debug mscore`, then `ninja -C build.debug install`.
4. Installed-app check or the headless smoke test above.

Report environmental warnings as environmental, not as failures. If a test target can't be built or run, say
so explicitly.

## Architecture

**Two namespace roots.** `muse::*` is the reusable framework in `src/framework/` (draw, ui, audio, global,
accessibility, network, diagnostics — no music knowledge). `mu::*` is MuseScore proper (engraving, notation,
project, importexport, …). Never reach from framework into app code.

**Module + DI wiring.** Each module has a `*module.cpp` (e.g. `src/engraving/engravingmodule.cpp`) whose
`registerExports()` registers implementations: `ioc()->registerExport<IFoo>(moduleName(), impl)`. Consumers use
`ioc()->resolve<IFoo>()`. Cross-module calls go through this — don't include a sibling module's internals.

**Layering:** `framework` → `engraving` (score model + layout, runs headless) → `notation` (editing, commands,
UI integration) → `appshell` (assembles the app). `importexport/` and `project/` (open/save/export) sit alongside.

### Inside `src/engraving/` (the fork's main surface)

- **`dom/`** — score object model. `EngravingItem` is the base; tree runs Score → Page → System → Measure →
  Segment → Chord/Rest → Note. Cast with `item->isType()` / `toType()`, not `dynamic_cast`. Computed geometry
  lives in per-element `LayoutData`, separate from layout algorithms.
- **`rendering/score/`** — layout + paint engine. Entry: `ScoreRenderer::layoutScore()`;
  `TLayout::layoutItem()` dispatches per type; one layout file per element (`ChordLayout`, `BeamLayout`,
  `SlurTieLayout`, `HorizontalSpacing`, `SystemLayout`, `PageLayout`, …). Most engraving tweaks land here.
- **`style/`** — `styledef.cpp` defines every style key (`Sid`) and its default. Behavior is mostly
  style-driven: a new layout option usually means a new `Sid` + default + read/write hook.
- **`rw/`** — serialization. Current format: `write/` and `read460/`. `read114/206/302/400/410/` are
  version-specific readers wired in `rwregister.cpp`; update the matching readers when changing storage.
- **`types/`** enums/types, **`infrastructure/`** SMuFL + font plumbing, **`compat/`** legacy adapters.

## Code style

`Code_Guide.md` is the source of truth. Key C++ rules:

- 4 spaces, no tabs; lines < 120 cols; `#pragma once`; minimal, category-grouped includes; forward-declare in
  headers.
- Lowercase `mu::*` / `muse::*` namespaces; `PascalCase` types, `camelCase` functions/vars, `UPPER_CASE`
  constants, `m_` members; acronyms camel-cased (`XmlReader`).
- `nullptr`; one declaration per line; no C-style casts; attached braces for control flow, braces always;
  no `else` after `return`; constructor-initializer colon on a new line.
- Match the surrounding file when it differs from the guide; don't fight the formatter.

Formatter: uncrustify with `tools/codestyle/uncrustify_musescore.cfg` — run
`tools/codestyle/uncrustify_run_file.sh <file>`. `hooks/install.sh` installs a pre-commit hook that checks
staged files. Both need `uncrustify` on `PATH`.

## Translations (i18n)

Every user-facing string must be translatable, and `share/locale/musescore_en.ts` must stay in sync with code.

- **Mark strings.** QML: `qsTrc("context", "…")`. C++: `mtrc` / `qtrc` / `trc("context", "…")` or
  `TranslatableString`. Qt Designer `.ui` strings are auto-extracted. A plain literal translated only at its use
  site (e.g. `qtrc(ctx, keyReturnedFromCpp)`) is invisible to `lupdate` — mark it at its definition with
  `QT_TRANSLATE_NOOP("context", "…")` using the **same** context as the use site (examples:
  `src/framework/global/utils.cpp` note names, `src/fontdesign/internal/fontdesigntypes.cpp`).
- **Regenerate** whenever you add / change / remove a string (needs Qt `lupdate` on `PATH`):
  `bash tools/translations/run_lupdate.sh`. The postprocess step fails on straight quotes (`'` `"` → `‘’` `“”`
  or `′` `″`), `...` (→ `…`), and leading/trailing/double spaces. Fix every error; only `editstyle.ui` and
  `symnames.cpp` are exempt. Ignore lupdate's pre-existing parser warnings (`.mm`/`.js`/fluidsynth/`api/v1`).
- Never hand-edit the `.ts`. Never touch other `*_<lang>.ts` / `instruments_*.ts` — they come from Transifex.

## Gotchas

- Unity builds can hide a missing `#include`; the non-unity `compile_commands` build exposes it.
- Root-level `*.mscx` files are gitignored scratch scores and `tmp/` is untracked scratch — never stage them.
- Do final runtime checks on the installed `build.install` bundle, not the raw `build.debug` binary.

## Commits and PRs

- **Subject format:** `类型：结果`, in Chinese, describing the user-visible outcome. Prefixes in use: `新增`
  (feature), `修正` / `修复` (fix), `文档` (docs), `构建` (build), `测试` (tests), `i18n`. Examples:
  `修正：换行后挂起装饰音不再错位`, `新增：样式中可开启提示性变音记号`.
- One change per commit. Check `git status --short` and `git diff --stat`, then stage only related files.
- **Before every commit, check i18n:** if any user-facing string was added, edited, or removed, run
  `run_lupdate.sh`, fix errors, and stage `share/locale/musescore_en.ts` in the **same** commit.
- **No AI attribution.** Never add `Co-Authored-By` / `Co-authored-by` trailers for Claude, Anthropic, Cursor,
  or any other assistant; commits are attributed to the maintainer only. Strip any trailer a tool injects.
- Push only when explicitly asked.
- PRs: problem statement, fix, affected modules, verification commands, and screenshots or exported files for
  UI/rendering changes; call out any test target that could not be run.
