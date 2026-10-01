---
name: fix-bug
description: End-to-end pipeline for one reported bug — a fresh help-me-debug agent proves the cause, a fresh tdd agent writes the failing test then the minimal fix, a third fresh agent verifies it independently, and git-commit lands it with the finding in its message. Use when a bug is reported (playtest feedback, crash, wrong rendering) and the user wants it fixed, not merely diagnosed.
---

# fix-bug

Drives one reported bug to a committed fix through four fresh, memoryless agents. Each stage hands the next one **evidence**, never an opinion: the debug agent hands over a proof, the implementer hands over a failing-then-passing test, the verifier hands over its own independent reproduction.

**Argument**: the bug report — free text, or a path to a file holding it. If none was given, ask the user for the symptom and how they hit it. That question and the commit gate are the only two moments this skill talks to a human.

One invocation = one bug. It is not a loop over a backlog; to chain bugs, invoke it again.

## Rule #1 — never wait on the human, except at the commit gate

Any `QUESTION:` from a sub-agent is answered by spawning a fresh `guideline` agent and relaying its answer back — never by pausing for the user. Same convention `prd-autopilot` already uses.

This bites hardest on stage 1: `help-me-debug`'s own "involve the user, they own the environment where the bug lives" checkpoints are redirected to a `guideline` agent here. **Say so in the debug agent's prompt**, so it designs tests it can run itself rather than tests that need the reporter's screen.

Be honest about what this costs: the bug was very likely observed on the reporter's machine (real window, real macOS), while this pipeline proves causes in a headless sandbox. A cause proven here is proven *about the code*, not about their environment. When the debug agent can only reach an environment-dependent hypothesis, that is an `UNPROVEN:` (see stage 1's gate) — not a green light.

## Rule #2 — no fix before a proven cause

Stage 2 never starts on a plausible cause. The gate after stage 1 is the whole point of this pipeline; waved through, this is just "guess a fix" with extra agents.

## Constraints every sub-agent inherits

Put these in each sub-agent prompt — a fresh agent knows none of them:

- **Never launch the raylib app interactively** (`CLAUDE.md`). Reproduction is headless: fixed frame count, scripted `events.json`, framebuffer dumped as text.
- **The Bash hooks reject absolute paths and `git -C`**. Use project-relative paths, never `cd`. They also reject backticks in a command line, heredocs included — write files with the Write tool, not a heredoc.
- **Only the orchestrator touches git.** Sub-agents never commit, stage, stash, branch or push.
- **The suite is `make test`** — prefer the Makefile target over invoking a runner directly.
- **This is a literal port.** A bug in a ported function is judged against `pico8/lua.lua`, the real cart source: the fix restores fidelity to those exact lines, it never "improves" them. A C-side bug (ownership, lifetime, NULL guard) is judged against the port's own established conventions, cited from a real site in the tree.
- Throwaway repro programs go in `tmp/`, never in `src/` or `tests/`.

## Pipeline

### 1. Prove the cause — fresh `help-me-debug` agent

Spawn a fresh, memoryless agent running the `help-me-debug` skill. Hand it the bug report **verbatim** — never your paraphrase, and never your own early theory. A summary is exactly where an unproven hypothesis quietly becomes a premise.

Its mandate: understand and prove, **change nothing** under `src/` or `tests/`. It may build, run, instrument throwaway copies in `tmp/`, and read anything.

Reporting contract:
- `PROVEN:` followed by the bug document from `help-me-debug`'s own template (Summary / Reproduction / Established cause / Scope / Inputs for prioritization / Unverified leads).
- `UNPROVEN:` followed by what was eliminated, what remains open, and exactly which observation is missing.
- `QUESTION:` for anything blocking, resolved via a `guideline` agent (Rule #1).

**Gate — read the proof, don't skim it.** Every claim about the cause must carry a file excerpt with path and line numbers, the output of a command actually run, or sourced documentation with proof of applicability. "Consistent with", "would explain" and "most likely" are not proofs.

- Proof holds → stage 2.
- `UNPROVEN:`, or a "proof" that is really a chain of reasoning → spawn **one** more debug agent, handed everything already eliminated so it doesn't retrace it. Still unproven after that → stop and report to the user what is known and which observation is missing. Stopping here is a legitimate outcome of this skill, not a failure of it.

### 2. Fix it — fresh `tdd` agent

Spawn a fresh, memoryless agent running the `tdd` skill. Hand it the stage-1 bug document **verbatim** as its context, plus the constraints above.

Its mandate, in this order:

1. **RED first**: a test that fails *because of this bug*, exercising the real path through the public interface — never one that asserts the shape of the fix. For a path only a human reaches (a real click dispatch, a rendered frame), reproduce it headless through the same entry point the human hits, never by calling the target function directly. The last three bugs of this port all hid precisely in the gap between "a test calls it" and "a human reaches it".
2. **GREEN**: the minimal fix the proven cause justifies. Nothing adjacent, no cleanup, no refactor of neighbouring code.
3. `make test` fully green.

It updates `README.md` only if user-visible behaviour changed. It never touches git — stage 4 owns that.

Reporting contract: `DONE:` plus what the test asserts, which files and lines changed, and why that is the minimal fix — or `QUESTION:` (Rule #1). `DONE:` means implemented, **not** verified.

### 3. Verify independently — fresh agent, no skill

Spawn a third fresh, memoryless agent. It is not handed the implementer's reasoning as truth; it re-establishes the facts itself:

1. **Prove the new test actually detects the bug**: rebuild a throwaway copy of the pre-fix code under `tmp/` (or revert the fix in a scratch copy) and show the new test failing against it, then passing against the tree. A test that passes both ways proves nothing — this repo already caught two such tests exactly this way.
2. Run `make test` in full.
3. Run the sanitizer build if the bug was a memory bug (use-after-free, NULL deref, overflow): 0 ASan/UBSan errors.
4. Read the diff: it must touch only what the proven cause justifies, and nothing else.

Reporting contract: `VERIFIED:` plus the evidence for each point, or `FAILED:` plus what broke.

On `FAILED:`, route by kind. A mechanical failure (test doesn't compile, wrong path, suite breakage) goes back to stage 2 with the detail. A fix that doesn't actually remove the symptom means the cause wasn't the cause: back to **stage 1**, not stage 2, handed the verifier's evidence. After one such round trip, stop and report to the user rather than spinning.

### 4. Commit the fix

Invoke the `git-commit` skill **yourself, in place** — don't delegate it to a sub-agent, since its proposal step waits for the user's go-ahead and a sub-agent can't collect it. This is the one stop in the pipeline.

The commit message is where the finding is recorded — no journal file, no prose in the code. Write it in French, strictly factual, from the stage-1 bug document and the stage-3 evidence:

- the symptom as reported, and what surfaced it (human playtest, scripted replay, sanitizer);
- **the hypothesis that was disproved**, when there was one — as valuable as the cause itself, since it's what spares the next reader the same detour;
- the proven cause, with file/function references;
- the fix retained, and what was deliberately *not* changed.

Write it yourself rather than delegating: you hold both the proof and the verification, a fresh agent would hold neither.

What the commit message must NOT absorb, because it belongs in the working tree instead:

- a trap a future reader could reintroduce → an `assert` carrying a message at the exact spot, or, when the invariant isn't checkable at runtime, a comment of at most 40 words there;
- a divergence from the cart that stays open → a numbered entry in `docs/trous-de-portage.md`, referenced from the code by a bare `/* TROU-n */`;
- the meaning of a minified name you had to work out → a line in `docs/glossaire-noms.md`.

Scope the commit to the code fix and its test (`fix:`). A glossary or gaps-file edit is a separate intention: commit it on its own (`docs:`).

## Stopping conditions

Stop and report to the user when the cause can't be proven after two debug agents, when a `FAILED:` verification survives one round trip, or when the user declines a commit proposal. Never widen the fix, never skip or weaken a test to get green, and never commit on an unproven cause.
