"""Keeps the identities of docs/renommage.tsv resolvable between the
sub-steps of step 3. The USR of a local, a parameter or a function static
holds the byte offset of its declaration, and the name of its function:
applying a slice shifts them, and `rename-apply` would refuse the rows of
the next sub-steps.

Rekey (after `make rename-apply`): for each renamed identifier row not in
the applied slice whose identity is in the extraction from before the
application and not in the one from after, the after declaration of the
same kind, name, file and line (renaming tokens moves no line), which must
be unique. Rewrites only those `identité` cells, the rest of the table byte
for byte; with one row unmatched or matched twice, writes nothing, exit 1.

Check (--check): every renamed identifier row of the sub-steps after
--substep has its identity in a fresh extraction, with its old name and
file; exit 1 otherwise, listing the rows."""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_derive  # noqa: E402
import rename_substeps  # noqa: E402
import rename_tables  # noqa: E402


def renamed_identifiers(rows):
    return [r for r in rows if r["catégorie"] == "identifiant" and r["statut"] == "renommé"]


def new_identities(rows, before, after, applied):
    """({old identity: new identity}, [problem...])."""
    before = {d.identity: d for d in before}
    after_ids = {d.identity for d in after}
    by_place = {}
    for d in after:
        by_place.setdefault((d.kind, d.name, d.file, d.line), []).append(d.identity)
    mapping = {}
    problems = []
    for r in renamed_identifiers(rows):
        identity = r["identité"]
        if identity in applied or identity in after_ids or identity not in before:
            continue
        d = before[identity]
        found = by_place.get((d.kind, d.name, d.file, d.line), [])
        if len(found) == 1:
            mapping[identity] = found[0]
        else:
            problems.append("%s (%s, %s:%s): %d candidate(s) after%s" % (
                identity, d.name, d.file, d.line, len(found),
                "".join(" " + f for f in found)))
    return mapping, problems


def rewrite(path, mapping):
    """Replaces the identité cell of the identifier rows in mapping; every
    other byte of the file stays."""
    with open(path, encoding="utf-8", newline="") as f:
        lines = f.read().split("\n")
    start, columns = rename_tables.header_line(lines)
    category, identity = columns.index("catégorie"), columns.index("identité")
    for i, line in enumerate(lines[start + 1:], start + 1):
        if not line or line.startswith("#"):
            continue
        cells = line.split("\t")
        if (len(cells) > identity and cells[category] == "identifiant"
                and cells[identity] in mapping):
            cells[identity] = mapping[cells[identity]]
            lines[i] = "\t".join(cells)
    with open(path, "w", encoding="utf-8", newline="") as f:
        f.write("\n".join(lines))


def read_slice(path):
    with open(path, encoding="utf-8") as f:
        next(f, None)
        return {line.rstrip("\n").split("\t")[3] for line in f if line.strip()}


def check(rows, declarations, substeps, substep):
    membership = rename_substeps.Membership(rows, substeps)
    current = substeps.index(substep)
    found = {d.identity: d for d in declarations}
    problems = []
    for r in renamed_identifiers(rows):
        step = membership.of_row(r)
        if step is None or substeps.index(step) <= current:
            continue
        d = found.get(r["identité"])
        if d is None or d.name != r["ancien"] or d.file != r["fichier"]:
            problems.append("%s %s (%s, %s): not in the extraction" % (
                step, r["identité"], r["ancien"], r["fichier"]))
    return problems


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--table", required=True)
    parser.add_argument("--before", help="declarations.tsv copied before the application")
    parser.add_argument("--after", help="declarations.tsv extracted after it")
    parser.add_argument("--slice", help="the applied slice")
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--declarations", help="with --check: a fresh declarations.tsv")
    parser.add_argument("--substeps", help="with --check: the sub-step table")
    parser.add_argument("--substep", help="with --check: the last applied sub-step")
    args = parser.parse_args(argv)
    _, rows = rename_tables.read_table(args.table)

    if args.check:
        if not (args.declarations and args.substeps and args.substep):
            parser.error("--check needs --declarations, --substeps and --substep")
        problems = check(rows, rename_derive.declarations_of(args.declarations),
                         rename_substeps.load(args.substeps), args.substep)
        for line in problems:
            print(line)
        print("rename_rekey --check %s: %d identity(ies) not found" % (args.substep,
                                                                      len(problems)))
        return 1 if problems else 0

    if not (args.before and args.after and args.slice):
        parser.error("--before, --after and --slice are required")
    mapping, problems = new_identities(rows, rename_derive.declarations_of(args.before),
                                       rename_derive.declarations_of(args.after),
                                       read_slice(args.slice))
    for line in problems:
        print(line)
    if problems:
        print("rename_rekey: %d identity(ies) without a single match; nothing written"
              % len(problems))
        return 1
    rewrite(args.table, mapping)
    print("rename_rekey: %d identity(ies) rekeyed in %s" % (len(mapping), args.table))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
