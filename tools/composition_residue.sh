#!/bin/sh
# usage: composition_residue.sh [--detail | --check <cat>[,<cat>...] | --champs] [dossier...]
# Compte, dans les .c/.h des dossiers (defaut : src tests, chemins imprimes
# tels que recus), les lignes de chaque categorie du residu de la phase 2,
# commentaires compris ; une ligne compte dans chaque categorie qu'elle touche.
# atteinte-à-prouver compte les lignes utiles de COMPOSITION_ATTEINTE ;
# inclusion-mutuelle compte des paires de modules hors tests/ ; au pied et a
# --check, ses lignes sont les #include qui realisent ces paires.
# usage-mutuel compte les paires de modules hors tests/ dont chacun emploie
# un nom que l'en-tete de l'autre est seul a declarer, include ou detour ;
# ses lignes sont, par paire et par sens, le premier emploi.
#   sans option : "<cat>\t<lignes>\t<fichiers>" par categorie non nulle
#                 ("<paires>\t<modules>" pour inclusion-mutuelle et
#                 usage-mutuel), puis
#                 "résidu 2 : N ligne(s), M fichier(s)" (couples fichier:ligne
#                 et fichiers distincts des categories de SORTIE) ; code 0
#                 quel que soit le compte.
#   --detail    : "<cat>\t<fichier>\t<lignes>" par couple non nul,
#                 "<cat>\t<a>\t<b>" par paire d'inclusion-mutuelle et
#                 d'usage-mutuel, sous void-ptr les
#                 "void-ptr-r2\t<fonction>\t<amont>" de COMPOSITION_R2, la
#                 ligne "info" de game/game_data.h s'il est lu, puis le meme
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
#                 qui finit par _new, _init, _create ou _init_instance.
#                 Recherche par nom : un majorant, un membre court compte
#                 pour tous ses types.
# Vide (sortie de la phase 2) : aucune ligne de categorie de SORTIE, et le pied
# "résidu 2 : 0 ligne(s), 0 fichier(s)".
# Code 2 sur option ou categorie inconnue, dossier absent, aucun .c/.h, liste
# COMPOSITION_ATTEINTE ou COMPOSITION_R2 absente, echec de grep, sed ou awk :
# une mesure sur rien passerait pour vide.

set -u

CATEGORIES="parent parent-link header-accessors defaults nu name-dispatch one-or-false void-ptr void-self entity-ptrs registries core-includes reachability-to-prove mutual-inclusion mutual-usage"

# Categories comptees dans le pied.
FOOTER_CATEGORIES="parent parent-link header-accessors defaults nu name-dispatch one-or-false void-ptr void-self entity-ptrs registries core-includes reachability-to-prove mutual-inclusion mutual-usage"

fail() {
    echo "composition_residue: $1" >&2
    exit 2
}

pattern() {
    case $1 in
        parent-link) printf '%s\n' '\bpico8_[a-z0-9_]+_of\b|\b[a-z0-9_]*(emitter|probe)_of\b|(->|\.|\(\*)parent\b' ;;
        header-accessors) printf '%s\n' '(\b|_)(get_(nu|layer|pos|map_pos|hit_box|cursor_sprite|gpio_signal|infraspace|fold_anchor|is_dead)|set_is_hovered)\b' ;;
        defaults) printf '%s\n' '\bpico8_[a-z0-9_]+_start(_t)?\b' ;;
        nu) printf '%s\n' '\bnu\b|get_nu\b|_nu_(tags|items)\b' ;;
        name-dispatch) printf '%s\n' '\bhp_(pose|ho)\b|\bpico8_ho_gp_has\b|\bpico8_hp_(mr|gp)_names\b|(->|\.)(mr|gp)\b' ;;
        one-or-false) printf '%s\n' '\b(pk|fp)_is_bool\b' ;;
        void-self) printf '%s\n' '\bvoid[[:space:]]*\*[[:space:]]*(const[[:space:]]+)?self' ;;
        core-includes) printf '%s\n' '^#include "game/(game_data|map_tables|data_init)\.h"' ;;
        *) return 1 ;;
    esac
}

# entity_state_of rend un enum depuis un nom d'etat : ce n'est pas une lignee.
PARENT_LINK_EXCLUSION='s/entity_state_of//g'
# Le "nu" echappe de la fixture de test_data_residue.c est une chaine de
# test, pas le champ. Pas de \b dans ces sed, le sed BSD ne le connait pas.
NU_EXCLUSION='s/\\"nu\\"//g'

exclusion() {
    case $1 in
        parent-link) printf '%s\n' "$PARENT_LINK_EXCLUSION" ;;
        nu) printf '%s\n' "$NU_EXCLUSION" ;;
    esac
}

# void-self : un objet du jeu recu en void * sous le nom self (self_ptr
# compris), ou que la ligne le declare : membre, typedef, prototype,
# definition, locale ; const lu autour de l'etoile. Un void * d'un autre nom
# n'est pas lu.

# Le void *ctx des parcours porte un contexte, pas une entite. Ce sed-ci veut
# le sed GNU (\b), comme les grep du script.
VOID_EXCLUSION='s/void \*+(r_)?ctx\b//g'

# ptrs-entite : les membres d'un type d'entite que registres compterait hors
# d'un type d'entite (REG_TYPES par jeton, void **, void *[N], pointeur de
# fonction qui rend void *). Listes qui ne portent pas d'entites
# (type.membre), chacune prouvee par son ecrivain : hy.invented des chaines
# (economy.c:284), jo.jy des entrees (economy_tiers.c:167), hp.i des
# enregistrements jy (research.c:233), jc.ts des particules
# (slipway_display.c:43). L'exemption va nom par nom : une ligne compte des
# qu'un de ses noms, ou une declaration sans nom lisible, n'est pas exempte.
ENTITY_PTRS_EXEMPT="economy_t.invented planet_t.needs research_project_t.inputs slipway_t.transit_dots"

# registres : hors tests/ et engine/pointer_list.[ch], les points de declaration qui
# tiennent des entites par void * hors des types d'entite (que ptrs-entite
# lit). Membre d'une struct (ou d'un objet struct, meme dans une fonction) qui
# nomme pico8_ptrs_t ou un alias, quel que soit son declarateur (tableau,
# **, const, plusieurs noms, pointeur de fonction), ou de type void ** ou
# void *[N] ; membre qui rend void * par pointeur de fonction ; objet de portee
# fichier, statique de fonction, extern de .h, de ces types ou void * ;
# prototype ou definition inline de .h qui prend ou rend un pico8_ptrs_t, ou
# rend void **. const et volatile sont lus autour de void et de chaque * ;
# chaque declaration d'une ligne de membre est lue, et un membre sans ";"
# en fin de ligne est joint aux lignes suivantes, compte a sa premiere. Une
# struct ou une union dont l'accolade ouvre la ligne suivante est vue ;
# __attribute__((...)) a deux niveaux de parentheses internes et
# _Alignas(...) a un niveau sont retires avant lecture.
# Non vus : alias d'alias, void * derriere un typedef, struct ou union
# imbriquee sur plusieurs lignes, membre void * simple, locale, macro,
# attribut a un niveau de plus ; de deux declarations de portee fichier sur
# une ligne, seul le premier nom est lu pour l'exemption.
# Exemptes, chacun prouve par son ecrivain : emitter.qy des particules
# (particle.c:65), le ji du contexte de planet_display.c des enregistrements
# jy (:79, :186) ; pico8_nv_ptrs sur hp.i (planet.c:319),
# pico8_gu_content_ptrs sur des chaines (tech_gate.c:26), pico8_jo_ex sur des
# enregistrements jy (planet_display.c:54) ; pico8_jp_ptrs et pico8_la_ptrs
# sur des enregistrements jy (planet_display.c:186, planet_trade.c:128), les
# listes d'entites passant par pico8_jp_ents et pico8_ents_cat.
REGISTRIES_EXEMPT="emitter_t.particles planet_inspection_panel_ctx_t.unmet_needs collect_map_pointer_list collect_contains_string research_display_planet_resource_row collect_filter_pointer_list collect_concat_pointer_list"

# En-tetes du noyau, par chemin relatif a src/ : noyau-inclut ne lit qu'eux.
CORE_HEADERS="engine/entity_class.h engine/entity.h engine/entity_registry.h"

REACHABILITY=${COMPOSITION_REACHABILITY:-tools/reachability_questions.txt}
R2=${COMPOSITION_R2:-tools/composition_residue_sites.txt}

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
    --fields) mode=fields; shift ;;
    --check)
        mode=check
        [ $# -ge 2 ] || fail "--check without a category"
        checked=$2
        shift 2
        [ -n "$checked" ] || fail "--check without a category"
        for cat in $(printf '%s\n' "$checked" | tr ',' ' '); do
            known "$cat" || fail "unknown category: $cat"
        done
        case ,$checked, in
            *,,*) fail "empty category in: $checked" ;;
        esac
        ;;
    -*) fail "unknown option: $1" ;;
esac
[ $# -gt 0 ] || set -- src tests

LC_ALL=C
export LC_ALL

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

for dir in "$@"; do
    [ -d "$dir" ] || fail "missing directory: $dir"
done
find "$@" \( -name '*.c' -o -name '*.h' \) -type f | sort > "$work/files"
[ -s "$work/files" ] || fail "no .c or .h file in: $*"
[ -f "$REACHABILITY" ] || fail "missing reachability list: $REACHABILITY"
[ -f "$R2" ] || fail "missing R2 list: $R2"

: > "$work/core"
while IFS= read -r file; do
    for name in $CORE_HEADERS; do
        case /$file in */"$name") printf '%s\n' "$file" >> "$work/core" ;; esac
    done
done < "$work/files"

# registres lit hors tests/ et engine/pointer_list.[ch] ; REG_TYPES est pico8_ptrs_t
# et ses alias "typedef pico8_ptrs_t X;" de ces fichiers (un alias d'alias
# n'est pas suivi).
grep -v '/tests/' "$work/files" | grep -v '^tests/' | grep -v '/engine/pointer_list\.[ch]$' | grep -v '^engine/pointer_list\.[ch]$' > "$work/regfiles" || true
REG_TYPES=pointer_list_t
if [ -s "$work/regfiles" ]; then
    grep -hE '^[[:space:]]*typedef[[:space:]]+(const[[:space:]]+)?pointer_list_t[[:space:]*]+[A-Za-z_][A-Za-z0-9_]*[[:space:]]*;' $(cat "$work/regfiles") > "$work/reg.alias"
    [ $? -le 1 ] || fail "grep failed"
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
function unattr(s) {
    gsub(/__attribute__[ \t]*\(\(([^()]|\(([^()]|\([^()]*\))*\))*\)\)[ \t]*/, "", s)
    gsub(/_Alignas[ \t]*\(([^()]|\([^()]*\))*\)[ \t]*/, "", s)
    return s
}
function braces(s,    t, n) {
    t = s; n = gsub(/\{/, "", t); depth += n
    t = s; n = gsub(/\}/, "", t); depth -= n
}
'

# Lecteur E. Un corps de struct de premier niveau s'ouvre par une ligne
# "struct ... {" hors de tout corps : une locale de fonction n'est pas un
# membre. E est la fermeture depuis entity_t par le premier
# membre non pointeur ; l'etiquette, le typedef de "} nom;" et celui de
# "typedef struct X Y;" nomment le meme type. Rend dans $work/parente et
# $work/ptrs-entité les lignes "<fichier>:<ligne>:<texte>", et dans
# $work/members les "<type>\t<membre>" que lit --champs.
read_structs() {
    awk -v exempt=" $ENTITY_PTRS_EXEMPT " -v out_parent="$work/parent" \
        -v reg_exempt=" $REGISTRIES_EXEMPT " -v out_reg="$work/registries.structs" -v reg_types="$REG_TYPES" \
        -v out_ptrs="$work/entity-ptrs" -v out_members="$work/members" "$CLEAN_AWK"'
    FNR == 1 { depth = 0; incomment = 0; inbody = 0; base = 0; phead = ""; mpend = "" }
    {
        c = unattr(clean($0))
        if (phead != "") { if (c ~ /^[ \t]*\{/) c = phead " " c; phead = "" }
        if (!inbody && (depth == 0 && c ~ /^((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?(struct|union)([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*$/ ||
                        depth > 0 && c ~ /^[ \t]*static[ \t]+(const[ \t]+)?(struct|union)([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*$/)) {
            phead = c
            next
        }
        if (!inbody && depth == 0 && c ~ /^typedef[ \t]+struct[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*;/) {
            split(c, w, /[ \t;]+/)
            alias_n++; alias_a[alias_n] = w[3]; alias_b[alias_n] = w[4]
        }
        if (!inbody && (depth == 0 && c ~ /^((static|extern)[ \t]+)?(typedef[ \t]+)?(const[ \t]+)?(struct|union)([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/ ||
                        depth > 0 && c ~ /^[ \t]*static[ \t]+(const[ \t]+)?(struct|union)([ \t]+[A-Za-z_][A-Za-z0-9_]*)?[ \t]*\{/)) {
            n++; inbody = 1; seen = 0; names[n] = " "; base = depth; mpend = ""
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
            m = c; ml = FNR; mt = $0
            if (c ~ /^[ \t]*#|[{}]/) mpend = ""
            else {
                if (mpend != "") { m = mpend " " c; ml = mline; mt = mtext }
                else { mline = FNR; mtext = $0 }
                if (m ~ /;[ \t]*$/) mpend = ""
                else { mpend = m; m = "" }
            }
            ns = split(m, sg, ";")
            for (s = 1; s < ns; s++) {
                g = sg[s] ";"
                if (g ~ /^[ \t]*(const[ \t]+)?void[ \t]*(\*[ \t]*)+\([ \t]*\*/) {
                    t = g; sub(/\).*/, "", t); sub(/.*[ \t*(]/, "", t)
                    nr[n]++; rmem[n, nr[n]] = t; rline[n, nr[n]] = ml; rtext[n, nr[n]] = mt
                } else if (g ~ ("(^|[^A-Za-z0-9_])(" reg_types ")([^A-Za-z0-9_]|$)") ||
                           g !~ /\(/ && g ~ /^[ \t]*((const|volatile)[ \t]+)*void([ \t]+(const|volatile))*[ \t]*\*([ \t]*(const|volatile))*[ \t]*(\*|[A-Za-z_][A-Za-z0-9_]*[ \t]*\[)/) {
                    t = g; sub(/;.*/, "", t); sub(/^[ \t]*(const[ \t]+)?[A-Za-z_][A-Za-z0-9_]*/, "", t)
                    nd = split(t, parts, ","); t = ""
                    for (i = 1; i <= nd; i++) {
                        d = parts[i]; sub(/\[.*/, "", d); sub(/[ \t]+$/, "", d)
                        if (match(d, /[A-Za-z_][A-Za-z0-9_]*$/)) t = t substr(d, RSTART, RLENGTH) " "
                    }
                    nr[n]++; rmem[n, nr[n]] = t; rline[n, nr[n]] = ml; rtext[n, nr[n]] = mt
                }
            }
        }
        braces(c)
        if (inbody && depth == base) {
            inbody = 0; mpend = ""
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
        e["entity_t"] = 1
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
        printf "" > out_parent
        printf "" > out_ptrs
        printf "" > out_members
        printf "" > out_reg
        for (i = 1; i <= alias_n; i++) alias_of[alias_a[i]] = alias_b[i]
        for (k = 1; k <= n; k++) {
            if (!sobj[k] && first[k] in e && first[k] != "entity_t")
                print sfile[k] ":" firstline[k] ":" firsttext[k] > out_parent
            if (!in_e(k) && sfile[k] !~ /(^|\/)tests\// && sfile[k] !~ /(^|\/)engine\/pointer_list\.[ch]$/) {
                m = split(names[k], nm, " ")
                for (j = 1; j <= nr[k]; j++) {
                    q = split(rmem[k, j], mm, " "); left = q
                    for (h = 1; h <= q; h++)
                        for (i = 1; i <= m; i++) if (index(reg_exempt, " " nm[i] "." mm[h] " ")) { left--; break }
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
            m = split(names[k], nm, " ")
            for (j = 1; j <= nr[k]; j++) {
                q = split(rmem[k, j], mm, " "); left = q
                for (h = 1; h <= q; h++)
                    for (i = 1; i <= m; i++) if (index(exempt, " " nm[i] "." mm[h] " ")) { left--; break }
                if ((left > 0 || q == 0) && !((k, rline[k, j]) in shown)) {
                    shown[k, rline[k, j]] = 1
                    print sfile[k] ":" rline[k, j] ":" rtext[k, j] > out_ptrs
                }
            }
        }
    }' $(cat "$work/files") || fail "awk failed"
}

# Lecteur de prototypes : dans les .h, une declaration de premier niveau,
# jointe jusqu'a son ";", comptee a sa premiere ligne. Un membre de struct
# (table de methodes, new_entity) n'est pas de premier niveau.
read_prototypes() {
    sep=$(printf '\001')
    grep -E '\.h$' "$work/files" | grep -v '/engine/pointer_list\.h$' | grep -v '^engine/pointer_list\.h$' > "$work/headers" || true
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
    }' $(cat "$work/headers") > "$work/protos" || fail "awk failed"
    cut -d"$sep" -f1 "$work/protos" > "$work/protos.where"
    cut -d"$sep" -f2- "$work/protos" | sed -E "$VOID_EXCLUSION" > "$work/protos.text" || fail "sed failed"
    paste -d"$sep" "$work/protos.where" "$work/protos.text" | awk -F"$sep" '
    {
        t = $2
        if (t ~ /^[ \t]*typedef/ || index(t, "void *") == 0) next
        if (!match(t, /(^|[^A-Za-z0-9_])pico8_[a-z0-9_]+\(/)) next
        name = substr(t, RSTART, RLENGTH - 1); sub(/^[^p]*/, "", name)
        if (name ~ /_get_[a-z0-9_]+$/) next
        print $1
    }' > "$work/void-ptr" || fail "awk failed"
}

# registres, suite : objets pico8_ptrs_t de portee fichier des .c et
# prototypes des .h (hors engine/pointer_list.h), hors tests/, joints comme ceux de
# void-ptr ; un nom de REGISTRES_NON_ENTITE est exempte.
read_registries() {
    [ -f "$work/parent" ] || read_structs
    : > "$work/registries.decl"
    if [ -s "$work/regfiles" ]; then
        awk -v exempt=" $REGISTRIES_EXEMPT " -v reg_types="$REG_TYPES" "$CLEAN_AWK"'
        FNR == 1 { depth = 0; incomment = 0; acc = "" }
        {
            c = unattr(clean($0))
            if (acc == "" && ((depth == 0 && $0 ~ /^([a-z]|_Alignas)/) || (depth > 0 && c ~ /^[ \t]*static[ \t]/))) {
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
                          (!fn && acc ~ /(^|[^A-Za-z0-9_])void([ \t]+(const|volatile))*[ \t]*\*/) ||
                          (fn && ret ~ /(^|[^A-Za-z0-9_])void([ \t]+(const|volatile))*[ \t]*\*([ \t]*(const|volatile))*[ \t]*\*/))
                    keep = FILENAME ~ /\.h$/ || !fn
                    if (ok && keep && !index(exempt, " " name " ")) print FILENAME ":" start ":" text
                    acc = ""
                }
            }
            braces(c)
        }' $(cat "$work/regfiles") > "$work/registries.decl" || fail "awk failed"
    fi
    cat "$work/registries.structs" "$work/registries.decl" | sort -u -t: -k1,1 -k2,2n > "$work/registries"
}

# Paires {a, b} de modules hors tests/ ou chacun inclut l'en-tete de l'autre,
# dans $work/inclusion-mutuelle.pairs ; dans $work/inclusion-mutuelle, les
# lignes #include qui realisent ces paires, au format des autres categories.
read_inclusions() {
    grep -v '/tests/' "$work/files" | grep -v '^tests/' > "$work/modfiles" || true
    : > "$work/mutual-inclusion.pairs"
    : > "$work/mutual-inclusion"
    [ -s "$work/modfiles" ] || return 0
    status=0
    grep -HnE '^#include "[^"]+\.h"' $(cat "$work/modfiles") > "$work/includes" || status=$?
    [ "$status" -le 1 ] || fail "grep failed"
    awk -v pairs="$work/mutual-inclusion.pairs" '
    NR == FNR { f = $0; sub(/.*\//, "", f); sub(/\.[ch]$/, "", f); mod[f] = 1; next }
    {
        path = $0; sub(/:.*/, "", path)
        a = path; sub(/.*\//, "", a); sub(/\.[ch]$/, "", a)
        b = $0; sub(/^[^:]*:[0-9]+:#include "/, "", b); sub(/\.h".*/, "", b); sub(/.*\//, "", b)
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
    }' "$work/modfiles" "$work/includes" > "$work/mutual-inclusion" || fail "awk failed"
    sort "$work/mutual-inclusion.pairs" > "$work/sorted" && mv "$work/sorted" "$work/mutual-inclusion.pairs"
    sort -t: -k1,1 -k2,2n "$work/mutual-inclusion" > "$work/sorted" && mv "$work/sorted" "$work/mutual-inclusion"
}

# Graphe d'usage de l'etude 2.11 (usegraph.py) : a -> b quand un .c/.h de a
# hors tests/ emploie un nom que b.h est seul a declarer, commentaires et
# chaines retires ; il voit la paire qu'un detour cache a inclusion-mutuelle.
# Noms lus par motif : #define, prototype (un niveau de parentheses
# internes), (*nom), dernier nom d'un typedef ou d'un extern hors des
# accolades, enumerateur ; pas les etiquettes de struct. Une fonction definie
# dans l'en-tete (static inline) est lue comme un prototype, ce que
# usegraph.py ne fait pas. Un nom que deux
# en-tetes declarent n'est a aucun : les etiquettes, lues sans cette regle,
# feraient six paires par les declarations anticipees de engine/entity_class.h.
# Rend dans $work/usage-mutuel.pairs les paires, dans $work/usage-mutuel le
# premier emploi de chaque sens (ordre de $work/files, puis des lignes).
read_usages() {
    grep -v '/tests/' "$work/files" | grep -v '^tests/' > "$work/modfiles" || true
    : > "$work/mutual-usage.pairs"
    : > "$work/mutual-usage"
    [ -s "$work/modfiles" ] || return 0
    grep '\.h$' "$work/modfiles" > "$work/modheaders" || true
    awk -v pairs="$work/mutual-usage.pairs" "$CLEAN_AWK"'
    function mod(path,    m) {
        m = path; sub(/.*\//, "", m); sub(/\.[ch]$/, "", m)
        return m
    }
    function declare(h, x) {
        if (x == "" || x == "PICO8" || x == "PICO8_" || x ~ /^__/ || ((x, h) in seen)) return
        seen[x, h] = 1; nown[x]++; own[x] = h
    }
    function declare_all(    h, t, m, x) {
        for (h in full) {
            t = tops[h]
            while (match(t, /[A-Za-z0-9_]+[ \t]*\(([^()]|\([^()]*\))*\)[ \t]*[;\001]/)) {
                x = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
                sub(/[^A-Za-z0-9_].*$/, "", x)
                if (x !~ /^(if|for|while|return|sizeof|switch|void|int|char|const|unsigned)$/) declare(h, x)
            }
            t = tops[h]
            while (match(t, /\([ \t]*\*[ \t]*[A-Za-z0-9_]+[ \t]*\)/)) {
                x = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
                gsub(/[^A-Za-z0-9_]/, "", x); declare(h, x)
            }
            t = tops[h]
            while (match(t, /typedef[ \t][^;{}]*[A-Za-z0-9_][ \t]*;/)) {
                x = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
                sub(/[ \t]*;$/, "", x); match(x, /[A-Za-z0-9_]+$/); declare(h, substr(x, RSTART))
            }
            t = tops[h]
            while (match(t, /extern[ \t][^;(]*[A-Za-z0-9_][ \t]*(\[[^]]*\])?[ \t]*;/)) {
                x = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
                sub(/[ \t]*;$/, "", x); sub(/\[[^]]*\]$/, "", x); sub(/[ \t]*$/, "", x)
                match(x, /[A-Za-z0-9_]+$/); declare(h, substr(x, RSTART))
            }
            t = full[h]
            while (match(t, /enum[ \t]*[A-Za-z0-9_]*[ \t]*\{[^}]*\}/)) {
                m = substr(t, RSTART, RLENGTH); t = substr(t, RSTART + RLENGTH)
                sub(/^[^{]*\{[ \t]*/, "", m); sub(/[ \t]*\}$/, "", m)
                while (match(m, /[A-Za-z0-9_]+[ \t]*(=[^,]*)?(,|$)/)) {
                    x = substr(m, RSTART, RLENGTH); m = substr(m, RSTART + RLENGTH)
                    sub(/[^A-Za-z0-9_].*$/, "", x); declare(h, x)
                }
            }
        }
    }
    FNR == 1 { incomment = 0 }
    pass == 1 {
        h = mod(FILENAME); c = clean($0); full[h] = full[h] c " "
        l = c
        while (match(l, /#define[ \t]+[A-Za-z0-9_]+/)) {
            x = substr(l, RSTART, RLENGTH); l = substr(l, RSTART + RLENGTH)
            sub(/^#define[ \t]+/, "", x); declare(h, x)
        }
        l = c
        while (match(l, /[{}]/)) {
            if (hdepth[h] == 0) tops[h] = tops[h] substr(l, 1, RSTART - 1) (substr(l, RSTART, 1) == "{" ? "\001" : "")
            hdepth[h] += substr(l, RSTART, 1) == "{" ? 1 : -1
            l = substr(l, RSTART + 1)
        }
        if (hdepth[h] == 0) tops[h] = tops[h] l " "
        next
    }
    !done { declare_all(); done = 1 }
    {
        a = mod(FILENAME); l = clean($0)
        while (match(l, /[A-Za-z0-9_]+/)) {
            x = substr(l, RSTART, RLENGTH); l = substr(l, RSTART + RLENGTH)
            if (x ~ /^[0-9]/ || !(x in nown) || nown[x] != 1 || own[x] == a) continue
            if (!((a, own[x]) in site)) site[a, own[x]] = FILENAME ":" FNR ":" $0
        }
    }
    END {
        for (k in site) {
            split(k, ab, SUBSEP)
            if (ab[1] < ab[2] && ((ab[2], ab[1]) in site)) {
                print ab[1] "\t" ab[2] > pairs
                print site[k]; print site[ab[2], ab[1]]
            }
        }
    }
    ' pass=1 $(cat "$work/modheaders") pass=2 $(cat "$work/modfiles") > "$work/mutual-usage" || fail "awk failed"
    sort "$work/mutual-usage.pairs" > "$work/sorted" && mv "$work/sorted" "$work/mutual-usage.pairs"
    sort -u -t: -k1,1 -k2,2n "$work/mutual-usage" > "$work/sorted" && mv "$work/sorted" "$work/mutual-usage"
}

# Rend dans $work/<cat> les lignes "<fichier>:<ligne>:<texte>", dans l'ordre
# des fichiers puis des lignes. grep rend 1 quand rien ne correspond.
# L'exclusion est un sed sur le texte, jamais un filtre sur la ligne entiere :
# un filtre cacherait une vraie occurrence voisine.
collect() {
    case $1 in
        parent | entity-ptrs)
            [ -f "$work/parent" ] || read_structs
            sort -t: -k1,1 -k2,2n "$work/$1" > "$work/sorted" && mv "$work/sorted" "$work/$1"
            return 0 ;;
        void-ptr) read_prototypes; return 0 ;;
        registries) read_registries; return 0 ;;
        mutual-inclusion) read_inclusions; return 0 ;;
        mutual-usage) read_usages; return 0 ;;
        reachability-to-prove)
            status=0
            grep -nE '^[^#[:space:]]' "$REACHABILITY" > "$work/numbers" || status=$?
            [ "$status" -le 1 ] || fail "grep failed"
            sed "s|^|$REACHABILITY:|" "$work/numbers" > "$work/$1"
            return 0 ;;
    esac
    list="$work/files"
    [ "$1" = core-includes ] && list="$work/core"
    : > "$work/$1"
    [ -s "$list" ] || return 0
    status=0
    excl=$(exclusion "$1")
    if [ -z "$excl" ]; then
        grep -HnE "$(pattern "$1")" $(cat "$list") > "$work/$1" || status=$?
        [ "$status" -le 1 ] || fail "grep failed"
        return 0
    fi
    grep -lE "$(pattern "$1")" $(cat "$list") > "$work/candidates" || status=$?
    [ "$status" -le 1 ] || fail "grep failed"
    while IFS= read -r file; do
        status=0
        sed -E "$excl" "$file" > "$work/excluded" || fail "sed failed"
        grep -nE "$(pattern "$1")" "$work/excluded" > "$work/numbers" || status=$?
        [ "$status" -le 1 ] || fail "grep failed"
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
fields() {
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
                if (fn !~ /(_new|_init|_create|_init_instance)$/) h[m]++
            }
        }
    }
    END {
        for (m in want) printf "%s\t%d\t%d\n", m, w[m], h[m] > (ARGV[1] ".counts")
    }' "$work/members.sorted" $(cat "$work/sources") || fail "awk failed"
    awk -F'\t' '
    NR == FNR { c[$1] = $2 "\t" $3; next }
    { print $1 "\t" $2 "\t" c[$2] }' "$work/members.sorted.counts" "$work/members.sorted" \
        | sort -t"$(printf '\t')" -k1,1 -k2,2 > "$work/fields" || fail "awk failed"
    cat "$work/fields"
    awk -F'\t' '{ n++; if ($3 == 0) k++; if ($4 == 0) z++ }
    END { printf "fields: %d; no write: %d; no write outside a constructor: %d\n", n, k, z }' "$work/fields"
}

if [ "$mode" = fields ]; then
    fields
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
    echo "guarded residue ($checked): $n line(s)"
    [ "$n" -eq 0 ]
    exit
fi

: > "$work/all"
for cat in $CATEGORIES; do
    collect "$cat"
    if [ -s "$work/$cat" ]; then
        case " $FOOTER_CATEGORIES " in
            *" $cat "*) cut -d: -f1,2 "$work/$cat" >> "$work/all" ;;
        esac
        if [ -f "$work/$cat.pairs" ] && [ "$mode" = detail ]; then
            sed "s/^/$cat\t/" "$work/$cat.pairs"
        elif [ -f "$work/$cat.pairs" ]; then
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
    grep '/game/game_data\.h$\|^game/game_data\.h$' "$work/files" | while IFS= read -r file; do
        printf 'info\t%s\t%s lines\n' "$file" "$(wc -l < "$file" | tr -d ' ')"
    done
fi
lines=$(sort -u "$work/all" | wc -l | tr -d ' ')
files=$(cut -d: -f1 "$work/all" | sort -u | wc -l | tr -d ' ')
echo "residue 2: $lines line(s), $files file(s)"
