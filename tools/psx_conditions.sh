# Sourced by tools/psx_check.sh and tools/psx_full_game_check.sh: the
# conditions both check. The caller sets check_name (the line prefix, such as
# psx-check), out_dir (conditions.txt is appended there), script_dir (the
# tools directory) and failed=0; a failed condition sets failed=1.

pass() {
    echo "$check_name: $1: ok${2:+, $2}"
    echo "$1 ok" >> "$out_dir/conditions.txt"
}

report() {
    echo "$check_name: $1: $2" >&2
    echo "$1 failed" >> "$out_dir/conditions.txt"
    failed=1
}

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

# usage: check_digests <tty> <pc-digest> <digest-out-dir>; the sprite-sheet, screen and map conditions
check_digests() {
    digest_compare=$(sh "$script_dir/psx_digest_compare.sh" "$1" "$2" "$3" 2>&1)
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
}
