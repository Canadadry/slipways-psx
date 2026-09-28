---
name: plan-pilot
description: Drives unsupervised, front-to-back implementation of a phased plan document, one step at a time — writes a PRD per step via to-prd, runs it via prd-autopilot, routes bugs to help-me-debug, runs warn-review per phase, and consults plan-review at its own discretion. Use to autopilot a whole plan file end to end, not for one-off steps.
---

# Plan Pilot

Generic, project-agnostic. Takes one argument: the path to a plan file organized into phases, each with numbered steps (the shape of `docs/plan.md` in this repo, the first real target — but read the given plan's actual structure, don't hardcode this one).

No human responds during this run. Every checkpoint below that would normally ask a person is resolved by a `guideline`-skill agent instead, same convention `prd-autopilot` already uses for `tdd`.

## Per-step cycle

Repeat until there's no next step:

### 1. Find the next step
Look in `docs/prd/done/` (create it if missing) for the highest `phase<N>-etape<M>-*.md`. The next step is the one immediately after it in the plan's own phase/step order. Nothing graduated yet → the plan's first step. Last step of the last phase already graduated → the plan is complete, report and stop.

### 2. Write its PRD
Spawn a fresh agent running the `to-prd` skill. See [prd-prompt.md](prd-prompt.md) for the exact prompt template — it must hand over the step verbatim as the agent's context (satisfying `to-prd`'s "don't interview, synthesize what you know"), redirect every `to-prd` "check with the user" checkpoint to a fresh `guideline` agent, name the output `phase<N>-etape<M>-<slug>.md` in `docs/prd/triage/`, and constrain scope to this one step — chunk internally via `to-prd`'s own "Implementation Chunks" if the step is too big for one TDD cycle, never merge steps.

### 2b. Write the next PRD in parallel
While step N is being implemented or verified, spawn the PRD agent for step N+1 in the background — it only reads and greps, so it never conflicts with the implementer. Its prompt adds: step N is not done yet; every assumption about N's outcome (file shapes, counts, deviations) is an explicit hypothesis labelled `H-<N>-k`; list the sections to re-check once N's plan note exists; read nothing uncommitted. It writes to `docs/prd/next/`, not `docs/prd/triage/`, so step 3's "only this step's PRD in the queue" still holds. When N graduates, compare the `H-<N>-k` hypotheses with N's final plan note and commit: all hold → move the PRD to `docs/prd/triage/`; any fails → send the same agent back to update the PRD first. Never start N+1's implementation before N graduates.

### 3. Run it
Invoke `prd-autopilot` as-is — don't reimplement its cycle. Only this step's PRD (and its own internal chunks, if any) sits in `docs/prd/triage/`, so "run until the queue is empty" naturally stops after this one step. Have it move the graduated PRD to `docs/prd/done/`, keeping the `phase<N>-etape<M>-<slug>.md` filename — step 1 of the next cycle depends on that filename surviving.

### 4. Bugs
Any bug report from an implementer or verifier agent, at any point in steps 2-3: spawn a fresh `help-me-debug` agent on it first. It explains the bug, it doesn't fix it — once it reports back, resume the current step's TDD work informed by that diagnosis. No separate fix pathway.

### 5. End of phase
Right after the last step of a phase graduates: run `warn-review` on the code touched during that phase. Its warnings are bare, unfixed design-smell flags — feed them as context into a `plan-review` consultation (step 6), don't act on them directly and don't drop them.

### 6. Consult plan-review, at your discretion
After any step — potentially every step, but never mechanically forced — judge for yourself whether what just happened was risky enough, or taught something new enough, to be worth checking against the rest of the plan. When it is: spawn a fresh `plan-review` agent, read its proposals (or its no-improvement line), and judge each one yourself against "does this concretely improve the odds of success" — same bar `plan-converge` uses. Apply accepted ones to the plan file as a normal edit.

Don't reuse `plan-converge`'s loop-to-convergence or its temp-git-branch machinery here — this is a single ad hoc consultation mid-development, where real commits are already landing normally through steps 2-3.

## Boundaries

- Never touch a plan step's content/scope yourself — only `plan-review`'s proposals (filtered by you) can change the plan, and even those are method-only by `plan-review`'s own contract.
- Never wait on a human. Every blocking question from a sub-skill (`to-prd`, `tdd` via `prd-autopilot`) gets redirected to a fresh `guideline` agent instead.
- Stop and report to the user only when the plan is complete, or when `prd-autopilot`'s own stopping conditions fire (its `FAILED:` case) — don't invent extra pause points.
