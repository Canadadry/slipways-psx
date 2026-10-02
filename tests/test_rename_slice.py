"""tools/rename_slice.py, black box: a mini docs/renommage.tsv, then the
table `make rename-apply` reads that it writes. Run by
`make rename-slice-test`, part of `make test`."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "rename_slice.py")
WORK = os.path.join(REPO, "tmp", "test_rename_slice_work")

COLUMNS = ["ancien", "nouveau", "fichier", "catégorie", "sorte", "statut",
           "module", "concept", "note", "identité"]

ROWS = [
    ["pico8_jo_kj", "planet_draw", "src/game/planet.h", "identifiant", "fonction", "renommé",
     "game/planet", "jo", "", "c:@F@pico8_jo_kj"],
    ["i", "i", "src/game/planet.c", "identifiant", "locale", "conservé",
     "game/planet", "", "", "c:planet.c@1@F@f@i"],
    ["pico8_jo_ev", "planet_draw_overlay", "src/game/planet_display.c", "identifiant", "fonction",
     "renommé", "game/planet_display", "jo", "", "c:@F@pico8_jo_ev"],
    ["pico8_jc_kj", "slipway_draw", "src/game/slipway.h", "identifiant", "fonction", "renommé",
     "game/slipway", "jc", "", "c:@F@pico8_jc_kj"],
    ["pico8_qh_kj", "structure_draw", "src/game/planet_family.c", "identifiant", "fonction",
     "renommé", "game/planet_family", "qh", "", "c:@F@pico8_qh_kj"],
    ["jo", "planet", "src/game/planet.c", "chaîne", "clé", "renommé",
     "game/planet", "jo", "lecteur", "chaîne:jo"],
]


SUBSTEPS = ("substep\tbound\tmodules\n"
            "3a\t0\tgame/slipway\n"
            "3b\t0\tgame/planet game/planet_display\n"
            "3c\t0\tgame/planet_family\n")


def run(*modules, substep=None):
    if os.path.isdir(WORK):
        shutil.rmtree(WORK)
    os.makedirs(WORK)
    table = os.path.join(WORK, "renommage.tsv")
    out = os.path.join(WORK, "slice.tsv")
    steps = os.path.join(WORK, "rename_substeps.tsv")
    with open(steps, "w") as f:
        f.write(SUBSTEPS)
    with open(table, "w") as f:
        f.write("# Statut : à valider\n" + "\t".join(COLUMNS) + "\n")
        for row in ROWS:
            f.write("\t".join(row) + "\n")
    args = [sys.executable, SCRIPT, "--table", table, "-o", out]
    for module in modules:
        args += ["--module", module]
    if substep:
        args += ["--substep", substep, "--substeps", steps]
    result = subprocess.run(args, capture_output=True, text=True)
    lines = []
    if os.path.exists(out):
        with open(out) as f:
            lines = f.read().splitlines()
    return result, lines


class Slice(unittest.TestCase):

    def test_renamed_identifiers_of_the_given_modules_in_apply_columns(self):
        result, lines = run("game/planet*")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(lines, [
            "ancien\tnouveau\tfichier\tidentité",
            "pico8_jo_kj\tplanet_draw\tsrc/game/planet.h\tc:@F@pico8_jo_kj",
            "pico8_jo_ev\tplanet_draw_overlay\tsrc/game/planet_display.c\tc:@F@pico8_jo_ev",
            "pico8_qh_kj\tstructure_draw\tsrc/game/planet_family.c\tc:@F@pico8_qh_kj",
        ])

    def test_exact_module_and_no_module_given(self):
        result, lines = run("game/planet")
        self.assertEqual(len(lines), 2)
        result, _ = run()
        self.assertNotEqual(result.returncode, 0)

    def test_substep_selects_exactly_its_modules_without_patterns(self):
        result, lines = run(substep="3b")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(lines, [
            "ancien\tnouveau\tfichier\tidentité",
            "pico8_jo_kj\tplanet_draw\tsrc/game/planet.h\tc:@F@pico8_jo_kj",
            "pico8_jo_ev\tplanet_draw_overlay\tsrc/game/planet_display.c\tc:@F@pico8_jo_ev",
        ])

    def test_unknown_substep_fails(self):
        result, _ = run(substep="3z")
        self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
