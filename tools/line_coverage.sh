#!/bin/sh
# usage: line_coverage.sh cover <pilote gcov> <arbre gcov> <events>...
#        line_coverage.sh diff <ensemble> <ensemble>
#        line_coverage.sh contrib <ensemble> [<ensemble>...]
#        line_coverage.sh gap <référence> <inatteignables> <ensemble>
# Un ensemble : une ligne `fichier:ligne` par ligne source exécutée, triée.
# cover : l'union des lignes couvertes par les rejeux, un processus par
#   fichier, chacun depuis l'écran titre. Rend 1, sans rien écrire sur la
#   sortie, si un fichier manque ou si un rejeu échoue.
# diff : les lignes du premier ensemble absentes du second (l'écart).
# contrib : les lignes du premier ensemble qu'aucun des autres ne couvre
#   (l'apport d'un fichier).
# gap : les lignes de la référence ni couvertes par l'ensemble ni listées dans
#   les inatteignables (`fichier:ligne raison`, seul le premier champ compte).

set -u

sorted() {
    LC_ALL=C sort -t: -k1,1 -k2,2n -u
}

# Les ensembles suivant le premier forment l'union à retirer du premier.
minus() {
    for set in "$@"; do
        if [ ! -f "$set" ]; then
            echo "ensemble absent: $set" >&2
            exit 1
        fi
    done
    first=$1
    shift
    awk -v first="$first" '
        FILENAME != first { seen[$0] = 1; next }
        !($0 in seen)' "$@" "$first" | sorted
}

gap() {
    for set in "$@"; do
        if [ ! -f "$set" ]; then
            echo "ensemble absent: $set" >&2
            exit 1
        fi
    done
    awk -v reference="$1" '
        FILENAME != reference { if (NF) seen[$1] = 1; next }
        !($0 in seen)' "$2" "$3" "$1" | sorted
}

cover() {
    tool=$1
    objdir=$2
    shift 2
    if [ $# -eq 0 ]; then
        echo "cover: aucun fichier d'événements" >&2
        exit 1
    fi

    abs_objdir=$(cd "$objdir" && pwd) || exit 1
    # Retire le chemin absolu de l'arbre : chaque rejeu écrit ses .gcda sous
    # son propre préfixe, rangés comme dans l'arbre.
    strip=$(printf '%s\n' "$abs_objdir" | awk -F/ '{ print NF - 1 }')

    work=$(mktemp -d "${TMPDIR:-/tmp}/line_coverage.XXXXXX")
    trap 'rm -rf "$work"' EXIT

    i=0
    failed=0
    for arg in "$@"; do
        i=$((i + 1))
        printf '%s\n' "$arg" > "$work/$i.events"
        if [ ! -f "$arg" ]; then
            echo "absent: $arg" >&2
            failed=1
        else
            (
                GCOV_PREFIX="$work/$i" GCOV_PREFIX_STRIP=$strip \
                    "$tool" "$arg" /dev/null "$work/$i.dump" > /dev/null 2> "$work/$i.err"
                echo $? > "$work/$i.status"
            ) &
        fi
    done
    wait

    n=$i
    i=0
    while [ "$i" -lt "$n" ]; do
        i=$((i + 1))
        [ -f "$work/$i.status" ] || continue
        status=$(cat "$work/$i.status")
        if [ "$status" -ne 0 ]; then
            echo "échec: $(cat "$work/$i.events") (code $status)" >&2
            sed 's/^/  /' "$work/$i.err" >&2
            failed=1
        fi
    done
    [ "$failed" -eq 0 ] || exit 1

    : > "$work/all"
    for dir in $(find "$work" -name '*.gcda' -exec dirname {} \; | sort -u); do
        rel=${dir#"$work"/*/}
        cp "$objdir/$rel"/*.gcno "$dir"/ || exit 1
        if ! gcov -t -o "$dir" "$dir"/*.gcda > "$work/gcov.out" 2> "$work/gcov.err" \
            || grep -q 'mismatch' "$work/gcov.err"; then
            echo "gcov: échec sur $dir" >&2
            cat "$work/gcov.err" >&2
            exit 1
        fi
        # Les en-têtes système (chemin absolu) ne sont pas du code du port.
        awk -F: '
            $3 == "Source" && $2 + 0 == 0 { src = $4; next }
            {
                count = $1; gsub(/ /, "", count)
                if (count ~ /^[0-9]/ && src !~ /^\//) {
                    line = $2; gsub(/ /, "", line)
                    print src ":" line
                }
            }' "$work/gcov.out" >> "$work/all"
    done
    sorted < "$work/all"
}

case "${1:-}" in
    cover)
        shift
        [ $# -ge 2 ] || { echo "usage: $0 cover <pilote gcov> <arbre gcov> <events>..." >&2; exit 1; }
        cover "$@"
        ;;
    diff)
        shift
        [ $# -eq 2 ] || { echo "usage: $0 diff <ensemble> <ensemble>" >&2; exit 1; }
        minus "$@"
        ;;
    contrib)
        shift
        [ $# -ge 1 ] || { echo "usage: $0 contrib <ensemble> [<ensemble>...]" >&2; exit 1; }
        minus "$@"
        ;;
    gap)
        shift
        [ $# -eq 3 ] || { echo "usage: $0 gap <référence> <inatteignables> <ensemble>" >&2; exit 1; }
        gap "$@"
        ;;
    *)
        sed -n '2,5s/^# //p' "$0" >&2
        exit 1
        ;;
esac
