#!/bin/sh
# usage: psx_full_game_check.sh <install-dir> <replay-exe> <gpu-exe> <pc-state> <pc-digest> <pc-frames>
#                               <replay-ceiling> <replay-timeout> <gpu-ceiling> <gpu-timeout> <budget-hblanks>
#                               <situations-table> <one-off-allowance-vsyncs> <size-tool> <stack-reserve> <out-dir>
# Boots the replay PS-EXE, then the GPU replay PS-EXE with an empty capture
# script (the capture handshake runs, nothing is captured), until they exit,
# then checks the conditions of make psx-full-game-check through
# tools/psx_conditions.sh: replay-done, state-game-lines, sprite-sheet,
# screen, map, gpu-replay-done, gpu-state-game-lines, gpu-draw-filter and
# gpu-draw-meter (the GPU boot printed "psx-replay: draw-filter off" and
# "psx-replay: draw-meter off"), then timer, frame-budget (no frame above
# <budget-hblanks>), zone-sizes and cap-sizes of tools/psx_frame_budget.sh on
# the GPU boot (outputs in <out-dir>/frame-budget), real-game-budget of
# tools/psx_real_game_budget.sh (its frames.txt joined with <situations-table>:
# no steady frame over <budget-hblanks>, one-off vsyncs over it at most
# <one-off-allowance-vsyncs>) and gpu-ram-room (64 KiB
# kernel area + GPU ELF + stack reserve <= 2 MiB). The unmetered GPU boot's
# gpu_* lines are zeros: zone-sizes and cap-sizes judge
# <out-dir>/gpu-state/judged_state.txt, its MIPS state with the gpu_* lines of
# <pc-state> instead. Every condition is checked before concluding, one line
# per condition, exit 1 if any failed; each also goes to
# <out-dir>/conditions.txt as "<condition> ok|failed". Prints every frame line
# behind "psx-full-game-check: frame ", and the frame summary and the two
# real-game figure lines behind "psx-full-game-check: ".
# Writes <out-dir>/measurements.txt: the MIPS zone/budget/cap lines, the vsync
# count and the size of the replay ELF (the .exe path with .elf), then the GPU
# replay's vsyncs, <pc-state>'s gpu_* lines, the backend's gpu-peak line, RAM
# room and ELF size, each behind "gpu ", the frame summary behind
# "gpu frame " and the real-game figure lines behind "gpu ".

set -u

usage="usage: psx_full_game_check.sh <install-dir> <replay-exe> <gpu-exe> <pc-state> <pc-digest> <pc-frames> <replay-ceiling> <replay-timeout> <gpu-ceiling> <gpu-timeout> <budget-hblanks> <situations-table> <one-off-allowance-vsyncs> <size-tool> <stack-reserve> <out-dir>"
[ $# -eq 16 ] || { echo "psx-full-game-check: $usage" >&2; exit 1; }
install_dir=$1
replay_exe=$2
gpu_exe=$3
pc_state=$4
pc_digest=$5
pc_frames=$6
replay_ceiling=$7
replay_timeout=$8
gpu_ceiling=$9
shift 9
gpu_timeout=$1
budget_hblanks=$2
situations=$3
one_off_allowance=$4
size_tool=$5
stack_reserve=$6
out_dir=$7
script_dir=$(cd "$(dirname "$0")" && pwd)

rm -rf "$out_dir"
mkdir -p "$out_dir"

echo "psx-full-game-check: booting the replay PS-EXE (at most ${replay_timeout}s)"
replay_boot=$(PSX_BOOT_TIMEOUT=$replay_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --exe "$replay_exe" "$replay_ceiling" "$out_dir/replay" 2>&1)
replay_status=$?
capture_script=$out_dir/empty_capture_script.txt
: > "$capture_script"
echo "psx-full-game-check: booting the GPU replay PS-EXE (at most ${gpu_timeout}s)"
gpu_boot=$(PSX_BOOT_TIMEOUT=$gpu_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --capture-script "$capture_script" --exe "$gpu_exe" "$gpu_ceiling" "$out_dir/gpu" 2>&1)
gpu_status=$?

failed=0
check_name=psx-full-game-check
. "$script_dir/psx_conditions.sh"

tty=$out_dir/replay.tty.txt
[ -f "$tty" ] || : > "$tty"
compare=$(sh "$script_dir/psx_state_compare.sh" "$tty" "$pc_state" "$out_dir" "$pc_frames" 2>&1)
"$size_tool" "${replay_exe%.exe}.elf" >> "$out_dir/measurements.txt" 2>&1
check_replay_done replay-done "$replay_status" "$replay_boot" "$tty" "$pc_frames"
check_game_lines state-game-lines "$compare"
check_digests "$tty" "$pc_digest" "$out_dir"

gpu_tty=$out_dir/gpu.tty.txt
[ -f "$gpu_tty" ] || : > "$gpu_tty"
gpu_compare=$(sh "$script_dir/psx_state_compare.sh" "$gpu_tty" "$pc_state" "$out_dir/gpu-state" "$pc_frames" 2>&1)
check_replay_done gpu-replay-done "$gpu_status" "$gpu_boot" "$gpu_tty" "$pc_frames"
check_game_lines gpu-state-game-lines "$gpu_compare"
filter_line=$(tr -d '\r' < "$gpu_tty" | grep '^psx-replay: draw-filter ' | tail -n 1)
if [ "$filter_line" = "psx-replay: draw-filter off" ]; then
    pass gpu-draw-filter
else
    report gpu-draw-filter "expected psx-replay: draw-filter off, got: ${filter_line:-(no line)}"
fi
meter_line=$(tr -d '\r' < "$gpu_tty" | grep '^psx-replay: draw-meter ' | tail -n 1)
if [ "$meter_line" = "psx-replay: draw-meter off" ]; then
    pass gpu-draw-meter
else
    report gpu-draw-meter "expected psx-replay: draw-meter off, got: ${meter_line:-(no line)}"
fi

pc_gpu_lines='^(zone gpu_frame|zone gpu_build|cap gpu_[a-z_]*) '
judged_state=$out_dir/gpu-state/judged_state.txt
mkdir -p "$out_dir/gpu-state"
{
    grep -Ev "$pc_gpu_lines" "$out_dir/gpu-state/psx_state.txt" 2>/dev/null
    grep -E "$pc_gpu_lines" "$pc_state"
} > "$judged_state"

frame_dir=$out_dir/frame-budget
frame_budget=$(sh "$script_dir/psx_frame_budget.sh" "$gpu_tty" "$judged_state" "$pc_frames" \
    "$budget_hblanks" "$frame_dir" 2>&1)
if [ -f "$frame_dir/frames.txt" ]; then
    sed -n '1s/^/psx-full-game-check: /p; 1!s/^/psx-full-game-check: frame /p' "$frame_dir/frames.txt"
fi
[ -f "$frame_dir/summary.txt" ] && sed 's/^/psx-full-game-check: /' "$frame_dir/summary.txt"
for condition in timer frame-budget zone-sizes cap-sizes; do
    verdict=$(grep "^$condition " "$frame_dir/verdicts.txt" 2>/dev/null | head -n 1)
    verdict=${verdict#"$condition "}
    case $verdict in
        ok) pass "$condition" ;;
        "ok "*) pass "$condition" "${verdict#ok }" ;;
        "failed "*) report "$condition" "${verdict#failed }" ;;
        *) report "$condition" "no verdict from psx_frame_budget.sh: $frame_budget" ;;
    esac
done

real_game=$(sh "$script_dir/psx_real_game_budget.sh" "$frame_dir/frames.txt" "$situations" \
    "$budget_hblanks" "$one_off_allowance" 2>&1)
real_game_figures=$(printf '%s\n' "$real_game" | grep '^real-game ')
printf '%s\n' "$real_game_figures" | sed 's/^/psx-full-game-check: /'
verdict=$(printf '%s\n' "$real_game" | grep '^real-game-budget ' | head -n 1)
verdict=${verdict#real-game-budget }
case $verdict in
    "ok "*) pass real-game-budget "${verdict#ok }" ;;
    "failed "*) report real-game-budget "${verdict#failed }" ;;
    *) report real-game-budget "no verdict from psx_real_game_budget.sh: $real_game" ;;
esac

check_ram_room gpu-ram-room "${gpu_exe%.exe}.elf"

{
    set -- $(tr -d '\r' < "$gpu_tty" | grep '^psx-replay: done ' | tail -n 1)
    [ "${5:-}" = vsyncs ] && echo "gpu vsyncs ${6:-}"
    grep -E "$pc_gpu_lines" "$pc_state" | sed 's/^/gpu /'
    tr -d '\r' < "$gpu_tty" | grep '^psx-replay: gpu-peak ' | tail -n 1 | sed 's/^psx-replay: /gpu /'
    [ -n "$room_line" ] && echo "gpu ram-room $room_line"
    printf '%s\n' "${elf_size:-}" | sed 's/^/gpu /'
    [ -f "$frame_dir/summary.txt" ] && sed 's/^/gpu frame /' "$frame_dir/summary.txt"
    printf '%s\n' "$real_game_figures" | sed 's/^/gpu /'
} >> "$out_dir/measurements.txt"

echo "psx-full-game-check: measurements in $out_dir/measurements.txt"
[ "$failed" -eq 0 ] || exit 1
echo "psx-full-game-check: all $(grep -c . "$out_dir/conditions.txt") conditions hold"
