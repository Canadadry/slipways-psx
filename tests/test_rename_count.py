"""tools/rename_count.py, black box: a mini docs/renommage.tsv, a mini
occurrences.tsv and a mini source tree, then the per-module counts it
prints. Run by `make rename-count-test`, part of `make test`."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "rename_count.py")
WORK = os.path.join(REPO, "tmp", "test_rename_count_work")

COLUMNS = ["ancien", "nouveau", "fichier", "catégorie", "sorte", "statut",
           "module", "concept", "note", "identité"]

ROWS = [
    ["pico8_jo_kj", "planet_draw", "src/game/planet.h", "identifiant", "fonction", "renommé",
     "game/planet", "jo", "", "c:@F@pico8_jo_kj"],
]

FILES = {
    "src/game/planet.h": "void pico8_jo_kj(void);\n",
}

OCCURRENCES = []


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def run(rows=None, files=None, occurrences=None, extra=()):
    rows = ROWS if rows is None else rows
    files = FILES if files is None else files
    occurrences = OCCURRENCES if occurrences is None else occurrences
    if os.path.isdir(WORK):
        shutil.rmtree(WORK)
    root = os.path.join(WORK, "tree")
    table = os.path.join(WORK, "renommage.tsv")
    occ = os.path.join(WORK, "occurrences.tsv")
    write(table, "# Statut : à valider\n" + "\t".join(COLUMNS) + "\n"
          + "".join("\t".join(r) + "\n" for r in rows))
    for path, text in files.items():
        write(os.path.join(root, path), text)
    write(occ, "identity\tfile\toffset\tlength\torigin\n"
          + "".join("\t".join(map(str, o)) + "\n" for o in occurrences))
    args = [sys.executable, SCRIPT, "--table", table, "--occurrences", occ, "--root", root]
    result = subprocess.run(args + list(extra), capture_output=True, text=True)
    return result, counts(result.stdout)


def counts(stdout):
    """{module: {column: n}} from the printed table."""
    table = {}
    header = None
    for line in stdout.splitlines():
        cells = line.split()
        if cells[:1] == ["module"]:
            header = cells[1:]
            continue
        if header and len(cells) == len(header) + 1 and all(c.isdigit() for c in cells[1:]):
            table[cells[0]] = dict(zip(header, map(int, cells[1:])))
    return table


class Count(unittest.TestCase):

    def test_long_name_in_code_is_an_identifier_and_never_fails(self):
        result, table = run()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(table["game/planet"]["identifier"], 1, result.stdout)
        self.assertEqual(table["TOTAL"]["total"], 1, result.stdout)

    def test_long_name_classified_by_context(self):
        result, table = run(files={"src/game/ui.c": (
            "/* calls `pico8_jo_kj()` then pico8_jo_kj again */\n"
            "static const char *k = \"pico8_jo_kj\";\n"
            "void f(void) { assert(k && \"`pico8_jo_kj` first\"); }\n"
            "// pico8_jo_kjx and xpico8_jo_kj are other names\n")})
        self.assertEqual(table["game/ui"], {"identifier": 0, "backtick": 2, "string": 1,
                                             "comment": 1, "total": 4}, result.stdout)

    def test_short_names_counted_from_the_ast_and_at_listed_sites(self):
        rows = ROWS + [
            ["p", "map_pos", "src/engine/entity.h", "identifiant", "champ", "renommé",
             "engine/entity", "p", "", "c:@S@pico8_struct_header@FI@p"],
            ["p", "p", "src/game/planet.c", "identifiant", "locale", "conservé",
             "game/planet", "", "", "c:planet.c@10@F@f@p"],
            ["jo", "planet", "src/game/planet.c", "chaîne", "clé", "renommé",
             "game/planet", "jo", "lecteur", "chaîne:jo"],
            ["jo", "planet", "tools/expected.txt", "chaîne", "clé", "renommé",
             "tools/expected", "jo", "lecteur", "chaîne:jo"],
        ]
        planet = ("void f(entity_t *e) { int p = e->p; }\n"
                  "/* `p` is the map position, `e->p` too, and p alone */\n"
                  "static const char *a = \"jo\", *b = \"p\", *c = \"jo \";\n")
        files = {
            "src/game/planet.c": planet,
            "src/game/ui.c": "static const char *a = \"jo\";\n",
            "tools/expected.txt": "# jo is the planet\njo 12\nbjo jo_x\n",
        }
        occurrences = [
            ["c:@S@pico8_struct_header@FI@p", "src/game/planet.c", planet.index("e->p") + 3, 1, "ref"],
            ["c:@S@pico8_struct_header@FI@p", "src/game/planet.c", planet.index("e->p") + 3, 1, "ref"],
            ["c:planet.c@10@F@f@p", "src/game/planet.c", planet.index("int p") + 4, 1, "decl"],
        ]
        result, table = run(rows, files, occurrences)
        self.assertEqual(table["game/planet"], {"identifier": 1, "backtick": 1, "string": 1,
                                                 "comment": 0, "total": 3}, result.stdout)
        self.assertNotIn("game/ui", table)
        self.assertEqual(table["tools/expected"]["string"], 1, result.stdout)

    def test_tmp_paths_in_makefile_and_scripts(self):
        rows = [["tmp/pico8_replay", "tmp/replay", "Makefile", "fichier", "binaire", "renommé",
                 "Makefile", "", "écrivain, lecteur", "fichier:tmp/pico8_replay"]]
        files = {
            "Makefile": "# builds $(TMP_DIR)/pico8_replay\n$(TMP_DIR)/pico8_replay: x.o\n"
                        "\t$(TMP_DIR)/pico8_replay_gcov tmp/pico8_replay.d\n",
            "tools/frames_ref.sh": "./tmp/pico8_replay --dump\n",
        }
        result, table = run(rows, files)
        self.assertEqual(table["Makefile"], {"identifier": 0, "backtick": 0, "string": 2,
                                              "comment": 1, "total": 3}, result.stdout)
        self.assertEqual(table["tools/frames_ref"]["string"], 1, result.stdout)

    def test_kept_names_and_new_names_are_not_searched_as_text(self):
        rows = ROWS + [
            ["pico8_jo_kj", "pico8_jo_kj", "src/game/other.c", "identifiant", "statique",
             "conservé", "game/other", "", "", "c:other.c@pico8_jo_kj"],
            ["old_name", "pico8_jo_cv", "src/game/other.c", "identifiant", "fonction", "renommé",
             "game/other", "", "", "c:@F@old_name"],
            ["pico8_jo_cv", "planet_refresh", "src/game/planet.h", "identifiant", "fonction",
             "renommé", "game/planet", "", "", "c:@F@pico8_jo_cv"],
        ]
        result, table = run(rows, {"src/game/planet.c": "pico8_jo_kj(); pico8_jo_cv(); old_name();\n"})
        self.assertEqual(table["game/planet"]["identifier"], 1, result.stdout)

    def test_module_filter_and_missing_occurrences_still_exit_zero(self):
        files = dict(FILES, **{"src/game/ui.c": "void g(void) { pico8_jo_kj(); }\n"})
        result, table = run(files=files, extra=["--module", "game/u*",
                                                "--occurrences", os.path.join(WORK, "none.tsv")])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(sorted(table), ["TOTAL", "game/ui"], result.stdout)
        self.assertIn("rename-extract", result.stdout)


if __name__ == "__main__":
    unittest.main()
