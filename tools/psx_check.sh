#!/bin/sh
# usage: psx_check.sh <install-dir> <replay-exe> <pad-exe> <pc-state> <pc-digest> <pc-frames> <pc-format-line>
#                     <pad-script> <replay-ceiling> <replay-timeout> <pad-ceiling> <pad-timeout> <size-tool> <nm-tool>
#                     <out-dir>
# Boots the replay PS-EXE and the pad probe until they exit, then checks the
# ten conditions of make psx-check. Every condition is checked before
# concluding, one line per condition, exit 1 if any failed. Writes
# <out-dir>/measurements.txt: the MIPS zone/budget/cap lines, the vsync count
# and the size of the replay ELF (the .exe path with .elf).

set -u

usage="usage: psx_check.sh <install-dir> <replay-exe> <pad-exe> <pc-state> <pc-digest> <pc-frames> <pc-format-line> <pad-script> <replay-ceiling> <replay-timeout> <pad-ceiling> <pad-timeout> <size-tool> <nm-tool> <out-dir>"
[ $# -eq 15 ] || { echo "psx-check: $usage" >&2; exit 1; }
install_dir=$1
replay_exe=$2
pad_exe=$3
pc_state=$4
pc_digest=$5
pc_frames=$6
pc_format=$7
pad_script=$8
replay_ceiling=$9
shift 9
replay_timeout=$1
pad_ceiling=$2
pad_timeout=$3
size_tool=$4
nm_tool=$5
out_dir=$6
script_dir=$(cd "$(dirname "$0")" && pwd)

rm -rf "$out_dir"
mkdir -p "$out_dir"

replay_boot=$(PSX_BOOT_TIMEOUT=$replay_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --exe "$replay_exe" "$replay_ceiling" "$out_dir/replay" 2>&1)
replay_status=$?
pad_boot=$(PSX_BOOT_TIMEOUT=$pad_timeout sh "$script_dir/psx_boot.sh" "$install_dir" --until-exit \
    --pad-script "$pad_script" --exe "$pad_exe" "$pad_ceiling" "$out_dir/pad" 2>&1)
pad_status=$?

tty=$out_dir/replay.tty.txt
[ -f "$tty" ] || : > "$tty"
replay_lines=$(tr -d '\r' < "$tty" | grep '^psx-replay: ')

failed=0
pass() {
    echo "psx-check: $1: ok"
}
report() {
    echo "psx-check: $1: $2" >&2
    failed=1
}

bad_magic=
for exe in "$replay_exe" "$pad_exe"; do
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

done_line=$(printf '%s\n' "$replay_lines" | grep '^psx-replay: done ' | tail -n 1)
stopped=$(tr -d '\r' < "$tty" | grep '^psx-state: stopped p8_fatal' | head -n 1)
set -- $done_line
if [ "$replay_status" -ne 0 ]; then
    report replay-done "$replay_boot"
elif [ -n "$stopped" ]; then
    report replay-done "the PSX replay stopped: ${stopped#psx-state: }"
elif [ "${4:-}" != "$pc_frames" ]; then
    report replay-done "expected $pc_frames frames, got: ${done_line:-(no done line)}"
else
    pass replay-done
fi

if printf '%s\n' "$compare" | grep -q '^psx-state-compare: game lines match'; then
    pass state-game-lines
else
    report state-game-lines "$(printf '%s\n' "$compare" | grep -v 'no done line\|^psx-state-compare: done after\|the PSX replay stopped')"
fi

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

replay_elf=${replay_exe%.exe}.elf
if ! symbols=$("$nm_tool" "$replay_elf" 2>&1); then
    report no-json "$nm_tool failed on $replay_elf: $symbols"
else
    json_symbols=$(printf '%s\n' "$symbols" | awk '
        BEGIN { split("p8_load_gfx_json p8_input_load p8_input_save p8_input_append p8_input_free", names, " ")
                for (i in names) json[names[i]] = 1 }
        NF == 3 && $2 != "U" && ($3 in json) { print $3 }')
    if [ -z "$json_symbols" ]; then
        pass no-json
    else
        report no-json "the replay ELF defines: $(echo $json_symbols)"
    fi
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
echo "psx-check: all ten conditions hold"
