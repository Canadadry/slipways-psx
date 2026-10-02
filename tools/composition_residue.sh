#!/bin/sh
# usage: composition_residue.sh [--detail | --check <cat>[,<cat>...] | --fields] [dir...]
# Counts, in the `.c` and `.h` files under the directories (default: src
# tests; paths printed as given), the lines of each category of the
# composition residue, comments included; a line counts in every category
# it matches. reachability-to-prove counts the useful lines of the
# COMPOSITION_REACHABILITY list. mutual-inclusion counts pairs of modules
# outside tests/; in the footer and under --check, its lines are the
# #include lines that make those pairs. mutual-usage counts the pairs of
# modules outside tests/ where each uses a name that only the header of the
# other declares, through an include or a detour; its lines are the first
# use, per pair and per direction.
#   no option : `<cat>\t<lines>\t<files>` per nonzero category
#               (`<pairs>\t<modules>` for mutual-inclusion and
#               mutual-usage), then
#               `residue 2: N line(s), M file(s)` (distinct file:line
#               couples and distinct files over the categories of
#               FOOTER_CATEGORIES); exit 0 whatever the count.
#   --detail  : `<cat>\t<file>\t<lines>` per nonzero couple,
#               `<cat>\t<module>\t<module>` per pair of mutual-inclusion
#               and of mutual-usage; under void-ptr, the
#               `void-ptr-r2\t<function>\t<upstream>` lines of the
#               COMPOSITION_R2 list; the `info\t<file>\tN lines` line of
#               game/game_data.h when it is read; then the same footer;
#               exit 0.
#   --check   : `<cat>: <file>:<line>:<text>` per occurrence of the named
#               categories, then `guarded residue (<cats>): N line(s)`;
#               exit 1 when N > 0. Any known category is accepted.
#   --fields  : `<type>\t<member>\t<writes>\t<outside>` per member of
#               the entity types of the headers outside tests/ (neither
#               the `header` member, nor an embedded parent, nor a
#               function pointer), sorted, where <outside> counts the
#               writes outside a constructor; then
#               `fields: N; no write: K; no write outside a constructor: M`;
#               exit 0. A write is a line of a `.c` file outside
#               tests/ that assigns, increments or decrements
#               `(->|.)<member>`, once a copy of the default
#               `pico8_X_start.<member>` is removed; it is outside a
#               constructor unless it sits in a function whose name ends
#               with `_new`, `_init`, `_create` or `_init_instance`.
#               Searched by name: an upper bound, since a short member
#               counts for all its types.
# Without option, an empty residue prints no category line and the footer
# `residue 2: 0 line(s), 0 file(s)`.
# Exit 2 on an unknown option or category, an empty category or none after
# --check, a missing directory, no `.c` or `.h` file, a missing
# COMPOSITION_REACHABILITY or COMPOSITION_R2 list, or a failed grep, sed or
# awk: a measure over nothing would pass for empty.

set -u

CATEGORIES="parent parent-link header-accessors defaults nu name-dispatch one-or-false void-ptr void-self entity-ptrs registries core-includes reachability-to-prove mutual-inclusion mutual-usage"

# Categories counted in the footer.
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

# entity_state_of maps a state name to its index: it is not a lineage.
PARENT_LINK_EXCLUSION='s/entity_state_of//g'
# The escaped "nu" of a fixture of tests/test_data_residue.c is a test
# string, not the field. Its escaped quotes delimit the word: no `\b` here.
NU_EXCLUSION='s/\\"nu\\"//g'

exclusion() {
    case $1 in
        parent-link) printf '%s\n' "$PARENT_LINK_EXCLUSION" ;;
        nu) printf '%s\n' "$NU_EXCLUSION" ;;
    esac
}

# void-self: a game object received as `void *` under the name self
# (self_ptr included), on a line that declares it: member, typedef,
# prototype, definition, local; const is read around the star. A `void *`
# of another name is not read.

# The `void *ctx` of the walks carries a context, not an entity. This sed
# needs GNU sed for `\b`, like the grep calls of the script.
VOID_EXCLUSION='s/void \*+(r_)?ctx\b//g'

# entity-ptrs: the members of an entity type that registries would count
# outside an entity type (REG_TYPES by token, `void **`, `void *[N]`, a
# function pointer that returns `void *`). Lists that hold no entity
# (type.member), each proved by its writer: economy_t.invented holds
# technology names (economy_invent, game/economy.c); planet_t.needs holds
# need_t records (planet_install_project in game/planet.c,
# update_tiers_visit in game/economy_tiers.c, planet_offer_trade in
# game/planet_trade.c); research_project_t.inputs holds need_t records
# (project_create, game/research.c); slipway_t.transit_dots holds particles
# (slipway_spawn_transit_dot, game/slipway_display.c). The exemption goes
# name by name: a line counts as soon as one of its names, or a declaration
# with no readable name, is not exempt.
ENTITY_PTRS_EXEMPT="economy_t.invented planet_t.needs research_project_t.inputs slipway_t.transit_dots"

# registries: outside tests/ and engine/pointer_list.[ch], the declaration
# points that hold entities through `void *` outside the entity types
# (which entity-ptrs reads). A member of a struct (or of a struct object,
# even inside a function) that names pointer_list_t or an alias, whatever
# its declarator (array, `**`, const, several names, function pointer), or
# of type `void **` or `void *[N]`; a member that returns `void *` through
# a function pointer; an object of file scope, a static of a function or
# an extern of a header, of these types or `void *`; a prototype or an
# inline definition of a header that takes or returns a pointer_list_t, or
# returns `void **`. const and volatile are read around void and each
# star; each declaration of a member line is read, and a member without a
# final semicolon is joined to the next lines and counted at its first.
# A struct or union whose brace opens the next line is seen;
# `__attribute__((...))` with two levels of inner parentheses and
# `_Alignas(...)` with one are removed before reading.
# Not seen: an alias of an alias, a `void *` behind a typedef, a struct or
# union nested over several lines, a plain `void *` member, a local, a
# macro, an attribute one level deeper; of two declarations of file scope
# on one line, only the first name is read for the exemption.
# Exempt, each proved by its writer: emitter_t.particles holds particles
# (emitter_init_in, game/particle.c);
# planet_inspection_panel_ctx_t.unmet_needs holds need_t records
# (planet_inspection_panel_filter_step, game/planet_display.c);
# collect_map_pointer_list maps the need_t records
# of research_project_t.inputs (planet_install_project, game/planet.c);
# collect_contains_string reads technology names (tech_gate_is_unlocked,
# game/tech_gate.c); research_display_planet_resource_row reads need_t
# records (planet_refresh in game/planet_display.c, and
# game/research_display.c); collect_filter_pointer_list and
# collect_concat_pointer_list take need_t records (planet_inspection_panel
# in game/planet_display.c, planet_offer_trade in game/planet_trade.c),
# lists of entities going through entity_list_filter and
# entity_list_concat.
REGISTRIES_EXEMPT="emitter_t.particles planet_inspection_panel_ctx_t.unmet_needs collect_map_pointer_list collect_contains_string research_display_planet_resource_row collect_filter_pointer_list collect_concat_pointer_list"

# Core headers, by path relative to src/: core-includes reads only them.
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

# registries reads outside tests/ and engine/pointer_list.[ch]; REG_TYPES is
# pointer_list_t and its aliases "typedef pointer_list_t name;" of these
# files (an alias of an alias is not followed).
grep -v '/tests/' "$work/files" | grep -v '^tests/' | grep -v '/engine/pointer_list\.[ch]$' | grep -v '^engine/pointer_list\.[ch]$' > "$work/regfiles" || true
REG_TYPES=pointer_list_t
if [ -s "$work/regfiles" ]; then
    grep -hE '^[[:space:]]*typedef[[:space:]]+(const[[:space:]]+)?pointer_list_t[[:space:]*]+[A-Za-z_][A-Za-z0-9_]*[[:space:]]*;' $(cat "$work/regfiles") > "$work/reg.alias"
    [ $? -le 1 ] || fail "grep failed"
    for a in $(sed -E 's/^.*[[:space:]*]([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*;.*$/\1/' "$work/reg.alias"); do
        REG_TYPES="$REG_TYPES|$a"
    done
fi

# Strips the comments, strings and character constants of a line, to count
# its braces; an open comment carries over to the next line (incomment).
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

# Struct reader. A struct body of top level opens on a "struct ... {" line
# outside any body: a local of a function is not a member. The entity
# types are the closure from entity_t by the first member that is not a
# pointer; the tag, the typedef of "} name;" and that of
# "typedef struct tag name;" name the same type. Writes to $work/parent and
# $work/entity-ptrs the "<file>:<line>:<text>" lines, to
# $work/registries.structs the struct members of registries, and to
# $work/members the "<type>\t<member>" lines that --fields reads.
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

# Prototype reader: in the headers, a declaration of top level, joined up
# to its ";", counted at its first line. A member of a struct (method
# table, new_entity) is not of top level.
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

# registries, continued: pointer_list_t objects of file scope of the `.c`
# files and prototypes of the headers (outside engine/pointer_list.h),
# outside tests/, joined like those of void-ptr; a name of
# REGISTRIES_EXEMPT is exempt.
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

# Pairs of modules outside tests/ where each includes the header of the
# other, in $work/mutual-inclusion.pairs; in $work/mutual-inclusion, the
# #include lines that make those pairs, in the format of the other
# categories.
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

# Usage graph: module A uses module B when a `.c` or `.h` file of A
# outside tests/ uses a name that only the header of B declares, comments
# and strings removed; it sees the pair that a detour hides from
# mutual-inclusion. Names read by pattern: #define, prototype (one level of
# inner parentheses), (*name), last name of a typedef or of an extern
# outside braces, enumerator; not the struct tags. A function defined in
# the header (static inline) is read as a prototype. A name that two
# headers declare belongs to neither: the tags, read without this rule,
# would make six pairs through the forward declarations of
# engine/entity_class.h. Writes the pairs to $work/mutual-usage.pairs, and
# to $work/mutual-usage the first use of each direction (order of
# $work/files, then of the lines).
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

# Writes to $work/<cat> the "<file>:<line>:<text>" lines, in the order of
# the files then of the lines. grep returns 1 when nothing matches.
# An exclusion is a sed on the text, never a filter on the whole line:
# a filter would hide a real occurrence next to it.
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

# The function of a line is the last definition of top level, its
# signature joined up to its final "{", closed by the "}" of the first
# column. Outside any function (an initializer of file scope, the
# `pico8_X_start` defaults included), a write is outside a constructor:
# the report is an upper bound, it does not declare constant what an
# initializer writes.
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
