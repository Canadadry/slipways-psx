"""tools/rename_strings_verify.py, black box: a mini docs/renommage.tsv, a
mini sub-step table and a mini source tree, then the sites it reports and
its exit code. Run by `make rename-strings-verify-test`, part of
`make test`."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "rename_strings_verify.py")
WORK = os.path.join(REPO, "tmp", "test_rename_strings_verify_work")

COLUMNS = ["ancien", "nouveau", "fichier", "catégorie", "sorte", "statut",
           "module", "concept", "note", "identité"]

SUBSTEPS = ("substep\tbound\tmodules\n"
            "3a\t0\tengine/entity_class engine/entity tools/composition_residue\n"
            "3b\t0\tgame/planet tests/test_planet tests/test_replay_usage Makefile\n")


def row(old, new, file, category, kind, note):
    module = os.path.splitext(file)[0].replace("src/", "", 1)
    status = "conservé" if old == new else "renommé"
    prefix = "fichier" if category == "fichier" else "chaîne"
    return [old, new, file, category, kind, status, module, "", note, "%s:%s" % (prefix, old)]


ROWS = [
    ["pico8_struct_header_t", "entity_t", "src/engine/entity_class.h", "identifiant", "type",
     "renommé", "engine/entity_class", "", "", "c:@T@pico8_struct_header_t"],
    row("jo", "planet", "src/game/planet.c", "chaîne", "clé d'étiquette", "écrivain"),
    row("jo", "planet", "src/engine/entity_class.h", "chaîne", "clé d'étiquette", "lecteur"),
    row("trade", "trade", "src/game/planet.c", "chaîne", "nom de classe", "écrivain"),
    row("pico8_struct_header_t", "entity_t", "tools/composition_residue.sh", "chaîne",
        "symbole cité", "lecteur"),
    row("lh", "keep_open", "src/game/planet.c", "chaîne", "clé", "écrivain"),
    row("tmp/pico8_replay", "tmp/replay", "Makefile", "fichier", "binaire", "écrivain"),
]

DONE = {
    "src/game/planet.c": 'void f(void) { tag("planet"); tag("lh"); class_name("trade"); }\n',
    "src/engine/entity_class.h": ('static int is_planet(const char *k) {\n'
                                  '    assert(k && "jo");\n'
                                  '    /* "jo" */\n'
                                  '    return !strcmp(k, "planet");\n'
                                  '}\n'),
    "tools/composition_residue.sh": "# was pico8_struct_header_t\ne[\"entity_t\"] = 1\n",
    "Makefile": "$(TMP_DIR)/pico8_replay: x.o\n",
}


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def run(files, substep="3a", rows=ROWS):
    if os.path.isdir(WORK):
        shutil.rmtree(WORK)
    root = os.path.join(WORK, "tree")
    table = os.path.join(WORK, "renommage.tsv")
    steps = os.path.join(WORK, "rename_substeps.tsv")
    write(table, "# Statut : validé\n" + "\t".join(COLUMNS) + "\n"
          + "".join("\t".join(r) + "\n" for r in rows))
    write(steps, SUBSTEPS)
    for path, text in files.items():
        write(os.path.join(root, path), text)
    return subprocess.run([sys.executable, SCRIPT, "--table", table, "--substeps", steps,
                           "--substep", substep, "--root", root],
                          capture_output=True, text=True)


class Verify(unittest.TestCase):

    def test_conforming_tree_passes(self):
        result = run(DONE)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("0 site(s)", result.stdout)

    def test_forgotten_site_fails(self):
        files = dict(DONE, **{"src/game/planet.c": 'void f(void) { tag("jo"); }\n'})
        result = run(files)
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("src/game/planet.c", result.stdout)
        self.assertIn("jo", result.stdout)

    def test_old_literal_left_next_to_the_new_one_fails(self):
        files = dict(DONE, **{"src/game/planet.c": 'void f(void) { tag("planet"); tag("jo"); }\n'})
        result = run(files)
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("src/game/planet.c", result.stdout)

    def test_old_name_left_in_a_script_fails(self):
        files = dict(DONE, **{"tools/composition_residue.sh":
                              "e[\"entity_t\"] = 1\nf[\"pico8_struct_header_t\"]\n"})
        result = run(files)
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("tools/composition_residue.sh", result.stdout)

    def test_tmp_path_written_with_the_make_variable_is_found(self):
        result = run(dict(DONE, **{"Makefile": "tmp/replay: x.o\n\t$(TMP_DIR)/pico8_replay\n"}),
                     substep="3b")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("Makefile", result.stdout)
        result = run(dict(DONE, **{"Makefile": "$(TMP_DIR)/replay: x.o\n"}), substep="3b")
        self.assertNotIn("Makefile", result.stdout)

    def test_a_long_name_counts_inside_a_longer_literal_and_a_cited_symbol_in_code(self):
        rows = ROWS + [
            row("tmp/pico8_replay", "tmp/replay", "tests/test_replay_usage.c", "fichier",
                "binaire", "lecteur"),
            row("test_pico8_jo", "test_planet", "tests/test_planet.c", "chaîne", "nom de test",
                "libellé"),
            row("pico8_struct_header_t", "entity_t", "src/engine/entity.h", "chaîne",
                "symbole cité", "écrivain"),
        ]
        files = dict(DONE, **{
            "tests/test_replay_usage.c": '#define DRIVER "./tmp/replay"\n',
            "tests/test_planet.c": 'void f(void) { printf("test_planet: all passed\\n"); }\n',
            "src/engine/entity.h": "typedef struct entity { int me; } entity_t;\n",
        })
        result = run(files, substep="3b", rows=rows)
        self.assertNotIn("test_replay_usage", result.stdout)
        self.assertNotIn("test_planet.c", result.stdout)
        result = run(files, rows=rows)
        self.assertEqual(result.returncode, 0, result.stdout)
        files["src/engine/entity.h"] = "typedef struct entity { int me; } pico8_struct_header_t;\n"
        files["tests/test_planet.c"] = 'void f(void) { printf("test_pico8_jo: ok\\n"); }\n'
        result = run(files, rows=rows)
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("src/engine/entity.h", result.stdout)
        result = run(files, substep="3b", rows=rows)
        self.assertIn("tests/test_planet.c", result.stdout)

    def test_a_site_where_neither_name_is_seen_is_listed_without_failing(self):
        rows = ROWS + [row("pico8_jo_t.jy", "planet_t.needs", "src/engine/entity_class.h",
                           "chaîne", "symbole cité", "écrivain")]
        result = run(DONE, rows=rows)
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("unseen", result.stdout)
        self.assertIn("pico8_jo_t.jy", result.stdout)

    def test_only_the_rows_of_the_substep_are_verified(self):
        result = run(dict(DONE, **{"src/game/planet.c":
                                   'void f(void) { tag("planet"); tag("lh"); }\n'}))
        self.assertEqual(result.returncode, 0, result.stdout)


if __name__ == "__main__":
    unittest.main()
