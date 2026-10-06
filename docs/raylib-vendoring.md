# raylib: vendoring and build

raylib isn't available as a system package in this project's environment, so it
is cloned and built from source, once, outside `make test`.

## Setup

```sh
make raylib-vendor-setup      # clone + build, slow and needs network, once
make raylib                   # compile and LINK the interactive binary
```

`raylib-vendor-setup` clones a pinned release tag (never a moving branch) into
`third_party/raylib/` (gitignored) and builds `libraylib.a` with raylib's own
build system (`third_party/raylib/src/Makefile`, `PLATFORM_DESKTOP`).

`make raylib` never runs the binary it produces — see `CLAUDE.md`. It requires
`third_party/raylib/src/libraylib.a` to exist already: without vendoring, it
fails with a plain "no rule to make target", just like `psx-hello` without its
toolchain.

## Per OS

The vendoring script and the Makefile's link flags check `uname -s`.

**Linux** — `apt-get` installs the X11/GL headers, and the link adds
`-lGL -lX11 -lpthread -ldl -lrt`.

`RAYLIB_VENDOR_SKIP_APT=1 make raylib-vendor-setup` skips the `sudo apt-get`
step, for anyone who already has the dependencies or can't grant sudo.

**macOS** — nothing to install; the Xcode Command Line Tools are enough. The
link uses `-framework OpenGL -framework Cocoa -framework IOKit
-framework CoreAudio -framework CoreVideo`, the set raylib's `src/Makefile`
documents for linking an external PLATFORM_DESKTOP program on OSX.

## Files involved

`src/cmd/game.c`, `src/platform/raylib_backend.c`.
