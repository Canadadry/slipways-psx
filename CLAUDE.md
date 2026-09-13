# Project notes

- Never try to run the project (e.g. `love game`, `make run`). It's a GUI app; don't launch it. Use `make test` to verify changes instead.
- The porting target is PSX (`PSn00bSDK`), not GBA — see `docs/plan.md`. There is no headless test-loop tooling for it yet (Phase 0 of the plan); the former GBA devkit/`mcp-mgba` tooling has been removed as obsolete.
- `psx-hello/` is a PSn00bSDK smoke-test project. `make psn00bsdk-toolchain-setup` builds the real toolchain from source (slow, ~30-45 min, network-heavy — do not run it expecting it to finish quickly) and has been run successfully; `make psx-hello` then builds the project and has also produced a real, valid `psx-hello.exe`. Booting it in an emulator is still unverified — no PCSX-Redux/DuckStation/no$psx was reachable. See `docs/psx-hello-world.md` for what's verified, what isn't, and why an earlier session wrongly assumed the toolchain build itself was blocked.
