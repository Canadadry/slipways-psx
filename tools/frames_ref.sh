#!/bin/sh
# usage: frames_ref.sh <outil pico8_frames> <dossier des captures> <dossier des worktrees>
#                      [<events> <reference .sha256> <empreinte courante> <capture>]...
# Pour chaque rejeu divergent, rejoue le fichier d'evenements au commit qui a
# pose sa reference et laisse la capture obtenue dans <dossier>/<events>.ref.bin.
# Rend non nul si une reference n'est pas commitee, si un vieux commit ne
# compile pas ou si un rejeu echoue.

set -u
LC_ALL=C
export LC_ALL

FRAME_BYTES=8192

tool=$1
frames_dir=$2
worktrees_dir=$3
shift 3

root=$(pwd)
worktree=
# Une ligne par rejeu divergent : <commit> <events> <capture>.
pending=$root/$frames_dir/frames-ref.pending

remove_worktree() {
    if [ -n "$worktree" ]; then
        git worktree remove --force "$worktree" > /dev/null 2>&1 || rm -rf "$worktree"
        git worktree prune
        worktree=
    fi
}

cleanup() {
    remove_worktree
    rm -f "$pending"
}

trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

mkdir -p "$root/$frames_dir"
: > "$pending"

while [ $# -ge 4 ]; do
    events=$1
    reference=$2
    current=$3
    capture=$4
    shift 4

    [ -f "$reference" ] || continue
    if [ ! -f "$current" ]; then
        echo "e2e-ref: $events : empreinte courante absente ($current)" >&2
        exit 1
    fi
    [ "$(cat "$reference")" = "$(cat "$current")" ] && continue

    # Hors du depot, ou pas encore commitee, la ligne n'a pas de commit :
    # git blame echoue, ou la designe par un hash tout a zero.
    commit=$(git blame --porcelain -L 1,1 -- "$reference" 2> /dev/null | sed -n '1s/ .*//p')
    case $commit in
        '' | *[!0]*) ;;
        *) commit= ;;
    esac
    if [ -z "$commit" ]; then
        echo "e2e-ref: $events : $reference n'est pas commitee -- aucune reference a reconstruire." >&2
        echo "e2e-ref:   la commiter avant de lancer e2e-ref : git status -- $reference" >&2
        exit 1
    fi
    echo "$commit $events $capture" >> "$pending"
done

if [ ! -s "$pending" ]; then
    echo "e2e-ref: aucun rejeu divergent -- rien a reconstruire."
    exit 0
fi

# Rend la premiere frame qui differe entre deux captures, ou rien si elles
# sont identiques. Sur un prefixe commun, c'est la frame qui suit la fin de
# la plus courte.
first_divergent_frame() {
    out=$(cmp "$1" "$2" 2> /dev/null)
    status=$?
    [ $status -eq 0 ] && return 0
    if [ $status -ne 1 ]; then
        echo "e2e-ref: cmp $1 $2 a echoue" >&2
        return 1
    fi
    byte=$(printf '%s\n' "$out" | sed -n -e 's/.* differ: byte \([0-9][0-9]*\).*/\1/p' -e 's/.* differ: char \([0-9][0-9]*\).*/\1/p')
    if [ -n "$byte" ]; then
        echo $(( (byte - 1) / $FRAME_BYTES ))
        return 0
    fi
    size1=$(wc -c < "$1" | tr -d ' ')
    size2=$(wc -c < "$2" | tr -d ' ')
    if [ "$size1" -lt "$size2" ]; then shorter=$size1; else shorter=$size2; fi
    echo $(( shorter / $FRAME_BYTES ))
}

for commit in $(cut -d' ' -f1 "$pending" | sort -u); do
    worktree=$root/$worktrees_dir/$commit
    if [ -e "$worktree" ]; then
        git worktree remove --force "$worktree" > /dev/null 2>&1 || rm -rf "$worktree"
        git worktree prune
    fi
    mkdir -p "$(dirname "$worktree")"
    echo "e2e-ref: reconstruction de la reference $commit"
    if ! git worktree add --detach "$worktree" "$commit" > /dev/null; then
        echo "e2e-ref: worktree de $commit impossible" >&2
        exit 1
    fi
    if ! (cd "$worktree" && MAKEFLAGS= MFLAGS= make tmp/pico8_replay < /dev/null > /dev/null); then
        echo "e2e-ref: le pilote de rejeu ne compile pas au commit $commit" >&2
        exit 1
    fi

    # Avant le retrait du raccourci de difficulte, le pilote ne partait de
    # l'ecran titre qu'avec ce drapeau ; les e2e partent tous du titre.
    flag=
    grep -q -e '--from-title' "$worktree/src/main.c" && flag=--from-title

    while read -r line_commit events capture; do
        [ "$line_commit" = "$commit" ] || continue
        ref_capture=$frames_dir/$events.ref.bin
        mkdir -p "$(dirname "$root/$ref_capture")"
        rm -f "$root/$ref_capture"
        if [ ! -f "$worktree/$events" ]; then
            echo "e2e-ref: $events n'existe pas au commit $commit" >&2
            exit 1
        fi
        if ! (cd "$worktree" && ./tmp/pico8_replay $flag "$events" "$root/$ref_capture" < /dev/null > /dev/null); then
            rm -f "$root/$ref_capture"
            echo "e2e-ref: $events : le rejeu echoue au commit $commit" >&2
            exit 1
        fi

        frame=$(first_divergent_frame "$ref_capture" "$capture") || exit 1
        if [ -z "$frame" ]; then
            echo "e2e-ref: $events : la capture reconstruite a $commit est identique a la capture courante."
            continue
        fi
        echo "e2e-ref: $events : premiere frame divergente $frame (reference $commit)"
        echo "e2e-ref:   reference : $tool frame $ref_capture $frame"
        echo "e2e-ref:   courante  : $tool frame $capture $frame"
    done < "$pending"

    remove_worktree
done

exit 0
