"""French counter (docs/plan-5.md, step 4): the French words left in the
code, per module of the file they are found in, in the four columns of
tools/rename_count.py:

  identifier  a part of a C identifier, split on `_`;
  backtick    inside `...` in a comment or an assert message;
  string      in a C string literal, or in the text of a script, data file
              or Makefile;
  comment     elsewhere in a comment or an assert message.

A word (letters, accented ones included) is French if it is one of
FUNCTION_WORDS (articles, prepositions, conjunctions, pronouns, auxiliaries,
negation and quantity adverbs; never a common English word such as `on`,
`a`, `plus`, nor a content word such as `place`, `file`, `limite`), or if it
has an accented Latin letter (the code is ASCII without accents; the signs
`×` and `÷` are not letters). Blind spot: a French sentence with neither a
function word nor an accent passes (`zone ... pleine, limite 4 places`).

Files: those of `cart-names-count` (rename_count.source_files), minus this
counter's own files, which hold its word list. Exceptions: lines
`column<TAB>module<TAB>word` of the exceptions file, each removing exactly
the mentions of that word in that column and module. With --substep, the
files of that sub-step's modules, each mention `done` or `later`: `later`
when the whole text of its literal, or of its data line, is also a literal
or a line of a file of a later sub-step (a likely writer elsewhere, whom
the string ownership rule leaves it to). Warns for each module read that is
in no sub-step. Exit 0 unless --check: then exit 1 if the TOTAL after the
exceptions is above 0."""

import argparse
import fnmatch
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_count  # noqa: E402
import rename_derive  # noqa: E402
import rename_substeps  # noqa: E402

COLUMNS = ("identifier", "backtick", "string", "comment")
OWN_FILES = ("tools/french_count.py", "tests/test_french_count.py", "tools/french_exceptions.txt")
FUNCTION_WORDS = frozenset("""
    le la les un une des du au aux
    de dans par pour sur sous avec sans chez vers entre depuis pendant selon jusqu jusque
    et ou mais donc ni que qui quand lorsque puisque parce comme dont quoi
    je tu il ils elle elles nous vous lui leur leurs eux moi toi soi
    celui celle ceux celles cela ceci ce cet cette ces sa ses nos vos notre votre
    est sont etre ete etait etaient sera seront avoir ont avait aura peut doit faut
    ne pas jamais rien aucun aucune tout tous toute toutes chaque trop peu beaucoup
    moins encore aussi deja apres tres meme
""".split())
# Latin-1 letters (without the signs U+00D7 and U+00F7), plus the ligatures.
ACCENTED = "À-ÖØ-öø-ÿŒœ"
WORD = re.compile("[A-Za-z%s]+" % ACCENTED)
HAS_ACCENT = re.compile("[%s]" % ACCENTED)
IDENTIFIER = re.compile(r"(?<![A-Za-z0-9_])[A-Za-z_][A-Za-z0-9_]*")
ESCAPE = re.compile(r"\\.")


def is_french(word):
    return word.lower() in FUNCTION_WORDS or HAS_ACCENT.search(word) is not None


def is_option(chunk, at):
    """A word right after a `-` that ends no word: a command option (`-ne`)."""
    return at > 0 and chunk[at - 1] == "-" and (at < 2 or not chunk[at - 2].isalpha())


def name_parts(chunk):
    """(offset, part) of the function words of the C names of a chunk that
    have two parts or more: a one-word name (`le`, `et`) is a name, often
    a kept two-letter local of the cart, not French."""
    found = []
    for match in IDENTIFIER.finditer(chunk):
        parts = match.group(0).split("_")
        if len([p for p in parts if p]) < 2:
            continue
        at = match.start()
        for part in parts:
            if part.lower() in FUNCTION_WORDS:
                found.append((at, part))
            at += len(part) + 1
    return found


def regions(path, text):
    ext = os.path.splitext(path)[1]
    if ext in rename_count.C_FILES:
        return rename_count.c_regions(text)
    return rename_count.text_regions(text, ext)


def mentions(path, text):
    """(offset, column, word, whole) of each French word of a file; whole is
    the text of its literal or data line, None in code and comments."""
    for kind, start, end in regions(path, text):
        chunk = text[start:end]
        if kind == "code":
            for at, part in name_parts(chunk):
                yield start + at, "identifier", part, None
        elif kind in ("comment", "message"):
            bare = chunk
            for match in reversed(list(rename_count.BACKTICK.finditer(chunk))):
                inner = match.group(1)
                found = [(w.start(), w.group(0)) for w in WORD.finditer(inner)
                         if HAS_ACCENT.search(w.group(0))]
                for at, word in sorted(found + name_parts(inner)):
                    yield start + match.start(1) + at, "backtick", word, None
                a, b = match.span()
                bare = bare[:a] + " " * (b - a) + bare[b:]
            for word in WORD.finditer(bare):
                if is_french(word.group(0)):
                    yield start + word.start(), "comment", word.group(0), None
        else:
            scanned = ESCAPE.sub("  ", chunk)
            whole = chunk.strip()
            for word in WORD.finditer(scanned):
                if is_french(word.group(0)) and not is_option(scanned, word.start()):
                    yield start + word.start(), "string", word.group(0), whole


def wholes(path, text):
    """The literals and data lines of a file, as `mentions` gives them."""
    return {text[start:end].strip() for kind, start, end in regions(path, text)
            if kind in ("string", "text")}


def read_exceptions(path):
    """Lines `column<TAB>module<TAB>word`; `#` lines are comments."""
    found = set()
    with open(path, encoding="utf-8") as f:
        for line in f.read().splitlines():
            if line.strip() and not line.startswith("#"):
                column, module, word = (line.split("\t") + ["", ""])[:3]
                found.add((column, module, word.lower()))
    return found


def read(root, path):
    with open(os.path.join(root, path), encoding="utf-8", errors="replace") as f:
        return f.read()


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", required=True)
    parser.add_argument("--exceptions", required=True)
    parser.add_argument("--substeps", help="the sub-step table")
    parser.add_argument("--module", action="append",
                        help="only these modules (shell patterns: game/planet*)")
    parser.add_argument("--substep", help="only this sub-step's modules, split done / later")
    parser.add_argument("--sites", action="store_true", help="list each mention first")
    parser.add_argument("--check", action="store_true", help="exit 1 if anything is counted")
    args = parser.parse_args(argv)
    if args.check and args.substep:
        parser.error("--check does not split by sub-step: drop --substep")

    substeps = rename_substeps.load(args.substeps) if args.substeps else None
    if args.substep and (substeps is None or args.substep not in substeps.modules):
        parser.error("unknown sub-step %s (or no --substeps)" % args.substep)
    exceptions = read_exceptions(args.exceptions)
    files = [p for p in rename_count.source_files(args.root) if p not in OWN_FILES]

    later_wholes = set()
    if args.substep:
        current = substeps.index(args.substep)
        for path in files:
            step = substeps.of_module(rename_derive.module_of(path))
            if step is not None and substeps.index(step) > current:
                later_wholes |= wholes(path, read(args.root, path))

    table = {}
    sites = []
    unplaced = set()
    for path in files:
        module = rename_derive.module_of(path)
        if args.substep and module not in substeps.modules[args.substep]:
            continue
        if args.module and not any(fnmatch.fnmatchcase(module, m) for m in args.module):
            continue
        if substeps is not None and substeps.of_module(module) is None:
            unplaced.add(module)
        text = read(args.root, path)
        for offset, column, word, whole in mentions(path, text):
            if (column, module, word.lower()) in exceptions:
                continue
            part = None
            if args.substep:
                part = "later" if whole is not None and whole in later_wholes else "done"
            row = table.setdefault((module, part), dict.fromkeys(COLUMNS, 0))
            row[column] += 1
            sites.append("%s:%d %s %s%s" % (path, text.count("\n", 0, offset) + 1, column, word,
                                            " " + part if part else ""))

    if args.sites:
        for site in sites:
            print(site)
    print_table(table, split=bool(args.substep))
    for module in sorted(unplaced):
        print("warning: module %s is in no sub-step of %s" % (module, args.substeps))
    return 1 if args.check and table else 0


def print_table(table, split):
    cells = lambda row: " ".join("%10d" % row[c] for c in COLUMNS)  # noqa: E731
    width = max([len(m) for m, _ in table] + [len("module")])

    def name(module, part):
        return "%-*s %-5s" % (width, module, part) if split else "%-*s" % (width, module)

    print("%s %s %10s" % (name("module", "part"), " ".join("%10s" % c for c in COLUMNS),
                          "total"))
    totals = {}
    for module, part in sorted(table):
        row = table[(module, part)]
        total = totals.setdefault(part, dict.fromkeys(COLUMNS, 0))
        for column in COLUMNS:
            total[column] += row[column]
        print("%s %s %10d" % (name(module, part), cells(row), sum(row.values())))
    for part in (("done", "later") if split else (None,)):
        total = totals.get(part, dict.fromkeys(COLUMNS, 0))
        print("%s %s %10d" % (name("TOTAL", part), cells(total), sum(total.values())))


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
