"""The slice of docs/renommage.tsv that `make rename-apply` applies: the
renamed identifier rows of the given modules (shell patterns on the `module`
column), or the modules of one sub-step of tools/rename_substeps.tsv
(exactly), as ancien, nouveau, fichier, identité. String and file rows stay
out: no tool rewrites them."""

import argparse
import fnmatch
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_substeps  # noqa: E402


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--table", required=True)
    parser.add_argument("--module", action="append", default=[],
                        help="a module of the table (game/planet) or a pattern (game/planet*)")
    parser.add_argument("--substep", help="a sub-step (3e): its modules, no pattern")
    parser.add_argument("--substeps", help="the sub-step table, with --substep")
    parser.add_argument("-o", "--output", required=True)
    args = parser.parse_args(argv)
    exact = set()
    if args.substep:
        substeps = rename_substeps.load(args.substeps)
        if args.substep not in substeps.modules:
            parser.error("unknown sub-step %s" % args.substep)
        exact = set(substeps.modules[args.substep])
    elif not args.module:
        parser.error("--module or --substep is required")

    columns = None
    out = []
    with open(args.table, encoding="utf-8") as f:
        for line in f.read().splitlines():
            if not line or line.startswith("#"):
                continue
            cells = line.split("\t")
            if columns is None:
                columns = cells
                continue
            row = dict(zip(columns, cells))
            if row["catégorie"] != "identifiant" or row["statut"] != "renommé":
                continue
            if row["module"] in exact or any(fnmatch.fnmatchcase(row["module"], m)
                                             for m in args.module):
                out.append([row["ancien"], row["nouveau"], row["fichier"], row["identité"]])
    with open(args.output, "w", encoding="utf-8") as f:
        f.write("ancien\tnouveau\tfichier\tidentité\n")
        for row in out:
            f.write("\t".join(row) + "\n")
    print("rename_slice: %d renamed identifier(s) of %s (%s)" % (
        len(out), args.substep or " ".join(args.module), args.output))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
