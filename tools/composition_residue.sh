#!/bin/sh
# usage: composition_residue.sh [--detail | --check <cat>[,<cat>...] | --champs] [dossier...]
# Compte, dans les .c/.h des dossiers (defaut : src tests, chemins imprimes
# tels que recus), les lignes de chaque categorie du residu de la phase 2,
# commentaires compris ; une ligne compte dans chaque categorie qu'elle touche.
# atteinte-à-prouver compte les lignes utiles de COMPOSITION_ATTEINTE ;
# inclusion-mutuelle compte des paires de modules hors tests/ ; au pied et a
# --check, ses lignes sont les #include qui realisent ces paires.
#   sans option : "<cat>\t<lignes>\t<fichiers>" par categorie non nulle
#                 ("<paires>\t<modules>" pour inclusion-mutuelle), puis
#                 "résidu 2 : N ligne(s), M fichier(s)" (couples fichier:ligne
#                 et fichiers distincts des categories de SORTIE) ; code 0
#                 quel que soit le compte.
#   --detail    : "<cat>\t<fichier>\t<lignes>" par couple non nul,
#                 "inclusion-mutuelle\t<a>\t<b>" par paire, sous void-ptr les
#                 "void-ptr-r2\t<fonction>\t<amont>" de COMPOSITION_R2, la
#                 ligne "info" de pico8_game_data.h s'il est lu, puis le meme
#                 pied ; code 0.
#   --check     : "<cat>: <fichier>:<ligne>:<texte>" par occurrence des
#                 categories nommees, puis
#                 "résidu gardé (<cats>) : N ligne(s)" ; code 1 si N > 0.
#                 Toute categorie connue est acceptee.
#   --champs    : releve de la regle 1, "<type>\t<membre>\t<ecritures>\t<hors
#                 constructeur>" par membre des types d'entite des .h hors
#                 tests/ (ni header, ni parente enchassee, ni pointeur de
#                 fonction), trie, puis "champs : N ; 0 écriture : K ; 0 hors
#                 constructeur : M" ; code 0. Une ecriture est une ligne d'un
#                 .c hors tests/ qui affecte, incremente ou decremente
#                 (->|.)<membre>, copie du defaut pico8_X_start.<membre>
#                 retiree ; elle est hors constructeur sauf dans une fonction
#                 qui finit par _new ou _init. Recherche par nom :
#                 un majorant, un membre court compte pour tous ses types.
# Vide (sortie de la phase 2) : aucune ligne de categorie de SORTIE, et le pied
# "résidu 2 : 0 ligne(s), 0 fichier(s)".
# Code 2 sur option ou categorie inconnue, dossier absent, aucun .c/.h, liste
# COMPOSITION_ATTEINTE ou COMPOSITION_R2 absente, echec de grep, sed ou awk :
# une mesure sur rien passerait pour vide.

set -u

CATEGORIES="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle"

# Categories comptees dans le pied.
SORTIE="parente lien-parent accesseurs-q6 défauts nu aiguillage-nom un-ou-faux void-ptr ptrs-entité registres noyau-inclut atteinte-à-prouver inclusion-mutuelle"

fail() {
    echo "composition_residue: $1" >&2
    exit 2
}

pattern() {
    case $1 in
        lien-parent) printf '%s\n' '\bpico8_[a-z0-9_]+_of\b|\b[a-z0-9_]*(emitter|probe)_of\b|(->|\.|\(\*)parent\b' ;;
        accesseurs-q6) printf '%s\n' '(\b|_)(get_(nu|fa|ra|p|kq|ql|fg|mt|fx|p_)|set_m_)\b' ;;
        défauts) printf '%s\n' '\bpico8_[a-z0-9_]+_start(_t)?\b' ;;
        nu) printf '%s\n' '\bnu\b|get_nu\b|_nu_(tags|items)\b' ;;
        aiguillage-nom) printf '%s\n' '\bhp_(pose|ho)\b|\bpico8_ho_gp_has\b|\bpico8_hp_(mr|gp)_names\b|(->|\.)(mr|gp)\b' ;;
        un-ou-faux) printf '%s\n' '\b(pk|fp)_is_bool\b' ;;
        noyau-inclut) printf '%s\n' '^#include "pico8_(game_data|map_tables|data_init)\.h"' ;;
        *) return 1 ;;
    esac
}

# pico8_state_of rend un enum depuis un nom d'etat : ce n'est pas une lignee.
LIEN_EXCLUSION='s/pico8_state_of//g'
# Le "nu" echappe de la fixture de test_data_residue.c est une chaine de
# test, pas le champ. Pas de \b dans ces sed, le sed BSD ne le connait pas.
NU_EXCLUSION='s/\\"nu\\"//g'

exclusion() {
    case $1 in
        lien-parent) printf '%s\n' "$LIEN_EXCLUSION" ;;
        nu) printf '%s\n' "$NU_EXCLUSION" ;;
    esac
}

# Le void *ctx des parcours porte un contexte, pas une entite. Ce sed-ci veut
# le sed GNU (\b), comme les grep du script.
VOID_EXCLUSION='s/void \*+(r_)?ctx\b//g'

# Listes pico8_ptrs_t d'un type d'entite qui ne portent pas d'entites
# (type.membre), chacune prouvee par son ecrivain : hy.invented des chaines
# (economy.c:284), jo.jy des entrees (economy_tiers.c:167), hp.i des
# enregistrements jy (research.c:233), jc.ts des particules (slipway_display.c:43).
PTRS_NON_ENTITE="pico8_hy_t.invented pico8_jo_t.jy pico8_hp_t.i pico8_jc_t.ts"

# registres : hors tests/ et pico8_ptrs.[ch], les points de declaration qui
# tiennent des entites par void * hors des types d'entite (que ptrs-entite
# lit). Membre d'une struct (ou d'un objet struct, meme dans une fonction) qui
# nomme pico8_ptrs_t ou un alias, quel que soit son declarateur (tableau,
# **, const, plusieurs noms, pointeur de fonction), ou de type void ** ou
# void *[N] ; membre qui rend void * par pointeur de fonction ; objet de portee
# fichier, statique de fonction, extern de .h, de ces types ou void * ;
# prototype ou definition inline de .h qui prend ou rend un pico8_ptrs_t, ou
# rend void **. Non vus : alias d'alias, void * derriere un typedef, struct
# imbriquee, membre void * simple, locale, macro. Exemptes, chacun prouve par
# son ecrivain : emitter.qy des particules (particle.c:65), le ji du contexte
# de planet_display.c des enregistrements jy (:79, :186) ; pico8_nv_ptrs sur
# hp.i (planet.c:319), pico8_gu_content_ptrs sur des chaines (tech_gate.c:26),
# pico8_jo_ex sur des enregistrements jy (planet_display.c:54).
REGISTRES_NON_ENTITE="pico8_emitter_t.qy pico8_jo_gh_ctx_t.ji pico8_nv_ptrs pico8_gu_content_ptrs pico8_jo_ex"

# En-tetes du noyau, par nom de fichier : noyau-inclut ne lit qu'eux.
NOYAU="pico8_struct.h pico8_ke.h pico8_entity.h"

ATTEINTE=${COMPOSITION_ATTEINTE:-tools/atteinte_a_prouver.txt}
R2=${COMPOSITION_R2:-tools/composition_r2_sites.txt}

known() {
    case " $CATEGORIES " in
        *" $1 "*) return 0 ;;
    esac
    return 1
}

mode=count
checked=
case ${1:-} in
    --detail) mode=detail; shift ;;
    --champs) mode=champs; shift ;;
    --check)
        mode=check
        [ $# -ge 2 ] || fail "--check sans categorie"
        checked=$2
        shift 2
        [ -n "$checked" ] || fail "--check sans categorie"
        for cat in $(printf '%s\n' "$checked" | tr ',' ' '); do
            known "$cat" || fail "categorie inconnue : $cat"
        done
        case ,$checked, in
            *,,*) fail "categorie vide dans : $checked" ;;
        esac
        ;;
    -*) fail "option inconnue : $1" ;;
esac
[ $# -gt 0 ] || set -- src tests

LC_ALL=C
export LC_ALL

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

for dir in "$@"; do
    [ -d "$dir" ] || fail "dossier absent : $dir"
done
find "$@" \( -name '*.c' -o -name '*.h' \) -type f | sort > "$work/files"
[ -s "$work/files" ] || fail "aucun .c/.h dans : $*"
[ -f "$ATTEINTE" ] || fail "liste d'atteinte absente : $ATTEINTE"
[ -f "$R2" ] || fail "liste R2 absente : $R2"

: > "$work/noyau"
while IFS= read -r file; do
    for name in $NOYAU; do
        [ "${file##*/}" = "$name" ] && printf '%s\n' "$file" >> "$work/noyau"
    done
done < "$work/files"

# registres lit hors tests/ et pico8_ptrs.[ch] ; REG_TYPES est pico8_ptrs_t
# et ses alias "typedef pico8_ptrs_t X;" de ces fichiers (un alias d'alias
# n'est pas suivi).
grep -v '/tests/' "$work/files" | grep -v '^tests/' | grep -v '/pico8_ptrs\.[ch]$' | grep -v '^pico8_ptrs\.[ch]$' > "$work/regfiles" || true
REG_TYPES=pico8_ptrs_t
if [ -s "$work/regfiles" ]; then
    grep -hE '^[[:space:]]*typedef[[:space:]]+(const[[:space:]]+)?pico8_ptrs_t[[:space:]*]+[A-Za-z_][A-Za-z0-9_]*[[:space:]]*;' $(cat "$work/regfiles") > "$work/reg.alias"
    [ $? -le 1 ] || fail "echec de grep"
    for a in $(sed -E 's/^.*[[:space:]*]([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*;.*$/\1/' "$work/reg.alias"); do
        REG_TYPES="$REG_TYPES|$a"
    done
fi

# Retire commentaires, chaines et caracteres d'une ligne, pour compter les
# accolades ; un commentaire ouvert passe a la ligne suivante (incomment).
CLEAN_AWK='
function clean(s,    out, c, q) {
    out = ""
    while (s != "") {
        if (incomment) {
            q = index(s, "*/")
            if (q == 0) return out
            s = substr(s, q + 2); incomment = 0; continue
        }
        c = substr(s, 1, 1)
        if (c == "/" && substr(s, 2, 1) == "*") { incomment = 1; s = substr(s, 3); out = out " "; continue }
        if (c == "/" && substr(s, 2, 1) == "/") return out
        if (c == "\"" || c == "\047") {
            s = substr(s, 2)
            while (s != "" && substr(s, 1, 1) != c) {
                if (substr(s, 1, 1) == "\\") s = substr(s, 2)
                s = substr(s, 2)
            }
            s = substr(s, 2); out = out c c; continue
        }
        out = out c; s = substr(s, 2)
    }
    return out
}
function braces(s,    t, n) {
    t = s; n = gsub(/\{/, "", t); depth += n
    t = s; n = gsub(/\}/, "", t); depth -= n
}
'

# Lecteur E. Un corps de struct de premier niveau s'ouvre par une ligne
# "struct ... {" hors de tout corps : une locale de fonction n'est pas un
# membre. E est la fermeture depuis pico8_struct_header_t par le premier
# membre non pointeur ; l'etiquette, le typedef de "} nom;" et celui de
# "typedef struct X Y;" nomment le meme type. Rend dans $work/parente et
# $work/ptrs-entité les lignes "<fichier>:<ligne>:<texte>", et dans
# $work/members les "<type>\t<membre>" que lit --champs.
read_structs() {
    awk -v non_entite=" $PTRS_NON_ENTITE " -v out_parente="$work/parente" \
        -v reg_non_entite=" $REGISTRES_NON_ENTITE " -v out_reg="$work/registres.structs" -v reg_types="$REG_TYPES" \
        -v out_ptrs="$work/ptrs-entité" -v out_members="$work/members" "$CLEAN_AWK"'
    FNR == 1 { depth = 0; incomment = 0; inbody = 0; base = 0 }
    {
        c = clean($0)
        if (!inbody && depth == 0 && c ~ /^typedef[ \t]+struct[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*;/) {
            split(c, w, /[ \t;]+/)
            alias_n++; alias_a[alias_n] = w[3]; alias_b[alias_n] = w[4]
        }
        if (!inbody && (depth == 0 && c ~ /^((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?struct([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/ ||
                        depth > 0 && c ~ /^[ \t]*static[ \t]+(const[ \t]+)?struct([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/)) {
            n++; inbody = 1; seen = 0; names[n] = " "; base = depth
            sfile[n] = FILENAME; sobj[n] = c ~ /^[ \t]*(static|extern)[ \t]/
            t = c; sub(/^[ \t]*((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?struct[ \t]*/, "", t); sub(/[ \t]*\{.*/, "", t)
            if (t != "") names[n] = names[n] t " "
            braces(c)
            if (depth == base) {
                inbody = 0
                if (match(c, /\}[ \t]*[A-Za-z_][A-Za-z0-9_]*/)) {
                    t = substr(c, RSTART + 1, RLENGTH - 1); sub(/^[ \t]*/, "", t)
                    names[n] = names[n] t " "
                }
                if (c ~ ("(^|[^A-Za-z0-9_])(" reg_types ")([^A-Za-z0-9_]|$)")) {
                    nr[n]++; rmem[n, nr[n]] = ""; rline[n, nr[n]] = FNR; rtext[n, nr[n]] = $0
                }
            }
            next
        }
        if (inbody && depth == base + 1 && c !~ /^[ \t]*$/ && c !~ /^[ \t]*\}/) {
            if (c !~ /[({]/ && match(c, /;/)) {
                t = substr(c, 1, RSTART - 1)
                nd = split(t, parts, ",")
                for (i = 1; i <= nd; i++) {
                    d = parts[i]; sub(/\[.*/, "", d); sub(/:.*/, "", d); sub(/[ \t]+$/, "", d)
                    if (!match(d, /[A-Za-z_][A-Za-z0-9_]*$/)) continue
                    nm_n[n]++; mname[n, nm_n[n]] = substr(d, RSTART, RLENGTH)
                    mfirst[n, nm_n[n]] = !seen
                }
            }
            if (!seen) {
                seen = 1
                if (c ~ /^[ \t]*(const[ \t]+)?(struct[ \t]+)?[A-Za-z_][A-Za-z0-9_]*[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*(\[[^]]*\])?[ \t]*;/) {
                    t = c; sub(/^[ \t]*(const[ \t]+)?(struct[ \t]+)?/, "", t); sub(/[ \t].*/, "", t)
                    first[n] = t; firstline[n] = FNR; firsttext[n] = $0
                }
            }
            if ($0 ~ /^[ \t]+(const[ \t]+)?pico8_ptrs_t[ \t]*\*?[ \t]*[a-z0-9_]+;/) {
                t = $0; sub(/;.*/, "", t); sub(/.*[ \t*]/, "", t)
                np[n]++; pmem[n, np[n]] = t; pline[n, np[n]] = FNR; ptext[n, np[n]] = $0
            }
            if (c ~ /^[ \t]*(const[ \t]+)?void[ \t]*(\*[ \t]*)+\([ \t]*\*/) {
                t = c; sub(/\).*/, "", t); sub(/.*[ \t*(]/, "", t)
                nr[n]++; rmem[n, nr[n]] = t; rline[n, nr[n]] = FNR; rtext[n, nr[n]] = $0
            } else if (c ~ /;/ &&
                       (c ~ ("(^|[^A-Za-z0-9_])(" reg_types ")([^A-Za-z0-9_]|$)") ||
                        c !~ /\(/ && c ~ /^[ \t]*(const[ \t]+)?void[ \t]*\*[ \t]*(const[ \t]+)?(\*|[A-Za-z_][A-Za-z0-9_]*[ \t]*\[)/)) {
                t = c; sub(/;.*/, "", t); sub(/^[ \t]*(const[ \t]+)?[A-Za-z_][A-Za-z0-9_]*/, "", t)
                nd = split(t, parts, ","); t = ""
                for (i = 1; i <= nd; i++) {
                    d = parts[i]; sub(/\[.*/, "", d); sub(/[ \t]+$/, "", d)
                    if (match(d, /[A-Za-z_][A-Za-z0-9_]*$/)) t = t substr(d, RSTART, RLENGTH) " "
                }
                nr[n]++; rmem[n, nr[n]] = t; rline[n, nr[n]] = FNR; rtext[n, nr[n]] = $0
            }
        }
        braces(c)
        if (inbody && depth == base) {
            inbody = 0
            if (match(c, /\}[ \t]*[A-Za-z_][A-Za-z0-9_]*/)) {
                t = substr(c, RSTART + 1, RLENGTH - 1); sub(/^[ \t]*/, "", t)
                names[n] = names[n] t " "
            }
        }
    }
    function in_e(k,    m, i, nm) {
        m = split(names[k], nm, " ")
        for (i = 1; i <= m; i++) if (nm[i] in e) return 1
        return 0
    }
    END {
        e["pico8_struct_header_t"] = 1
        do {
            grown = 0
            for (k = 1; k <= n; k++) {
                if (sobj[k] || !(first[k] in e) || in_e(k)) continue
                m = split(names[k], nm, " ")
                for (i = 1; i <= m; i++) e[nm[i]] = 1
                grown = 1
            }
            for (i = 1; i <= alias_n; i++) {
                if ((alias_a[i] in e) != (alias_b[i] in e)) {
                    e[alias_a[i]] = 1; e[alias_b[i]] = 1; grown = 1
                }
            }
        } while (grown)
        printf "" > out_parente
        printf "" > out_ptrs
        printf "" > out_members
        printf "" > out_reg
        for (i = 1; i <= alias_n; i++) alias_of[alias_a[i]] = alias_b[i]
        for (k = 1; k <= n; k++) {
            if (!sobj[k] && first[k] in e && first[k] != "pico8_struct_header_t")
                print sfile[k] ":" firstline[k] ":" firsttext[k] > out_parente
            if (!in_e(k) && sfile[k] !~ /(^|\/)tests\// && sfile[k] !~ /(^|\/)pico8_ptrs\.[ch]$/) {
                m = split(names[k], nm, " ")
                for (j = 1; j <= nr[k]; j++) {
                    q = split(rmem[k, j], mm, " "); left = q
                    for (h = 1; h <= q; h++)
                        for (i = 1; i <= m; i++) if (index(reg_non_entite, " " nm[i] "." mm[h] " ")) { left--; break }
                    if (left > 0 || q == 0) print sfile[k] ":" rline[k, j] ":" rtext[k, j] > out_reg
                }
            }
            if (!in_e(k) || sobj[k]) continue
            if (sfile[k] ~ /\.h$/ && sfile[k] !~ /(^|\/)tests\//) {
                m = split(names[k], nm, " "); type = nm[m]
                if (type in alias_of) type = alias_of[type]
                for (j = 1; j <= nm_n[k]; j++) {
                    if (mname[k, j] == "header") continue
                    if (mfirst[k, j] && first[k] in e) continue
                    print type "\t" mname[k, j] > out_members
                }
            }
            for (j = 1; j <= np[k]; j++) {
                m = split(names[k], nm, " "); skip = 0
                for (i = 1; i <= m; i++) if (index(non_entite, " " nm[i] "." pmem[k, j] " ")) skip = 1
                if (!skip) print sfile[k] ":" pline[k, j] ":" ptext[k, j] > out_ptrs
            }
        }
    }' $(cat "$work/files") || fail "echec de awk"
}

# Lecteur de prototypes : dans les .h, une declaration de premier niveau,
# jointe jusqu'a son ";", comptee a sa premiere ligne. Un membre de struct
# (table de methodes, new_entity) n'est pas de premier niveau.
read_prototypes() {
    sep=$(printf '\001')
    grep -E '\.h$' "$work/files" | grep -v '/pico8_ptrs\.h$' | grep -v '^pico8_ptrs\.h$' > "$work/headers" || true
    : > "$work/void-ptr"
    [ -s "$work/headers" ] || return 0
    awk -v sep="$sep" "$CLEAN_AWK"'
    FNR == 1 { depth = 0; incomment = 0; acc = "" }
    {
        c = clean($0)
        if (acc == "" && depth == 0 && $0 ~ /^[a-z]/) { start = FNR; text = $0; acc = " " }
        if (acc != "") {
            acc = acc " " c
            if (acc ~ /[;{]/) {
                if (acc !~ /\{/ || index(acc, ";") < index(acc, "{"))
                    print FILENAME ":" start ":" text sep acc
                acc = ""
            }
        }
        braces(c)
    }' $(cat "$work/headers") > "$work/protos" || fail "echec de awk"
    cut -d"$sep" -f1 "$work/protos" > "$work/protos.where"
    cut -d"$sep" -f2- "$work/protos" | sed -E "$VOID_EXCLUSION" > "$work/protos.text" || fail "echec de sed"
    paste -d"$sep" "$work/protos.where" "$work/protos.text" | awk -F"$sep" '
    {
        t = $2
        if (t ~ /^[ \t]*typedef/ || index(t, "void *") == 0) next
        if (!match(t, /(^|[^A-Za-z0-9_])pico8_[a-z0-9_]+\(/)) next
        name = substr(t, RSTART, RLENGTH - 1); sub(/^[^p]*/, "", name)
        if (name ~ /_dispatch$/ || name ~ /_get_[a-z0-9_]+$/) next
        print $1
    }' > "$work/void-ptr" || fail "echec de awk"
}

# registres, suite : objets pico8_ptrs_t de portee fichier des .c et
# prototypes des .h (hors pico8_ptrs.h), hors tests/, joints comme ceux de
# void-ptr ; un nom de REGISTRES_NON_ENTITE est exempte.
read_registres() {
    [ -f "$work/parente" ] || read_structs
    : > "$work/registres.decl"
    if [ -s "$work/regfiles" ]; then
        awk -v non=" $REGISTRES_NON_ENTITE " -v reg_types="$REG_TYPES" "$CLEAN_AWK"'
        FNR == 1 { depth = 0; incomment = 0; acc = "" }
        {
            c = clean($0)
            if (acc == "" && ((depth == 0 && $0 ~ /^[a-z]/) || (depth > 0 && c ~ /^[ \t]*static[ \t]/))) {
                start = FNR; text = $0; acc = " "; local = depth > 0
            }
            if (acc != "") {
                acc = acc " " c
                if (acc ~ /[;{=]/) {
                    fn = !local && match(acc, /[A-Za-z_][A-Za-z0-9_]*[ \t]*\(/)
                    if (fn) { name = substr(acc, RSTART, RLENGTH); sub(/[ \t]*\($/, "", name); ret = substr(acc, 1, RSTART - 1) }
                    else { t = acc; sub(/[ \t]*(\[[^]]*\])?[ \t]*[;=].*/, "", t); sub(/.*[ \t*]/, "", t); name = t }
                    ok = acc !~ /^[ \t]*typedef/ &&
                         (acc ~ ("(^|[^A-Za-z0-9_])(" reg_types ")([^A-Za-z0-9_]|$)") ||
                          (!fn && acc ~ /(^|[^A-Za-z0-9_])void[ \t]*\*/) ||
                          (fn && ret ~ /(^|[^A-Za-z0-9_])void[ \t]*\*[ \t]*(const[ \t]+)?\*/))
                    keep = FILENAME ~ /\.h$/ || !fn
                    if (ok && keep && !index(non, " " name " ")) print FILENAME ":" start ":" text
                    acc = ""
                }
            }
            braces(c)
        }' $(cat "$work/regfiles") > "$work/registres.decl" || fail "echec de awk"
    fi
    cat "$work/registres.structs" "$work/registres.decl" | sort -u -t: -k1,1 -k2,2n > "$work/registres"
}

# Paires {a, b} de modules hors tests/ ou chacun inclut l'en-tete de l'autre,
# dans $work/inclusion-mutuelle.pairs ; dans $work/inclusion-mutuelle, les
# lignes #include qui realisent ces paires, au format des autres categories.
read_inclusions() {
    grep -v '/tests/' "$work/files" | grep -v '^tests/' > "$work/modfiles" || true
    : > "$work/inclusion-mutuelle.pairs"
    : > "$work/inclusion-mutuelle"
    [ -s "$work/modfiles" ] || return 0
    status=0
    grep -HnE '^#include "[^"]+\.h"' $(cat "$work/modfiles") > "$work/includes" || status=$?
    [ "$status" -le 1 ] || fail "echec de grep"
    awk -v pairs="$work/inclusion-mutuelle.pairs" '
    NR == FNR { f = $0; sub(/.*\//, "", f); sub(/\.[ch]$/, "", f); mod[f] = 1; next }
    {
        path = $0; sub(/:.*/, "", path)
        a = path; sub(/.*\//, "", a); sub(/\.[ch]$/, "", a)
        b = $0; sub(/^[^:]*:[0-9]+:#include "/, "", b); sub(/\.h".*/, "", b)
        if (!(b in mod)) next
        edge[a, b] = 1
        n++; line[n] = $0; from[n] = a; to[n] = b
    }
    END {
        for (k in edge) {
            split(k, ab, SUBSEP)
            if (ab[1] < ab[2] && ((ab[2], ab[1]) in edge)) print ab[1] "\t" ab[2] > pairs
        }
        for (i = 1; i <= n; i++)
            if (to[i] != from[i] && ((to[i], from[i]) in edge)) print line[i]
    }' "$work/modfiles" "$work/includes" > "$work/inclusion-mutuelle" || fail "echec de awk"
    sort "$work/inclusion-mutuelle.pairs" > "$work/sorted" && mv "$work/sorted" "$work/inclusion-mutuelle.pairs"
    sort -t: -k1,1 -k2,2n "$work/inclusion-mutuelle" > "$work/sorted" && mv "$work/sorted" "$work/inclusion-mutuelle"
}

# Rend dans $work/<cat> les lignes "<fichier>:<ligne>:<texte>", dans l'ordre
# des fichiers puis des lignes. grep rend 1 quand rien ne correspond.
# L'exclusion est un sed sur le texte, jamais un filtre sur la ligne entiere :
# un filtre cacherait une vraie occurrence voisine.
collect() {
    case $1 in
        parente | ptrs-entité)
            [ -f "$work/parente" ] || read_structs
            sort -t: -k1,1 -k2,2n "$work/$1" > "$work/sorted" && mv "$work/sorted" "$work/$1"
            return 0 ;;
        void-ptr) read_prototypes; return 0 ;;
        registres) read_registres; return 0 ;;
        inclusion-mutuelle) read_inclusions; return 0 ;;
        atteinte-à-prouver)
            status=0
            grep -nE '^[^#[:space:]]' "$ATTEINTE" > "$work/numbers" || status=$?
            [ "$status" -le 1 ] || fail "echec de grep"
            sed "s|^|$ATTEINTE:|" "$work/numbers" > "$work/$1"
            return 0 ;;
    esac
    list="$work/files"
    [ "$1" = noyau-inclut ] && list="$work/noyau"
    : > "$work/$1"
    [ -s "$list" ] || return 0
    status=0
    excl=$(exclusion "$1")
    if [ -z "$excl" ]; then
        grep -HnE "$(pattern "$1")" $(cat "$list") > "$work/$1" || status=$?
        [ "$status" -le 1 ] || fail "echec de grep"
        return 0
    fi
    grep -lE "$(pattern "$1")" $(cat "$list") > "$work/candidates" || status=$?
    [ "$status" -le 1 ] || fail "echec de grep"
    while IFS= read -r file; do
        status=0
        sed -E "$excl" "$file" > "$work/excluded" || fail "echec de sed"
        grep -nE "$(pattern "$1")" "$work/excluded" > "$work/numbers" || status=$?
        [ "$status" -le 1 ] || fail "echec de grep"
        cut -d: -f1 "$work/numbers" > "$work/numbers.cut"
        awk -v f="$file" 'NR == FNR { keep[$1] = 1; next } FNR in keep { print f ":" FNR ":" $0 }' \
            "$work/numbers.cut" "$file" >> "$work/$1"
    done < "$work/candidates"
}

# Releve de la regle 1. La fonction d'une ligne est la derniere definition de
# premier niveau, signature jointe jusqu'a son "{" final, close par la "}" de
# colonne 0. Hors de toute fonction (initialiseur de fichier, dont les
# defauts pico8_X_start), une ecriture est hors constructeur : le releve est
# un majorant, il ne declare pas constant ce qu'un initialiseur ecrit.
champs() {
    read_structs
    sort -u "$work/members" > "$work/members.sorted"
    grep -E '\.c$' "$work/files" | grep -v '/tests/' | grep -v '^tests/' > "$work/sources" || true
    awk -F'\t' '
    FILENAME == ARGV[1] { want[$2] = 1; next }
    FNR == 1 { fn = ""; pend = "" }
    {
        line = $0
        if (line ~ /^}/) { fn = ""; pend = "" }
        else if (pend != "") {
            if (line ~ /\)[ \t]*\{[ \t]*$/) { fn = pend; pend = "" }
            else if (line ~ /;/) pend = ""
        } else if (line ~ /^[a-z]/ && match(line, /[A-Za-z_][A-Za-z0-9_]*\(/)) {
            name = substr(line, RSTART, RLENGTH - 1)
            if (line ~ /\)[ \t]*\{[ \t]*$/) fn = name
            else if (line !~ /[;=]/) pend = name
        }
        split("", done)
        rest = line
        while (match(rest, /(->|\.)[A-Za-z_][A-Za-z0-9_]*/)) {
            m = substr(rest, RSTART, RLENGTH); sub(/^(->|\.)/, "", m)
            rest = substr(rest, RSTART + RLENGTH)
            if (!(m in want) || (m in done)) continue
            done[m] = 1
            s = line
            gsub("=[ \t]*pico8_[a-z0-9_]+_start\\." m "([^A-Za-z0-9_]|$)", " ", s)
            if (s ~ ("(->|\\.)" m "[ \t]*(=[^=]|\\+=|-=|\\*=|/=|\\+\\+|--)") ||
                s ~ ("(\\+\\+|--)[A-Za-z0-9_>.()-]*(->|\\.)" m "([^A-Za-z0-9_]|$)")) {
                w[m]++
                if (fn !~ /(_new|_init)$/) h[m]++
            }
        }
    }
    END {
        for (m in want) printf "%s\t%d\t%d\n", m, w[m], h[m] > (ARGV[1] ".counts")
    }' "$work/members.sorted" $(cat "$work/sources") || fail "echec de awk"
    awk -F'\t' '
    NR == FNR { c[$1] = $2 "\t" $3; next }
    { print $1 "\t" $2 "\t" c[$2] }' "$work/members.sorted.counts" "$work/members.sorted" \
        | sort -t"$(printf '\t')" -k1,1 -k2,2 > "$work/champs" || fail "echec de awk"
    cat "$work/champs"
    awk -F'\t' '{ n++; if ($3 == 0) k++; if ($4 == 0) z++ }
    END { printf "champs : %d ; 0 écriture : %d ; 0 hors constructeur : %d\n", n, k, z }' "$work/champs"
}

if [ "$mode" = champs ]; then
    champs
    exit 0
fi

if [ "$mode" = check ]; then
    : > "$work/all"
    for cat in $(printf '%s\n' "$checked" | tr ',' ' '); do
        collect "$cat"
        sed "s|^|$cat: |" "$work/$cat"
        cut -d: -f1,2 "$work/$cat" >> "$work/all"
    done
    n=$(sort -u "$work/all" | wc -l | tr -d ' ')
    echo "résidu gardé ($checked) : $n ligne(s)"
    [ "$n" -eq 0 ]
    exit
fi

: > "$work/all"
for cat in $CATEGORIES; do
    collect "$cat"
    if [ -s "$work/$cat" ]; then
        case " $SORTIE " in
            *" $cat "*) cut -d: -f1,2 "$work/$cat" >> "$work/all" ;;
        esac
        if [ "$cat" = inclusion-mutuelle ] && [ "$mode" = detail ]; then
            sed 's/^/inclusion-mutuelle\t/' "$work/$cat.pairs"
        elif [ "$cat" = inclusion-mutuelle ]; then
            pairs=$(wc -l < "$work/$cat.pairs" | tr -d ' ')
            modules=$(tr '\t' '\n' < "$work/$cat.pairs" | sort -u | wc -l | tr -d ' ')
            printf '%s\t%s\t%s\n' "$cat" "$pairs" "$modules"
        elif [ "$mode" = detail ]; then
            cut -d: -f1 "$work/$cat" | uniq -c | awk -v c="$cat" '{ print c "\t" $2 "\t" $1 }'
        else
            lines=$(wc -l < "$work/$cat" | tr -d ' ')
            files=$(cut -d: -f1 "$work/$cat" | sort -u | wc -l | tr -d ' ')
            printf '%s\t%s\t%s\n' "$cat" "$lines" "$files"
        fi
    fi
    if [ "$mode" = detail ] && [ "$cat" = void-ptr ]; then
        grep -E '^[^#[:space:]]' "$R2" | awk -F'\t' '{ print "void-ptr-r2\t" $1 "\t" $3 }'
    fi
done
if [ "$mode" = detail ]; then
    grep '/pico8_game_data\.h$\|^pico8_game_data\.h$' "$work/files" | while IFS= read -r file; do
        printf 'info\t%s\t%s lignes\n' "$file" "$(wc -l < "$file" | tr -d ' ')"
    done
fi
lines=$(sort -u "$work/all" | wc -l | tr -d ' ')
files=$(cut -d: -f1 "$work/all" | sort -u | wc -l | tr -d ' ')
echo "résidu 2 : $lines ligne(s), $files fichier(s)"
