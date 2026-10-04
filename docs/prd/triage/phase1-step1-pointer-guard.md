---
title: "Pointer guard"
description: "Nothing counts or sorts the data pointers held in game state, so plan 13 cannot prove that its later steps lower that count and that no step adds one."
status: needs-triage
---

## Problem Statement

Plan 13 (`docs/plan-13.md`) removes every pointer from game state that can dangle or that needs a paragraph to explain. Steps 3 to 5 lower the number of pointers, and the other steps must not raise it. Today nothing measures that number. A reviewer cannot tell how many data pointers sit in the struct fields and file-scope variables of `src/game/` and of `entity_class.h`. They cannot tell which of those pointers section 2 of the plan allows to stay, or which later step is meant to remove each one. A commit could add a new `entity_t *` link or a new `entity_list_t` and `make test` would still pass. Lists of entity pointers are invisible too: an `entity_list_t` holds `entity_t **` inside its own type, so a search for `*` in game code never sees it.

## Solution

This step adds a pointer guard modelled on the existing allocation guard:

- **The script.** `tools/pointer_guard.sh` reads the source text of `src/game/*.c`, `src/game/*.h` and `src/engine/entity_class.h`. For each file it reports two numbers:
  - the data pointers declared in struct (and union) fields and in file-scope variables, `const` ones included;
  - the struct fields and file-scope variables that hold a list of entity pointers, whether held by pointer or by value.
- **The exception list.** `tools/pointer_guard_exceptions.txt` records today's numbers, one line per file. The guard requires strict equality with it.
- **Wiring.** A `pointer-guard` target runs inside `make test`, and a unit test proves that the guard fails in every way it should.
- **The sorting.** The step sorts every counted pointer by hand into section 7 of the plan. Each pointer is either allowed by section 2, or assigned to step 3, 4 or 5. Anything that fits none of these goes under an `Unsorted` heading.
- **Cost.** The guard reads text only. It compiles nothing and does not depend on any built object. It runs as a single `awk` pass, in parallel with the unit tests under the existing `-j`, so `make test` gets no slower.

## User Stories

1. As a plan-pilot run, I want `make test` to fail when a commit adds a data pointer to a game struct field, so that no step of plan 13 can raise a pointer count without anyone noticing.
2. As a plan-pilot run, I want `make test` to fail when a commit removes a pointer but leaves the exception list unchanged, so that the list always states today's exact count and every decrease is recorded.
3. As a developer, I want the failure message to print the exact line to write, add or remove in the exception list, so that I can fix a red guard without working the number out myself.
4. As a developer, I want a new game file that declares pointers to fail the guard until it gets its own line, so that splitting or adding a file cannot hide pointers.
5. As a developer, I want a listed file that has no counted pointers left, or that no longer exists, to fail with "line to remove", so that the list never carries stale lines.
6. As a developer, I want `const` pointers counted like any other, so that a `const` alias into memory that can be freed is not mistaken for safe static data.
7. As a developer, I want pointers to pointers (`const vec_t *const *pos`) counted as one pointer per declarator, so that the number matches the fields a person reads.
8. As a developer, I want arrays of pointers (`g_projects[N]`, `period_names_table[]`) counted as one per declarator, so that file-scope tables are seen.
9. As a developer, I want every declarator of a line like `int *a, *b, c;` counted on its own (two here), so that comma declarations cannot hide pointers.
10. As a developer, I want file-scope variables with an initializer (`static entity_t *g_main_mouse = NULL;`, `const char *const names[] = {...};`) counted from the text before `=`, so that initializer braces do not hide the declaration.
11. As a developer, I want fields of structs nested in other structs, of unions, and of struct types defined inside a function counted, so that every struct field is covered wherever its type is defined.
12. As a developer, I want function-pointer fields, function-pointer typedefs and arrays of function pointers left out, so that vtables such as `entity_class_t` do not swamp the count.
13. As a developer, I want the runtime handles `allocator_t`, `p8_graphics_t` and `p8_vram_t` left out, `const` or not, so that only the pointers section 2 means to remove or review are counted.
14. As a developer, I want locals, parameters, function prototypes, function definitions that return a pointer, and `typedef` lines outside a struct body left out, so that only stored state is counted.
15. As a developer, I want `extern` declarations left out and only the definition counted, so that a variable declared in a header and defined in a `.c` file is not counted twice.
16. As a developer, I want `*` inside comments, string literals, character literals and preprocessor lines ignored, so that text cannot change the count.
17. As a developer, I want a field or variable of type `entity_list_t` counted in the list count whether it is held by value, by pointer or as an array, so that the pointers the list type hides become visible.
18. As a developer, I want an `entity_list_t *` field counted in both counts (it is a pointer and a list), so that steps 3 and 5 each lower the count they own.
19. As a developer, I want a struct-typed variable that is not itself a pointer (`static const data_row_t row = {...}`) left out of the count, so that the pointers inside that struct are counted once, at the struct definition.
20. As a plan-pilot run, I want the guard to exit 2 when it is given no file or cannot read the list, so that a guard over nothing is never taken for green.
21. As a developer, I want a one-line summary (`pointer-guard files N pointers P entity-lists L`), so that progress across plan 13 can be read from the `make test` output.
22. As a plan-pilot run, I want `make test` no slower than before, so that the CLAUDE.md rule about the fast loop holds.
23. As the user, I want every counted pointer sorted in section 7 of the plan as allowed, step 3, step 4 or step 5, so that I can check the plan's scope against the real code before step 2 starts.
24. As the user, I want any pointer that fits no category listed under its own `Unsorted` heading, so that the run stops and I decide, instead of an agent forcing it into a category.
25. As a later step's PRD writer, I want the sorting to name each pointer precisely (file, struct or variable, field), so that my step's scope can be read straight from section 7.
26. As a warn-review run at the end of phase 1, I want the sorting to say which step owns each pointer, so that I can report a warning as "planned for step N" instead of as a finding.
27. As a developer, I want the exception list ordered so that files whose counted pointers all stay (`const` pointers to static data) come last, so that the list's final shape at the end of plan 13 is already visible.
28. As a developer, I want the guard's unit test to need no compiler, so that it adds nothing noticeable to `make test`.

## Implementation Decisions

- **Script interface.** `pointer_guard.sh <exception list> <source file>...`. Paths are reported exactly as given; the Makefile passes them relative to the repository root (`src/game/planet.h`). Exit codes: 0 when the counts match, 1 on any difference, 2 with no source file or an unreadable list. These are the same exit codes as `alloc_guard.sh`.
- **One process, text only.** A single `awk` invocation reads every file, with no loop over files in the shell and no `objdump` or compiler. Inside it:
  1. strip comments, string and character literals, and preprocessor lines;
  2. split the text into statements at `;`, `{` and `}`, and track brace depth with a stack that records whether each `{` opens a struct/union body or something else (a function body, an initializer, an enum);
  3. examine a statement only when it ends at depth 0 (file scope) or directly inside a struct/union body.

  For a file-scope statement that opens an initializer, the declaration is taken from the text before `=`; the guard must not lose it when the brace opens. A rough per-file prototype took about 0.5 s over the 92 files with a character-by-character loop and one process per file. It also missed `period_names_table` because of this initializer trap. The real script must use one process and finish well under 100 ms.
- **What counts as a data pointer.** Each declarator that contains `*` and is not a function pointer (`(*name)`) counts as one pointer, whatever its number of stars or array dimensions.
  - Left out: declarations whose type names `allocator_t`, `p8_graphics_t` or `p8_vram_t`, `extern` declarations, prototypes and function definitions at file scope, and `typedef` statements outside a struct body.
  - The `typedef struct {...} name_t;` trailer that closes a struct is not a declarator.
  - A typedef that hides a pointer is not seen. Today `src/game/` and `entity_class.h` have none. The sorting records that this was checked.
- **What counts as a list of entity pointers.** A declarator, counted as for pointers, whose type is `entity_list_t`, held by value, by pointer or as an array.
  - The guard cannot read a `pointer_list_t`'s element type from the text. Every `pointer_list_add` call in `src/game/` was read for this PRD. None of the game's `pointer_list_t` fields or file-scope variables holds entities: `invented` holds names, `needs` and `inputs` hold need entries, and `particles` and `transit_dots` hold `particle_t`. So the guard counts `entity_list_t` only, and the sorting in section 7 records this reading.
  - Settled from the plan and the code, not by `guideline`: this sub-agent has no tool to spawn another agent. The plan's "or a `pointer_list_t` of entities" matches zero declarations today. A `pointer_list_t` of entities added later is a regression that warn-review checks.
- **Exception list format.** Pointer lines are `<file> <count>`, as the plan states. List lines use a separate three-field form, `entity-lists <file> <count>`, so that one file holds both counts and neither line can be mistaken for the other.
  - `#` starts a comment. The header comment explains how to read a line, as in `alloc_guard_exceptions.txt`.
  - A file with zero pointers and zero lists has no line.
  - Order: files with a pointer to remove come first, files whose pointers all stay come last, then the `entity-lists` lines.
- **Comparison and messages.** Strict equality per file and per count, reported as in `alloc_guard.sh`:
  - "not listed; line to add: …";
  - "N listed; line to write: …";
  - "no longer has any; line to remove: …";
  - "is not among the files given; line to remove: …".

  Messages are sorted with `LC_ALL=C`, then the summary line is printed.
- **Makefile.**
  - A `POINTER_GUARD_EXCEPTIONS` variable and a `pointer-guard` phony target. Its prerequisites are only the script, the list and the scanned sources, never objects, so `-j` starts it at once.
  - `pointer-guard` is added to the `test:` prerequisites.
  - `test_pointer_guard` joins the unit list, with a build rule like `test_alloc_guard`'s.
  - The legend at the top of the `Makefile` gets one entry. Only command-reading help is allowed there, per CLAUDE.md § Comments.
- **The sorting (explicit deliverable, written into section 7 of `docs/plan-13.md`).** Under a heading for step 1's sorting, there is one subsection per category:
  - **Allowed by section 2:** `entity_t.class`, and the `const` pointers to static data that is never freed (archetypes, records, tables, string literals). Each one is checked by reading where it points.
  - **Step 3:** links to an entity or to another object that can die, such as `down_t.held`, `targeting.target`, `slipway.start/end/targeted_by`, the `g_main_*` variables that point to entities, and every `entity_list_t`.
  - **Step 4:** the 12 header pointers of `entity_t` other than `class`.
  - **Step 5:** owned components, `const` aliases into memory that can be freed, and lists held by pointer.
  - **Unsorted:** anything else. One example is the `vec_t *` fields of `planet_keys_t`, a parameter bundle, unless reading the code puts them in a category above. Do not force a pointer into a category.

  Each entry names the file, the struct or variable, and the field, so every counted pointer appears exactly once. The total per file must equal the exception list. The section also records the `entity-lists` lines, the check that no pointer-hiding typedef exists, and the `pointer_list_t` reading above.
  - This PRD does not write step 1's graduation note in section 7; plan-pilot writes it.
  - A non-empty `Unsorted` list is not a failure of this PRD. It is the plan's stop signal for plan-pilot.
- **Nothing in `src/` changes.** Every `*.sha256` in `tests/e2e/` stays identical.

## Implementation Chunks

1. **Guard script, test first.** Write `tests/test_pointer_guard.c` and `tools/pointer_guard.sh` through TDD, one counting rule at a time, with a fixture directory under `tmp/`. Nothing is wired into `make test` yet, except the unit test's build rule and its entry in the unit list.
2. **Exception list and wiring.** Run the guard on the real files, write `tools/pointer_guard_exceptions.txt` from its output (with the ordering above), add the `pointer-guard` target to `make test`, and add the Makefile legend entry. Check that `make test-all` is green and that the wall time of `make test` has not gone up (compare `time make test` before and after, several runs each).
3. **Sorting.** Read every counted declaration and write the categories into section 7 of `docs/plan-13.md`. Check that the totals per file match the list.

## Testing Decisions

- A good test drives the script from the outside: it writes fixture source files and an exception list, runs the script, and checks the exit code and the printed lines. It never checks how the script parses.
- Module tested: `tools/pointer_guard.sh`, through a new flat test, `tests/test_pointer_guard.c`. The prior art is `tests/test_alloc_guard.c`:
  - the same `system()` harness, with the exit code written to a file;
  - a work directory under `tmp/`;
  - `read_file`/`write_file` helpers;
  - one function per behavior.

  Unlike the alloc guard test, the fixtures are text and nothing is compiled, so the test stays cheap.
- Cases:
  - **Pass and fail:** exact counts pass; a raised count fails and prints the line to write; a lowered count fails; a new unlisted file fails with the line to add; a listed file at zero fails with "line to remove"; a listed file missing from the arguments fails.
  - **Bad input:** no source file exits 2; an unreadable list exits 2.
  - **One test per pointer kind counted:**
    - plain field;
    - `const` field;
    - pointer to pointer;
    - array of pointers;
    - field in a nested struct, a union, and a struct defined inside a function;
    - file-scope `static` and non-`static` variable;
    - file-scope variable with an initializer (both `= NULL` and `= {...}`);
    - comma declarators;
    - pointer field in a header.
  - **Left out:** function-pointer field and typedef; prototype; function returning a pointer; local and parameter; `extern`; each runtime handle; `*` in a comment, a string and a `#define`; a non-pointer struct variable.
  - **List count:** `entity_list_t` by value, by pointer, as an array and at file scope, each failing on a raised `entity-lists` count; `entity_list_t *` counted in both counts.
  - **Summary:** the summary line is checked once.
- The real exception list is checked by `make test` itself, not by a unit test.
- The e2e replays are not touched: this step changes no game code.

## Out of Scope

- Removing or changing any pointer. Steps 3 to 6 do that.
- The handle table (step 2), `make sanitize-net` and `make full-game` (required from step 2 on).
- Counting pointers in parameters, kept addresses or locals. Steps 6 and warn-review cover those.
- Scanning `src/engine/` beyond `entity_class.h`, or `src/p8/`, `src/platform/`, `src/devtools/`.
- Detecting a `pointer_list_t` of entities by its element type.
- Writing step 1's graduation note in section 7. plan-pilot writes it after this PRD lands.
- Moving `entity_t` or `entity_class.h` out of `engine/`.

## Further Notes

- From this step on, no step raises a count. Steps 3 to 5 lower the counts, and steps 2 and 6 leave them unchanged.
- A rough prototype over today's tree found about 240 pointers in 38 files and 13 `entity_list_t` declarations. It undercounted (it missed initialized tables), so treat these numbers as an order of magnitude only. The real numbers come from the finished script.
  - `entity_class.h` alone should come to 18 pointers: the 13 of `entity_t`, `down_t.held`, the 3 data pointers of `planet_keys_t`, and `entity_kind_t.record`.
  - Expected heavy files include `game_data.h`, `map_tables.h`, `ui_button.h`, `planet.h`, `slipway.h` and `message_box.h`. Most of their pointers are probably `const` pointers to static tables.
- If the count in `entity_class.h` does not match the plan's "12 pointers besides `class`" in step 4, section 7 must say so; the next PRD checks its hypotheses against those lines.
