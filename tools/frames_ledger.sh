#!/bin/sh
# usage: frames_ledger.sh <depot> <registre> <releve> <repertoire des rapports>
# Confronte le registre des references d'empreintes (docs/plan-2.md §6.4) a
# l'historique git de <depot> depuis sa ligne "base <commit>". Lignes du
# registre, # en tete pour un commentaire :
#   <commit> | <etape> | pose
#   <commit> | <etape> | divergence | <bloc> | <ligne du releve>
# Une pose n'ajoute que des *.frames.hash ; une divergence en reecrit, et
# <repertoire>/<etape>.txt et <etape>-attribution.txt sont dans l'arbre de ce
# commit, ses fichiers DIVERGENT sont exactement les references reecrites, et
# la ligne citee figure mot pour mot dans une section "## <bloc> ..." du releve.
# Le repertoire des rapports est relatif au depot.
# Rend 1 sur erreur de registre, 2 si <depot> n'est pas la racine d'un depot
# git ou si la base est injoignable (clone superficiel) : un historique
# tronque passerait pour conforme.

set -u

repo=$1
ledger=$2
probe=$3
reports=$4

fatal() {
    echo "frames_ledger: $1" >&2
    exit 2
}

trim() {
    printf '%s\n' "$1" | sed 's/^[[:space:]]*//; s/[[:space:]]*$//'
}

[ -d "$repo" ] || fatal "depot introuvable : $repo"
[ -f "$ledger" ] || fatal "registre introuvable : $ledger"
[ -f "$probe" ] || fatal "releve introuvable : $probe"
top=$(git -C "$repo" rev-parse --show-toplevel 2>/dev/null) || fatal "pas un depot git : $repo"
[ "$top" = "$(cd "$repo" && pwd -P)" ] || fatal "pas la racine d'un depot git : $repo"

base=$(sed -n 's/^base[[:space:]][[:space:]]*\([^[:space:]]*\).*/\1/p' "$ledger" | head -n 1)
[ -n "$base" ] || fatal "registre sans ligne base"
base_full=$(git -C "$repo" rev-parse --verify --quiet "$base^{commit}") || fatal "base injoignable : $base"

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# Une ligne "<commit> <statut> <chemin>" par reference touchee ; un renommage
# garde son statut R suivi des deux chemins.
git -C "$repo" log --reverse -M --format='@%H' --name-status "$base_full..HEAD" -- '*.frames.hash' \
    | awk '/^@/ { c = substr($0, 2); next } NF { $1 = $1; print c, $0 }' > "$work/touched"
cut -d' ' -f1 "$work/touched" | awk '!seen[$0]++' > "$work/history"

errors=0
poses=0
divergences=0
: > "$work/listed"

report_error() {
    echo "$1 erreur : $2"
    errors=$((errors + 1))
}

while IFS= read -r line <&3; do
    case $line in
        '#'* | '' | base*) continue ;;
    esac
    commit=$(trim "$(echo "$line" | cut -d'|' -f1)")
    step=$(trim "$(echo "$line" | cut -d'|' -f2)")
    kind=$(trim "$(echo "$line" | cut -d'|' -f3)")
    block=$(trim "$(echo "$line" | cut -s -d'|' -f4)")
    cited=$(trim "$(echo "$line" | cut -s -d'|' -f5-)")
    label="$commit $step"
    full=$(git -C "$repo" rev-parse --verify --quiet "$commit^{commit}") || {
        report_error "$label" "commit introuvable"
        continue
    }
    echo "$full" >> "$work/listed"
    if ! grep -qxF "$full" "$work/history"; then
        report_error "$label" "ne touche aucun *.frames.hash depuis la base"
        continue
    fi
    grep "^$full " "$work/touched" | cut -d' ' -f2- > "$work/statuses"
    grep '^M ' "$work/statuses" | cut -d' ' -f2 | LC_ALL=C sort > "$work/rewritten"
    before=$errors
    while read -r status paths; do
        case $status in
            A | M) ;;
            *) report_error "$label" "statut $status non prevu par le registre : $paths" ;;
        esac
    done < "$work/statuses"
    case $kind in
        pose)
            poses=$((poses + 1))
            while IFS= read -r path; do
                report_error "$label" "pose qui reecrit $path"
            done < "$work/rewritten"
            ;;
        divergence)
            divergences=$((divergences + 1))
            [ -s "$work/rewritten" ] || report_error "$label" "divergence sans reference reecrite"
            for suffix in .txt -attribution.txt; do
                git -C "$repo" cat-file -e "$full:$reports/$step$suffix" 2>/dev/null \
                    || report_error "$label" "rapport absent du commit : $reports/$step$suffix"
            done
            if git -C "$repo" cat-file -e "$full:$reports/$step.txt" 2>/dev/null; then
                git -C "$repo" show "$full:$reports/$step.txt" \
                    | sed -n 's/^\(.*\) : DIVERGENT$/\1.frames.hash/p' | LC_ALL=C sort -u > "$work/declared"
                LC_ALL=C comm -23 "$work/rewritten" "$work/declared" > "$work/outside"
                LC_ALL=C comm -13 "$work/rewritten" "$work/declared" > "$work/stale"
                while IFS= read -r path; do
                    report_error "$label" "reecrite hors rapport : $path"
                done < "$work/outside"
                while IFS= read -r path; do
                    report_error "$label" "declaree divergente non reecrite : $path"
                done < "$work/stale"
            fi
            if [ -z "$block" ] || [ -z "$cited" ]; then
                report_error "$label" "divergence sans bloc ni ligne de releve"
            elif ! CITED=$cited awk -v b="$block" '
                    /^## / { inside = ($2 == b); next }
                    { sub(/[[:space:]]+$/, "") }
                    inside && $0 == ENVIRON["CITED"] { found = 1 }
                    END { exit !found }' "$probe"; then
                report_error "$label" "ligne de releve introuvable dans le bloc $block : $cited"
            fi
            ;;
        *)
            report_error "$label" "genre inconnu : $kind"
            ;;
    esac
    [ "$errors" -ne "$before" ] || echo "$label ok"
done 3< "$ledger"

while IFS= read -r full; do
    grep -qxF "$full" "$work/listed" || report_error "$full" "absent du registre"
done < "$work/history"

echo "frames-ledger commits $(wc -l < "$work/history" | tr -d ' ') poses $poses divergences $divergences erreurs $errors"
[ "$errors" -eq 0 ]
