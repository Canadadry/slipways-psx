"""Anti-cart counter (docs/plan-5.md section 5, guard 1): the old names of
the renamed rows of docs/renommage.tsv still present in the code, per module
of the file they are found in, in four columns:

  identifier  in C code;
  backtick    inside `...` in a comment or an assert message;
  string      in a C string literal, or in the text of a script, data file
              or Makefile;
  comment     bare in a comment or an assert message (outside backticks).

A distinctive name (three characters or more, not a plain lowercase word,
neither kept by another row nor some row's new name) is found by text, as a
whole word. Any other name is ambiguous: as an identifier it is counted from
the AST occurrences of its renamed identities only; in a backtick span or a
string, only when the span or the string is exactly the name, and in a string
only at the sites the table lists for it. With --substep, the files of that
sub-step's modules (tools/rename_substeps.tsv), each mention filed under
`done` (its name belongs to that sub-step or an earlier one) or `later`,
then the residue bound.

--part text counts what a text search alone finds, without reading the
extraction; --part ast the identifier column of the ambiguous names, from
the extraction; the two add up, column by column, to the count without
--part. Exit 0 unless --check: then exit 1 if the TOTAL is above 0, and 2
if the extraction the count needs is missing or older than the sources."""

import argparse
import fnmatch
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_derive  # noqa: E402
import rename_strings  # noqa: E402
import rename_substeps  # noqa: E402
import rename_tables  # noqa: E402

COLUMNS = ("identifier", "backtick", "string", "comment")
C_FILES = (".c", ".h")
TEXT_FILES = (".c", ".h", ".sh", ".txt", ".py", ".json", ".p8")
WORD = re.compile(r"[A-Za-z0-9_]+")
BACKTICK = re.compile(r"`([^`\n]+)`")
TMP_PATH = re.compile(r"^tmp/(.+)$")
LINE_COMMENT = {".sh": "#", ".txt": "#", ".py": "#", ".p8": "--", "": "#"}


# --- the table ---------------------------------------------------------------

class Names:
    """What the counter looks for, read from the table."""

    def __init__(self, rows):
        renamed = [r for r in rows if r["statut"] == "renommé"]
        kept = {r["ancien"] for r in rows if r["statut"] != "renommé"}
        new = {r["nouveau"] for r in renamed}
        olds = {r["ancien"] for r in renamed}
        self.ambiguous = {n for n in olds if is_ambiguous(n, kept, new)}
        self.distinctive_words = {n for n in olds - self.ambiguous if WORD.fullmatch(n)}
        self.paths = [(n, path_pattern(n)) for n in sorted(olds - self.ambiguous)
                      if not WORD.fullmatch(n) and "/" in n]
        self.ambiguous_identities = {r["identité"] for r in renamed
                                     if r["catégorie"] == "identifiant"
                                     and r["ancien"] in self.ambiguous}
        self.string_sites = {}  # file -> ambiguous names the table lists there
        for r in renamed:
            if r["catégorie"] != "identifiant" and r["ancien"] in self.ambiguous:
                self.string_sites.setdefault(r["fichier"], set()).add(r["ancien"])


def is_ambiguous(name, kept, new):
    return (len(name) < 3 or re.fullmatch(r"[a-z]+", name) is not None
            or name in kept or name in new)


def path_pattern(name):
    """`tmp/pico8_replay` is also written `$(TMP_DIR)/pico8_replay`."""
    match = TMP_PATH.match(name)
    body = re.escape(match.group(1)) if match else re.escape(name)
    prefix = r"(?:tmp|\$\(TMP_DIR\))/" if match else ""
    return re.compile(r"(?<![A-Za-z0-9_])%s%s(?![A-Za-z0-9_])" % (prefix, body))


# --- regions of a file -------------------------------------------------------

def c_regions(text):
    """(kind, start, end) of a C file, kind in code, comment, string,
    message (an assert message); character constants are skipped."""
    i, n = 0, len(text)
    code_start = 0
    message_end = None
    while i < n:
        if text.startswith("//", i) or text.startswith("/*", i):
            if text[i + 1] == "/":
                j = text.find("\n", i)
                j = n if j < 0 else j
            else:
                j = text.find("*/", i + 2)
                j = n if j < 0 else j + 2
            yield "code", code_start, i
            yield "comment", i, j
            i = code_start = j
        elif text[i] in "'\"":
            quote = text[i]
            j = i + 1
            while j < n and text[j] != quote:
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            if quote == '"':
                yield "code", code_start, i
                continued = message_end is not None and not text[message_end:i].strip()
                if rename_strings.assert_message(text, i) or continued:
                    message_end = j
                    yield "message", i + 1, j - 1
                else:
                    message_end = None
                    yield "string", i + 1, j - 1
                code_start = j
            i = j
        else:
            i += 1
    yield "code", code_start, n


def text_regions(text, ext):
    """(kind, start, end) of a script, data file or Makefile: comment lines,
    and text, which is all the rest."""
    marker = LINE_COMMENT.get(ext)
    start = 0
    for line in text.splitlines(keepends=True):
        stripped = line.lstrip()
        kind = "comment" if marker and stripped.startswith(marker) else "text"
        yield kind, start, start + len(line)
        start += len(line)


# --- counting ----------------------------------------------------------------

def count_file(path, text, names, ast_spans, part=None, located=None, halves=("text", "ast")):
    """{column: n} for one file; ast_spans maps the (offset, length) of the
    AST occurrences of ambiguous renamed identities in it to their identity;
    halves says which of the text and AST counts to make.
    With part(path, column, name, identity) -> key, {key: {column: n}}
    instead; located maps the character offset of an AST occurrence of a
    distinctive renamed identity to that identity, given to part when a
    mention found by text starts there."""
    located = located or {}
    split = {}
    flat = dict.fromkeys(COLUMNS, 0)

    def add(column, name=None, identity=None):
        if part is None:
            flat[column] += 1
        else:
            split.setdefault(part(path, column, name, identity), dict.fromkeys(COLUMNS, 0))[column] += 1

    ext = os.path.splitext(path)[1]
    regions = c_regions(text) if ext in C_FILES else text_regions(text, ext)
    if "text" not in halves:
        regions = []
    listed = names.string_sites.get(path, set())
    for kind, start, end in regions:
        chunk = text[start:end]
        if kind == "code":
            for at, name in distinctive(chunk, names, positions=True):
                add("identifier", name, located.get(start + at))
        elif kind in ("comment", "message"):
            spans = []
            for match in BACKTICK.finditer(chunk):
                inner = match.group(1)
                for name in distinctive(inner, names):
                    add("backtick", name)
                if inner in names.ambiguous:
                    add("backtick", inner)
                spans.append(match.span())
            bare = chunk
            for a, b in reversed(spans):
                bare = bare[:a] + " " * (b - a) + bare[b:]
            for name in distinctive(bare, names):
                add("comment", name)
        elif kind == "string":
            for name in distinctive(chunk, names):
                add("string", name)
            if chunk in listed:
                add("string", chunk)
        elif kind == "text":
            for name in distinctive(chunk, names):
                add("string", name)
            for w in WORD.findall(chunk):
                if w in listed:
                    add("string", w)
    for identity in (ast_spans.values() if "ast" in halves else ()):
        add("identifier", identity=identity)
    return flat if part is None else split


def distinctive(chunk, names, positions=False):
    """The distinctive names in a chunk, once per mention, with their
    offset in it if positions: paths first, then whole words outside them."""
    spans = []
    found = []
    for name, pattern in names.paths:
        for m in pattern.finditer(chunk):
            spans.append(m.span())
            found.append((m.start(), name))
    for match in WORD.finditer(chunk):
        if match.group(0) in names.distinctive_words and not any(
                a <= match.start() < b for a, b in spans):
            found.append((match.start(), match.group(0)))
    return found if positions else [name for _, name in found]


def char_offsets(text, byte_spans):
    """{character offset: identity} for {(byte offset, length): identity}."""
    if text.isascii():
        return {offset: identity for (offset, _), identity in byte_spans.items()}
    wanted = {offset for offset, _ in byte_spans}
    at = {}
    position = 0
    for i, c in enumerate(text):
        if position in wanted:
            at[position] = i
        position += len(c.encode("utf-8"))
    return {at[offset]: identity for (offset, _), identity in byte_spans.items() if offset in at}


def read_occurrences(path, identities):
    """{file: {(offset, length): identity}} of the given identities."""
    found = {}
    with open(path, encoding="utf-8") as f:
        next(f, None)
        for line in f:
            cells = line.rstrip("\n").split("\t")
            if len(cells) >= 4 and cells[0] in identities:
                found.setdefault(cells[1], {})[(int(cells[2]), int(cells[3]))] = cells[0]
    return found


def source_files(root):
    """The code: src/, tests/, tools/, the Makefile, without what the
    dictionary step skips (its own tools, the e2e scripts, third_party)."""
    found = []
    for top in ("src", "tests", "tools"):
        for directory, dirs, files in os.walk(os.path.join(root, top)):
            rel_dir = os.path.relpath(directory, root)
            dirs[:] = sorted(d for d in dirs if not any(
                (rel_dir + "/" + d).startswith(p) for p in rename_strings.SKIPPED_PARTS))
            for name in sorted(files):
                path = os.path.join(rel_dir, name)
                if name.endswith(TEXT_FILES) and not rename_strings.SKIPPED_FILES.match(path):
                    found.append(path)
    if os.path.exists(os.path.join(root, "Makefile")):
        found.append("Makefile")
    return found


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--table", required=True)
    parser.add_argument("--occurrences",
                        help="occurrences.tsv of `make rename-extract`; not read by --part text")
    parser.add_argument("--root", required=True)
    parser.add_argument("--module", action="append",
                        help="only these modules (shell patterns: game/planet*)")
    parser.add_argument("--substep", help="only this sub-step's modules, split done / later")
    parser.add_argument("--substeps", help="the sub-step table, with --substep")
    parser.add_argument("--part", choices=("text", "ast"), help="only this half of the count")
    parser.add_argument("--check", action="store_true",
                        help="exit 1 if anything is counted, 2 without a fresh extraction")
    args = parser.parse_args(argv)
    if args.check and args.substep:
        parser.error("--check does not split by sub-step: drop --substep")
    if args.part and args.substep:
        parser.error("--part does not split by sub-step: drop --substep")
    if args.part != "text" and not args.occurrences:
        parser.error("--occurrences is required, except with --part text")
    halves = (args.part,) if args.part else ("text", "ast")

    _, rows = rename_tables.read_table(args.table)
    names = Names(rows)
    part = None
    if args.substep:
        substeps = rename_substeps.load(args.substeps)
        if args.substep not in substeps.modules:
            parser.error("unknown sub-step %s" % args.substep)
        membership = rename_substeps.Membership(rows, substeps)
        current = substeps.index(args.substep)

        def part(path, column, name, identity):
            step = (membership.of_identity(identity) if identity
                    else membership.of_mention(name, column, path))
            return "done" if step is None or substeps.index(step) <= current else "later"
    files = source_files(args.root)
    notes = []
    distinct = set()
    if args.substep:
        distinct = {r["identité"] for r in rows if r["catégorie"] == "identifiant"
                    and r["statut"] == "renommé" and r["ancien"] in names.distinctive_words}
    if args.part == "text":
        ast = {}
    elif os.path.exists(args.occurrences):
        ast = read_occurrences(args.occurrences, names.ambiguous_identities | distinct)
        newest = max((os.path.getmtime(os.path.join(args.root, p)) for p in files
                      if p.endswith(C_FILES)), default=0)
        if os.path.getmtime(args.occurrences) < newest:
            notes.append("warning: %s is older than the sources; run `make rename-extract`"
                         % args.occurrences)
    else:
        ast = {}
        notes.append("warning: no %s: ambiguous identifiers not counted; run "
                     "`make rename-extract`" % args.occurrences)
    if args.check and notes:
        print(notes[0].replace("warning:", "error:", 1), file=sys.stderr)
        return 2

    table = {}
    for path in files:
        module = rename_derive.module_of(path)
        if args.substep and module not in substeps.modules[args.substep]:
            continue
        if args.module and not any(fnmatch.fnmatchcase(module, m) for m in args.module):
            continue
        with open(os.path.join(args.root, path), encoding="utf-8", errors="replace") as f:
            text = f.read()
        spans = ast.get(path, {})
        located = None
        if distinct:
            located = char_offsets(text, {k: v for k, v in spans.items() if v in distinct})
            spans = {k: v for k, v in spans.items() if v not in distinct}
        found = count_file(path, text, names, spans, part, located, halves)
        if part is None:
            found = {module: found}
        else:
            found = {(module, key): counts for key, counts in found.items()}
        for key, counts in found.items():
            if any(counts.values()):
                row = table.setdefault(key, dict.fromkeys(COLUMNS, 0))
                for column in COLUMNS:
                    row[column] += counts[column]

    if part is not None:
        print_split(table, args.substep, substeps.bound[args.substep])
        for note in notes:
            print(note)
        return 0

    total = dict.fromkeys(COLUMNS, 0)
    width = max([len(m) for m in table] + [len("module")])
    print("%-*s %s %s" % (width, "module", " ".join("%10s" % c for c in COLUMNS), "%10s" % "total"))
    for module in sorted(table):
        row = table[module]
        for column in COLUMNS:
            total[column] += row[column]
        print("%-*s %s %10d" % (width, module, " ".join("%10d" % row[c] for c in COLUMNS),
                                sum(row.values())))
    print("%-*s %s %10d" % (width, "TOTAL", " ".join("%10d" % total[c] for c in COLUMNS),
                            sum(total.values())))
    for note in notes:
        print(note)
    return 1 if args.check and sum(total.values()) else 0


def print_split(table, substep, bound):
    width = max([len(m) for m, _ in table] + [len("module")])
    cells = lambda row: " ".join("%10d" % row[c] for c in COLUMNS)  # noqa: E731
    print("%-*s %-5s %s %10s" % (width, "module", "part", " ".join("%10s" % c for c in COLUMNS),
                                 "total"))
    totals = {key: dict.fromkeys(COLUMNS, 0) for key in ("done", "later")}
    for module, key in sorted(table):
        row = table[(module, key)]
        for column in COLUMNS:
            totals[key][column] += row[column]
        print("%-*s %-5s %s %10d" % (width, module, key, cells(row), sum(row.values())))
    for key in ("done", "later"):
        print("%-*s %-5s %s %10d" % (width, "TOTAL", key, cells(totals[key]),
                                     sum(totals[key].values())))
    print("residue %s: later identifier %d, bound %s" % (substep, totals["later"]["identifier"],
                                                         bound))


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
