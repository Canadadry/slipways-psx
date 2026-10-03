"""Cart citations (docs/plan-5.md, step 4): what the comments and assert
messages still cite of the cart that `cart-names-count` does not see, per
module of the file, in three columns:

  cart   the word `cart`, any case, whole (`cart_load` is not it), outside
         the files of the module game/cart;
  lua    the word `Lua`, any case, or `lua.lua`;
  short  outside backticks, the old name of a renamed row of the table of
         two characters or less, as a whole word not next to an apostrophe
         (`it's`) nor joined by a hyphen (`re-entry`), not a file name piece
         (`p.c`), not in `i.e.`, `e.g.` nor an escape (`\\t`), unless it is
         a common English word (ENGLISH); a French word that is also an old
         name stays counted.

Reads the comment and message regions of tools/rename_count.py, in the
files it reads, minus this tool's own files and the --exceptions file.
Exceptions: lines `column<TAB>module<TAB>token`, each removing exactly the
citations of that token, any case, in that column and module. Prints one
site per line (`file:line column token`), then the counts per module, then,
over the whole tree (no --module, no --substep), a `warning:` line for each
exceptions line that removed nothing. Never fails: exit 0."""

import argparse
import fnmatch
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_count  # noqa: E402
import rename_derive  # noqa: E402
import rename_substeps  # noqa: E402
import rename_tables  # noqa: E402

COLUMNS = ("cart", "lua", "short")
OWN_FILES = ("tools/cart_citations.py", "tests/test_cart_citations.py")
CART_MODULE = "game/cart"
ENGLISH = frozenset("a I at on is in to of by as or an up it go no do if be we me my so us he "
                    "am ok x y".split())
CART = re.compile(r"(?<![A-Za-z0-9_])cart(?![A-Za-z0-9_])", re.IGNORECASE)
LUA = re.compile(r"(?<![A-Za-z0-9_.])lua(?:\.lua)?(?![A-Za-z0-9_])", re.IGNORECASE)
# Two characters at most between neither a word character (accented letters
# included), an apostrophe, nor a hyphen that joins a word (`re-entry`).
NAME = re.compile(r"(?<![\w'\u2019])(?<!\w-)\w{1,2}(?![\w'\u2019])(?!-\w)")
NOT_NAMES = re.compile(r"\b(?:i\.e|e\.g)\.|\\[a-z]")
EXTENSIONS = frozenset("c h o d sh py txt json md p8 tsv".split())


def short_names(table):
    return {r["ancien"] for r in rename_tables.read_table(table)[1]
            if r["statut"] == "renommé" and len(r["ancien"]) <= 2} - ENGLISH


def citations(path, text, module, names):
    """(offset, column, token) of each citation of a file."""
    for kind, start, end in rename_count.c_regions(text) \
            if path.endswith(rename_count.C_FILES) \
            else rename_count.text_regions(text, os.path.splitext(path)[1]):
        if kind not in ("comment", "message"):
            continue
        chunk = text[start:end]
        if module != CART_MODULE:
            for match in CART.finditer(chunk):
                yield start + match.start(), "cart", match.group(0)
        for match in LUA.finditer(chunk):
            yield start + match.start(), "lua", match.group(0)
        bare = rename_count.BACKTICK.sub(lambda m: " " * len(m.group(0)), chunk)
        bare = NOT_NAMES.sub(lambda m: " " * len(m.group(0)), bare)
        for match in NAME.finditer(bare):
            if match.group(0) in names and not in_file_name(bare, match):
                yield start + match.start(), "short", match.group(0)


def in_file_name(chunk, match):
    """`p.c`: a file stem before its extension, or the extension itself."""
    after = re.match(r"\.(\w+)", chunk[match.end():])
    if after and after.group(1) in EXTENSIONS:
        return True
    return match.group(0) in EXTENSIONS and chunk[match.start() - 1:match.start()] == "." \
        and match.start() >= 2 and chunk[match.start() - 2].isalnum()


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--table", required=True)
    parser.add_argument("--root", required=True)
    parser.add_argument("--substeps", help="the sub-step table, with --substep")
    parser.add_argument("--module", action="append",
                        help="only these modules (shell patterns: game/planet*)")
    parser.add_argument("--substep", help="only this sub-step's modules")
    parser.add_argument("--exceptions",
                        help="lines `column<TAB>module<TAB>token` not counted; not read")
    args = parser.parse_args(argv)

    selected = None
    if args.substep:
        substeps = rename_substeps.load(args.substeps) if args.substeps else None
        if substeps is None or args.substep not in substeps.modules:
            parser.error("unknown sub-step %s (or no --substeps)" % args.substep)
        selected = set(substeps.modules[args.substep])
    names = short_names(args.table)
    skipped = set(OWN_FILES)
    exceptions = set()
    numbered = []
    used = set()
    if args.exceptions:
        numbered = rename_tables.read_numbered_triples(args.exceptions)
        exceptions = {(column, module, token.lower()) for _, (column, module, token) in numbered}
        skipped.add(os.path.relpath(os.path.abspath(args.exceptions), os.path.abspath(args.root)))

    table = {}
    for path in rename_count.source_files(args.root):
        module = rename_derive.module_of(path)
        if path in skipped or (selected is not None and module not in selected):
            continue
        if args.module and not any(fnmatch.fnmatchcase(module, m) for m in args.module):
            continue
        with open(os.path.join(args.root, path), encoding="utf-8", errors="replace") as f:
            text = f.read()
        for offset, column, token in citations(path, text, module, names):
            if (column, module, token.lower()) in exceptions:
                used.add((column, module, token.lower()))
                continue
            print("%s:%d %s %s" % (path, text.count("\n", 0, offset) + 1, column, token))
            table.setdefault(module, dict.fromkeys(COLUMNS, 0))[column] += 1

    total = dict.fromkeys(COLUMNS, 0)
    width = max([len(m) for m in table] + [len("module")])
    print("%-*s %s %10s" % (width, "module", " ".join("%10s" % c for c in COLUMNS), "total"))
    for module in sorted(table) + ["TOTAL"]:
        row = table.get(module, total)
        if module != "TOTAL":
            for column in COLUMNS:
                total[column] += row[column]
        print("%-*s %s %10d" % (width, module, " ".join("%10d" % row[c] for c in COLUMNS),
                                sum(row.values())))
    if args.exceptions and not args.module and not args.substep:
        for line in rename_tables.dead_lines(args.exceptions, numbered, used,
                                             lambda t: (t[0], t[1], t[2].lower())):
            print("warning: " + line)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
