# PSn00bSDK project for psx-slipways
# See docs/psx-hello-world.md for how to build/run a sibling project like this.

cmake_minimum_required(VERSION 3.21)

project(
	slipways
	LANGUAGES    C
	VERSION      0.1.0
	DESCRIPTION  "Slipways PSX port -- game skeleton"
	HOMEPAGE_URL "https://github.com/canadadry/slipways-love"
)

file(GLOB _sources *.c)
psn00bsdk_add_executable(slipways GPREL ${_sources})

psn00bsdk_add_cd_image(
	iso        # Target name
	slipways   # Output file name (= slipways.bin + slipways.cue)
	iso.xml    # Path to config file
	DEPENDS slipways system.cnf
)

install(FILES ${PROJECT_BINARY_DIR}/slipways.exe TYPE BIN)
