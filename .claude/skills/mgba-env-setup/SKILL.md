---
name: mgba-env-setup
description: Configure a headless Linux environment to run the mGBA Game Boy Advance emulator with the mcp-mgba MCP server, when there is no X server / display available. Use this whenever the user wants to install, set up, or troubleshoot mGBA + mcp-mgba on a headless server or any machine without a graphical display.
---

# mGBA + mcp-mgba Headless Environment Setup

Sets up mGBA (GBA emulator) and the mcp-mgba MCP server on a Linux machine with no X server, using a virtual framebuffer (Xvfb).

## 1. Install dependencies

```bash
sudo apt update
sudo apt install -y xvfb mgba-qt nodejs npm git
```

If `mgba-qt` isn't in the distro's repos, build from source (github.com/mgba-emu/mgba) or use an AppImage instead.

## 2. Install mcp-mgba

```bash
git clone https://github.com/dmang-dev/mcp-mgba.git
cd mcp-mgba
npm install
npm run build
```

Optional global install: `npm install -g mcp-mgba`.

## 3. Launch mGBA under a virtual display

```bash
xvfb-run -a --server-args="-screen 0 1024x768x24" mgba-qt -l /path/to/mcp-mgba/lua/bridge.lua /path/to/rom.gba
```

- `xvfb-run -a` picks a free virtual display automatically (no real X server needed).
- `-l <script>` loads the Lua bridge script at startup — check `mgba-qt --help` for the exact flag name on the installed version, since it varies by build. If the flag isn't available, the script must be loaded via **File > Load script** in the Qt UI, which still works over Xvfb but requires an interactive session (e.g. via VNC) instead of a pure batch launch.
- Confirm it worked by checking for `[mcp-mgba] bridge listening on 127.0.0.1:8765` in mGBA's script console/log output.

## 4. Start the MCP server

```bash
node /path/to/mcp-mgba/dist/index.js
```

Or, if installed globally: `mcp-mgba`.

Override the bridge address if needed: `MGBA_HOST` / `MGBA_PORT` env vars (default `127.0.0.1:8765`).

## 5. Register with the MCP client (e.g. Claude Desktop/Code)

```json
{
  "mcpServers": {
    "mgba": {
      "command": "node",
      "args": ["/absolute/path/to/mcp-mgba/dist/index.js"]
    }
  }
}
```

Restart the client after editing its config.

## Troubleshooting

- **"bind failed" on script load** — a previous mGBA instance still holds the port; kill it and relaunch.
- **mGBA won't start at all** — verify Xvfb is actually running (`ps aux | grep Xvfb`) and that `mgba-qt` was launched through `xvfb-run`, not bare.
- **Bridge never reports "listening"** — the Lua script may have failed to load silently; check mGBA's scripting console for errors, or confirm the `-l` flag is supported on the installed mGBA version.
