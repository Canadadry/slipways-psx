"""The string net of a sub-step of step 3: for each renamed `chaîne` or
`fichier` row of docs/renommage.tsv that belongs to the sub-step
(tools/rename_substeps.py), at its site, the new name is present and the
old one absent. Recognised as the anti-cart counter does: in a C file, a
short or plain lowercase name (or an old name the counter finds ambiguous)
only as a string literal exactly equal to it, any other name as a whole word
in a literal, and a cited symbol also as a whole word in the code, where
its declaration is; assert messages and comments never count. In a script,
a tools/ data file or the Makefile, the name as a whole word outside comment
lines, a tmp/ path also as $(TMP_DIR)/. Rewrites nothing. Prints one
line per failing site, then the sites where neither name is seen (a
`type.field` citation, a bare binary name: to check by hand), then the
total; exit 1 if a site fails."""

import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_count  # noqa: E402
import rename_substeps  # noqa: E402
import rename_tables  # noqa: E402


CITED = "symbole cité"


def exact_only(name, names):
    return name in names.ambiguous or len(name) < 3 or re.fullmatch(r"[a-z]+", name) is not None


def mentions(path, text, names):
    """The function (name, sorte) -> whether the name is at this site."""
    ext = os.path.splitext(path)[1]
    if ext in rename_count.C_FILES:
        regions = list(rename_count.c_regions(text))
        literals = [text[a:b] for kind, a, b in regions if kind == "string"]
        code = "\n".join(text[a:b] for kind, a, b in regions if kind == "code")
        exact = set(literals)
        joined = "\n".join(literals)

        def found(name, kind):
            if exact_only(name, names):
                return name in exact
            pattern = rename_count.path_pattern(name)
            return pattern.search(joined) is not None or (
                kind == CITED and pattern.search(code) is not None)
        return found
    body = "".join(text[a:b] for kind, a, b in rename_count.text_regions(text, ext)
                   if kind == "text")
    return lambda name, kind: rename_count.path_pattern(name).search(body) is not None


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--table", required=True)
    parser.add_argument("--substeps", required=True)
    parser.add_argument("--substep", required=True)
    parser.add_argument("--root", required=True)
    args = parser.parse_args(argv)

    _, rows = rename_tables.read_table(args.table)
    substeps = rename_substeps.load(args.substeps)
    if args.substep not in substeps.modules:
        parser.error("unknown sub-step %s" % args.substep)
    membership = rename_substeps.Membership(rows, substeps)
    names = rename_count.Names(rows)
    sites = [r for r in rows if r["catégorie"] != "identifiant" and r["statut"] == "renommé"
             and membership.of_row(r) == args.substep]

    found = {}
    failing = 0
    unseen = []
    for r in sites:
        path = r["fichier"]
        if path not in found:
            full = os.path.join(args.root, path)
            text = ""
            if os.path.exists(full):
                with open(full, encoding="utf-8", errors="replace") as f:
                    text = f.read()
            found[path] = mentions(path, text, names)
        new = found[path](r["nouveau"], r["sorte"])
        old = found[path](r["ancien"], r["sorte"])
        if not new and not old:
            unseen.append(r)
            continue
        problems = []
        if not new:
            problems.append("new name %s missing" % r["nouveau"])
        if old:
            problems.append("old name %s still there" % r["ancien"])
        if problems:
            failing += 1
            print("%s: %s -> %s (%s): %s" % (path, r["ancien"], r["nouveau"], r["sorte"],
                                             "; ".join(problems)))
    for r in unseen:
        print("unseen %s: %s -> %s (%s)" % (r["fichier"], r["ancien"], r["nouveau"], r["sorte"]))
    print("rename_strings_verify %s: %d site(s) of %d failing, %d unseen" % (
        args.substep, failing, len(sites), len(unseen)))
    return 1 if failing else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
