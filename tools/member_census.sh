#!/bin/sh
# Folds the observations of the -DPICO8_MEMBER_CENSUS runs with the static
# half, checks the tool's integrity (P1 to P4), and writes the table.
#
#   tools/member_census.sh <table> <census dir> <replays> <unit tests>
#
# <census dir> holds gen/pico8_member_census_table.h, raw/replay/*.txt and
# raw/unit/*.txt; the report goes to <census dir>/report.txt. Exits non-zero
# only on the tool's integrity, never on the table's content.
set -e

ROOT=$(dirname "$0")/..
cd "$ROOT"

TABLE=$1
DIR=$2
N_REPLAYS=$3
N_UNITS=$4
REPORT=$DIR/report.txt
WORK=$DIR/fold
LC_ALL=C
export LC_ALL

rm -rf "$WORK"
mkdir -p "$WORK"
: > "$REPORT"
status=0

fail() {
    echo "member-census: $*" | tee -a "$REPORT" >&2
    status=1
}

# Function context for every line of src/*.c: file, line, enclosing
# function (or "=<variable>" inside a file-scope initializer, "-" elsewhere
# at file scope), then the code with comments and literals blanked.
awk '
FNR == 1 { depth = 0; func_name = "-"; cand = ""; in_comment = 0 }
{
    line = $0
    code = ""
    while (length(line) > 0) {
        if (in_comment) {
            e = index(line, "*/")
            if (e == 0) { line = ""; break }
            line = substr(line, e + 2)
            in_comment = 0
            continue
        }
        s = index(line, "/*")
        c = index(line, "//")
        if (c > 0 && (s == 0 || c < s)) { code = code substr(line, 1, c - 1); line = ""; break }
        if (s == 0) { code = code line; break }
        code = code substr(line, 1, s - 1) " "
        line = substr(line, s + 2)
        in_comment = 1
    }
    stripped = code
    gsub(/"([^"\\]|\\.)*"/, "\"\"", stripped)
    gsub(/'\''([^'\''\\]|\\.)*'\''/, "0", stripped)
    if (depth == 0) {
        if (stripped ~ /^[A-Za-z_].*\(/ && stripped !~ /=/) {
            cand = stripped
            sub(/\(.*$/, "", cand)
            sub(/^.*[ *]/, "", cand)
        } else if (stripped ~ /=[ \t]*\{/) {
            cand = stripped
            sub(/[ \t]*(\[[^]]*\])?[ \t]*=.*$/, "", cand)
            sub(/^.*[ *]/, "", cand)
            cand = "=" cand
        }
    }
    ctx = depth == 0 ? (stripped ~ /=[ \t]*\{/ ? cand : "-") : func_name
    opens = gsub(/\{/, "{", stripped)
    closes = gsub(/\}/, "}", stripped)
    if (depth == 0 && opens > 0) {
        func_name = cand
        ctx = cand
    }
    depth += opens - closes
    if (depth == 0) { func_name = "-" }
    printf "%s\t%d\t%s\t%s\n", FILENAME, FNR, ctx, code
}
' $(ls src/*.c | grep -v 'src/pico8_member_census.c$') > "$WORK/lines.tsv"

# Rows: typedef, member, access, from the C table the census build compiled.
awk '
/g_census_typedefs\[\] = \{/ { mode = "t"; next }
/g_census_members\[\] = \{/ { mode = "m"; next }
/^\};/ { mode = ""; next }
mode == "t" {
    line = $0
    sub(/^[ \t]*\{"/, "", line)
    name = line; sub(/".*$/, "", name)
    rest = line; sub(/^[^,]*,[ \t]*/, "", rest)
    parent = rest; sub(/,.*$/, "", parent)
    by = rest; sub(/^[^,]*,[ \t]*/, "", by); sub(/\}.*$/, "", by)
    printf "T\t%d\t%s\t%s\t%s\n", nt++, name, parent, by
}
mode == "m" && /^[ \t]*\{[0-9]/ {
    line = $0
    sub(/^[ \t]*\{/, "", line)
    td = line; sub(/,.*$/, "", td)
    name = line; sub(/^[^"]*"/, "", name); sub(/".*$/, "", name)
    by = line; sub(/\},[ \t]*$/, "", by); sub(/^.*,[ \t]*/, "", by)
    printf "M\t%s\t%s\t%s\n", td, name, by
}
' "$DIR/gen/pico8_member_census_table.h" > "$WORK/ctable.tsv"

awk -F'\t' '
$1 == "T" { tname[$2] = $3; tby[$2] = $5 }
$1 == "M" { printf "%s\t%s\t%s\n", tname[$2], $3, tby[$2] ? "nom" : "offset" }
' "$WORK/ctable.tsv" > "$WORK/rows.tsv"
awk -F'\t' '$1 == "T" { printf "%s\t%s\n", $3, $4 }' "$WORK/ctable.tsv" > "$WORK/typedefs.tsv"

# P4: rows = the exit script's list.
sh tools/entity_value_members.sh | grep -v '^membres ' | awk '{ printf "%s\t%s\n", $2, $3 }' > "$WORK/list.tsv"
cut -f1,2 "$WORK/rows.tsv" > "$WORK/rows_pairs.tsv"
if cmp -s "$WORK/list.tsv" "$WORK/rows_pairs.tsv"; then
    echo "P4 OK : $(wc -l < "$WORK/rows.tsv" | tr -d ' ') lignes = sortie de tools/entity_value_members.sh" >> "$REPORT"
else
    fail "P4 : les lignes de la table ($(wc -l < "$WORK/rows_pairs.tsv" | tr -d ' ')) ne sont pas la sortie de tools/entity_value_members.sh ($(wc -l < "$WORK/list.tsv" | tr -d ' '))"
    diff "$WORK/list.tsv" "$WORK/rows_pairs.tsv" | head -20 >> "$REPORT" || true
fi

# P1: every struct reader or writer call is the central one or a chaining
# inside another reader or writer; kind->field reads a kind, not an entity.
awk -F'\t' '
{
    code = $4
    if (code !~ /(pico8_[a-z0-9_]+_field|->field|->set_field)\(/) { next }
    if (code ~ /^[A-Za-z_]/) { next }
    tmp = code
    gsub(/pico8_entity_(get|set)_field\(/, "", tmp)
    gsub(/kind->field\(/, "", tmp)
    if (tmp !~ /(pico8_[a-z0-9_]+_field|->field|->set_field)\(/) { next }
    if ($3 ~ /_field$/) { next }
    printf "%s:%s: %s (dans %s)\n", $1, $2, code, $3
}
' "$WORK/lines.tsv" > "$WORK/p1.txt"
if [ -s "$WORK/p1.txt" ]; then
    fail "P1 : lecteur ou écrivain de struct appelé hors du point central et d'un chaînage"
    cat "$WORK/p1.txt" >> "$REPORT"
else
    echo "P1 OK : lecteurs et écrivains par nom centralisés (kind->field excepté)" >> "$REPORT"
fi

# P2 static: a SEE after every coverage record in the same function; a SEE
# for every object that receives the struct tag.
awk -F'\t' '
{
    key = $1 SUBSEP $3
    if ($4 ~ /^[ \t]+.*pico8_class_coverage_record\(/) { rec[key] = $2 }
    if ($4 ~ /PICO8_MEMBER_CENSUS_SEE\(/) {
        see_last[key] = $2
        has_see[key] = 1
        arg = $4; sub(/^.*PICO8_MEMBER_CENSUS_SEE\([ \t]*&?/, "", arg); sub(/[ \t,].*$/, "", arg)
        see_var[$1 SUBSEP "=" arg] = 1
    }
    if ($4 ~ /PICO8_TYPE_TAG_STRUCT/ && $4 !~ /[=!]=/ && $4 !~ /assert/) {
        ntag++
        tag_file[ntag] = $1; tag_line[ntag] = $2; tag_ctx[ntag] = $3
    }
    if ($3 != "-" && $3 !~ /^=/) {
        tmp = $4
        while (match(tmp, /pico8_[a-z0-9_]+_struct_init\(/)) {
            callee = substr(tmp, RSTART, RLENGTH - 1)
            callers[callee] = callers[callee] " " $1 SUBSEP $3
            tmp = substr(tmp, RSTART + RLENGTH)
        }
    }
}
END {
    for (k in rec) {
        split(k, p, SUBSEP)
        if (!(k in see_last) || see_last[k] < rec[k]) {
            printf "P2 : %s: %s appelle pico8_class_coverage_record sans PICO8_MEMBER_CENSUS_SEE après\n", p[1], p[2]
        }
    }
    for (i = 1; i <= ntag; i++) {
        ctx = tag_ctx[i]
        key = tag_file[i] SUBSEP ctx
        ok = 0
        if (ctx ~ /^=/) {
            ok = (key in see_var)
        } else if (key in has_see) {
            ok = 1
        } else if (ctx ~ /_struct_init$/ && (ctx in callers)) {
            ok = 1
            n = split(callers[ctx], cs, " ")
            for (j = 1; j <= n; j++) {
                split(cs[j], q, SUBSEP)
                if (!(cs[j] in has_see) && q[2] !~ /_struct_init$/) { ok = 0 }
            }
        }
        if (!ok) {
            printf "P2 : %s:%s: l objet qui reçoit PICO8_TYPE_TAG_STRUCT (%s) n a pas de PICO8_MEMBER_CENSUS_SEE\n", tag_file[i], tag_line[i], ctx
        }
    }
    printf "sites-tag %d\n", ntag > "/dev/stderr"
}
' "$WORK/lines.tsv" > "$WORK/p2.txt" 2> "$WORK/p2_count.txt"
if [ -s "$WORK/p2.txt" ]; then
    fail "P2 statique : constructeur ou objet à en-tête sans PICO8_MEMBER_CENSUS_SEE"
    sort "$WORK/p2.txt" >> "$REPORT"
else
    echo "P2 statique OK : $(awk -F'\t' '$4 ~ /^[ \t]+.*pico8_class_coverage_record\(/' "$WORK/lines.tsv" | wc -l | tr -d ' ') constructeurs, $(sed 's/sites-tag //' "$WORK/p2_count.txt") sites de tag, tous suivis d un SEE" >> "$REPORT"
fi

# P3: every offset table entry names a row, directly or through an embedded
# parent; the number of `.offset` sites is printed so a new form shows.
awk -F'\t' '
FILENAME == ARGV[1] { tdname[FNR - 1] = $1; tdparent[FNR - 1] = $2; next }
FILENAME == ARGV[2] { has[$1 SUBSEP $2] = 1; next }
FNR == 1 && !linked {
    linked = 1
    for (i in tdname) {
        if (tdparent[i] >= 0) { parent_name[tdname[i]] = tdname[tdparent[i]] }
    }
}
{
    code = $4
    while (match(code, /offsetof\([ \t]*pico8_[a-z0-9_]+_t[ \t]*,[ \t]*[a-z0-9_.]+[ \t]*\)/)) {
        e = substr(code, RSTART, RLENGTH)
        code = substr(code, RSTART + RLENGTH)
        sub(/^offsetof\([ \t]*/, "", e)
        sub(/[ \t]*\)$/, "", e)
        td = e
        sub(/[ \t]*,.*$/, "", td)
        m = e
        sub(/^.*[,.][ \t]*/, "", m)
        nentries++
        t = td
        found = (t SUBSEP m) in has
        while (!found && (t in parent_name)) {
            t = parent_name[t]
            found = (t SUBSEP m) in has
        }
        if (!found) {
            printf "P3 : %s:%s: offsetof(%s) ne nomme aucune ligne de la table\n", $1, $2, e
        }
    }
    if ($4 ~ /\.offset/ && $4 !~ /offsetof/) { noffset++ }
}
END { printf "%d %d\n", nentries, noffset > "/dev/stderr" }
' "$WORK/typedefs.tsv" "$WORK/rows.tsv" "$WORK/lines.tsv" > "$WORK/p3.txt" 2> "$WORK/p3_count.txt"
read -r P3_ENTRIES P3_SITES < "$WORK/p3_count.txt"
if [ -s "$WORK/p3.txt" ]; then
    fail "P3 : entrée de table d'offsets hors de la table"
    cat "$WORK/p3.txt" >> "$REPORT"
else
    echo "P3 OK : $P3_ENTRIES entrées offsetof, toutes des lignes de la table ; sites .offset : $P3_SITES" >> "$REPORT"
fi

# A member that hides a same-named member of an embedded parent: the most
# derived one wins, as pico8_member_census.c resolves names.
awk -F'\t' '
FILENAME == ARGV[1] { tdname[FNR - 1] = $1; tdparent[FNR - 1] = $2; next }
FNR == 1 && !linked {
    linked = 1
    for (i in tdname) {
        if (tdparent[i] >= 0) { parent_name[tdname[i]] = tdname[tdparent[i]] }
    }
}
{ has[$1 SUBSEP $2] = 1; nrow++; rt[nrow] = $1; rm[nrow] = $2 }
END {
    for (r = 1; r <= nrow; r++) {
        t = rt[r]
        while (t in parent_name) {
            t = parent_name[t]
            if ((t SUBSEP rm[r]) in has) { printf "masque : %s.%s masque %s.%s\n", rt[r], rm[r], t, rm[r] }
        }
    }
}
' "$WORK/typedefs.tsv" "$WORK/rows.tsv" > "$WORK/shadow.txt"
echo "membres qui masquent un parent : $(wc -l < "$WORK/shadow.txt" | tr -d ' ')" >> "$REPORT"
cat "$WORK/shadow.txt" >> "$REPORT"

# Dynamic half: union of every process's observations.
nreplay_files=$(ls "$DIR/raw/replay" | wc -l | tr -d ' ')
nunit_files=$(ls "$DIR/raw/unit" | wc -l | tr -d ' ')
if [ "$nreplay_files" -ne "$N_REPLAYS" ]; then
    fail "runs : $nreplay_files sortie(s) de rejeu pour $N_REPLAYS rejeu(x)"
fi
echo "sorties : $nreplay_files de rejeu(x), $nunit_files de processus de TU ($N_UNITS TU)" >> "$REPORT"
cat "$DIR"/raw/replay/*.txt > "$WORK/replay.txt" 2> /dev/null || : > "$WORK/replay.txt"
cat "$DIR"/raw/unit/*.txt > "$WORK/unit.txt" 2> /dev/null || : > "$WORK/unit.txt"

n_unknown_replay=$(grep -c '^U' "$WORK/replay.txt" || true)
n_unknown_unit=$(grep -c '^U' "$WORK/unit.txt" || true)
if [ "$n_unknown_replay" -ne 0 ]; then
    fail "P2 dynamique : $n_unknown_replay descripteur(s) inconnu(s) dans les rejeux"
else
    echo "P2 dynamique OK : 0 descripteur inconnu dans les rejeux ; $n_unknown_unit dans la passe des TU (structs locales aux tests)" >> "$REPORT"
fi
if grep -q '^X' "$WORK/replay.txt" "$WORK/unit.txt"; then
    fail "erreurs de l'outil (une écriture par nom hors membre est un champ sans règle du §5 : FAILED)"
    grep -h '^X' "$WORK/replay.txt" "$WORK/unit.txt" | sort | uniq -c >> "$REPORT"
fi
overflow=$(awk -F'\t' '$1 == "S" && $2 == "debordements" { s += $3 } END { print s + 0 }' "$WORK/replay.txt" "$WORK/unit.txt")
if [ "$overflow" -ne 0 ]; then
    fail "stockage statique débordé $overflow fois"
fi
for k in muets differees nil-par-nom hors-membre; do
    printf '%s (rejeux, TU) : %s, %s\n' "$k" \
        "$(awk -F'\t' -v k=$k '$1 == "S" && $2 == k { s += $3 } END { print s + 0 }' "$WORK/replay.txt")" \
        "$(awk -F'\t' -v k=$k '$1 == "S" && $2 == k { s += $3 } END { print s + 0 }' "$WORK/unit.txt")" >> "$REPORT"
done
awk -F'\t' '$1 == "M" && $5 ~ /o/ { print "cibles struct tronquées : " $2 "." $3 }' "$WORK/replay.txt" "$WORK/unit.txt" | sort -u >> "$REPORT"

# Static half: nil tests and `= nil` made directly on a member.
awk -F'\t' '
{
    code = $4
    rest = code
    while (match(rest, /_truthy\([^()]*(->|\.)[a-z0-9_]+\)/)) {
        s = substr(rest, RSTART, RLENGTH); rest = substr(rest, RSTART + RLENGTH)
        sub(/\)$/, "", s); sub(/^.*(->|\.)/, "", s)
        printf "%s\t%s:%s\ttruthy\n", s, $1, $2
    }
    rest = code
    while (match(rest, /(->|\.)[a-z0-9_]+\.as\.ref (==|!=) NULL/)) {
        s = substr(rest, RSTART, RLENGTH); rest = substr(rest, RSTART + RLENGTH)
        sub(/\.as\.ref.*$/, "", s); sub(/^(->|\.)/, "", s)
        printf "%s\t%s:%s\tas.ref\n", s, $1, $2
    }
    rest = code
    if ($3 !~ /(_struct_init|_new|_struct_new)$/) {
        while (match(rest, /(->|\.)[a-z0-9_]+ = pico8_value_ref\(NULL\)/)) {
            s = substr(rest, RSTART, RLENGTH); rest = substr(rest, RSTART + RLENGTH)
            sub(/ = .*$/, "", s); sub(/^(->|\.)/, "", s)
            printf "%s\t%s:%s\t= nil\n", s, $1, $2
        }
    }
}
' "$WORK/lines.tsv" > "$WORK/static.tsv"

# Fold: one line per row, in the exit script's order.
awk -F'\t' -v n_replays="$N_REPLAYS" -v n_units="$N_UNITS" -v report="$REPORT" '
FILENAME == ARGV[1] { nrows++; rt[nrows] = $1; rm[nrows] = $2; ra[nrows] = $3; bymember[$2] = bymember[$2] + 1; next }
FILENAME == ARGV[2] {
    nstat[$1]++
    stat_sites[$1] = stat_sites[$1] " " $2 "(" $3 ")"
    next
}
FILENAME == ARGV[3] || FILENAME == ARGV[4] {
    if ($1 == "M") {
        key = $2 SUBSEP $3
        n = split($4, ks, ",")
        for (i = 1; i <= n; i++) { if (ks[i] != "-") { kinds[key, ks[i]] = 1; kindlist[key] = kindlist[key] "," ks[i] } }
        for (i = 1; i <= length($5); i++) { flags[key, substr($5, i, 1)] = 1 }
    } else if ($1 == "T") {
        seen[$2] += $3
        if ($4 == 1) { inhv[$2] = 1 }
    } else if ($1 == "S" && $2 == "muets") {
        muted += $3
    }
    next
}
END {
    print "# Recensement des membres pico8_value_t des structs d'\''entité (plan 3, 1b.0)."
    print "# Produit par make member-census ; ne pas éditer à la main. Une entrée de la liste 1b, pas un oracle."
    print "# genres : ensemble des genres tenus aux points d'\''observation (fin de constructeur ou d'\''init de module,"
    print "#   lecture et écriture par nom, fin de update60 et de draw sur hv) : fixe, chaine, bool, fonction,"
    print "#   ref:liste, ref:table, ref:<typedef> (cible struct), ref:autre (REF non NULL sans tag) ; nil n'\''est"
    print "#   pas un genre ; - : aucun genre observé, le PRD du membre ajoute une caractérisation dirigée."
    print "# nil : observé (au moins une preuve), tenu (a valu nil sans preuve), jamais. Tenir nil n'\''est pas"
    print "#   observer nil : les mises à nil de l'\''init donnent tenu."
    print "# preuves-nil : lu (get_field rend 0 sur le membre nil), écrit (set_field écrit nil), stat (test de"
    print "#   vérité ou = nil direct trouvé par grep sur ce nom, un seul typedef le porte), stat? (plusieurs)."
    print "# par-nom : l lu, e écrit par pico8_entity_get_field/set_field. accès : offset (typedef d'\''un en-tête,"
    print "#   instantané possible), nom (typedef d'\''un .c, vu seulement par nom)."
    print "# Angle mort : un genre écrit directement puis écrasé avant l'\''observation suivante n'\''est pas vu."
    print "# struct\tmembre\taccès\tgenres\tnil\tpreuves-nil\tpar-nom"
    for (r = 1; r <= nrows; r++) {
        key = rt[r] SUBSEP rm[r]
        g = ""
        n = split(substr(kindlist[key], 2), ks, ",")
        delete used
        cnt = 0
        for (i = 1; i <= n; i++) { if (!(ks[i] in used)) { used[ks[i]] = 1; sorted[++cnt] = ks[i] } }
        for (i = 2; i <= cnt; i++) { v = sorted[i]; j = i - 1; while (j >= 1 && sorted[j] > v) { sorted[j + 1] = sorted[j]; j-- } sorted[j + 1] = v }
        for (i = 1; i <= cnt; i++) { g = g (i > 1 ? "," : "") sorted[i] }
        if (g == "") { g = "-"; nogenre++ }
        p = ""
        if ((key, "l") in flags) { p = p ",lu" }
        if (rm[r] in nstat) { p = p (bymember[rm[r]] > 1 ? ",stat?" : ",stat") }
        if ((key, "e") in flags) { p = p ",écrit" }
        p = substr(p, 2)
        if (p != "") { nil = "observé"; nobs++ } else if ((key, "h") in flags) { nil = "tenu"; nheld++ } else { nil = "jamais" }
        if (p == "") { p = "-" }
        bn = ""
        if ((key, "r") in flags) { bn = bn "l" }
        if ((key, "w") in flags) { bn = bn "e" }
        if (bn == "") { bn = "-" }
        printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\n", rt[r], rm[r], ra[r], g, nil, p, bn
        if (rm[r] in nstat) { printf "statique %s.%s :%s\n", rt[r], rm[r], stat_sites[rm[r]] >> report }
        if ((key, "m") in flags) { printf "lecteur muet : %s.%s\n", rt[r], rm[r] >> report }
        if ((key, "d") in flags) { printf "lecture différée (descripteur pas encore vu) : %s.%s\n", rt[r], rm[r] >> report }
        if (g == "-") { printf "sans genre : %s.%s\n", rt[r], rm[r] >> report }
        td_rows[rt[r]] = 1
    }
    printf "# lignes %d ; sans genre %d ; nil observé %d ; nil tenu seulement %d ; lecteurs muets %d ; rejeux %d ; TU %d\n", nrows, nogenre, nobs, nheld, muted, n_replays, n_units
    for (t in td_rows) {
        if (!(t in inhv)) { printf "jamais dans hv : %s (vu %d fois)\n", t, seen[t] >> report }
    }
}
' "$WORK/rows.tsv" "$WORK/static.tsv" "$WORK/replay.txt" "$WORK/unit.txt" > "$WORK/table.txt"

nstat_sites=$(wc -l < "$WORK/static.tsv" | tr -d ' ')
{
    echo "moitié statique : $nstat_sites site(s) ($(grep -c 'truthy$' "$WORK/static.tsv" || true) _truthy, $(grep -c 'as.ref$' "$WORK/static.tsv" || true) as.ref, $(grep -c '= nil$' "$WORK/static.tsv" || true) = nil)"
    echo "membres statiques ambigus (stat?) : $(awk -F'\t' '$6 ~ /stat\?/' "$WORK/table.txt" | wc -l | tr -d ' ') ; non ambigus (stat) : $(awk -F'\t' '$6 ~ /stat($|,)/' "$WORK/table.txt" | wc -l | tr -d ' ')"
    echo "noms statiques hors table : $(cut -f1 "$WORK/static.tsv" | sort -u | while read -r m; do grep -q "	$m	" "$WORK/rows.tsv" || printf '%s ' "$m"; done)"
} >> "$REPORT"

if [ "$status" -eq 0 ]; then
    cp "$WORK/table.txt" "$TABLE"
    tail -1 "$TABLE"
    echo "member-census: OK -- table $TABLE, rapport $REPORT"
else
    echo "member-census: ÉCHEC -- table non écrite, rapport $REPORT" >&2
fi
exit $status
