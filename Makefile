.PHONY: psn00bsdk-toolchain-setup psx-hello psx-hello-clean

# No `build`/`run`/`test` targets yet: the Love2D prototype (`game/`) was
# removed in favor of the PC/raylib literal port described in docs/plan.md.
# Phase 1 of that plan adds a C build + test setup here.

psn00bsdk-toolchain-setup:
	sh tools/install_psn00bsdk_toolchain.sh

psx-hello:
	cmake -S psx-hello -B psx-hello/build && cmake --build psx-hello/build

psx-hello-clean:
	rm -rf psx-hello/build
