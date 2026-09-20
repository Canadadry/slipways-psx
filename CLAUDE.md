# Project notes

- The active porting target is **PC via raylib, software rendering** — see `docs/plan.md`. Strategy: literal function-by-function port of `pico8/lua.lua` (the actual decompiled cart source), not a rewrite from the design doc. PSX (previous target) is paused, not abandoned — resumed only once a fully playable PC/C version exists.
- Never launch the raylib app interactively (no live window) at any phase before Phase 5 of `docs/plan.md`. Non-interactive checks are fine: running it for a fixed number of frames driven by a scripted `events.json` and dumping the framebuffer as text is the intended verification method — see `docs/plan.md`'s testing strategy.
- Audio (`sfx`/`music`) is explicitly out of scope for this port, not deferred.
- The Love2D prototype (`game/`) has been removed — it embodied the abandoned "rewrite from the design doc" approach.
- `psx-hello/` is a PSn00bSDK smoke-test project (real toolchain build, not attempted in a sandboxed session — see `docs/psx-hello-world.md` for what was and wasn't verified and why). `make psn00bsdk-toolchain-setup` builds the real toolchain from source (slow, ~30-45 min, network-heavy — do not run it expecting it to finish quickly); `make psx-hello` builds the project once the toolchain is installed.
