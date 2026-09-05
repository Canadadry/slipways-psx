# PRD-writing agent prompt

Fill in the placeholders and hand this as the entire prompt to a fresh, memoryless agent invoked with the `to-prd` skill. There is no prior conversation for it to draw on — this prompt's own content is what `to-prd`'s "synthesize what you already know" instruction should synthesize from.

```
Write a PRD for the next step of the plan, following the `to-prd` skill.

Plan file: <path to plan file>
Phase: <phase number/name>
Step: <step number> — <step text, copied verbatim from the plan>

This step's text IS the context to synthesize the PRD from — do not look
for a prior conversation and do not interview the user, per to-prd's own
instructions.

Read the plan file yourself for full context (objective, any guardrails/
decisions-log sections it has) rather than relying only on the excerpt
above.

No human is available to answer to-prd's "check with the user" points
(expected modules, which modules to test, whether to cut into chunks).
Wherever to-prd tells you to check with the user, spawn a fresh agent
running the `guideline` skill instead: ask it the exact question to-prd
would have asked a human, giving it this step's context, and use its
answer in place of a human's.

Scope: this PRD covers this one step, no more, no less. Do not add
content or scope not explicitly described by this step or by the plan's
own guardrails. If the step is too big for a single TDD cycle, use
to-prd's "Implementation Chunks" section to split it into sub-units of
THIS step — never fold other plan steps into this PRD.

Output: write the PRD to docs/prd/triage/phase<N>-etape<M>-<slug>.md
(to-prd's default triage location, with this filename convention instead
of a free-form slug) — nothing else.
```
