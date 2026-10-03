"""Mechanical check of docs/renommage.tsv before any human review: the table
is exhaustive against the extraction of `make rename-extract`, and no new
name collides with another, shadows one, or is a C keyword or a C library
name; string and file names are consistent and file targets unique. Prints
every violation and exits 1 if there is one; the form rules of
docs/plan-5.md section 4 are printed as warnings only."""

import argparse
import itertools
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_tables  # noqa: E402

C_KEYWORDS = set("""
auto break case char const continue default do double else enum extern float for goto if
inline int long register restrict return short signed sizeof static struct switch typedef
union unsigned void volatile while _Alignas _Alignof _Atomic _Bool _Complex _Generic
_Imaginary _Noreturn _Static_assert _Thread_local alignas alignof bool constexpr false
nullptr static_assert thread_local true typeof typeof_unqual _BitInt _Decimal32 _Decimal64
_Decimal128
""".split())
# Names of the C library headers the code includes: a file-scope name or a
# macro spelled like one breaks the build or hides the library.
C_LIBRARY = set("""
abort abs acos asin atan atan2 atexit atof atoi atol bsearch calloc ceil clearerr clock cos
cosh difftime div errno exit exp fabs fclose feof ferror fflush fgetc fgetpos fgets floor
fmod fopen fprintf fputc fputs fread free freopen frexp fscanf fseek fsetpos ftell fwrite
getc getchar getenv gets gmtime isalnum isalpha iscntrl isdigit isgraph islower isprint
ispunct isspace isupper isxdigit labs ldexp ldiv localtime log log10 longjmp malloc mblen
memchr memcmp memcpy memmove memset mktime modf perror pow printf putc putchar puts qsort
raise rand realloc remove rename rewind scanf setbuf setjmp setvbuf signal sin sinh
snprintf sprintf sqrt srand sscanf strcat strchr strcmp strcoll strcpy strcspn strerror
strftime strlen strncat strncmp strncpy strpbrk strrchr strspn strstr strtod strtok strtol
strtoul strdup system tan tanh time tmpfile tmpnam tolower toupper ungetc vfprintf vprintf
vsnprintf vsprintf assert offsetof NULL EOF FILE size_t ptrdiff_t va_list va_start va_end
va_arg va_copy int8_t int16_t int32_t int64_t uint8_t uint16_t uint32_t uint64_t intptr_t
uintptr_t INT_MAX INT_MIN UINT_MAX CHAR_BIT stdin stdout stderr round lround trunc fmin
fmax isnan isinf INFINITY NAN M_PI close open read write unlink sleep usleep getpid index
""".split())

ORDINARY = ("function", "global", "static", "type", "enum_constant", "macro")
LOCALS = ("local", "param")
IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


class Decl:
    def __init__(self, row):
        """row: a declaration as rename_tables.read_declarations gives it."""
        self.identity, self.kind, self.name = row["identity"], row["kind"], row["name"]
        self.parent, self.file, self.linkage = row["parent"], row["file"], row["linkage"]
        self.flags = row["flags"]
        self.line = int(row["line"])
        self.scope = int(row["scope_lines"]) if row["scope_lines"] != "-" else None
        self.new = None

    @property
    def local(self):
        return self.kind in LOCALS or (self.kind == "static" and self.parent != "-")

    @property
    def prototype_param(self):
        return self.kind == "param" and (self.scope is None or "/" in self.parent)

    @property
    def visibility(self):
        """"program" for what a header or an external linkage shares, else
        the .c file it is local to."""
        if self.file.endswith(".h") or self.linkage == "extern":
            return "program"
        return self.file

    def lines(self):
        end = self.line + self.scope if self.scope is not None else 10 ** 9
        return self.line, end


class Report:
    def __init__(self):
        self.violations = []
        self.warnings = []

    def fail(self, text):
        self.violations.append(text)

    def warn(self, text):
        self.warnings.append(text)


def describe(d):
    return "%s %s (%s:%d, %s)" % (d.kind, d.name, d.file, d.line, d.identity)


# --- exhaustiveness --------------------------------------------------------

def check_exhaustive(header, rows, declarations, remainder, report):
    if not any(re.match(r"^#\s*Statut\s*:", line) for line in header):
        report.fail("table: no `# Statut : …` line in the header")
    by_identity = {}
    for row in rows:
        if row["catégorie"] != "identifiant":
            continue
        if row["identité"] in by_identity:
            report.fail("identity %s: two identifier rows" % row["identité"])
        by_identity[row["identité"]] = row
    extracted = {d.identity: d for d in declarations}
    for identity, d in extracted.items():
        row = by_identity.get(identity)
        if row is None:
            report.fail("missing row: extracted %s" % describe(d))
            continue
        if row["ancien"] != d.name or row["fichier"] != d.file:
            report.fail("row %s: says %s in %s, the extraction %s in %s" % (
                identity, row["ancien"], row["fichier"], d.name, d.file))
        d.new = row["nouveau"]
    for identity, row in by_identity.items():
        if identity not in extracted:
            report.fail("row without extracted identity: %s %s (%s)" % (
                row["ancien"], identity, row["fichier"]))
    for row in rows:
        new, old = row["nouveau"], row["ancien"]
        if not new:
            report.fail("empty new name: %s %s (%s)" % (row["catégorie"], old, row["identité"]))
            continue
        expected = "conservé" if new == old else "renommé"
        if row["statut"] != expected:
            report.fail("status of %s -> %s (%s) should be %s" % (old, new, row["identité"], expected))
        if row["catégorie"] == "identifiant" and not IDENTIFIER.match(new):
            report.fail("not a C identifier: %s -> %s (%s)" % (old, new, row["identité"]))
    if remainder:
        report.fail("remainder not empty: %d name(s) the derivation left undecided" % len(remainder))


# --- collisions ------------------------------------------------------------

def overlap(a, b):
    a0, a1 = a.lines()
    b0, b1 = b.lines()
    return a0 <= b1 and b0 <= a1


def meet(a, b):
    """Two file-scope declarations see each other: one is shared by the
    program (assumed included), or both live in the same file."""
    va, vb = a.visibility, b.visibility
    return va == "program" or vb == "program" or va == vb


def new_collision(a, b):
    """Same new name, different old names: a collision the rename creates."""
    return a.identity != b.identity and a.new == b.new and a.name != b.name


def check_collisions(declarations, report):
    named = [d for d in declarations if d.new]
    by_new = {}
    for d in named:
        by_new.setdefault(d.new, []).append(d)
    for new, group in sorted(by_new.items()):
        if len(group) < 2:
            continue
        for a, b in itertools.combinations(group, 2):
            if not new_collision(a, b):
                continue
            reason = collision_reason(a, b)
            if reason:
                report.fail("%s %s: %s and %s" % (reason, new, describe(a), describe(b)))
    for d in named:
        if d.new != d.name and (d.new in C_KEYWORDS or re.match(r"^_[A-Z_]", d.new)):
            report.fail("reserved word %s: %s" % (d.new, describe(d)))
        elif d.new != d.name and d.new in C_LIBRARY and not d.local and d.kind != "field":
            report.fail("C library name %s: %s" % (d.new, describe(d)))


def struct_of(field):
    """The struct of a field by its identity: the parent column says
    `(anonymous)` for every unnamed struct."""
    return field.identity.rsplit("@FI@", 1)[0]


def collision_reason(a, b):
    """Why two declarations with one new name cannot coexist, or None."""
    kinds = {a.kind, b.kind}
    if "macro" in kinds:
        macro, other = (a, b) if a.kind == "macro" else (b, a)
        if other.prototype_param:
            return "macro meets prototype parameter"
        if macro.visibility == "program" or macro.file == other.file:
            return "macro collision"
        return None
    if a.kind == "field" or b.kind == "field":
        if a.kind == b.kind == "field" and struct_of(a) == struct_of(b):
            return "field collision"
        return None
    if "tag" in kinds:
        if a.kind == b.kind == "tag" and meet(a, b):
            return "tag collision"
        return None
    if a.prototype_param or b.prototype_param:
        return None
    if a.local and b.local:
        if a.file == b.file and a.parent == b.parent and overlap(a, b):
            return "local collision"
        return None
    if a.local or b.local:
        local, outer = (a, b) if a.local else (b, a)
        if outer.visibility in ("program", local.file):
            return "local shadows %s" % outer.kind
        return None
    if meet(a, b):
        return "collision"
    return None


# --- strings and files -----------------------------------------------------

def check_others(rows, root, report):
    names = {}
    for row in rows:
        if row["catégorie"] == "identifiant":
            continue
        names.setdefault(row["identité"], []).append(row)
    first = {}
    for identity, group in sorted(names.items()):
        news = {r["nouveau"] for r in group}
        if len(news) > 1:
            report.fail("%s: several new names %s" % (identity, ", ".join(sorted(news))))
        first[identity] = group[0]
    by_kind = {}
    for identity, row in first.items():
        if row["catégorie"] == "chaîne" and row["sorte"] != "symbole cité" and row["nouveau"]:
            for kind in row["sorte"].split(" + "):
                by_kind.setdefault((kind, row["nouveau"]), []).append(row)
    for (kind, new), group in sorted(by_kind.items()):
        olds = sorted({r["ancien"] for r in group})
        if len(olds) > 1:
            report.fail("string collision %s (%s): %s" % (new, kind, ", ".join(olds)))
    paths = [r for r in first.values()
             if r["catégorie"] == "fichier" or r["sorte"] == "chemin tmp"]
    renamed_away = {r["ancien"] for r in paths if r["statut"] == "renommé"}
    occupied = (existing_files(root) | makefile_artifacts(root)) - renamed_away
    targets = {}
    for row in paths:
        if row["nouveau"]:
            targets.setdefault(row["nouveau"], []).append(row["ancien"])
    for target, olds in sorted(targets.items()):
        if len(set(olds)) > 1:
            report.fail("file target %s for %s" % (target, ", ".join(sorted(set(olds)))))
        for old in olds:
            if old != target and target in occupied:
                report.fail("file target %s of %s already exists" % (target, old))


def existing_files(root):
    found = set()
    for top in ("src", "tests", "tools"):
        for directory, dirs, files in os.walk(os.path.join(root, top)):
            dirs[:] = [d for d in dirs if d != "__pycache__"]
            for name in files:
                found.add(os.path.relpath(os.path.join(directory, name), root))
    return found


def makefile_artifacts(root):
    """tmp/ names the Makefile writes or reads: binaries, outputs, folders."""
    path = os.path.join(root, "Makefile")
    if not os.path.exists(path):
        return set()
    with open(path, encoding="utf-8") as f:
        text = f.read()
    return {"tmp/" + m for m in re.findall(r"\$\(TMP_DIR\)/([A-Za-z0-9_.-]+)", text)}


# --- section 4 form --------------------------------------------------------

def check_form(declarations, report):
    for d in declarations:
        if not d.new or d.new == d.name:
            continue
        if d.kind == "type" and not re.match(r"^[a-z][a-z0-9_]*_t$", d.new):
            report.warn("type not snake_case_t: %s" % describe_new(d))
        if d.kind in ("macro", "enum_constant") and not re.match(r"^[A-Z][A-Z0-9_]*$", d.new):
            report.warn("%s not UPPER_CASE: %s" % (d.kind, describe_new(d)))
        if re.match(r"^p8_", d.new, re.I) and not d.file.startswith("src/p8/"):
            report.warn("p8_ outside src/p8/: %s" % describe_new(d))


def describe_new(d):
    return "%s -> %s (%s)" % (d.name, d.new, d.file)


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--table", required=True)
    parser.add_argument("--declarations", required=True)
    parser.add_argument("--remainder", help="the remainder of `make rename-derive`")
    parser.add_argument("--root", required=True, help="the tree whose file names the targets must avoid")
    args = parser.parse_args(argv)

    header, rows = rename_tables.read_table(args.table)
    declarations = [Decl(row) for row in rename_tables.read_declarations(args.declarations)]
    remainder = []
    if args.remainder and os.path.exists(args.remainder):
        _, remainder = rename_tables.read_table(args.remainder)
    report = Report()
    check_exhaustive(header, rows, declarations, remainder, report)
    check_collisions(declarations, report)
    check_others(rows, args.root, report)
    check_form(declarations, report)
    for text in report.warnings:
        print("warning: " + text)
    for text in report.violations:
        print("violation: " + text)
    counts = {}
    for row in rows:
        counts[row["catégorie"]] = counts.get(row["catégorie"], 0) + 1
    print("rename_check: %s ; %d violation(s), %d warning(s)" % (
        ", ".join("%d %s" % (n, c) for c, n in sorted(counts.items())),
        len(report.violations), len(report.warnings)))
    return 1 if report.violations else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
