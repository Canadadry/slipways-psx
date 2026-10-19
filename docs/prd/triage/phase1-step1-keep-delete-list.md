---
title: "Plan 0 keep/delete list of files, targets and CI jobs"
description: "Steps 2 to 4 of plan 0 need one committed, mechanically checked list saying which file, Makefile target and CI job the fork keeps and which it deletes."
status: needs-triage
---

## Problem Statement

The fork of `slipways-psx` holds 821 tracked files, about 80 public Makefile
targets and two CI workflows. Most of it serves the PSX port, the raylib app,
demo authoring, `pad_author` and profiling, none of which the LÖVE port needs.
Steps 2 to 4 of plan 0 delete those parts. Without one agreed list they would
each have to work out again what the kept targets need. That is how a file the
replay links quietly gets deleted along with a directory the plan names. For
example, `src/platform/` is marked for deletion as a whole, yet the replay and
a dozen unit tests link `src/platform/fatal_pc.c`.

## Solution

One committed data file, `docs/love-port/plan-0-files.txt`, marks every
tracked file, every public or `test`-reached Makefile target and every CI job
as `keep` or `delete`. Each deleted entry carries the step that deletes it.
Each kept entry says why it is kept. A one-off check script proves two things:

1. **Nothing is missing** (under-inclusion). Every kept target builds and passes
   in a copy of the repository that contains only the files marked `keep`.
2. **Nothing extra is kept** (over-inclusion). Every file kept "because a target
   builds from it" really is reached by one of those targets.

The script also checks that the list covers exactly the tracked files. Steps 2
to 4 then delete by tag and can rerun the same script to prove they deleted
nothing a kept target needs.

## User Stories

1. As the step-2 implementer, I want every PSX file, target and CI item tagged `psx`, so that I can delete exactly those without re-deriving the PSX closure.
2. As the step-3 implementer, I want every raylib file, target and CI job tagged `raylib`, so that I can delete them without reading the Makefile again.
3. As the step-4 implementer, I want every remaining deleted item (demo authoring, `pad_author`, profiling, unused binaries, devtools and tools) tagged `tool`, so that "no file marked delete left" is a plain filter of the list.
4. As any later implementer, I want files that the plan deletes as a group but a kept target builds from (such as `src/platform/fatal_pc.c`) marked `keep`, so that a directory-wide delete never breaks the replay.
5. As a reviewer, I want the list to cover every tracked file exactly once, so that a file nobody thought about cannot slip through unmarked.
6. As a reviewer, I want each kept file tagged with why it is kept (`build` or `plan`), so that I can tell a needed source from a file the plan's Rules keep by name.
7. As a reviewer, I want a check that every kept target builds and passes from kept files only, so that I can trust the deletions without reading 2400 Makefile lines.
8. As a reviewer, I want a check that every file tagged `keep build` is actually reached by a kept target, so that the list does not keep dead files by caution.
9. As a step-2-to-4 implementer, I want to rerun the same check after my deletions, so that I prove that no kept target lost a file it builds from.
10. As a developer of the fork, I want `make test` and `make e2e` unchanged in speed and content by this step, so that the fast loop stays fast.
11. As a developer, I want every `tests/e2e/*.json.sha256` unchanged, so that the source of truth is untouched.
12. As a reviewer, I want each CI job marked keep or delete, with the targets the kept job calls marked keep, so that steps 2 to 4 know what "every target a kept CI job calls" means.
13. As a reviewer, I want the generators of kept code (bindings, demo unit, sine table) marked keep together with their inputs, so that the generated units still rebuild.
14. As a reviewer, I want the attract demo data the game plays kept even though its authoring tools go, so that the attract replay still passes.
15. As the step-5 implementer, I want the list's header to say that step 5 deletes both the list and its check script, so that nothing is left referring to a deleted file.
16. As a maintainer, I want pipeline files (`.claude/`, `progress.md` and its hook script, `docs/love-port/`) kept, so that the plan-pilot run that executes plan 0 keeps working.
17. As a reviewer, I want targets that write into the tree (the baselines and the sine table) proven to reproduce the committed files, so that keeping them is safe.
18. As a reviewer, I want each `delete` decision that the plan does not name explicitly recorded in this PRD, so that the choice is reviewable.
19. As a reviewer, I want the list format to have one item per line and to be easy to grep, so that each later step's delete is one filter.

## Implementation Decisions

**The list (data, not code).**

- Path: `docs/love-port/plan-0-files.txt`, as named by the plan. Lines starting
  with `#` are comments. The header states the format, the tag meanings, and
  that step 5 deletes the list and its check script.
- One entry per line, four fields separated by single spaces:
  `<kind> <mark> <tag> <name> [KEY=VALUE ...]`.
  - `kind` is `file`, `target` or `ci`.
  - `mark` is `keep` or `delete`.
  - `tag` for `delete` is `psx` (deleted by step 2), `raylib` (step 3) or `tool`
    (step 4). For `keep` it is `build` (a kept target or a kept CI job builds
    from it, reads it or runs it) or `plan` (kept by name in the plan's Rules or
    needed by the pipeline, but not read by any target).
  - `name` is a tracked path for `file`, a Makefile goal for `target`, and
    `<workflow file>:<job id>` for `ci`.
  - Optional `KEY=VALUE` arguments, on `target` lines only, are the variables the
    check passes to that goal (for example the `EVENTS` file that `replay`
    requires).
- Entries are sorted by kind, then name. No tracked path has a space or a
  non-ASCII character, so the format needs no quoting.
- Coverage: the `file` entries are exactly the output of `git ls-files`, minus
  `docs/prd/`. That directory belongs to the PRD pipeline and changes while the
  plan runs (this PRD lands with the step's first chunk), so it is never listed.
  The list lists itself (`keep plan`).
- Which targets get a line: every target with a `## ` help line, plus every
  prerequisite of `test` and `test-all`. That includes `pad-author-self-check`,
  which has no help line. Per-test `unit-run-*` goals get no line: a unit test
  is kept or deleted through its `tests/test_<name>.c` file line.
- A composite target (`test`, `test-all`, `unit`) is marked `keep`. Its `delete`
  prerequisites are listed as their own `target ... delete` lines; steps 2 to 4
  remove them from the composite.

**Classification rules.** The implementer applies these to the Makefile as it
is now:

- `keep build`:
  - every file reachable from the kept targets, through make prerequisites, the
    compiler's `.d` files, or the files the replay records as read;
  - the generators of kept code (`bindings_generate` with
    `bindings.default.cfg`, `demo_generate` with its devtools sources and
    `demo/demo.json`, `gen_quarter_wave`);
  - the sources the replay links, even devtools that plan step 4 would otherwise
    call unused (screencast, pad metrics, draw filter, capture list, VRAM
    digest): kept means "a kept target builds from it" today;
  - `src/platform/fatal_pc.c` and anything else under `src/platform/` that a
    kept target links. The rule "nothing the replays need is deleted" overrides
    the plan's directory-wide wording. Step 3 deletes only what the list marks.
- Kept targets: `replay`, `e2e`, `e2e-baseline`, `e2e-ref`, `state` (with and without
  `EVENTS`), `end-state`, `end-state-baseline`, `coverage`, `sanitize-net`,
  `full-game`, `sin-table`, `help`, `float-link`, the guards `constants-check`,
  `float-grep`, `float-guard`, `alloc-guard`, `pointer-guard`, `random-guard`,
  `button-guard`, `data-guard`, `p8-guard`, and the composites `unit`, `test`,
  `test-all`.
- A unit test listed in `UNIT_TESTS` is `keep build` if and only if every file
  its binary builds from is kept and it tests a kept module. Otherwise it is
  `delete`, with the tag of what it tests. Examples: tests of `keymap` →
  `raylib`; tests of `psx_*` or the PSX pad log → `psx`.
- Tests and tools that are not in `UNIT_TESTS` and that no kept target reaches
  are deleted.
- `worst-case/`: the `.json` games and their `.end-state` files are kept,
  because `full-game`, `state` and `end-state` read them. The `.pad` scripts and
  the `attract_cycle` files are `delete tool`, unless the build check proves a
  kept target reads them.
- `demo/`: `demo.json` is kept (it is compiled into the game, and the attract
  replay depends on it). `demo.pad` and `demo_record.hex` are `delete tool`.
- `tests/e2e/` is kept whole. The `.json`, `.cfg`, `.json.sha256` and
  `.end-state` files are `build`; the `.txt` notes and `.gitkeep` are `plan`.
- `keep plan` covers:
  - `CLAUDE.md`, `README.md`, `docs/love-port/`, `docs/save-format.md` and the
    images in `docs/img/` (step 5 rewrites or prunes them);
  - `.claude/`, `.gitignore`, `progress.md`, `tools/progress_check.sh`;
  - the list itself, and its check script.
- `.github/workflows/ci.yml` is kept. `ci ci.yml:test keep build` and
  `ci ci.yml:raylib delete raylib`. The `test` job calls `test-all`,
  `sanitize-net` and `full-game`, which must all be kept targets.
  `.github/workflows/psx-release.yml` is `delete psx`.
- The `delete` decisions that the plan's Delete list does not name explicitly
  are taken here:
  - `coverage-compare` → `tool`: not part of the port, and no kept target or
    skill calls it.
  - `e2e-ref` → `keep plan`: `check-e2e` and `tools/frames_check.sh` use it to
    find the first diverging frame of a red e2e while the C is the reference.
    Step 8 is skipped (section 5 unanswered), so the history it reads stays.
  - `resume-check`, `save-fuzz`, `save-worst-case`, `rng-sync-probe` → `tool`.
    These are C-only debugging probes that are outside every kept CI job.
  - `situations`, `situation-report`, `steady-windows` and every `pad-metrics*`
    target → `tool`. Of these, any whose only purpose is PSX timing
    (`situations-check`, `short-run-situations`, `psx-real-game-budget`) →
    `psx`.
  - `frame-profile*`, `steady-profile`, `lever-estimate`, `frame-profile-test`
    → `tool`.
  - `psx-*` and `psn00bsdk-toolchain-setup` → `psx`.
  - `raylib` and `raylib-vendor-setup` → `raylib`.
  - `keymap.default.cfg` and the keymap generator → `raylib`.
  - `psx/` and the PSX docs, scripts and patches → `psx`.
  - `docs/raylib-vendoring.md` and `tools/install_raylib_vendor.sh` →
    `raylib`.
- Tag choice when a deleted file serves several deleted groups: the earliest
  step that removes its last user. Under that rule, a tool used only by PSX
  targets and by profiling targets is tagged `tool`, because step 4 deletes the
  profiling targets.
- Untracked vendored trees (`third_party/psn00bsdk`, `third_party/pcsx-redux`,
  `third_party/raylib`) are gitignored and not listed. Steps 2 and 3 delete
  them as part of their own work.

**The check script.**

- It is a POSIX `sh` script in `tools/`, named `plan0_files_check.sh`. It is
  listed as `keep plan`, and step 5 deletes it together with the list. It is
  **not** wired into `make test`, `make e2e` or `make test-all`. It runs once
  in this step, and steps 2 to 4 may rerun it. The step commits no Makefile
  change.
- Mode `list` (fast, a few hundred milliseconds), which checks that:
  - every line is well-formed: known kind, mark and tag, with a tag valid for
    its mark;
  - no name appears twice;
  - the `file` names are exactly `git ls-files` minus `docs/prd/`;
  - every `keep` file exists;
  - every `target` name is a goal the Makefile knows. The script checks this
    against make's database (`make -pn`), so a typo is caught.
  - every `ci` job named exists in its workflow, and every target a `keep` CI
    job's steps call is a `target ... keep` line.

  A `delete` file may be absent, because a later step has already deleted it.
  The mode also prints the number of `delete` entries still present per tag:
  "no file marked delete left" in step 4 means the count is zero for every tag.
- Mode `build` (one-off, about one to three minutes):
  1. Run `list`.
  2. Copy only the `keep` files into a fresh directory under `tmp/`, which is
     gitignored and never committed.
  3. In that copy, run each `target ... keep` goal in its own `make`
     invocation, with its `KEY=VALUE` arguments.
     - Composites are not run as such. Their kept prerequisites are run
       instead, plus `unit-run-<name>` for each kept `tests/test_<name>.c` that
       appears in the Makefile's `UNIT_TESTS`. The script reads `UNIT_TESTS`
       from make's database.
     - Goals that write into the tree (`e2e-baseline`, `end-state-baseline`,
       `sin-table`) run last.
     - Any failure fails the check and names the goal.
  4. Check that every file of the copy is byte-identical to its original. This
     proves that the baselines and the sine-table generator reproduce the
     committed files, so every `.json.sha256` is unchanged.
  5. Reach check (over-inclusion). Rerun the same goals with `make -n -d`, now
     that the `.d` files exist, and collect every "Considering target file"
     name that is a tracked path. Every `keep build` file must be in that set,
     or in the `.d` files the replay writes for the files it reads at run time.
     Files that fail are reported, and the implementer either retags them
     `keep plan` (only for a plan-Rules reason, written in a `#` comment on the
     line above) or turns them into `delete`.
- The copy is not a git repository. A kept goal that needs git (`e2e-ref`)
  is not run there; its files are checked by the reach check like any other.
- Speed rule: nothing in this step touches `make test` or `make e2e`. The
  `build` mode is the step's verification; it is not a new target.

**Verification of the step** (from the plan): `plan0_files_check.sh build`
passes, which shows that every kept target builds from kept files only, that
every file not kept is marked `delete`, and that every file kept as `build` is
reached. In the real tree, `make test-all`, `make sanitize-net` and
`make full-game` (the targets of the kept CI job) pass, with every
`tests/e2e/*.json.sha256` unchanged.

## Implementation Chunks

1. **List mode and the full list.** Write the script's `list` mode and the
   first complete `plan-0-files.txt`, drafted from the Makefile with the rules
   above. Done when `list` passes on the real tree and `make test` and
   `make e2e` are green.
2. **Build mode and correcting the marks.** Add the `build` mode: the pruned
   copy, the per-goal runs, the identical-tree check and the reach check. Fix
   every mark it proves wrong until it passes. Then run `make test-all`,
   `make sanitize-net` and `make full-game` in the real tree. Done when all
   pass.

## Testing Decisions

- A good test here exercises the observable outcome: the kept targets pass in
  a tree reduced to the kept files, and the list matches the tracked files. It
  does not test how the script parses lines.
- The check script gets no unit test in `tests/`. It lives only for steps 1
  to 4, and a test in `make test` would make the fast loop slower and would
  outlive the script by one step. Its correctness is shown once by hand, and
  each result is recorded in the commit message:
  - **Removed line**: with one `file` line removed, `list` fails and names the
    path.
  - **Needed file deleted**: with `src/platform/fatal_pc.c` marked `delete`,
    `build` fails on `replay`.
  - **Unreached file kept**: with an unreached file (for example
    `tools/psx_boot.sh`) marked `keep build`, the reach check fails and names
    it.

  Each sabotage is undone before the commit.
- The source of truth stays the e2e replays: `make e2e` with every `.sha256`
  unchanged, in the real tree and in the pruned copy.
- Prior art: the other `tools/*_check.sh` and guard scripts (`frames_check.sh`,
  `end_state.sh`, `float_guard.sh`) for the shape of a POSIX check script. The
  `coverage-baseline` recipe in the Makefile shows how to run `make` in another
  tree with `MAKEFLAGS` cleared, which the `build` mode needs too.

## Out of Scope

- Deleting anything: steps 2 to 4 do that.
- Editing the Makefile, the CI workflow, `CLAUDE.md`, `README` or the docs:
  steps 2 to 5.
- Moving `fatal_pc.c` (or any other kept `src/platform/` file) out of
  `src/platform/`: that is a later step's choice, if one wants it.
- Answering the plan's open question on clearing the history (step 8): the
  pilot skipped it, the history is kept.
- Any Lua code or the fake LÖVE.

## Further Notes

- The kept CI job (`test`) still calls `test-all`, which still includes
  `pad-author-self-check` until step 4. "Every target a kept CI job calls still
  passes locally" therefore stays true through steps 2 and 3 with no change to
  `test`. Step 4 removes the `delete` prerequisite from `test`.
- Everything ran headless. The raylib binary is never built or run in the
  pruned copy: the `raylib` target is `delete`, and `third_party/` is not
  copied.
- macOS: the `build` mode needs `flock` for the locked goals (`e2e`), as the
  real tree does. GNU Make here is 3.81, whose `-d` output still prints
  "Considering target file" lines.
