#!/bin/sh
# usage: class_coverage.sh <outil pico8_class_coverage> <dossier> [<exemptions>]
# Rejoue chaque <dossier>/*events.json (sans descendre dans les sous-dossiers),
# un processus par rejeu. Rend 1 si une classe est à 0 sans être exemptée.

set -u

tool=$1
dir=$2
exempt=${3:-}

work=$(mktemp -d "${TMPDIR:-/tmp}/class_coverage.XXXXXX")
trap 'rm -rf "$work"' EXIT

run=0
crashed=0
for events in "$dir"/*events.json; do
    [ -f "$events" ] || continue
    "$tool" "$events" > "$work/out" 2> "$work/err"
    status=$?
    if [ "$status" -eq 2 ]; then
        echo "illisible: $events"
        continue
    fi
    if [ "$status" -ne 0 ]; then
        echo "échec: $events (code $status)"
        sed 's/^/  /' "$work/err"
        crashed=$((crashed + 1))
        continue
    fi
    run=$((run + 1))
    cat "$work/out" >> "$work/all"
done
touch "$work/all"

if [ -n "$exempt" ]; then
    cp "$exempt" "$work/exempt"
else
    : > "$work/exempt"
fi

# FILENAME et non NR == FNR : un fichier d'exemptions vide ferait lire les
# comptes comme des exemptions.
awk -v run="$run" -v exempt_file="$work/exempt" '
    FILENAME == exempt_file {
        if ($1 != "") { reason = $0; sub(/^[^ ]+ +/, "", reason); exempt[$1] = reason }
        next
    }
    {
        total[$1] += $2
        if ($2 > 0) replays[$1]++
    }
    END {
        for (name in total) {
            if (name == "inconnue" && total[name] == 0) continue
            line = sprintf("%s  %d  %d/%d", name, total[name], replays[name] + 0, run)
            if (name in exempt) line = line "  exemptée (" exempt[name] ")"
            print total[name] "\t" line
        }
    }' "$work/exempt" "$work/all" > "$work/lines"

sort -t "$(printf '\t')" -k1,1n -k2,2 "$work/lines" | cut -f2- | tee "$work/sorted"

missing=$(awk '$2 == 0 && $4 != "exemptée" && $1 != "inconnue" { printf "%s%s", sep, $1; sep = " " }' "$work/sorted")
if [ "$crashed" -gt 0 ]; then
    echo "couverture: ÉCHEC -- $crashed rejeu(x) en échec"
    exit 1
fi
if [ "$run" -eq 0 ]; then
    echo "couverture: ÉCHEC -- aucun rejeu lisible dans $dir"
    exit 1
fi
if [ -n "$missing" ]; then
    echo "couverture: ÉCHEC -- jamais instanciées hors exemption : $missing ($run rejeu(x))"
    exit 1
fi
echo "couverture: OK -- chaque classe non exemptée est instanciée ($run rejeu(x))"
exit 0
