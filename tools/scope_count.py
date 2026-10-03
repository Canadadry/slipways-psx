"""Scope counter (docs/plan-5.md section 5, guard 2): the declarations of
`make rename-extract` whose name has one or two characters and is refused,
per module of the declaring file. Refused: a type, tag, function, field,
enum constant, macro, static or global (one column each); a local or a
parameter whose scope spans more than 40 lines (column long-scope), and so
is a static declared in a function, judged as a local by its scope. Allowed:
any other local or parameter (a parameter of a function-pointer type has the
scope of its prototype), and the (kind, parent, name) triples of the
exceptions file. Exit 0 unless --check: then exit 1 if the total is above 0,
and 2 if the declarations are missing or older than the sources.

With --cryptic and --table docs/renommage.tsv, the cryptic-locals list
instead: each local or parameter whose scope spans more than 40 lines (a
parameter of a function-pointer type has the scope of its prototype) and
whose name has three characters or less, or has a part, split on `_`, that
is no part of the new name of a renamed row of the table (the English a
human validated); the parameters `argc` and `argv` of a function `main`
are not candidates. One line per candidate, then the total per module."""

import argparse
import fnmatch
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_derive  # noqa: E402
import rename_substeps  # noqa: E402
import rename_tables  # noqa: E402

REFUSED_KINDS = ("type", "tag", "function", "field", "enum_constant", "macro", "static",
                 "global")
COLUMNS = REFUSED_KINDS + ("long-scope",)
SCOPE_KINDS = ("local", "param")
MAX_SCOPE_LINES = 40
SOURCE_DIRS = ("src", "tests", "tools")
MAIN_PARAMS = ("argc", "argv")


def column_of(row):
    """The column a declaration is refused under, or None if allowed."""
    if len(row["name"]) > 2:
        return None
    if row["kind"] == "static" and row["parent"] != "-":
        # Not through is_long_scope: --cryptic would gain the long statics.
        return "long-scope" if past_max_scope(row) else None
    if row["kind"] in REFUSED_KINDS:
        return row["kind"]
    return "long-scope" if is_long_scope(row) else None


def is_long_scope(row):
    if row["kind"] not in SCOPE_KINDS:
        return False
    if row["kind"] == "param" and "." in row["parent"]:
        # A param of a function-pointer type: the extraction names its
        # translation unit as parent and runs its scope to the unit's end.
        return False
    return past_max_scope(row)


def past_max_scope(row):
    return row["scope_lines"].isdigit() and int(row["scope_lines"]) > MAX_SCOPE_LINES


def validated_words(table):
    """The lowercase `_` parts of the new names of the renamed rows."""
    words = set()
    for row in rename_tables.read_table(table)[1]:
        if row.get("statut") == "renommé":
            words.update(w.lower() for w in re.split(r"[^A-Za-z0-9]+", row["nouveau"]) if w)
    return words


def is_cryptic(row, words):
    if not is_long_scope(row):
        return False
    if row["kind"] == "param" and row["parent"] == "main" and row["name"] in MAIN_PARAMS:
        return False
    parts = [p.lower() for p in row["name"].split("_") if p]
    return len(row["name"]) <= 3 or any(p not in words for p in parts)


def newest_source(root):
    newest = 0
    for top in SOURCE_DIRS:
        for directory, _, files in os.walk(os.path.join(root, top)):
            for name in files:
                if name.endswith((".c", ".h")):
                    newest = max(newest, os.path.getmtime(os.path.join(directory, name)))
    return newest


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--declarations", required=True,
                        help="declarations.tsv of `make rename-extract`")
    parser.add_argument("--exceptions", help="the exceptions file; not read by --cryptic")
    parser.add_argument("--root", required=True)
    parser.add_argument("--module", action="append",
                        help="only these modules (shell patterns: game/planet*)")
    parser.add_argument("--substep", help="only this sub-step's modules")
    parser.add_argument("--substeps", help="the sub-step table, with --substep")
    parser.add_argument("--cryptic", action="store_true",
                        help="list the cryptic long-scope locals and params instead")
    parser.add_argument("--table", help="docs/renommage.tsv, with --cryptic")
    parser.add_argument("--check", action="store_true",
                        help="exit 1 if anything is refused, 2 without a fresh extraction")
    args = parser.parse_args(argv)
    if args.cryptic and not args.table:
        parser.error("--cryptic needs --table")
    if not args.cryptic and not args.exceptions:
        parser.error("the following arguments are required: --exceptions")
    if args.check and args.cryptic:
        parser.error("--check guards the scope count: drop --cryptic")
    if args.check and stale(args):
        print("error: %s is missing or older than the sources; run `make rename-extract`"
              % args.declarations, file=sys.stderr)
        return 2

    selected = None
    if args.substep:
        substeps = rename_substeps.load(args.substeps)
        if args.substep not in substeps.modules:
            parser.error("unknown sub-step %s" % args.substep)
        selected = set(substeps.modules[args.substep])

    def selected_module(row):
        module = rename_derive.module_of(row["file"])
        if selected is not None and module not in selected:
            return None
        if args.module and not any(fnmatch.fnmatchcase(module, m) for m in args.module):
            return None
        return module

    if args.cryptic:
        print_cryptic(args, selected_module)
        return 0

    exceptions = set(rename_tables.read_triples(args.exceptions))
    table = {}
    for row in rename_tables.read_declarations(args.declarations):
        column = column_of(row)
        if column is None or (row["kind"], row["parent"], row["name"]) in exceptions:
            continue
        module = selected_module(row)
        if module is None:
            continue
        table.setdefault(module, dict.fromkeys(COLUMNS, 0))[column] += 1

    total = dict.fromkeys(COLUMNS, 0)
    width = max([len(m) for m in table] + [len("module")])
    print("%-*s %s %10s" % (width, "module", " ".join("%13s" % c for c in COLUMNS), "total"))
    for module in sorted(table) + ["total"]:
        row = table.get(module, total)
        if module != "total":
            for column in COLUMNS:
                total[column] += row[column]
        print("%-*s %s %10d" % (width, module, " ".join("%13d" % row[c] for c in COLUMNS),
                                sum(row.values())))
    warn_if_stale(args)
    return 1 if args.check and sum(total.values()) else 0


def print_cryptic(args, selected_module):
    words = validated_words(args.table)
    found = []
    for row in rename_tables.read_declarations(args.declarations):
        module = selected_module(row)
        if module is not None and is_cryptic(row, words):
            found.append((module, row["kind"], row["name"], row["parent"], row["scope_lines"]))
    found.sort(key=lambda c: (c[0], c[3], c[2]))
    print("module kind name parent scope_lines")
    for candidate in found:
        print("\t".join(candidate))
    print("module candidates")
    per_module = {}
    for candidate in found:
        per_module[candidate[0]] = per_module.get(candidate[0], 0) + 1
    for module in sorted(per_module):
        print("%s %d" % (module, per_module[module]))
    print("total %d" % len(found))
    warn_if_stale(args)


def stale(args):
    return (not os.path.exists(args.declarations)
            or os.path.getmtime(args.declarations) < newest_source(args.root))


def warn_if_stale(args):
    if os.path.getmtime(args.declarations) < newest_source(args.root):
        print("warning: %s is older than the sources; run `make rename-extract`"
              % args.declarations)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
