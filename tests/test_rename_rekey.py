"""tools/rename_rekey.py, black box: a mini docs/renommage.tsv, mini
extractions from before and after an application, the applied slice, then
the identity cells it rewrites, its exit code, and its check mode. Run by
`make rename-rekey-test`, part of `make test` (no libclang needed)."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "rename_rekey.py")
WORK = os.path.join(REPO, "tmp", "test_rename_rekey_work")

COLUMNS = ["ancien", "nouveau", "fichier", "catégorie", "sorte", "statut",
           "module", "concept", "note", "identité"]
DECLARATION_COLUMNS = ["identity", "kind", "name", "parent", "file", "line", "linkage",
                       "scope_lines", "occurrences", "flags"]

SUBSTEPS = ("substep\tbound\tmodules\n"
            "3a\t0\tengine/entity_class\n"
            "3b\t0\tgame/planet\n")

HEADER = ("# Table de renommage.\n# Statut : validé\n\n"
          + "\t".join(COLUMNS) + "\n")

TABLE_ROWS = [
    ["me", "state", "src/engine/entity_class.h", "identifiant", "champ", "renommé",
     "engine/entity_class", "", "", "c:@S@pico8_struct_header@FI@me"],
    ["jo", "planet", "src/game/planet.c", "identifiant", "locale", "renommé",
     "game/planet", "jo", "", "c:planet.c@100@F@f@jo"],
    ["kj", "draw_fn", "src/game/planet.c", "identifiant", "paramètre", "renommé",
     "game/planet", "", "", "c:planet.c@200@F@pico8_jo_kj@kj"],
    ["i", "i", "src/game/planet.c", "identifiant", "locale", "conservé",
     "game/planet", "", "", "c:planet.c@300@F@f@i"],
    ["jo", "planet", "src/game/planet.c", "chaîne", "clé d'étiquette", "renommé",
     "game/planet", "jo", "lecteur", "chaîne:jo"],
]

BEFORE = [
    ["c:@S@pico8_struct_header@FI@me", "field", "me", "pico8_struct_header",
     "src/engine/entity_class.h", "12"],
    ["c:planet.c@100@F@f@jo", "local", "jo", "f", "src/game/planet.c", "5"],
    ["c:planet.c@200@F@pico8_jo_kj@kj", "param", "kj", "pico8_jo_kj", "src/game/planet.c", "9"],
    ["c:planet.c@300@F@f@i", "local", "i", "f", "src/game/planet.c", "6"],
]

AFTER = [
    ["c:@S@entity@FI@state", "field", "state", "entity", "src/engine/entity_class.h", "12"],
    ["c:planet.c@103@F@f@jo", "local", "jo", "f", "src/game/planet.c", "5"],
    ["c:planet.c@206@F@planet_draw@kj", "param", "kj", "planet_draw", "src/game/planet.c", "9"],
    ["c:planet.c@309@F@f@i", "local", "i", "f", "src/game/planet.c", "6"],
]

SLICE = [["me", "state", "src/engine/entity_class.h", "c:@S@pico8_struct_header@FI@me"]]


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def declarations(rows):
    return ("# partial\tsrc/x.c\tdefault\tsomething\n" + "\t".join(DECLARATION_COLUMNS) + "\n"
            + "".join("\t".join(r + ["local", "3", "1", ""]) + "\n" for r in rows))


def table_text(rows):
    return HEADER + "".join("\t".join(r) + "\n" for r in rows) + "\n"


def setup(rows=None, before=None, after=None):
    if os.path.isdir(WORK):
        shutil.rmtree(WORK)
    paths = {name: os.path.join(WORK, name) for name in
             ("renommage.tsv", "before.tsv", "after.tsv", "slice.tsv", "rename_substeps.tsv")}
    write(paths["renommage.tsv"], table_text(TABLE_ROWS if rows is None else rows))
    write(paths["before.tsv"], declarations(BEFORE if before is None else before))
    write(paths["after.tsv"], declarations(AFTER if after is None else after))
    write(paths["slice.tsv"], "ancien\tnouveau\tfichier\tidentité\n"
          + "".join("\t".join(r) + "\n" for r in SLICE))
    write(paths["rename_substeps.tsv"], SUBSTEPS)
    return paths


def rekey(paths):
    return subprocess.run([sys.executable, SCRIPT, "--table", paths["renommage.tsv"],
                           "--before", paths["before.tsv"], "--after", paths["after.tsv"],
                           "--slice", paths["slice.tsv"]], capture_output=True, text=True)


def check(paths, declarations_path, substep="3a"):
    return subprocess.run([sys.executable, SCRIPT, "--table", paths["renommage.tsv"],
                           "--check", "--declarations", declarations_path,
                           "--substeps", paths["rename_substeps.tsv"], "--substep", substep],
                          capture_output=True, text=True)


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read()


class Rekey(unittest.TestCase):

    def test_shifted_locals_and_params_of_renamed_functions_get_their_new_identity(self):
        paths = setup()
        result = rekey(paths)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        expected = [list(r) for r in TABLE_ROWS]
        expected[1][9] = "c:planet.c@103@F@f@jo"
        expected[2][9] = "c:planet.c@206@F@planet_draw@kj"
        self.assertEqual(read(paths["renommage.tsv"]), table_text(expected))
        self.assertIn("2", result.stdout)

    def test_two_candidates_fail_without_writing(self):
        paths = setup(after=AFTER + [["c:planet.c@150@F@g@jo", "local", "jo", "g",
                                      "src/game/planet.c", "5"]])
        result = rekey(paths)
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("c:planet.c@100@F@f@jo", result.stdout)
        self.assertEqual(read(paths["renommage.tsv"]), table_text(TABLE_ROWS))

    def test_no_candidate_fails_without_writing(self):
        moved = [r if r[2] != "kj" else r[:5] + ["10"] for r in AFTER]
        paths = setup(after=moved)
        result = rekey(paths)
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("c:planet.c@200@F@pico8_jo_kj@kj", result.stdout)
        self.assertEqual(read(paths["renommage.tsv"]), table_text(TABLE_ROWS))

    def test_check_passes_when_every_remaining_identity_is_extracted(self):
        paths = setup()
        self.assertEqual(rekey(paths).returncode, 0)
        result = check(paths, paths["after.tsv"])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_check_fails_on_an_identity_not_found(self):
        paths = setup()
        result = check(paths, paths["after.tsv"])
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("c:planet.c@100@F@f@jo", result.stdout)
        self.assertNotIn("pico8_struct_header@FI@me", result.stdout)


if __name__ == "__main__":
    unittest.main()
