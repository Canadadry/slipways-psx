#!/bin/sh
# usage: psx_check.sh <install-dir> <replay-exe> <gpu-exe> <pad-exe> <pc-state> <pc-digest> <pc-frames>
#                     <pc-format-line> <pad-script> <replay-ceiling> <replay-timeout> <gpu-ceiling> <gpu-timeout>
#                     <pad-ceiling> <pad-timeout> <size-tool> <nm-tool> <stack-reserve> <capture-list>
#                     <pc-capture-dir> <compare-tool> <ceilings> <references> <game-exe> <game-script>
#                     <game-ceiling> <game-timeout> <game-difficulty> <game-cue> <dumpsxiso> <game-disc-exe>
#                     <pad-log-tool> <replay-tool> <out-dir>
# Boots the replay PS-EXE, the GPU replay PS-EXE (twice, both driven by
# <pc-capture-dir>/capture_script.txt), the pad probe and the game's CD image
# <game-cue> (driven by <game-script>) until they exit, then checks the thirty
# conditions of make psx-check, the seven GPU capture ones through
# tools/psx_capture_check.sh, five game ones through tools/psx_game_check.sh
# (outputs in <out-dir>/game) and game-cue and game-cd-exe through
# tools/psx_cd_check.sh (<game-disc-exe> on the disc equals <game-exe>,
# outputs in <out-dir>/game-cd). Every condition is
# checked before concluding, one line per condition, exit 1 if any failed;
# each also goes to <out-dir>/conditions.txt as "<condition> ok|failed".
# Writes <out-dir>/measurements.txt: the MIPS zone/budget/cap lines, the vsync
# count and the size of the replay ELF (the .exe path with .elf), then the GPU
# replay's vsyncs, meter lines, RAM room and ELF size, each line behind "gpu ",
# the capture report lines behind "gpu capture ", then the game's vsyncs,
# frames, RAM room and ELF size behind "game ", then the .bin's size and the
# CD boot's vsyncs from power-on behind "game cd ".

set -u

usage="usage: psx_check.sh <install-dir> <replay-exe> <gpu-exe> <pad-exe> <pc-state> <pc-digest> <pc-frames> <pc-format-line> <pad-script> <replay-ceiling> <replay-timeout> <gpu-ceiling> <gpu-timeout> <pad-ceiling> <pad-timeout> <size-tool> <nm-tool> <stack-reserve> <capture-list> <pc-capture-dir> <compare-tool> <ceilings> <references> <game-exe> <game-script> <game-ceiling> <game-timeout> <game-difficulty> <game-cue> <dumpsxiso> <game-disc-exe> <pad-log-tool> <replay-tool> <out-dir>"
[ $# -eq 34 ] || { echo "psx-check: $usage" >&2; exit 1; }
install_dir=$1
replay_exe=$2
gpu_exe=$3
pad_exe=$4
pc_state=$5
pc_digest=$6
pc_frames=$7
pc_format=$8
pad_script=$9
shift 9
replay_ceiling=$1
replay_timeout=$2
gpu_ceiling=$3
gpu_timeout=$4
pad_ceiling=$5
pad_timeout=$6
size_tool=$7
nm_tool=$8
stack_reserve=$9
shift 9
capture_list=$1
pc_capture_dir=$2
compare_tool=$3
ceilings=$4
references=$5
game_exe=$6
game_script=$7
game_ceiling=$8
game_timeout=$9
shift 9
game_difficulty=$1
game_cue=$2
dumpsxiso=$3
game_disc_exe=$4
pad_log_tool=$5
replay_tool=$6
out_dir=$7
script_dir=$(cd "$(dirname "$0")" && pwd)

rm -rf "$out_dir"
mkdir -p "$out_dir"

echo "psx-check: booting the replay PS-EXE (at most ${replay_timeout}s)"
replay_boot=$(PSX_BOOT_TIMEOUT=$replay_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --exe "$replay_exe" "$replay_ceiling" "$out_dir/replay" 2>&1)
replay_status=$?
capture_script=$pc_capture_dir/capture_script.txt
echo "psx-check: booting the GPU replay PS-EXE (at most ${gpu_timeout}s)"
gpu_boot=$(PSX_BOOT_TIMEOUT=$gpu_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --capture-script "$capture_script" --exe "$gpu_exe" "$gpu_ceiling" "$out_dir/gpu" 2>&1)
gpu_status=$?
echo "psx-check: booting the GPU replay PS-EXE again (at most ${gpu_timeout}s)"
# Only for gpu-capture-deterministic: one boot cannot prove two boots agree.
PSX_BOOT_TIMEOUT=$gpu_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --capture-script "$capture_script" --exe "$gpu_exe" "$gpu_ceiling" "$out_dir/gpu-repeat/gpu" \
    > "$out_dir/gpu-repeat.boot.txt" 2>&1
echo "psx-check: booting the pad probe (at most ${pad_timeout}s)"
pad_boot=$(PSX_BOOT_TIMEOUT=$pad_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --pad-script "$pad_script" --exe "$pad_exe" "$pad_ceiling" "$out_dir/pad" 2>&1)
pad_status=$?
echo "psx-check: booting the game CD image (at most ${game_timeout}s)"
game_boot=$(PSX_BOOT_TIMEOUT=$game_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --pad-script "$game_script" --iso "$game_cue" "$game_ceiling" "$out_dir/game" 2>&1)
game_status=$?

tty=$out_dir/replay.tty.txt
[ -f "$tty" ] || : > "$tty"
replay_lines=$(tr -d '\r' < "$tty" | grep '^psx-replay: ')

failed=0
pass() {
    echo "psx-check: $1: ok${2:+, $2}"
    echo "$1 ok" >> "$out_dir/conditions.txt"
}
report() {
    echo "psx-check: $1: $2" >&2
    echo "$1 failed" >> "$out_dir/conditions.txt"
    failed=1
}

bad_magic=
for exe in "$replay_exe" "$gpu_exe" "$pad_exe" "$game_exe"; do
    [ "$(head -c 8 "$exe" 2>/dev/null)" = "PS-X EXE" ] || bad_magic="$bad_magic $exe"
done
if [ -z "$bad_magic" ]; then
    pass magic
else
    report magic "not starting with PS-X EXE (elf2x failed?):$bad_magic"
fi

asserts=$(printf '%s\n' "$replay_lines" | sed -n 's/^psx-replay: asserts //p' | head -n 1)
if [ "$asserts" = on ]; then
    pass asserts-on
else
    report asserts-on "psx-replay: asserts ${asserts:-(no line)}"
fi

psx_format=$(printf '%s\n' "$replay_lines" | sed -n 's/^psx-replay: //p' | grep '^format ' | head -n 1)
if [ -n "$psx_format" ] && [ "$psx_format" = "$pc_format" ]; then
    pass format
else
    report format "differs from the PC's:
  psx: ${psx_format:-(no line)}
  pc:  $pc_format"
fi

compare=$(sh "$script_dir/psx_state_compare.sh" "$tty" "$pc_state" "$out_dir" "$pc_frames" 2>&1)
"$size_tool" "${replay_exe%.exe}.elf" >> "$out_dir/measurements.txt" 2>&1

# usage: check_replay_done <condition> <boot-status> <boot-output> <tty> <frames>
check_replay_done() {
    done_line=$(tr -d '\r' < "$4" | grep '^psx-replay: done ' | tail -n 1)
    stopped=$(tr -d '\r' < "$4" | grep '^psx-state: stopped p8_fatal' | head -n 1)
    set -- "$1" "$2" "$3" "$5" $done_line
    if [ "$2" -ne 0 ]; then
        report "$1" "$3"
    elif [ -n "$stopped" ]; then
        report "$1" "the PSX replay stopped: ${stopped#psx-state: }"
    elif [ "${8:-}" != "$4" ]; then
        report "$1" "expected $4 frames, got: ${done_line:-(no done line)}"
    else
        pass "$1"
    fi
}

# usage: check_game_lines <condition> <psx_state_compare.sh output>
check_game_lines() {
    if printf '%s\n' "$2" | grep -q '^psx-state-compare: game lines match'; then
        pass "$1"
    else
        report "$1" "$(printf '%s\n' "$2" | grep -v 'no done line\|^psx-state-compare: done after\|the PSX replay stopped')"
    fi
}

check_replay_done replay-done "$replay_status" "$replay_boot" "$tty" "$pc_frames"
check_game_lines state-game-lines "$compare"

digest_compare=$(sh "$script_dir/psx_digest_compare.sh" "$tty" "$pc_digest" "$out_dir" 2>&1)
for kind in sprite-sheet screen map; do
    if printf '%s\n' "$digest_compare" | grep -q "^psx-digest-compare: $kind: ok\$"; then
        pass "$kind"
    else
        reason=$(printf '%s\n' "$digest_compare" | awk -v prefix="psx-digest-compare: $kind: " '
            index($0, prefix) == 1 { found = 1; print substr($0, length(prefix) + 1); next }
            /^psx-digest-compare: / { found = 0; next }
            found')
        report "$kind" "${reason:-$digest_compare}"
    fi
done

gpu_tty=$out_dir/gpu.tty.txt
[ -f "$gpu_tty" ] || : > "$gpu_tty"
gpu_compare=$(sh "$script_dir/psx_state_compare.sh" "$gpu_tty" "$pc_state" "$out_dir/gpu-state" "$pc_frames" 2>&1)
check_replay_done gpu-replay-done "$gpu_status" "$gpu_boot" "$gpu_tty" "$pc_frames"
check_game_lines gpu-state-game-lines "$gpu_compare"

# usage: check_meter <condition> <what> <psx-state-tty> <pc-state>; sets psx_meter (the PSX side's meter lines)
check_meter() {
    meter_lines='^(zone gpu_frame|cap gpu_cluts|cap gpu_patterns) '
    psx_meter=$(sed -n 's/^psx-state: //p' "$3" 2>/dev/null | grep -E "$meter_lines")
    pc_meter=$(grep -E "$meter_lines" "$4" 2>/dev/null)
    if [ -n "$pc_meter" ] && [ "$psx_meter" = "$pc_meter" ]; then
        pass "$1"
    else
        report "$1" "$2's meter lines differ from the PC's:
  psx: $(echo ${psx_meter:-(no line)})
  pc:  $(echo ${pc_meter:-(no line)})"
    fi
}

check_meter gpu-cost "the GPU replay" "$out_dir/gpu-state/tty.txt" "$pc_state"
gpu_meter=$psx_meter

kernel_area=65536
ram_bytes=2097152
# usage: check_ram_room <condition> <elf>; sets elf_size (the size tool's output) and room_line
# ("<used> <limit> <left>", empty when the size tool failed)
check_ram_room() {
    room_line=
    if ! elf_size=$("$size_tool" "$2" 2>&1); then
        report "$1" "$size_tool failed on $2: $elf_size"
        return
    fi
    room_line=$(printf '%s\n' "$elf_size" | awk -v kernel="$kernel_area" -v stack="$stack_reserve" -v ram="$ram_bytes" '
        NR == 2 { used = kernel + $1 + $2 + $3 + stack; print used, ram, ram - used }')
    set -- "$1" "$2" $room_line
    if [ -z "$room_line" ]; then
        report "$1" "no size line for $2: $elf_size"
    elif [ "$5" -ge 0 ]; then
        pass "$1" "$5 bytes left in $4 (kernel $kernel_area + ELF + stack $stack_reserve = $3)"
    else
        report "$1" "kernel $kernel_area + ELF + stack $stack_reserve = $3 bytes, over $4 by $((-$5))"
    fi
}

gpu_elf=${gpu_exe%.exe}.elf
check_ram_room ram-room "$gpu_elf"
gpu_size=$elf_size
gpu_room_line=$room_line

sh "$script_dir/psx_capture_report.sh" "$compare_tool" "$capture_list" "$pc_capture_dir" "$out_dir/gpu" \
    "$out_dir/gpu-captures"
sh "$script_dir/psx_capture_check.sh" "$capture_list" "$pc_capture_dir" "$gpu_tty" "$out_dir/gpu" \
    "$out_dir/gpu-repeat/gpu" "$out_dir/gpu-captures" "$ceilings" "$references" "$out_dir/conditions.txt" || failed=1

{
    gpu_done=$(tr -d '\r' < "$gpu_tty" | grep '^psx-replay: done ' | tail -n 1)
    set -- $gpu_done
    [ "${5:-}" = vsyncs ] && echo "gpu vsyncs ${6:-}"
    printf '%s\n' "$gpu_meter" | grep . | sed 's/^/gpu /'
    [ -n "$gpu_room_line" ] && echo "gpu ram-room $gpu_room_line"
    printf '%s\n' "${gpu_size:-}" | sed 's/^/gpu /'
    sed 's/^/gpu capture /' "$out_dir/gpu-captures/report.txt"
} >> "$out_dir/measurements.txt"

game_elf=${game_exe%.exe}.elf
for elf in "${replay_exe%.exe}.elf" "$gpu_elf" "$game_elf"; do
    if ! symbols=$("$nm_tool" "$elf" 2>&1); then
        json_symbols="($nm_tool failed on $elf: $symbols)"
    else
        json_symbols=$(printf '%s\n' "$symbols" | awk '
            BEGIN { split("p8_load_gfx_json p8_input_load p8_input_save p8_input_append p8_input_free", names, " ")
                    for (i in names) json[names[i]] = 1 }
            NF == 3 && $2 != "U" && ($3 in json) { print $3 }')
    fi
    [ -z "$json_symbols" ] || no_json_failure="${no_json_failure:-}$elf defines: $(echo $json_symbols); "
done
if [ -z "${no_json_failure:-}" ]; then
    pass no-json
else
    report no-json "$no_json_failure"
fi

if [ "$pad_status" -ne 0 ]; then
    report pad-script "$pad_boot"
elif pad=$(sh "$script_dir/psx_pad_compare.sh" "$pad_script" "$out_dir/pad.tty.txt" 2>&1); then
    pass pad-script
else
    report pad-script "$pad"
fi

sh "$script_dir/psx_game_check.sh" "$out_dir/game.tty.txt" "$game_status" "$game_boot" "$game_script" \
    "$pad_log_tool" "$replay_tool" "$game_difficulty" "$out_dir/game" \
    > "$out_dir/game-check.out.txt" 2> "$out_dir/game-check.err.txt"
for condition in game-done game-pad-log game-pad-states game-state-game-lines game-title-left; do
    prefix="psx-game-check: $condition: "
    line=$(grep -F "$prefix" "$out_dir/game-check.out.txt" | head -n 1)
    case $line in
        "${prefix}ok") pass "$condition" ;;
        "${prefix}ok, "*) pass "$condition" "${line#"${prefix}ok, "}" ;;
        *)
            reason=$(awk -v prefix="$prefix" '
                index($0, prefix) == 1 { found = 1; print substr($0, length(prefix) + 1); next }
                /^psx-game-check: / { found = 0; next }
                found' "$out_dir/game-check.err.txt")
            report "$condition" "${reason:-no line from psx_game_check.sh: $(cat "$out_dir/game-check.err.txt")}"
            ;;
    esac
done

check_meter game-gpu-cost "the CD game" "$out_dir/game/game-state/tty.txt" "$out_dir/game/pc_state.txt"

sh "$script_dir/psx_cd_check.sh" "$game_cue" "$game_exe" "$game_disc_exe" "$dumpsxiso" "$out_dir/game-cd" \
    > "$out_dir/game-cd-check.out.txt" 2> "$out_dir/game-cd-check.err.txt"
for condition in cue cd-exe; do
    prefix="psx-cd-check: $condition: "
    line=$(grep -F "$prefix" "$out_dir/game-cd-check.out.txt" | head -n 1)
    case $line in
        "${prefix}ok, "*) pass "game-$condition" "${line#"${prefix}ok, "}" ;;
        *)
            reason=$(awk -v prefix="$prefix" '
                index($0, prefix) == 1 { found = 1; print substr($0, length(prefix) + 1); next }
                /^psx-cd-check: / { found = 0; next }
                found' "$out_dir/game-cd-check.err.txt")
            report "game-$condition" "${reason:-no line from psx_cd_check.sh: $(cat "$out_dir/game-cd-check.err.txt")}"
            ;;
    esac
done

check_ram_room game-ram-room "$game_elf"

{
    set -- $(tr -d '\r' < "$out_dir/game.tty.txt" 2>/dev/null | grep '^psx-replay: done game ' | tail -n 1)
    if [ "${5:-}" = vsyncs ]; then
        echo "game vsyncs ${6:-}"
        echo "game frames $4"
    fi
    [ -n "$room_line" ] && echo "game ram-room $room_line"
    printf '%s\n' "${elf_size:-}" | sed 's/^/game /'
    # Size only: mkpsxiso stamps the build time into the .bin, so a hash would differ on every build.
    if bin=$(sh "$script_dir/psx_cue_check.sh" "$game_cue" 2>/dev/null); then
        bin_bytes=$(wc -c < "$bin" | tr -d ' ')
        echo "game cd bin-bytes $bin_bytes sectors $((bin_bytes / 2352))"
    fi
    boot_vsyncs=$(printf '%s\n' "$game_boot" | sed -n 's/.*: exited with 0 after \([0-9]*\) vsyncs.*/\1/p' | tail -n 1)
    [ -n "$boot_vsyncs" ] && echo "game cd boot-vsyncs $boot_vsyncs"
} >> "$out_dir/measurements.txt"

echo "psx-check: measurements in $out_dir/measurements.txt"
[ "$failed" -eq 0 ] || exit 1
echo "psx-check: all thirty conditions hold"
