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


SUBSTEPS = ("substep\tbound\tmodules\n"
            "3a\t7\tengine/entity_class\n"
            "3b\t2\tgame/planet\n"
            "3c\t0\tgame/ui\n")


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
    write(os.path.join(WORK, "rename_substeps.tsv"), SUBSTEPS)
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


def parts(stdout):
    """{(module, part): {column: n}} and the residue line of a --substep run."""
    table = {}
    header = None
    residue = None
    for line in stdout.splitlines():
        cells = line.split()
        if cells[:2] == ["module", "part"]:
            header = cells[2:]
        elif cells[:1] == ["residue"]:
            residue = line
        elif header and len(cells) == len(header) + 2 and all(c.isdigit() for c in cells[2:]):
            table[(cells[0], cells[1])] = dict(zip(header, map(int, cells[2:])))
    return table, residue


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


class LocalNames(unittest.TestCase):
    """An ambiguous name renamed only as locals or parameters counts in a
    backtick span only in the files of those rows."""

    MEM = ["mem", "memory", "src/game/a.c", "identifiant", "locale", "renommé",
           "game/a", "", "", "c:a.c@10@F@f@mem"]
    FILES = {"src/game/a.c": "/* `mem` */\n", "src/game/b.h": "/* `mem` */\n"}

    def test_counted_in_the_file_of_the_local_only(self):
        result, table = run([self.MEM], self.FILES)
        self.assertEqual(table["game/a"]["backtick"], 1, result.stdout)
        self.assertNotIn("game/b", table, result.stdout)

    def test_a_parameter_row_too(self):
        param = ["mem", "memory", "src/game/c.c", "identifiant", "paramètre", "renommé",
                 "game/c", "", "", "c:c.c@20@F@g@mem"]
        files = dict(self.FILES, **{"src/game/c.c": "/* `mem` */\n"})
        result, table = run([self.MEM, param], files)
        self.assertEqual(table["game/a"]["backtick"], 1, result.stdout)
        self.assertEqual(table["game/c"]["backtick"], 1, result.stdout)
        self.assertNotIn("game/b", table, result.stdout)

    def test_a_field_row_makes_it_count_everywhere(self):
        field = ["mem", "memory", "src/game/c.h", "identifiant", "champ", "renommé",
                 "game/c", "", "", "c:@S@c@FI@mem"]
        result, table = run([self.MEM, field], self.FILES)
        self.assertEqual(table["game/a"]["backtick"], 1, result.stdout)
        self.assertEqual(table["game/b"]["backtick"], 1, result.stdout)

    def test_a_distinctive_local_counts_everywhere(self):
        row = ["obj_len", "object_length", "src/game/a.c", "identifiant", "locale", "renommé",
               "game/a", "", "", "c:a.c@10@F@f@obj_len"]
        result, table = run([row], {"src/game/b.h": "/* `obj_len` */\n"})
        self.assertEqual(table["game/b"]["backtick"], 1, result.stdout)


class Parts(unittest.TestCase):
    """--part text|ast: one half of the count each, the AST half being the
    identifier column of the ambiguous names; --check turns the table into a
    guard (exit 1 on any residue, 2 without a usable extraction)."""

    ROWS = ROWS + [
        ["p", "map_pos", "src/engine/entity.h", "identifiant", "champ", "renommé",
         "engine/entity", "p", "", "c:@S@h@FI@p"],
    ]
    PLANET = ("void f(entity_t *e) { pico8_jo_kj(); int q = e->p; }\n"
              "/* `p` and `pico8_jo_kj` */\n")
    FILES = {"src/game/planet.c": PLANET}
    OCCURRENCES = [["c:@S@h@FI@p", "src/game/planet.c", PLANET.index("e->p") + 3, 1, "ref"]]

    def run_part(self, *extra, files=None, occurrences=None):
        return run(self.ROWS, self.FILES if files is None else files,
                   self.OCCURRENCES if occurrences is None else occurrences, extra=extra)

    def test_text_part_reads_no_occurrences_and_warns_nothing(self):
        result, table = self.run_part("--part", "text",
                                      "--occurrences", os.path.join(WORK, "none.tsv"))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn("warning:", result.stdout)
        self.assertEqual(table["game/planet"], {"identifier": 1, "backtick": 2, "string": 0,
                                                 "comment": 0, "total": 3}, result.stdout)

    def test_text_part_needs_no_occurrences_option(self):
        args = [sys.executable, SCRIPT, "--table", os.path.join(WORK, "renommage.tsv"),
                "--root", os.path.join(WORK, "tree"), "--part", "text"]
        self.run_part()
        result = subprocess.run(args, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(counts(result.stdout)["TOTAL"]["total"], 3, result.stdout)

    def test_ast_part_counts_only_the_ambiguous_identifiers(self):
        result, table = self.run_part("--part", "ast")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(table["game/planet"], {"identifier": 1, "backtick": 0, "string": 0,
                                                 "comment": 0, "total": 1}, result.stdout)

    def test_the_two_parts_add_up_to_the_whole_count(self):
        _, whole = self.run_part()
        _, text = self.run_part("--part", "text")
        _, ast = self.run_part("--part", "ast")
        for module in whole:
            for column in whole[module]:
                self.assertEqual(text.get(module, {}).get(column, 0)
                                 + ast.get(module, {}).get(column, 0),
                                 whole[module][column], (module, column))

    def test_check_fails_on_a_residue_and_passes_on_a_clean_tree(self):
        result, table = self.run_part("--part", "text", "--check")
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertEqual(table["TOTAL"]["total"], 3, result.stdout)
        result, table = self.run_part("--part", "text", "--check",
                                      files={"src/game/planet.c": "void f(void);\n"})
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(table["TOTAL"]["total"], 0, result.stdout)

    def test_check_of_the_ast_part_without_an_extraction_exits_two(self):
        result, _ = self.run_part("--part", "ast", "--check",
                                  "--occurrences", os.path.join(WORK, "none.tsv"))
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertIn("make rename-extract", result.stderr)
        result, _ = self.run_part("--check", "--occurrences", os.path.join(WORK, "none.tsv"))
        self.assertEqual(result.returncode, 2, result.stdout)

    def test_check_of_the_ast_part_with_a_stale_extraction_exits_two(self):
        self.run_part()
        source = os.path.join(WORK, "tree", "src", "game", "planet.c")
        later = os.path.getmtime(os.path.join(WORK, "occurrences.tsv")) + 10
        os.utime(source, (later, later))
        args = [sys.executable, SCRIPT, "--table", os.path.join(WORK, "renommage.tsv"),
                "--occurrences", os.path.join(WORK, "occurrences.tsv"),
                "--root", os.path.join(WORK, "tree"), "--part", "ast", "--check"]
        result = subprocess.run(args, capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertIn("make rename-extract", result.stderr)

    def test_check_is_refused_with_substep(self):
        result, _ = self.run_part("--check", "--substep", "3b",
                                  "--substeps", os.path.join(WORK, "rename_substeps.tsv"))
        self.assertEqual(result.returncode, 2)
        self.assertIn("--check", result.stderr)


class Substep(unittest.TestCase):
    """--substep: the files of the sub-step's modules, each mention filed
    under done (its name's sub-step is this one or an earlier one) or later."""

    ROWS = [
        ["pico8_jo_kj", "planet_draw", "src/game/planet.h", "identifiant", "fonction", "renommé",
         "game/planet", "jo", "", "c:@F@pico8_jo_kj"],
        ["pico8_ui_kj", "ui_draw", "src/game/ui.h", "identifiant", "fonction", "renommé",
         "game/ui", "ui", "", "c:@F@pico8_ui_kj"],
        ["pico8_shared", "planet_shared", "src/game/planet.c", "identifiant", "statique",
         "renommé", "game/planet", "", "", "c:planet.c@pico8_shared"],
        ["pico8_shared", "ui_shared", "src/game/ui.c", "identifiant", "statique",
         "renommé", "game/ui", "", "", "c:ui.c@pico8_shared"],
        ["p", "map_pos", "src/engine/entity_class.h", "identifiant", "champ", "renommé",
         "engine/entity_class", "p", "", "c:@S@h@FI@p"],
        ["p", "pos", "src/game/ui.h", "identifiant", "champ", "renommé",
         "game/ui", "p", "", "c:@S@u@FI@p"],
        ["jo", "planet", "src/game/planet.c", "chaîne", "clé d'étiquette", "renommé",
         "game/planet", "jo", "lecteur", "chaîne:jo"],
        ["jo", "planet", "src/game/ui.c", "identifiant", "locale", "renommé",
         "game/ui", "jo", "", "c:ui.c@12@F@f@jo"],
    ]
    PLANET = ("void f(h_t *e, u_t *u) {\n"
              "    pico8_jo_kj(); pico8_ui_kj(); pico8_shared();\n"
              "    e->p = u->p;\n"
              "    lookup(\"jo\");\n"
              "}\n"
              "/* see `pico8_ui_kj` */\n")
    FILES = {"src/game/planet.c": PLANET,
             "src/game/ui.c": "void g(void) { pico8_jo_kj(); pico8_ui_kj(); }\n"}
    OCCURRENCES = [
        ["c:@S@h@FI@p", "src/game/planet.c", PLANET.index("e->p") + 3, 1, "ref"],
        ["c:@S@u@FI@p", "src/game/planet.c", PLANET.index("u->p") + 3, 1, "ref"],
    ]

    def run_substep(self, substep):
        return run(self.ROWS, self.FILES, self.OCCURRENCES,
                   extra=["--substep", substep,
                          "--substeps", os.path.join(WORK, "rename_substeps.tsv")])

    def test_mentions_split_between_done_and_later(self):
        result, _ = self.run_substep("3b")
        self.assertEqual(result.returncode, 0, result.stderr)
        table, residue = parts(result.stdout)
        self.assertEqual(table[("game/planet", "done")],
                         {"identifier": 2, "backtick": 0, "string": 1, "comment": 0,
                          "total": 3}, result.stdout)
        self.assertEqual(table[("game/planet", "later")],
                         {"identifier": 3, "backtick": 1, "string": 0, "comment": 0,
                          "total": 4}, result.stdout)
        self.assertNotIn(("game/ui", "done"), table)
        self.assertEqual(table[("TOTAL", "done")]["identifier"], 2, result.stdout)
        self.assertEqual(table[("TOTAL", "later")]["identifier"], 3, result.stdout)
        self.assertEqual(residue, "residue 3b: later identifier 3, bound 2", result.stdout)

    def test_a_homonym_found_by_text_follows_the_identity_the_ast_puts_there(self):
        rows = self.ROWS + [
            ["cls_of", "class_of", "src/game/planet.c", "identifiant", "statique", "renommé",
             "game/planet", "", "", "c:planet.c@cls_of"],
            ["cls_of", "class_of", "src/game/ui.c", "identifiant", "statique", "renommé",
             "game/ui", "", "", "c:ui.c@cls_of"],
        ]
        planet = "/* \u00e9t\u00e9 */ static int cls_of(void);\n#define C cls_of\n"
        occurrences = [["c:planet.c@cls_of", "src/game/planet.c",
                        len(planet[:planet.index("cls_of")].encode("utf-8")), 6, "decl"]]
        result, _ = run(rows, {"src/game/planet.c": planet}, occurrences,
                        extra=["--substep", "3b",
                               "--substeps", os.path.join(WORK, "rename_substeps.tsv")])
        table, _ = parts(result.stdout)
        self.assertEqual(table[("game/planet", "done")]["identifier"], 1, result.stdout)
        self.assertEqual(table[("game/planet", "later")]["identifier"], 1, result.stdout)

    def test_last_substep_has_nothing_later(self):
        result, _ = self.run_substep("3c")
        table, residue = parts(result.stdout)
        self.assertEqual(table[("game/ui", "done")]["identifier"], 2, result.stdout)
        self.assertEqual(table[("TOTAL", "later")]["total"], 0, result.stdout)
        self.assertEqual(residue, "residue 3c: later identifier 0, bound 0", result.stdout)


if __name__ == "__main__":
    unittest.main()
