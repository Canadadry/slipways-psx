# PSn00bSDK project for psx-hello
# See docs/psx-hello-world.md for how to build/run this.

cmake_minimum_required(VERSION 3.21)

project(
	psx-hello
	LANGUAGES    C
	VERSION      0.1.0
	DESCRIPTION  "Slipways PSX port -- hello world / controller smoke test"
	HOMEPAGE_URL "https://github.com/canadadry/slipways-love"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(psx-hello GPREL ${_sources})
psn00bsdk_add_cd_image(psx-hello-iso psx-hello iso.xml DEPENDS psx-hello system.cnf)

install(FILES ${PROJECT_BINARY_DIR}/psx-hello.exe TYPE BIN)
