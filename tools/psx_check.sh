#!/bin/sh
# usage: psx_check.sh <install-dir> <replay-exe> <gpu-exe> <pad-exe> <pc-state> <pc-digest> <pc-frames>
#                     <pc-format-line> <pad-script> <replay-ceiling> <replay-timeout> <gpu-ceiling> <gpu-timeout>
#                     <pad-ceiling> <pad-timeout> <size-tool> <nm-tool> <stack-reserve> <capture-list>
#                     <pc-capture-dir> <compare-tool> <ceilings> <references> <out-dir>
# Boots the replay PS-EXE, the GPU replay PS-EXE (twice, both driven by
# <pc-capture-dir>/capture_script.txt) and the pad probe until they exit, then
# checks the twenty-one conditions of make psx-check, the seven GPU capture
# ones through tools/psx_capture_check.sh. Every condition is checked before
# concluding, one line per condition, exit 1 if any failed; each also goes to
# <out-dir>/conditions.txt as "<condition> ok|failed".
# Writes <out-dir>/measurements.txt: the MIPS zone/budget/cap lines, the vsync
# count and the size of the replay ELF (the .exe path with .elf), then the GPU
# replay's vsyncs, meter lines, RAM room and ELF size, each line behind "gpu ",
# then the capture report lines behind "gpu capture ".

set -u

usage="usage: psx_check.sh <install-dir> <replay-exe> <gpu-exe> <pad-exe> <pc-state> <pc-digest> <pc-frames> <pc-format-line> <pad-script> <replay-ceiling> <replay-timeout> <gpu-ceiling> <gpu-timeout> <pad-ceiling> <pad-timeout> <size-tool> <nm-tool> <stack-reserve> <capture-list> <pc-capture-dir> <compare-tool> <ceilings> <references> <out-dir>"
[ $# -eq 24 ] || { echo "psx-check: $usage" >&2; exit 1; }
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
out_dir=$6
script_dir=$(cd "$(dirname "$0")" && pwd)

rm -rf "$out_dir"
mkdir -p "$out_dir"

replay_boot=$(PSX_BOOT_TIMEOUT=$replay_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --exe "$replay_exe" "$replay_ceiling" "$out_dir/replay" 2>&1)
replay_status=$?
capture_script=$pc_capture_dir/capture_script.txt
gpu_boot=$(PSX_BOOT_TIMEOUT=$gpu_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --capture-script "$capture_script" --exe "$gpu_exe" "$gpu_ceiling" "$out_dir/gpu" 2>&1)
gpu_status=$?
# Only for gpu-capture-deterministic: one boot cannot prove two boots agree.
PSX_BOOT_TIMEOUT=$gpu_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --capture-script "$capture_script" --exe "$gpu_exe" "$gpu_ceiling" "$out_dir/gpu-repeat/gpu" \
    > "$out_dir/gpu-repeat.boot.txt" 2>&1
pad_boot=$(PSX_BOOT_TIMEOUT=$pad_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --pad-script "$pad_script" --exe "$pad_exe" "$pad_ceiling" "$out_dir/pad" 2>&1)
pad_status=$?

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
for exe in "$replay_exe" "$gpu_exe" "$pad_exe"; do
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

# usage: check_replay_done <condition> <boot-status> <boot-output> <tty>
check_replay_done() {
    done_line=$(tr -d '\r' < "$4" | grep '^psx-replay: done ' | tail -n 1)
    stopped=$(tr -d '\r' < "$4" | grep '^psx-state: stopped p8_fatal' | head -n 1)
    set -- "$1" "$2" "$3" $done_line
    if [ "$2" -ne 0 ]; then
        report "$1" "$3"
    elif [ -n "$stopped" ]; then
        report "$1" "the PSX replay stopped: ${stopped#psx-state: }"
    elif [ "${7:-}" != "$pc_frames" ]; then
        report "$1" "expected $pc_frames frames, got: ${done_line:-(no done line)}"
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

check_replay_done replay-done "$replay_status" "$replay_boot" "$tty"
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
check_replay_done gpu-replay-done "$gpu_status" "$gpu_boot" "$gpu_tty"
check_game_lines gpu-state-game-lines "$gpu_compare"

meter_lines='^(zone gpu_frame|cap gpu_cluts|cap gpu_patterns) '
gpu_meter=$(sed -n 's/^psx-state: //p' "$out_dir/gpu-state/tty.txt" 2>/dev/null | grep -E "$meter_lines")
pc_meter=$(grep -E "$meter_lines" "$pc_state")
if [ -n "$pc_meter" ] && [ "$gpu_meter" = "$pc_meter" ]; then
    pass gpu-cost
else
    report gpu-cost "the GPU replay's meter lines differ from the PC's:
  psx: $(echo ${gpu_meter:-(no line)})
  pc:  $(echo ${pc_meter:-(no line)})"
fi

gpu_elf=${gpu_exe%.exe}.elf
kernel_area=65536
ram_bytes=2097152
if ! gpu_size=$("$size_tool" "$gpu_elf" 2>&1); then
    report ram-room "$size_tool failed on $gpu_elf: $gpu_size"
    room_line=
else
    room_line=$(printf '%s\n' "$gpu_size" | awk -v kernel="$kernel_area" -v stack="$stack_reserve" -v ram="$ram_bytes" '
        NR == 2 { used = kernel + $1 + $2 + $3 + stack; print used, ram, ram - used }')
    set -- $room_line
    if [ -z "$room_line" ]; then
        report ram-room "no size line for $gpu_elf: $gpu_size"
    elif [ "$3" -ge 0 ]; then
        pass ram-room "$3 bytes left in $2 (kernel $kernel_area + ELF + stack $stack_reserve = $1)"
    else
        report ram-room "kernel $kernel_area + ELF + stack $stack_reserve = $1 bytes, over $2 by $((-$3))"
    fi
fi

sh "$script_dir/psx_capture_report.sh" "$compare_tool" "$capture_list" "$pc_capture_dir" "$out_dir/gpu" \
    "$out_dir/gpu-captures"
sh "$script_dir/psx_capture_check.sh" "$capture_list" "$pc_capture_dir" "$gpu_tty" "$out_dir/gpu" \
    "$out_dir/gpu-repeat/gpu" "$out_dir/gpu-captures" "$ceilings" "$references" "$out_dir/conditions.txt" || failed=1

{
    gpu_done=$(tr -d '\r' < "$gpu_tty" | grep '^psx-replay: done ' | tail -n 1)
    set -- $gpu_done
    [ "${5:-}" = vsyncs ] && echo "gpu vsyncs ${6:-}"
    printf '%s\n' "$gpu_meter" | grep . | sed 's/^/gpu /'
    [ -n "$room_line" ] && echo "gpu ram-room $room_line"
    printf '%s\n' "${gpu_size:-}" | sed 's/^/gpu /'
    sed 's/^/gpu capture /' "$out_dir/gpu-captures/report.txt"
} >> "$out_dir/measurements.txt"

for elf in "${replay_exe%.exe}.elf" "$gpu_elf"; do
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

echo "psx-check: measurements in $out_dir/measurements.txt"
[ "$failed" -eq 0 ] || exit 1
echo "psx-check: all twenty-one conditions hold"
