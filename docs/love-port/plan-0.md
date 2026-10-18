# Plan 0 — Fork cleanup for the LÖVE port

## 1. Goal

Turn the fork of `slipways-psx` into a repository that holds only what the
LÖVE port needs: the C game as the reference, its headless replay, the 20 e2e
replays, plus two outputs measured from the C while it still exists — the
sprite sheet and font as PNGs, and the order in which plan 3 will make the
replays pass. The shared design is in `docs/love-port/design.md`; read it first.

## 2. Rules

- Runs in the **fork**, never in `slipways-psx`.
- `make test` and `make e2e` stay green after every step, with every
  `tests/e2e/*.json.sha256` unchanged. Nothing the 20 replays need is deleted:
  the attract demo data the game plays (`attract.json`) stays, even if the
  tools that authored it go.
- Keep: `src/p8/`, `src/engine/`, `src/game/`, the parts of `src/devtools/`
  that `cmd/replay.c` and `make state` link (replay source, dump, state,
  input, save storage), `src/cmd/replay.c`, `tests/e2e/`,
  `tools/frames_check.sh`, `tools/end_state.sh`, `tools/coverage.sh`, the
  coverage target, and the unit tests and guards of what is kept.
- Delete: PSX (code under `src/platform/psx_*`, `src/cmd/psx_*`, the PSX
  docs, toolchain and emulator scripts, the release workflow, PSX captures and
  targets), raylib (`src/platform/`, vendoring script and doc, the `raylib`
  CI job), the demo authoring tools, `pad_author`, profiling, and the unit
  tests, guards and Makefile targets of everything deleted.
- `CLAUDE.md`, `README` and `docs/` are updated to describe the fork: no PSX
  or raylib left, the port described by `docs/love-port/design.md`.
- Not this plan: any Lua, the fake LÖVE.

## 3. Phases and steps

### Phase 1 — Delete

1. Delete PSX: sources, binaries, Makefile targets, docs, scripts, the
   release workflow. Verified by `make test-all` green, sha256 unchanged, and
   `grep -ri psx` finding only historical mentions in `tests/e2e/*.txt`.
2. Delete raylib: `src/platform/`, its targets, vendoring, CI job, and its
   `CLAUDE.md` rules ("never launch the raylib app"). Verified by
   `make test-all` green.
3. Delete demo authoring, `pad_author`, profiling, and the remaining unused
   `src/cmd/` binaries, devtools and tools. Verified by `make test-all` green
   and `make help` listing no deleted target.
4. Rewrite `CLAUDE.md` and `README` for the fork. Verified by reading: no
   reference to anything deleted.

### Phase 2 — Measure from the C

5. A C tool `src/cmd/asset_export.c` (`make asset-export`) writes
   `game/sprites.png` (the 128×128 sprite sheet) and `game/font.png` (every
   glyph of the font, in a documented grid) in the 16 palette colours of
   `src/p8/palette.c`, colour 0 opaque. Verified by a unit test that reads
   both PNGs back and compares every pixel with the C data.
6. `tests/e2e/order.txt`: for each replay, the number of lines of
   `src/game/` and `src/engine/` it executes (the coverage target, one
   replay at a time), sorted ascending, one `<replay> <lines>` per line.
   Verified by re-running the measurement and getting the same file.

## 4. Done when

- `make test-all` green, every `.json.sha256` unchanged.
- `game/sprites.png`, `game/font.png` and `tests/e2e/order.txt` committed.
- No PSX, raylib, demo-authoring or profiling code or target left.

## 5. Open questions

- Clear the fork's git history (one root commit holding the cleaned tree)?
  The fork does not need it, and the C reference stays readable in the
  tree until plan 4 deletes it; `slipways-psx` keeps the full history.
  To decide before step 1: if yes, it is the last step of this plan.

## 6. Step notes
