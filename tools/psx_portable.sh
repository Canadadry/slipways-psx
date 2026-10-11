# Sourced by the PSX tool scripts: GNU coreutils commands that macOS lacks.

# Prints the lowercase sha256 of a file.
sha256_of() {
    if command -v sha256sum >/dev/null 2>&1; then
        sha256sum "$1" | cut -d ' ' -f 1
    else
        shasum -a 256 "$1" | cut -d ' ' -f 1
    fi
}

cpu_count() {
    getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1
}

# Runs a command, killed after <seconds>; returns 124 then, as timeout(1) does.
run_with_timeout() {
    seconds=$1
    shift
    if command -v timeout >/dev/null 2>&1; then
        timeout "$seconds" "$@"
        return $?
    fi
    timed_out=$(mktemp)
    rm -f "$timed_out"
    "$@" &
    command_pid=$!
    (
        sleep "$seconds"
        : > "$timed_out"
        kill -TERM "$command_pid" 2>/dev/null
        sleep 5
        kill -KILL "$command_pid" 2>/dev/null
    ) &
    watcher_pid=$!
    wait "$command_pid"
    command_status=$?
    kill "$watcher_pid" 2>/dev/null
    wait "$watcher_pid" 2>/dev/null
    if [ -e "$timed_out" ]; then
        rm -f "$timed_out"
        return 124
    fi
    return "$command_status"
}
