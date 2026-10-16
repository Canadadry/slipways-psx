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

option(PSX_GAME_ASSERTS "Keep asserts in psx-game, the game on the CD image" OFF)
option(PSX_MEASURED_REPLAY_ASSERTS "Keep asserts in psx-gpu-replay and psx-gpu-replay-breakdown" ON)
set(PSX_GAME_OPT O3 CACHE STRING "Optimization level of psx-game: O2, O3 or Os")
set(PSX_MEASURED_REPLAY_OPT O3 CACHE STRING "Optimization level of psx-gpu-replay and psx-gpu-replay-breakdown: O2, O3 or Os")
foreach(_variable PSX_GAME_OPT PSX_MEASURED_REPLAY_OPT)
	set_property(CACHE ${_variable} PROPERTY STRINGS O2 O3 Os)
	if(NOT ${_variable} MATCHES "^(O2|O3|Os)$")
		message(FATAL_ERROR "${_variable} must be O2, O3 or Os, not '${${_variable}}'")
	endif()
endforeach()
option(PSX_GPU_REPLAY_DRAW_FILTER "Install the draw filter in psx-gpu-replay and psx-gpu-replay-breakdown (psx-check's masked captures)" OFF)
option(PSX_GPU_REPLAY_DRAW_METER "Install the draw meter in psx-gpu-replay and psx-gpu-replay-breakdown (psx-check's gpu-cost)" OFF)
option(PSX_PROBE "Build psx-gpu-replay as make psx-probe's probe build: doubled GPU frame buffer, delay helper" OFF)

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
	# The SDK's release config defines NDEBUG; -U comes after its -D and keeps
	# the asserts. Only psx-game (PSX_GAME_ASSERTS OFF) and the two measured
	# replays (PSX_MEASURED_REPLAY_ASSERTS OFF) drop it.
	if((NOT PSX_GAME_ASSERTS AND _target STREQUAL "psx-game")
		OR (NOT PSX_MEASURED_REPLAY_ASSERTS AND _target MATCHES "^psx-gpu-replay(-breakdown)?$"))
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
if(PSX_GPU_REPLAY_DRAW_FILTER)
	target_compile_definitions(psx-gpu-replay PRIVATE PSX_GPU_REPLAY_DRAW_FILTER)
	target_compile_definitions(psx-gpu-replay-breakdown PRIVATE PSX_GPU_REPLAY_DRAW_FILTER)
endif()
if(PSX_GPU_REPLAY_DRAW_METER)
	target_compile_definitions(psx-gpu-replay PRIVATE PSX_GPU_REPLAY_DRAW_METER)
	target_compile_definitions(psx-gpu-replay-breakdown PRIVATE PSX_GPU_REPLAY_DRAW_METER)
endif()
# A target's own options come before the SDK interface's trailing -O2 and
# lose to it; a linked interface library's come after. make psx-build and make
# psx-short-runs check the last -O with tools/psx_opt_level.sh.
function(psx_target_opt _level)
	if(_level STREQUAL "O2")
		return()
	endif()
	if(NOT TARGET psx-opt-${_level})
		add_library(psx-opt-${_level} INTERFACE)
		target_compile_options(psx-opt-${_level} INTERFACE -${_level})
	endif()
	foreach(_target ${ARGN})
		target_link_libraries(${_target} PRIVATE psx-opt-${_level})
	endforeach()
endfunction()
psx_target_opt(${PSX_GAME_OPT} psx-game)
psx_target_opt(${PSX_MEASURED_REPLAY_OPT} psx-gpu-replay psx-gpu-replay-breakdown)
if(PSX_PROBE)
	target_compile_definitions(psx-gpu-replay PRIVATE PSX_PROBE)
	target_sources(psx-gpu-replay PRIVATE ${SLIPWAYS_SRC_DIR}/platform/psx_probe.c ${SLIPWAYS_SRC_DIR}/platform/psx_fine_clock.c)
endif()

psn00bsdk_add_cd_image(psx-game-iso slipways iso.xml DEPENDS psx-game system.cnf)
