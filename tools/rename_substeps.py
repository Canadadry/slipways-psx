"""The sub-steps of step 3 (docs/plan-5.md, section 6, 3a-3l), read from
tools/rename_substeps.tsv, and the sub-step each row of docs/renommage.tsv
belongs to:

  1. an identifier row: the sub-step of its module;
  2. a tag key, message key, state name or class name (or a combination):
     the first sub-step (3a), wherever its site;
  3. a file row: the last sub-step (3l);
  4. another string row sited in tools/: the sub-step of the identifier it
     cites (same old name; for `type.field`, the field of that type); with
     no single cited identifier, rule 5, and the row is listed in `uncited`;
  5. another string row: the first sub-step among the modules of the writer
     sites of its identity; with no writer, of its reader sites; with
     neither, of all its sites.

Run as a script: the membership check of `make test` (every module of the
table in exactly one sub-step, every sub-step with a bound); exit 1 on a
violation."""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_tables  # noqa: E402

FIRST_SUBSTEP_KINDS = {"clé d'étiquette", "clé de message", "nom d'état", "nom de classe"}
WRITER, READER = "écrivain", "lecteur"


class Substeps:
    """order: the sub-steps in plan order; bound: {substep: residue bound,
    None if missing}; modules: {substep: [module...]}; duplicates: modules
    listed more than once."""

    def __init__(self, entries):
        self.order = [name for name, _, _ in entries]
        self.bound = {name: bound for name, bound, _ in entries}
        self.modules = {name: modules for name, _, modules in entries}
        self._of = {}
        self.duplicates = []
        for name, _, modules in entries:
            for module in modules:
                if module in self._of:
                    self.duplicates.append(module)
                self._of.setdefault(module, name)

    def of_module(self, module):
        return self._of.get(module)

    def index(self, substep):
        return self.order.index(substep)

    def first(self, substeps):
        return min(substeps, key=self.index)

    def last(self, substeps):
        return max(substeps, key=self.index)


def load(path):
    """Lines `substep<TAB>bound<TAB>module module...`, after a header line;
    `#` lines are comments."""
    entries = []
    header = True
    with open(path, encoding="utf-8") as f:
        for line in f.read().splitlines():
            if not line.strip() or line.startswith("#"):
                continue
            if header:
                header = False
                continue
            cells = line.split("\t") + ["", ""]
            bound = int(cells[1]) if cells[1].strip() else None
            entries.append((cells[0].strip(), bound, cells[2].split()))
    return Substeps(entries)


def roles(row):
    """`écrivain, lecteur` -> {écrivain, lecteur}; text after `;` is a note."""
    return {part.strip() for part in row["note"].split(";")[0].split(",")}


class Membership:
    """The sub-step of each row of the table (rows as read by
    rename_tables.read_table)."""

    def __init__(self, rows, substeps):
        self.substeps = substeps
        self.uncited = []
        self._identifiers = {}  # old name -> identifier rows
        self._sites = {}  # string identity -> its rows
        for r in rows:
            if r["catégorie"] == "identifiant":
                self._identifiers.setdefault(r["ancien"], []).append(r)
            else:
                self._sites.setdefault(r["identité"], []).append(r)
        self._of = {}
        for r in rows:
            self._of[id(r)] = self._assign(r)
        self._by_identity = {}
        self._by_name = {}  # (old name, identifier?) -> sub-steps of its renamed rows
        self._by_site = {}  # (old name, file) -> sub-step of its string or file row
        for r in rows:
            if r["statut"] != "renommé" or self._of[id(r)] is None:
                continue
            step = self._of[id(r)]
            code = r["catégorie"] == "identifiant"
            if code:
                self._by_identity[r["identité"]] = step
            else:
                self._by_site[(r["ancien"], r["fichier"])] = step
            self._by_name.setdefault((r["ancien"], code), set()).add(step)

    def of_row(self, row):
        return self._of[id(row)]

    def of_identity(self, identity):
        return self._by_identity.get(identity)

    def of_mention(self, name, column, path):
        """The sub-step a mention of an old name found by text belongs to:
        in a string, its row at that site, else the last of its string rows;
        as an identifier, the last of its identifier rows; else (or if it has
        no such row) the last of all its renamed rows. What remains of a name
        several sub-steps share belongs to the latest."""
        if column == "string" and (name, path) in self._by_site:
            return self._by_site[(name, path)]
        own = {"string": False, "identifier": True}.get(column)
        steps = self._by_name.get((name, own)) if own is not None else None
        if not steps:
            steps = self._by_name.get((name, True), set()) | self._by_name.get((name, False), set())
        return self.substeps.last(steps) if steps else None

    def _assign(self, row):
        if row["catégorie"] == "identifiant":
            return self.substeps.of_module(row["module"])
        if row["catégorie"] == "fichier":
            return self.substeps.order[-1]
        if FIRST_SUBSTEP_KINDS & {k.strip() for k in row["sorte"].split("+")}:
            return self.substeps.order[0]
        if row["fichier"].startswith("tools/"):
            cited = self._cited(row["ancien"])
            if cited is not None:
                return self.substeps.of_module(cited["module"])
            self.uncited.append(row)
        return self._by_sites(row["identité"])

    def _cited(self, name):
        """The single identifier row a tools/ mention names, or None."""
        owner, _, field = name.rpartition(".")
        candidates = self._identifiers.get(field or name, [])
        if owner:
            tags = {owner, owner[:-2] if owner.endswith("_t") else owner}
            candidates = [r for r in candidates
                          if any("@S@%s@" % t in r["identité"] or "@SA@%s@" % t in r["identité"]
                                 for t in tags)]
        return candidates[0] if len(candidates) == 1 else None

    def _by_sites(self, identity):
        sites = self._sites[identity]
        for role in (WRITER, READER):
            chosen = [r for r in sites if role in roles(r)]
            if chosen:
                break
        else:
            chosen = sites
        return self.substeps.first({self.substeps.of_module(r["module"]) for r in chosen})


def violations(rows, substeps):
    found = []
    for module in sorted(set(substeps.duplicates)):
        found.append("module listed twice: %s" % module)
    for name in substeps.order:
        if substeps.bound[name] is None:
            found.append("sub-step without a bound: %s" % name)
    for module in sorted({r["module"] for r in rows if r["module"]}):
        if substeps.of_module(module) is None:
            found.append("module in no sub-step: %s" % module)
    return found


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--table", required=True)
    parser.add_argument("--substeps", required=True)
    args = parser.parse_args(argv)
    _, rows = rename_tables.read_table(args.table)
    substeps = load(args.substeps)
    found = violations(rows, substeps)
    for line in found:
        print(line)
    print("rename_substeps: %d sub-step(s), %d module(s); %d violation(s)" % (
        len(substeps.order), sum(len(m) for m in substeps.modules.values()), len(found)))
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
