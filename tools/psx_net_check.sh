#!/bin/sh
# usage: psx_net_check.sh <sources>
# Exit 1 when the list is missing or a host it names does not answer.

set -u

. "$(cd "$(dirname "$0")" && pwd)/psx_portable.sh"
GIT_TERMINAL_PROMPT=0
export GIT_TERMINAL_PROMPT

sources=$1
if [ ! -f "$sources" ]; then
    echo "psx-net-check: missing sources list: $sources" >&2
    exit 1
fi

http_code() {
    curl -s -o /dev/null -L -r 0-0 --connect-timeout 10 --max-time 20 \
        -w '%{http_code}' "$1" </dev/null 2>/dev/null
}

failed=0
while read -r component kind url pin; do
    case $component in ''|'#'*) continue ;; esac
    host=$(echo "$url" | sed -e 's|^[a-z]*://||' -e 's|[/:].*||')
    case $kind in
        git)
            run_with_timeout 30 git ls-remote "$url" HEAD </dev/null >/dev/null 2>&1
            ok=$? ;;
        file)
            case $(http_code "$url") in 2??) ok=0 ;; *) ok=1 ;; esac ;;
        apt)
            case $(http_code "$url/dists/$pin/Release") in 2??) ok=0 ;; *) ok=1 ;; esac ;;
        host)
            # A 404 from the host itself answers; curl reports 000 when the
            # proxy refuses the CONNECT or nothing answers.
            case $(http_code "$url") in 000|'') ok=1 ;; *) ok=0 ;; esac ;;
        *)
            echo "psx-net-check: $component: unknown kind $kind" >&2
            failed=1
            continue ;;
    esac
    if [ "$ok" -eq 0 ]; then
        echo "psx-net-check: $component: $host reachable"
    else
        echo "psx-net-check: $component: $host unreachable" >&2
        failed=1
    fi
done < "$sources"

exit $failed
