# PSn00bSDK project for the game's PSX build: `make psx-build` hands it the
# source lists (absolute paths) and the src/ directory; nothing is globbed so
# that a file added to the core cannot be missing here.

cmake_minimum_required(VERSION 3.21)

project(
	slipways-psx
	LANGUAGES    C
	VERSION      0.1.0
	DESCRIPTION  "Slipways PSX port -- the game and its CD image, core replays, pad probe and largest-slots run"
	HOMEPAGE_URL "https://github.com/canadadry/slipways-love"
)

foreach(_required SLIPWAYS_SRC_DIR PSX_REPLAY_SOURCES PSX_GPU_REPLAY_SOURCES PSX_GPU_REPLAY_BREAKDOWN_SOURCES PSX_PAD_PROBE_SOURCES PSX_GAME_SOURCES PSX_LARGEST_SLOTS_SOURCES)
	if(NOT DEFINED ${_required})
		message(FATAL_ERROR "${_required} is not set: build through make psx-build")
	endif()
endforeach()

option(PSX_MEASURED_REPLAY_ASSERTS "Keep asserts in psx-gpu-replay and psx-gpu-replay-breakdown" ON)

foreach(_target psx-replay psx-gpu-replay psx-gpu-replay-breakdown psx-pad-probe psx-game psx-largest-slots)
	if(_target STREQUAL "psx-replay")
		set(_sources ${PSX_REPLAY_SOURCES})
	elseif(_target STREQUAL "psx-gpu-replay")
		set(_sources ${PSX_GPU_REPLAY_SOURCES})
	elseif(_target STREQUAL "psx-gpu-replay-breakdown")
		set(_sources ${PSX_GPU_REPLAY_BREAKDOWN_SOURCES})
	elseif(_target STREQUAL "psx-game")
		set(_sources ${PSX_GAME_SOURCES})
	elseif(_target STREQUAL "psx-largest-slots")
		set(_sources ${PSX_LARGEST_SLOTS_SOURCES})
	else()
		set(_sources ${PSX_PAD_PROBE_SOURCES})
	endif()
	psn00bsdk_add_executable(${_target} GPREL ${_sources})
	target_include_directories(${_target} PRIVATE ${SLIPWAYS_SRC_DIR})
	# The SDK's release config defines NDEBUG; -U comes after its -D, so the
	# core's asserts stay compiled in, as in the PC -O0 binary.
	if(NOT PSX_MEASURED_REPLAY_ASSERTS AND _target MATCHES "^psx-gpu-replay(-breakdown)?$")
		# Helpers and locals that only feed an assert go unused under NDEBUG;
		# the sources must not change, so these kinds stay warnings only.
		target_compile_options(${_target} PRIVATE -std=c99 -Wall -Wextra -Werror
			-Wno-error=unused-variable -Wno-error=unused-but-set-variable
			-Wno-error=unused-function -Wno-error=unused-parameter)
	else()
		target_compile_options(${_target} PRIVATE -std=c99 -Wall -Wextra -Werror -UNDEBUG)
	endif()
	target_link_options(${_target} PRIVATE -Wl,--wrap=_assert_abort -Wl,--wrap=snprintf -Wl,--wrap=vsnprintf)
endforeach()
target_compile_definitions(psx-gpu-replay-breakdown PRIVATE PSX_GPU_REPLAY_BREAKDOWN)

psn00bsdk_add_cd_image(psx-game-iso slipways iso.xml DEPENDS psx-game system.cnf)
