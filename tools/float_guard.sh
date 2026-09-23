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
    echo "float_guard: aucune source" >&2
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
# Garde -mgeneral-regs-only sur le coeur (docs/plan-2.md §6.3), produite par
# tools/float_guard.sh via make float-guard. Ne pas editer a la main.
#
# Une ligne par fichier du coeur qui ne compile pas sans registre flottant.
# La liste doit etre vide.
#
# Angle mort sur x86_64 : un argument double passe a une fonction externe
# compile sans erreur sous la garde. Il est tenu par l'absence de prototype
# flottant dans le coeur : un tel appel y est une declaration implicite, donc
# une erreur.
#
EOF
LC_ALL=C sort "$failed" | sed 's/^/echec /'
count=$(wc -l < "$failed" | tr -d ' ')
echo "#"
echo "garde fichiers $# en-echec $count"
[ "$count" -eq 0 ]
