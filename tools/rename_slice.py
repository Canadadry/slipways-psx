"""The slice of docs/renommage.tsv that `make rename-apply` applies: the
renamed identifier rows of the given modules (shell patterns on the `module`
column), as ancien, nouveau, fichier, identité. String and file rows stay
out: no tool rewrites them."""

import argparse
import fnmatch
import sys


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--table", required=True)
    parser.add_argument("--module", action="append", required=True,
                        help="a module of the table (game/planet) or a pattern (game/planet*)")
    parser.add_argument("-o", "--output", required=True)
    args = parser.parse_args(argv)

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
            if any(fnmatch.fnmatchcase(row["module"], m) for m in args.module):
                out.append([row["ancien"], row["nouveau"], row["fichier"], row["identité"]])
    with open(args.output, "w", encoding="utf-8") as f:
        f.write("ancien\tnouveau\tfichier\tidentité\n")
        for row in out:
            f.write("\t".join(row) + "\n")
    print("rename_slice: %d renamed identifier(s) of %s (%s)" % (
        len(out), " ".join(args.module), args.output))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
