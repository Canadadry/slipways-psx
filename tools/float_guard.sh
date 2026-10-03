#!/bin/sh
# usage: float_guard.sh <compiler> <cflags> <scratch directory> <source.c>...
# Writes the report on the standard output. Exit 1 when a file fails under
# `-mgeneral-regs-only`, 2 with no source: a guard over nothing would pass
# for green. The objects are never linked: on x86_64, their double
# arguments go on the stack, an ABI inconsistent with the rest. The
# diagnostic of each file stays in <directory>/<file>.err.

set -u

cc=$1
cflags=$2
work=$3
shift 3

if [ $# -eq 0 ]; then
    echo "float_guard: no source" >&2
    exit 2
fi

mkdir -p "$work"
failed=$work/failed.txt
: > "$failed"
for src in "$@"; do
    base=$work/$(echo "$src" | tr '/' '_')
    # cflags is deliberately unquoted: it is a list of options.
    if ! $cc $cflags -mgeneral-regs-only -c -o "$base.o" "$src" 2> "$base.err"; then
        echo "$src" >> "$failed"
    fi
done

cat <<'EOF'
# -mgeneral-regs-only guard over the core (no floating-point register),
# written by tools/float_guard.sh through make float-guard. Do not edit.
#
# One line per file of the core that does not compile without
# floating-point registers. The list must be empty.
#
# Blind spot on x86_64: a double argument passed to an external function
# compiles without error under the guard. The absence of any floating-point
# prototype in the core holds it: such a call is an implicit declaration
# there, hence an error.
#
EOF
LC_ALL=C sort "$failed" | sed 's/^/failed /'
count=$(wc -l < "$failed" | tr -d ' ')
echo "#"
echo "float-guard files $# failing $count"
[ "$count" -eq 0 ]
