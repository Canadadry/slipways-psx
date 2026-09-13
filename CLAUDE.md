# Project notes

- Never try to run the project (e.g. `love game`, `make run`). It's a GUI app; don't launch it. Use `make test` to verify changes instead.
- The porting target is PSX (`PSn00bSDK`), not GBA — see `docs/plan.md`. There is no headless test-loop tooling for it yet (Phase 0 of the plan); the former GBA devkit/`mcp-mgba` tooling has been removed as obsolete.
- `psx-hello/` is a PSn00bSDK smoke-test project (real toolchain build, not attempted in a sandboxed session — see `docs/psx-hello-world.md` for what was and wasn't verified and why). `make psn00bsdk-toolchain-setup` builds the real toolchain from source (slow, ~30-45 min, network-heavy — do not run it expecting it to finish quickly); `make psx-hello` builds the project once the toolchain is installed.
