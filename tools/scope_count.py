"""Scope counter (docs/plan-5.md section 5, guard 2): the declarations of
`make rename-extract` whose name has one or two characters and is refused,
per module of the declaring file. Refused: a type, tag, function, field,
enum constant, macro, static or global (one column each); a local or a
parameter whose scope spans more than 40 lines (column long-scope). Allowed:
any other local or parameter (a parameter of a function-pointer type has the
scope of its prototype), and the (kind, parent, name) triples of the
exceptions file. Never fails: exit 0."""

import argparse
import fnmatch
import os
import sys

REFUSED_KINDS = ("type", "tag", "function", "field", "enum_constant", "macro", "static",
                 "global")
COLUMNS = REFUSED_KINDS + ("long-scope",)
SCOPE_KINDS = ("local", "param")
MAX_SCOPE_LINES = 40
SOURCE_DIRS = ("src", "tests", "tools")


def module_of(path):
    """`src/game/planet.c` -> `game/planet`."""
    stem = os.path.splitext(path)[0]
    return stem[len("src/"):] if stem.startswith("src/") else stem


def read_declarations(path):
    rows = []
    header = None
    with open(path, encoding="utf-8") as f:
        for line in f.read().splitlines():
            if not line or line.startswith("#"):
                continue
            cells = line.split("\t")
            if header is None:
                header = cells
                continue
            rows.append(dict(zip(header, cells)))
    return rows


def read_exceptions(path):
    """Lines `kind<TAB>parent<TAB>name`; `#` lines are comments."""
    found = set()
    with open(path, encoding="utf-8") as f:
        for line in f.read().splitlines():
            if line.strip() and not line.startswith("#"):
                found.add(tuple(line.split("\t")[:3]))
    return found


def column_of(row):
    """The column a declaration is refused under, or None if allowed."""
    if len(row["name"]) > 2:
        return None
    if row["kind"] in REFUSED_KINDS:
        return row["kind"]
    if row["kind"] == "param" and "." in row["parent"]:
        # A param of a function-pointer type: the extraction names its
        # translation unit as parent and runs its scope to the unit's end.
        return None
    if row["kind"] in SCOPE_KINDS and row["scope_lines"].isdigit() \
            and int(row["scope_lines"]) > MAX_SCOPE_LINES:
        return "long-scope"
    return None


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
    parser.add_argument("--exceptions", required=True)
    parser.add_argument("--root", required=True)
    parser.add_argument("--module", action="append",
                        help="only these modules (shell patterns: game/planet*)")
    parser.add_argument("--substep", help="only this sub-step's modules")
    parser.add_argument("--substeps", help="the sub-step table, with --substep")
    args = parser.parse_args(argv)

    selected = None
    if args.substep:
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        import rename_substeps
        substeps = rename_substeps.load(args.substeps)
        if args.substep not in substeps.modules:
            parser.error("unknown sub-step %s" % args.substep)
        selected = set(substeps.modules[args.substep])

    exceptions = read_exceptions(args.exceptions)
    table = {}
    for row in read_declarations(args.declarations):
        column = column_of(row)
        if column is None or (row["kind"], row["parent"], row["name"]) in exceptions:
            continue
        module = module_of(row["file"])
        if selected is not None and module not in selected:
            continue
        if args.module and not any(fnmatch.fnmatchcase(module, m) for m in args.module):
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
    if os.path.getmtime(args.declarations) < newest_source(args.root):
        print("warning: %s is older than the sources; run `make rename-extract`"
              % args.declarations)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
