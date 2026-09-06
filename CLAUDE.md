# Project notes

- Never try to run the project (e.g. `love game`, `make run`). It's a GUI app; don't launch it. Use `make test` to verify changes instead.
- `make install` sets up the headless `mgba` MCP server (canadadry/mcp-mgba, a submodule under `tools/mcp-mgba`) used for the GBA-port testing loop (see `docs/plan.md`): initializes submodules, builds the native shim, and builds the npm package. Run once after cloning. `make mcp-mgba-test` runs its test suite.
