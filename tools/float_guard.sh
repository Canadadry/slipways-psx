#!/bin/sh
# usage: float_guard.sh <cc> <cflags> <repertoire jetable> <source.c>...
# Ecrit le rapport sur la sortie standard. Rend 1 si un fichier echoue sous
# -mgeneral-regs-only, 2 sans source : une garde sur rien passerait pour verte.
# Les objets ne sont jamais lies : sur x86_64, leurs arguments double passent
# sur la pile, ABI incoherente avec le reste. Le diagnostic de chaque fichier
# reste dans <repertoire>/<fichier>.err.

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
failed=$work/echecs.txt
: > "$failed"
for src in "$@"; do
    base=$work/$(echo "$src" | tr '/' '_')
    # cflags est volontairement non quote : c'est une liste d'options.
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
