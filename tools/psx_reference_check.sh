#!/bin/sh
# usage: psx_reference_check.sh <reference> <capture>
# Exit 0 when the sha256 of the whole capture equals the one-line hash in the
# reference; exit 1 otherwise, messages prefixed psx-reference:.

set -u

fail() {
    echo "psx-reference: $*" >&2
    exit 1
}

[ $# -eq 2 ] || fail "usage: psx_reference_check.sh <reference> <capture>"
reference=$1
capture=$2

[ -f "$capture" ] || fail "missing capture: $capture"
[ -s "$capture" ] || fail "empty capture: $capture"
[ -f "$reference" ] || fail "missing reference: $reference; run make psx-hello-e2e-baseline"

expected=$(cat "$reference")
case $expected in
    *[!0-9a-f]*|'') fail "not a sha256 line: $reference" ;;
esac
[ ${#expected} -eq 64 ] || fail "not a sha256 line: $reference"

if command -v sha256sum >/dev/null 2>&1; then
    h=$(sha256sum "$capture") || fail "cannot hash $capture"
else
    h=$(shasum -a 256 "$capture") || fail "cannot hash $capture"
fi
actual=${h%% *}

if [ "$actual" != "$expected" ]; then
    echo "psx-reference: $capture differs from $reference" >&2
    echo "psx-reference:   expected $expected" >&2
    echo "psx-reference:   actual   $actual" >&2
    echo "psx-reference: if the change is intended, run make psx-hello-e2e-baseline" >&2
    exit 1
fi
echo "psx-reference: $capture matches $reference ($actual)"
