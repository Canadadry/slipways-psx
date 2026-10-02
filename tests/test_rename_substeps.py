"""tools/rename_substeps.py: a mini docs/renommage.tsv and a mini sub-step
table, then the sub-step it gives each row (module API) and the membership
check it runs (black box, exit code). Run by `make rename-substeps-test`,
part of `make test`."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "rename_substeps.py")
WORK = os.path.join(REPO, "tmp", "test_rename_substeps_work")
sys.path.insert(0, os.path.join(REPO, "tools"))
import rename_count  # noqa: E402
import rename_substeps  # noqa: E402

COLUMNS = ["ancien", "nouveau", "fichier", "catégorie", "sorte", "statut",
           "module", "concept", "note", "identité"]

SUBSTEPS = ("# a comment\n"
            "substep\tbound\tmodules\n"
            "3a\t10\tengine/entity_class tools/class_coverage_exempt\n"
            "3b\t5\tgame/planet tests/test_planet tools/composition_residue\n"
            "3c\t0\tgame/planet_family Makefile\n")


def row(old, new, file, category="identifiant", kind="champ", note="", identity=None):
    module = os.path.splitext(file)[0].replace("src/", "", 1)
    status = "conservé" if old == new else "renommé"
    return [old, new, file, category, kind, status, module, "", note,
            identity or ("c:@%s" % old if category == "identifiant" else "%s:%s" % (
                "fichier" if category == "fichier" else "chaîne", old))]


ROWS = [
    row("me", "state", "src/engine/entity_class.h", identity="c:@S@h@FI@me"),
    row("pico8_jo_kj", "planet_draw", "src/game/planet.c", kind="fonction"),
    row("pico8_qh_kj", "structure_draw", "src/game/planet_family.c", kind="fonction"),
    row("jo", "planet", "src/game/planet_family.c", "chaîne", "clé d'étiquette + nom de classe",
        "lecteur"),
    row("ph", "normal", "tests/test_planet.c", "chaîne", "nom d'état", "écrivain"),
    row("iv", "toast", "src/game/planet.c", "chaîne", "nom de classe + clé", "écrivain"),
    row("tmp/pico8_replay", "tmp/replay", "tests/test_planet.c", "fichier", "binaire", "lecteur"),
    row("pico8_qh_kj", "structure_draw", "tools/composition_residue.sh", "chaîne", "symbole cité",
        "lecteur", identity="chaîne:pico8_qh_kj"),
    row("pico8_qh_kj", "structure_draw", "tests/test_planet.c", "chaîne", "symbole cité",
        "écrivain", identity="chaîne:pico8_qh_kj"),
    row("pico8_qh_kj", "structure_draw", "src/game/planet_family.c", "chaîne", "symbole cité",
        "écrivain", identity="chaîne:pico8_qh_kj"),
    row("lh", "keep_open", "src/game/planet_family.c", "chaîne", "clé", "lecteur"),
    row("lh", "keep_open", "tests/test_planet.c", "chaîne", "clé", "lecteur, libellé"),
    row("h.me", "entity.state", "tools/composition_residue.sh", "chaîne", "symbole cité",
        "lecteur"),
    row("p", "pos", "tools/composition_residue.sh", "chaîne", "symbole cité", "lecteur"),
    row("p", "pos", "src/game/planet_family.c", "chaîne", "symbole cité", "écrivain"),
    row("p", "pos", "src/game/planet.c", identity="c:@S@a@FI@p"),
    row("p", "map_pos", "src/game/planet_family.c", identity="c:@S@b@FI@p"),
]


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def tables(rows=None, substeps=SUBSTEPS):
    if os.path.isdir(WORK):
        shutil.rmtree(WORK)
    table = os.path.join(WORK, "renommage.tsv")
    steps = os.path.join(WORK, "rename_substeps.tsv")
    write(table, "# Statut : validé\n" + "\t".join(COLUMNS) + "\n"
          + "".join("\t".join(r) + "\n" for r in (ROWS if rows is None else rows)))
    write(steps, substeps)
    return table, steps


def check(rows=None, substeps=SUBSTEPS):
    table, steps = tables(rows, substeps)
    return subprocess.run([sys.executable, SCRIPT, "--table", table, "--substeps", steps],
                          capture_output=True, text=True)


def assigned(rows=None):
    table, steps = tables(rows)
    rows = rename_count.read_table(table)
    membership = rename_substeps.Membership(rows, rename_substeps.load(steps))
    return {(r["ancien"], r["fichier"], r["identité"]): membership.of_row(r) for r in rows}, membership


class Table(unittest.TestCase):

    def test_ordered_substeps_bounds_and_module_lookup(self):
        _, steps = tables()
        substeps = rename_substeps.load(steps)
        self.assertEqual(substeps.order, ["3a", "3b", "3c"])
        self.assertEqual(substeps.bound["3b"], 5)
        self.assertEqual(substeps.modules["3b"],
                         ["game/planet", "tests/test_planet", "tools/composition_residue"])
        self.assertEqual(substeps.of_module("game/planet"), "3b")
        self.assertIsNone(substeps.of_module("game/planet_famil"))

    def test_membership_check_passes_when_every_module_is_in_one_substep(self):
        result = check()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_membership_check_fails_on_a_module_in_no_substep(self):
        result = check(ROWS + [row("x_y", "xy", "src/game/menu.c")])
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("game/menu", result.stdout)

    def test_membership_check_fails_on_a_module_listed_twice(self):
        result = check(substeps=SUBSTEPS + "3d\t0\tgame/planet\n")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("game/planet", result.stdout)

    def test_membership_check_fails_on_a_missing_bound(self):
        result = check(substeps=SUBSTEPS.replace("3c\t0\t", "3c\t\t"))
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("3c", result.stdout)


class Rules(unittest.TestCase):

    def test_identifier_follows_its_module(self):
        found, _ = assigned()
        self.assertEqual(found[("pico8_jo_kj", "src/game/planet.c", "c:@pico8_jo_kj")], "3b")
        self.assertEqual(found[("me", "src/engine/entity_class.h", "c:@S@h@FI@me")], "3a")

    def test_tag_message_state_and_class_strings_are_3a_wherever_their_site(self):
        found, _ = assigned()
        self.assertEqual(found[("jo", "src/game/planet_family.c", "chaîne:jo")], "3a")
        self.assertEqual(found[("ph", "tests/test_planet.c", "chaîne:ph")], "3a")
        self.assertEqual(found[("iv", "src/game/planet.c", "chaîne:iv")], "3a")

    def test_file_rows_are_the_last_substep(self):
        found, _ = assigned()
        self.assertEqual(found[("tmp/pico8_replay", "tests/test_planet.c",
                                "fichier:tmp/pico8_replay")], "3c")

    def test_a_tools_mention_follows_the_identifier_it_cites(self):
        found, _ = assigned()
        self.assertEqual(found[("pico8_qh_kj", "tools/composition_residue.sh",
                                "chaîne:pico8_qh_kj")], "3c")
        self.assertEqual(found[("h.me", "tools/composition_residue.sh", "chaîne:h.me")], "3a")

    def test_other_strings_follow_their_first_writer(self):
        found, _ = assigned()
        self.assertEqual(found[("pico8_qh_kj", "tests/test_planet.c", "chaîne:pico8_qh_kj")], "3b")
        self.assertEqual(found[("pico8_qh_kj", "src/game/planet_family.c",
                                "chaîne:pico8_qh_kj")], "3b")

    def test_a_key_without_writer_follows_its_first_reader(self):
        found, _ = assigned()
        self.assertEqual(found[("lh", "src/game/planet_family.c", "chaîne:lh")], "3b")

    def test_a_tools_mention_of_an_ambiguous_name_falls_back_and_is_listed(self):
        found, membership = assigned()
        self.assertEqual(found[("p", "tools/composition_residue.sh", "chaîne:p")], "3c")
        self.assertEqual([(r["ancien"], r["fichier"]) for r in membership.uncited],
                         [("p", "tools/composition_residue.sh")])


if __name__ == "__main__":
    unittest.main()
