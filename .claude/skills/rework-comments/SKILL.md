---
name: rework-comments
description: Brings a repository buried under work-log narration (phase numbers, references to specs, copied-in source code, histories of abandoned fixes) back to comments that carry only what the code does not say — a sequential phase 1 that extracts what could no longer be reconstructed, then a phase 2 with one agent per module under a four-destination rule. Use when the comment-to-code ratio is out of proportion, when you want to delete comments massively without losing what they held, when a port or agent-driven generation has left a logbook in the sources, when the user talks about slimming down, trimming, reworking or cleaning up a repository's comments, or when a few files written under the old regime land in a repository that has already been reworked (merge, long-lived branch, outside contribution) and need to be caught up.
---

# rework-comments

A repository written by successive waves of agents accumulates **work-log narration** in its comments: phase and batch numbers, references to PRDs, copied-in reference code, justifications for how modules were split, histories of abandoned fixes. It is read once, and paid for on every later read of the file.

This skill cuts that volume down to what has a reason to live in the code, **without changing any behavior**.

A worked example from start to finish (a C port of a PICO-8 cart, 18,544 → 2,966 comment lines, 47% → 12%): [worked-example.md](worked-example.md). The rest of this file is the method, to apply elsewhere.

## Two modes of use — pick one before you start

A repository is reworked only once; catching up comes back with every merge. The two uses do not follow the same path.

**Full campaign** — the repository has never been reworked, and narration is everywhere. Phase 1 then phase 2, in that order, with the ratio measured over the whole repository.

**One-off catch-up** — the repository has already been reworked, and a few new files have arrived written under the old regime: a merge, a long-lived branch, an outside contribution. This is the most frequent use once the campaign is over. In that case:

- **Skip phase 1.** The glossary and the gaps file already exist: they are read-only, and, as in a campaign, the agent does not write to them — it proposes its additions and corrections in its report.
- **The destination rule (four, or three if the repository no longer has a glossary) is already in the repository's instructions file** (`CLAUDE.md` or equivalent). The agent reads it there; the brief does not need to copy it, only point to it.
- **Measure the ratio per file**, never over the repository: on two files, the global figure does not move and tells you nothing.
- **A single agent** if the files fit in what one wave would give it. The "header and body together" rule still applies.

A half-reworked repository — a glossary but no gaps file, or the other way round — is still a catch-up. Do not create the missing half for symmetry; create it the day a real gap shows up.

## Prerequisite, non-negotiable

**A test suite that attests the behavior, and that is green before you start.** The method adds `assert`s; a wrong assert aborts the process. On the reference project, an agent added one that brought down the whole suite (see below). The safety net caught it, not the review.

Without a test suite: add no asserts at all, and destination 3 shrinks to its prose variant. Tell the user rather than improvising.

**This is the orchestrator's obligation, not a box the agent ticks**: the agent is not allowed to run the suite (the build directory is shared). Say so explicitly in the brief, otherwise the agent believes it is working without a net — or worse, believes it has one where there is none.

**And some files are out of the suite's reach.** On the reference project, two of them included a graphics library that was not vendored, in a repository that also forbids opening a window: no runnable target builds them. A wrong assert there would survive the whole suite and only blow up in a human's hands. **In a file that no runnable target reaches, add no assert, even an established one.** List these files before opening phase 2 and tell their agents.

Record the starting ratio; it is the progress measure for every wave:

```sh
# adapt to the language; here C/C++ comments vs lines of code
cloc --by-file src/ | tail -5
```

**Give this command to the agents; it is the definition.** "Comment line" has at least three meanings (a line that is entirely comment, a line containing one, `cloc`'s own definition): without a canonical command, each agent writes its own counter and the figures are comparable neither with each other nor with yours. It is the only measure the method presents as objective; letting it depend on each agent's tooling empties it of meaning.

## The rule: four destinations

Every comment you meet goes to **one** of these four destinations, or it goes away.

1. **The meaning of a name** → a glossary file, one line per symbol. The code does not repeat it.
2. **A known gap with the reference**, or a piece deliberately left undone → a numbered entry in a gaps file. The code refers to it only through a **bare marker**, `/* GAP-7 */`, alone on its line above the declaration concerned — no summary next to it; it would go stale.
3. **A trap a future reader would reintroduce** → `assert(cond && "short message")` at the exact site if it can be checked at runtime, **otherwise 40 words at most**.
4. **A contract that neither the name nor the glossary carries** — who owns the memory, who frees it, what the function returns when the field is missing, what a NULL triggers → one sentence in the header. One exception, named so that it does not become a loophole: **a file format or a protocol that something outside the repository must be able to read** is a legitimate long contract — it has nowhere else to live. Everything else fits in one sentence, or moves to a `docs/format-*.md`.

Everything else goes into the **commit message**. Git keeps it, and nobody pays for it in tokens on every read of the file. `TODO` is still allowed: it admits that a case is not handled.

## Phase 1 — full campaign only, sequential, while the prose is still there

The orchestrator does it alone, before any agent. It extracts what could no longer be reconstructed once the comments are trimmed.

1. **The glossary.** Useful when identifiers do not carry their meaning: transliterated minified code, symbols inherited from a binary format, domain abbreviations. Build it by reading every header, one line per symbol, and check mechanically that every symbol cited still exists. *Transposing*: if the names already speak for themselves, this file has no reason to exist — do not create it for symmetry. It can become a glossary of domain vocabulary, or nothing.
2. **The numbered gaps.** One entry per divergence from the reference (another implementation, a spec, a replaced system) or per acknowledged omission. **Re-check every entry against the code** rather than copying the comment: that is where you find stale claims, and sometimes a gap that has become reachable without anyone noting it.
3. **Delete the work logs** that git now makes redundant, and patch every dangling reference.
4. **Rewire the process that produced the prose** — otherwise it grows back. Concretely: write the four-destination rule into the repository's instructions file (`CLAUDE.md` or equivalent), and **fix the skills that wrote narration**. On the reference project, a bug-fixing skill appended a log paragraph on every run; that step was removed, and the finding now goes into the commit message.

Commit phase 1 before opening phase 2.

## Narration does not live only in comments

A string meant for a human — banner, help message, status line — is documentation that executes, and it goes stale exactly like a comment. On the reference project, a rejected playback mode had disappeared from the comments **and** the code, but survived in the banner the program prints on every launch: the only sentence the human reads before looking at what they are about to sign off on promised a behavior that no longer exists.

Hence the rule, to put in every brief: **a string meant for a human is checked like a comment, and a divergence is reported — never fixed in this pass.** It is code: the diff would stop being purely subtractive, and the fix deserves its own `fix:` commit.

## Phase 2 — one agent per module

In a one-off catch-up, this is where you start.

One agent, one module, and **the header and the body together** (`.h` + `.c`, interface + implementation): the trap is often described in the header while the assert that replaces it must live in the body. An agent that sees only one of the two files loses half of the transformation.

The brief to copy: [agent-brief.md](agent-brief.md). The report to require: [report-template.md](report-template.md).

Group modules into coherent waves (4 to 6 agents), one wave at a time.

## The central rule: an assert must be verifiable, not just plausible

Agents cannot run the build — they share the build directory — so they **reason instead of measuring**, and serious reasoning sometimes encodes an assumption rather than an invariant.

What happened: an agent added an assert requiring a data spec to end with a comma. The rule was true, and it was the costliest trap in the repository. The agent had validated it seriously — reimplementing the parser's state machine offline and running it over all 58 known specs. The first `make test` said otherwise: measured, the specs decoded from memory end with **two** commas, and the index does not land in the same place. The condition was off by one. Assert removed, rule put back into prose.

Hence the instruction, to put in bold in every agent's prompt:

> **If it cannot establish that an assert holds other than by reasoning, it writes 40 words.** One assert too many costs more than one comment too many.

Establishing means: exhaustively enumerating the call sites, measuring a length or some content outside the target language, or proving the invariant **locally, with no assumption about the callers**. Not: "I don't see how that could happen."

Later waves refused five asserts for this reason, and one of those refusals produced the best find of the project: the agent computed the margin before writing (66 tickets in a pool for 90 possible draws), the computation said no — and that "no" was an out-of-bounds read bug nobody had seen. **Explicitly ask for the list of asserts considered and then rejected**: it is worth as much as the ones that get added.

## The other rules, all tested in the field

- **Verify every status claim before believing it.** "Not ported yet", "never exercised", "no reader", "dead code": about fifty turned out to be false. They were not lies when written, they aged — and that is what makes them dangerous; several steered attention away from the one place that needed examining. A status claim gets verified (grep, caller count) or deleted.
- **A test file has its own rule.** Its comment carries *why this value*, which no code comment carries: an assertion on a pixel, a range or a frame index cannot be recovered without its reason. The retention rate is legitimately higher there. **You do not modify or delete an existing assertion**: neither its value nor its presence. Changing what a test proves in a cleanup commit means slipping it through without review. **Adding** one is allowed if it is proven like any other assert: it is listed in the report, and the orchestrator makes a `test:` commit of it alongside the `docs:` commit.
- **Agents write neither to the glossary nor to the gaps file.** Several would share them. They *report* missing entries and errors in their report, and the orchestrator applies them in batches. On the reference project: about forty entries added this way, and **four errors by the orchestrator itself** found by its agents.
- **The "what I hesitated to delete" section is the most useful part of the report.** It is what surfaces the real gaps, the glossary errors and the bugs declared fixed that are only half fixed. Ask for it explicitly, with the instruction **neither to fill it out of politeness nor to leave it out**.

## The orchestrator's role

0. **The mechanical check works everywhere, including where compilation fails.** `gcc -fpreprocessed -dD -E -P` does not resolve `#include`s: it only strips comments. A file that does not compile here — non-vendored dependency, missing header — can therefore still be checked, byte for byte. Never tell an agent to "review the diff by hand" for those files: you would take away its only proof. A second, free safety net: the number of lines of code must be identical before and after.
1. **Compile each module in isolation** as soon as its agent hands back (`make src/obj/module.o` or the equivalent). That catches the missing `#include <assert.h>` and nested comments.
2. **Run the full suite only on a stabilized tree** — every agent of the wave has handed back. A build on a half-rewritten tree tells you nothing, and this is precisely what made it possible to attribute the assert failure correctly.
3. **Commit module by module**, with the narration in the message: the before/after ratio, the asserts added and how each was proven, the stale claims taken apart, the glossary corrections.
4. **Keep commits that add `abort()`s** (an assert is one) separate from those that only delete text. A purely subtractive commit must read as such, and be revertable without taking anything else with it.
5. **Recommend line-range replacement**, which refuses if the range contains anything other than comment, over exact-string replacement copied from a read. The former cannot touch code and fails loudly; the latter gets a newline wrong and pastes approximate text.
6. **Record the ratio at every wave.** It is the useful progress measure, and the only one that does not depend on judgment.

## What the method does not guarantee

It produced one failure: a wrong assert that aborted the whole test suite. It was written by a conscientious agent who had validated its rule offline before adding it. The review did not catch it; the first `make test` of the next wave did.

The method is therefore safe **exactly to the extent that the test suite is**. If the suite does not cover a path, a wrong assert on that path will survive until a user hits it. That is the argument for the prerequisite at the top of this file, and for the verifiable-assert rule.
