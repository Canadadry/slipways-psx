#!/bin/sh
# usage: e2e_cleanup.sh <nom> <frames> <ensemble> [<nom> <frames> <ensemble>...]
# La passe de nettoyage finale : du plus long au plus court (à frames égales,
# par nom), retire tout fichier dont chaque ligne est couverte par l'union
# des fichiers restants. Écrit les noms retirés, dans l'ordre du retrait ;
# ne supprime rien. Un ensemble : une ligne `fichier:ligne` par ligne
# couverte (line_coverage.sh cover).
# Au fil de l'eau, un nouveau fichier se juge seul contre ceux déjà là :
# line_coverage.sh contrib, make e2e-judge.

set -u

usage() {
    sed -n '2p' "$0" | sed 's/^# //' >&2
    exit 1
}

[ $# -ge 3 ] && [ $(($# % 3)) -eq 0 ] || usage

work=$(mktemp -d "${TMPDIR:-/tmp}/e2e_cleanup.XXXXXX") || exit 1
trap 'rm -rf "$work"' EXIT

: > "$work/files"
while [ $# -gt 0 ]; do
    case $2 in
        '' | *[!0-9]*)
            echo "e2e_cleanup: $1 : nombre de frames invalide: $2" >&2
            exit 1
            ;;
    esac
    if [ ! -f "$3" ]; then
        echo "e2e_cleanup: $1 : ensemble absent: $3" >&2
        exit 1
    fi
    printf '%s %s %s\n' "$1" "$2" "$3" >> "$work/files"
    shift 3
done

LC_ALL=C sort -k2,2nr -k1,1 "$work/files" > "$work/order"

# covers[l] : combien de fichiers restants couvrent la ligne l.
awk '
    NR == FNR { name[FNR] = $1; set[FNR] = $3; n = FNR; next }
    END {
        for (i = 1; i <= n; i++) {
            while ((getline line < set[i]) > 0) {
                if (line != "" && !((i, line) in has)) {
                    has[i, line] = 1
                    lines[i, ++count[i]] = line
                    covers[line]++
                }
            }
            close(set[i])
        }
        for (i = 1; i <= n; i++) {
            redundant = 1
            for (j = 1; j <= count[i]; j++) {
                if (covers[lines[i, j]] < 2) { redundant = 0; break }
            }
            if (redundant) {
                for (j = 1; j <= count[i]; j++) covers[lines[i, j]]--
                print name[i]
            }
        }
    }' "$work/order"
