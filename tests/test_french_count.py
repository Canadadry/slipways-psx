"""tools/french_count.py, black box: a mini source tree, a mini sub-step
table and an exceptions file, then the per-module counts it prints. Run by
`make french-count-test`, part of `make test`."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "french_count.py")
WORK = os.path.join(REPO, "tmp", "test_french_count_work")

SUBSTEPS = ("substep\tbound\tmodules\n"
            "3a\t0\tengine/widget tests/test_widget tools/widget_expected\n"
            "3b\t0\tgame/gadget tools/gadget\n")
EXCEPTIONS = "# column\tmodule\tword\n"


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def run(files, exceptions=EXCEPTIONS, extra=()):
    shutil.rmtree(WORK, ignore_errors=True)
    root = os.path.join(WORK, "tree")
    for path, text in files.items():
        write(os.path.join(root, path), text)
    write(os.path.join(WORK, "substeps.tsv"), SUBSTEPS)
    write(os.path.join(WORK, "exceptions.txt"), exceptions)
    result = subprocess.run(
        [sys.executable, SCRIPT, "--root", root,
         "--exceptions", os.path.join(WORK, "exceptions.txt"),
         "--substeps", os.path.join(WORK, "substeps.tsv")] + list(extra),
        capture_output=True, text=True)
    return result


def counts(result):
    """{module: {column: n}} from the printed table."""
    table = {}
    header = None
    for line in result.stdout.splitlines():
        cells = line.split()
        if cells[:1] == ["module"]:
            header = cells[1:]
        elif header and len(cells) == len(header) + 1 and all(c.isdigit() for c in cells[1:]):
            table[cells[0]] = dict(zip(header, map(int, cells[1:])))
    return table


def parts(result):
    """{(module, part): {column: n}} from the printed --substep table."""
    table = {}
    header = None
    for line in result.stdout.splitlines():
        cells = line.split()
        if cells[:2] == ["module", "part"]:
            header = cells[2:]
        elif header and len(cells) == len(header) + 2 and all(c.isdigit() for c in cells[2:]):
            table[(cells[0], cells[1])] = dict(zip(header, map(int, cells[2:])))
    return table


class Detection(unittest.TestCase):

    def test_a_function_word_in_a_comment_is_counted_and_never_fails(self):
        result = run({"src/game/gadget.c": "/* the gadget, pour the record */\nint x;\n"})
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(counts(result)["game/gadget"],
                         {"identifier": 0, "backtick": 0, "string": 0, "comment": 1,
                          "total": 1}, result.stdout)

    def test_each_region_has_its_column(self):
        result = run({
            "src/game/gadget.c": (
                "/* see `les_items` and dans the middle */\n"
                "static int count_des_items;\n"
                "static const char *k = \"read sans fail\";\n"
                "void f(void) { assert(k && \"broken avec `count_des` here\"); }\n"),
            "tools/gadget.sh": "# a comment chez us\necho \"done pour now\"\n",
        })
        table = counts(result)
        self.assertEqual(table["game/gadget"], {"identifier": 1, "backtick": 2, "string": 1,
                                                 "comment": 2, "total": 6}, result.stdout)
        self.assertEqual(table["tools/gadget"], {"identifier": 0, "backtick": 0, "string": 1,
                                                  "comment": 1, "total": 2}, result.stdout)

    def test_a_one_word_name_in_code_or_backticks_is_a_name_not_french(self):
        result = run({"src/game/gadget.c": (
            "/* `le` and `et.p` are kept locals; `ne()` too; `pas_de_place` is not */\n"
            "static int f(int le, int et) { return le + et; }\n")})
        self.assertEqual(counts(result)["game/gadget"],
                         {"identifier": 0, "backtick": 2, "string": 0, "comment": 0,
                          "total": 2}, result.stdout)

    def test_a_command_option_is_not_a_word(self):
        result = run({"tools/gadget.sh": "if [ \"$s\" -ne 0 ]; then grep -nE x; fi\necho rendez-vous\n"})
        self.assertEqual(counts(result)["tools/gadget"]["string"], 1, result.stdout)

    def test_an_escape_does_not_glue_to_the_next_word(self):
        result = run({"tools/gadget.sh": "printf 'x\\tdans\\n'\n"})
        self.assertEqual(counts(result)["tools/gadget"]["string"], 1, result.stdout)

    def test_a_data_line_is_a_string(self):
        result = run({"tools/widget_expected.txt": "widget 3 jamais seen\n"})
        self.assertEqual(counts(result)["tools/widget_expected"]["string"], 1, result.stdout)

    def test_an_accented_word_counts_but_not_signs(self):
        result = run({"src/game/gadget.c": (
            "/* déjà drawn; 3 × 4 — then → next; a÷b */\n"
            "static const char *k = \"zone pleine, limite atteinte à 4\";\n"
            "/* Œuvre and naïve */\n")})
        self.assertEqual(counts(result)["game/gadget"],
                         {"identifier": 0, "backtick": 0, "string": 1, "comment": 3,
                          "total": 4}, result.stdout)

    def test_homographs_are_not_counted(self):
        result = run({"src/game/gadget.c": (
            "/* Find a free place in the file; the limite is on as or plus son car but */\n"
            "static int place_file_limite(int place, int file) { return place + file; }\n"
            "static const char *k = \"place file limite\";\n")})
        self.assertNotIn("game/gadget", counts(result), result.stdout)


class Exceptions(unittest.TestCase):

    def test_a_line_removes_exactly_its_column_module_and_word(self):
        files = {
            "src/game/gadget.c": "/* dans dans sans */\nstatic const char *k = \"dans\";\n",
            "src/engine/widget.c": "/* dans */\n",
        }
        result = run(files, EXCEPTIONS + "comment\tgame/gadget\tdans\n")
        table = counts(result)
        self.assertEqual(table["game/gadget"], {"identifier": 0, "backtick": 0, "string": 1,
                                                 "comment": 1, "total": 2}, result.stdout)
        self.assertEqual(table["engine/widget"]["comment"], 1, result.stdout)


class Check(unittest.TestCase):
    """--check: the French guard, exit 1 while a word is left after the
    exceptions."""

    FILES = {"src/game/gadget.c": "/* the gadget, pour the record */\n"}

    def test_check_fails_on_a_word_and_passes_on_a_clean_tree(self):
        result = run(self.FILES, extra=["--check"])
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertEqual(counts(result)["TOTAL"]["total"], 1, result.stdout)
        result = run({"src/game/gadget.c": "/* the gadget */\n"}, extra=["--check"])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_check_passes_when_an_exception_removes_the_only_word(self):
        result = run(self.FILES, EXCEPTIONS + "comment\tgame/gadget\tpour\n", extra=["--check"])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(counts(result)["TOTAL"]["total"], 0, result.stdout)

    def test_check_still_lists_the_sites(self):
        result = run(self.FILES, extra=["--check", "--sites"])
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("src/game/gadget.c:1 comment pour", result.stdout)

    DEAD = EXCEPTIONS + "comment\tgame/gadget\tpour\nstring\tgame/gadget\tdans\n"

    def test_check_fails_on_an_exception_line_that_removed_nothing(self):
        result = run(self.FILES, self.DEAD, extra=["--check"])
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("exceptions.txt:3: removed nothing: string game/gadget dans",
                      result.stderr)
        self.assertNotIn("exceptions.txt:2:", result.stderr)
        self.assertEqual(counts(result)["TOTAL"]["total"], 0, result.stdout)

    def test_a_dead_line_is_judged_only_by_check_over_the_whole_tree(self):
        result = run(self.FILES, self.DEAD)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn("removed nothing", result.stdout + result.stderr)
        result = run(self.FILES, self.DEAD, extra=["--check", "--module", "game/gadget"])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn("removed nothing", result.stdout + result.stderr)

    def test_check_is_refused_with_substep(self):
        result = run(self.FILES, extra=["--check", "--substep", "3b"])
        self.assertEqual(result.returncode, 2)
        self.assertIn("--check", result.stderr)


class Substep(unittest.TestCase):
    """--substep: the files of its modules; a mention in a literal or a data
    line whose whole text a later sub-step's file also has is `later`."""

    FILES = {
        "src/engine/widget.c": (
            "/* read dans order */\n"
            "static const char *a = \"exemptée\";\n"
            "static const char *b = \"rendu à droite\";\n"),
        "tools/widget_expected.txt": "widget 0 exemptée (abstraite)\nwidget 1 rien\n",
        "tools/gadget.sh": "awk '$4 != \"exemptée\"'\necho 'widget 0 exemptée (abstraite)'\n"
                           "printf 'x'\n",
        "tools/gadget.txt": "widget 0 exemptée (abstraite)\n",
        "src/game/gadget.c": "/* dans */\nstatic const char *c = \"exemptée\";\n",
    }

    def test_literals_and_lines_repeated_by_a_later_substep_are_later(self):
        result = run(self.FILES, extra=["--substep", "3a"])
        self.assertEqual(result.returncode, 0, result.stderr)
        table = parts(result)
        self.assertEqual(table[("engine/widget", "done")],
                         {"identifier": 0, "backtick": 0, "string": 1, "comment": 1,
                          "total": 2}, result.stdout)
        self.assertEqual(table[("engine/widget", "later")]["string"], 1, result.stdout)
        self.assertEqual(table[("tools/widget_expected", "done")]["string"], 1, result.stdout)
        self.assertEqual(table[("tools/widget_expected", "later")]["string"], 1, result.stdout)
        self.assertNotIn(("game/gadget", "done"), table, result.stdout)
        self.assertEqual(table[("TOTAL", "done")]["total"], 3, result.stdout)
        self.assertEqual(table[("TOTAL", "later")]["total"], 2, result.stdout)


class Files(unittest.TestCase):

    def test_own_files_and_dictionary_tools_are_not_read(self):
        result = run({
            "tools/french_count.py": "# les des dans\n",
            "tests/test_french_count.py": "# les des dans\n",
            "tools/french_exceptions.txt": "comment\tgame/gadget\tdans\n",
            "tools/rename_overrides.tsv": "name\tdes\tles\n",
            "tools/rename_widget.py": "# les\n",
            "tests/test_rename_widget.py": "# les\n",
            "tools/ast_rename/x.py": "# les\n",
            "tests/e2e/x.txt": "les\n",
            "third_party/x.c": "/* les */\n",
            "src/game/gadget.c": "/* les */\n",
        })
        self.assertEqual(sorted(counts(result)), ["TOTAL", "game/gadget"], result.stdout)

    def test_warns_for_each_module_read_in_no_substep(self):
        result = run({"src/game/gadget.c": "int x;\n", "src/game/stray.c": "int y;\n",
                      "tools/stray.sh": "true\n"})
        self.assertEqual(result.returncode, 0, result.stderr)
        warnings = [l for l in result.stdout.splitlines() if l.startswith("warning:")]
        self.assertEqual(len(warnings), 2, result.stdout)
        self.assertIn("game/stray", warnings[0])
        self.assertIn("tools/stray", warnings[1])

    def test_module_filter_and_sites(self):
        result = run({"src/game/gadget.c": "int x;\n/* sans */\n",
                      "src/engine/widget.c": "/* sans */\n"},
                     extra=["--module", "game/g*", "--sites"])
        self.assertEqual(sorted(counts(result)), ["TOTAL", "game/gadget"], result.stdout)
        self.assertIn("src/game/gadget.c:2 comment sans", result.stdout)


if __name__ == "__main__":
    unittest.main()
