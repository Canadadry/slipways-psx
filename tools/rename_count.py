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
only at the sites the table lists for it. Never fails: exit 0."""

import argparse
import fnmatch
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_derive  # noqa: E402
import rename_strings  # noqa: E402

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
        self.paths = [path_pattern(n) for n in sorted(olds - self.ambiguous)
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


def read_table(path):
    rows = []
    columns = None
    with open(path, encoding="utf-8") as f:
        for line in f.read().splitlines():
            if not line or line.startswith("#"):
                continue
            cells = line.split("\t")
            if columns is None:
                columns = cells
                continue
            rows.append(dict(zip(columns, cells + [""] * (len(columns) - len(cells)))))
    return rows


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

def count_file(path, text, names, ast_spans):
    """{column: n} for one file; ast_spans are the (offset, length) of the
    AST occurrences of ambiguous renamed identities in it."""
    out = dict.fromkeys(COLUMNS, 0)
    ext = os.path.splitext(path)[1]
    regions = c_regions(text) if ext in C_FILES else text_regions(text, ext)
    listed = names.string_sites.get(path, set())
    for kind, start, end in regions:
        chunk = text[start:end]
        if kind == "code":
            out["identifier"] += len(distinctive(chunk, names))
        elif kind in ("comment", "message"):
            spans = []
            for match in BACKTICK.finditer(chunk):
                inner = match.group(1)
                hits = len(distinctive(inner, names)) + (inner in names.ambiguous)
                out["backtick"] += hits
                spans.append(match.span())
            bare = chunk
            for a, b in reversed(spans):
                bare = bare[:a] + " " * (b - a) + bare[b:]
            out["comment"] += len(distinctive(bare, names))
        elif kind == "string":
            out["string"] += len(distinctive(chunk, names)) + (chunk in listed)
        elif kind == "text":
            out["string"] += len(distinctive(chunk, names))
            out["string"] += sum(1 for w in WORD.findall(chunk) if w in listed)
    out["identifier"] += len(ast_spans)
    return out


def distinctive(chunk, names):
    """Spans of the distinctive names in a chunk: paths first, then whole
    words outside them."""
    spans = []
    for pattern in names.paths:
        spans += [m.span() for m in pattern.finditer(chunk)]
    for match in WORD.finditer(chunk):
        if match.group(0) in names.distinctive_words and not any(
                a <= match.start() < b for a, b in spans):
            spans.append(match.span())
    return spans


def read_occurrences(path, identities):
    """{file: {(offset, length)}} of the given identities."""
    found = {}
    with open(path, encoding="utf-8") as f:
        next(f, None)
        for line in f:
            cells = line.rstrip("\n").split("\t")
            if len(cells) >= 4 and cells[0] in identities:
                found.setdefault(cells[1], set()).add((int(cells[2]), int(cells[3])))
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
    parser.add_argument("--occurrences", required=True,
                        help="occurrences.tsv of `make rename-extract`")
    parser.add_argument("--root", required=True)
    parser.add_argument("--module", action="append",
                        help="only these modules (shell patterns: game/planet*)")
    args = parser.parse_args(argv)

    names = Names(read_table(args.table))
    files = source_files(args.root)
    notes = []
    if os.path.exists(args.occurrences):
        ast = read_occurrences(args.occurrences, names.ambiguous_identities)
        newest = max((os.path.getmtime(os.path.join(args.root, p)) for p in files
                      if p.endswith(C_FILES)), default=0)
        if os.path.getmtime(args.occurrences) < newest:
            notes.append("warning: %s is older than the sources; run `make rename-extract`"
                         % args.occurrences)
    else:
        ast = {}
        notes.append("warning: no %s: ambiguous identifiers not counted; run "
                     "`make rename-extract`" % args.occurrences)

    table = {}
    for path in files:
        module = rename_derive.module_of(path)
        if args.module and not any(fnmatch.fnmatchcase(module, m) for m in args.module):
            continue
        with open(os.path.join(args.root, path), encoding="utf-8", errors="replace") as f:
            text = f.read()
        found = count_file(path, text, names, ast.get(path, ()))
        if any(found.values()):
            row = table.setdefault(module, dict.fromkeys(COLUMNS, 0))
            for column in COLUMNS:
                row[column] += found[column]

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
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
