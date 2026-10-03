"""tools/scope_count.py: a mini declarations.tsv, an exceptions file and a
sub-step table, then the per-module refusal counts it prints (black box,
exit code). Run by `make scope-count-test`, part of `make test`."""

import os
import shutil
import subprocess
import sys
import time
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "scope_count.py")
WORK = os.path.join(REPO, "tmp", "test_scope_count_work")

HEADER = ["identity", "kind", "name", "parent", "file", "line", "linkage",
          "scope_lines", "occurrences", "flags"]
EXCEPTIONS = "# kind\tparent\tname\nfield\twidget_vec_t\tx\nfield\twidget_vec_t\ty\n"
SUBSTEPS = ("substep\tbound\tmodules\n"
            "3a\t0\tengine/widget tests/test_widget\n"
            "3b\t0\tgame/gadget\n")


def decl(kind, name, file, parent="-", scope="-"):
    return ["c:%s@%s@%s" % (file, parent, name), kind, name, parent, file, "1",
            "-", str(scope), "1", "-"]


def run(rows, *args):
    shutil.rmtree(WORK, ignore_errors=True)
    os.makedirs(WORK)
    declarations = os.path.join(WORK, "declarations.tsv")
    with open(declarations, "w", encoding="utf-8") as f:
        f.write("# partial\tsrc/x.c\tdefault\tignored\n")
        for cells in [HEADER] + rows:
            f.write("\t".join(cells) + "\n")
    with open(os.path.join(WORK, "exceptions.txt"), "w", encoding="utf-8") as f:
        f.write(EXCEPTIONS)
    with open(os.path.join(WORK, "substeps.tsv"), "w", encoding="utf-8") as f:
        f.write(SUBSTEPS)
    result = subprocess.run(
        [sys.executable, SCRIPT, "--declarations", declarations,
         "--exceptions", os.path.join(WORK, "exceptions.txt"), "--root", WORK] + list(args),
        capture_output=True, text=True)
    return result


def table(result):
    """{module: {column: count}} from the printed table."""
    lines = result.stdout.splitlines()
    columns = lines[0].split()[1:]
    found = {}
    for line in lines[1:]:
        if line.startswith("warning:"):
            continue
        cells = line.split()
        found[cells[0]] = dict(zip(columns, map(int, cells[1:])))
    return found


class ScopeCount(unittest.TestCase):

    def test_each_refused_kind_of_a_short_name_has_its_column(self):
        kinds = ["type", "tag", "function", "field", "enum_constant", "macro", "static",
                 "global"]
        result = run([decl(kind, "q", "src/game/gadget.c") for kind in kinds])
        self.assertEqual(result.returncode, 0, result.stderr)
        row = table(result)["game/gadget"]
        for kind in kinds:
            self.assertEqual(row[kind], 1, kind)
        self.assertEqual(row["long-scope"], 0)
        self.assertEqual(row["total"], len(kinds))

    def test_names_of_three_characters_or_more_are_not_counted(self):
        found = table(run([decl("field", "abc", "src/game/gadget.c"),
                           decl("function", "q", "src/game/gadget.c")]))
        self.assertEqual(found["game/gadget"]["field"], 0)
        self.assertEqual(found["game/gadget"]["total"], 1)

    def test_a_local_or_param_is_allowed_up_to_forty_lines(self):
        found = table(run([decl("local", "i", "src/game/gadget.c", "run", 40),
                           decl("param", "n", "src/game/gadget.c", "run", 40),
                           decl("local", "e", "src/game/gadget.c", "run", 41),
                           decl("param", "dx", "src/game/gadget.c", "run", 41)]))
        self.assertEqual(found["game/gadget"]["long-scope"], 2)
        self.assertEqual(found["game/gadget"]["total"], 2)

    def test_a_static_of_a_function_is_judged_as_a_local_by_its_scope(self):
        found = table(run([decl("static", "dy", "tests/test_widget.c", "fold", 40),
                           decl("static", "kw", "tests/test_widget.c", "fold", 41),
                           decl("static", "CV", "tests/test_widget.c", "-", 3)]))
        self.assertEqual(found["tests/test_widget"]["long-scope"], 1)
        self.assertEqual(found["tests/test_widget"]["static"], 1)
        self.assertEqual(found["tests/test_widget"]["total"], 2)

    def test_a_param_of_a_function_pointer_type_has_prototype_scope(self):
        found = table(run([decl("param", "e", "src/game/gadget.h", "src/game/gadget.c", 90),
                           decl("param", "e", "src/game/gadget.c", "run", 90)]))
        self.assertEqual(found["game/gadget"]["long-scope"], 1)

    def test_x_and_y_of_the_vector_type_are_excepted_but_not_of_another_type(self):
        found = table(run([decl("field", "x", "src/engine/widget.h", "widget_vec_t"),
                           decl("field", "y", "src/engine/widget.h", "widget_vec_t"),
                           decl("field", "x", "src/engine/widget.h", "widget_box_t")]))
        self.assertEqual(found["engine/widget"]["field"], 1)

    def test_rows_are_per_module_of_the_declaring_file_with_a_total(self):
        found = table(run([decl("field", "q", "src/engine/widget.h"),
                           decl("field", "w", "src/engine/widget.c"),
                           decl("global", "g", "tests/test_widget.c")]))
        self.assertEqual(found["engine/widget"]["field"], 2)
        self.assertEqual(found["tests/test_widget"]["global"], 1)
        self.assertEqual(found["total"]["total"], 3)

    def test_modules_and_substep_select_the_rows(self):
        rows = [decl("field", "q", "src/engine/widget.h"),
                decl("field", "q", "src/game/gadget.c"),
                decl("field", "q", "src/game/gadget_family.c")]
        found = table(run(rows, "--module", "game/gadget"))
        self.assertEqual(sorted(found), ["game/gadget", "total"])
        found = table(run(rows, "--module", "game/gadget*"))
        self.assertEqual(sorted(found), ["game/gadget", "game/gadget_family", "total"])
        found = table(run(rows, "--substep", "3b",
                          "--substeps", os.path.join(WORK, "substeps.tsv")))
        self.assertEqual(sorted(found), ["game/gadget", "total"])

    def test_warns_when_the_extraction_is_older_than_the_sources(self):
        result = run([decl("field", "q", "src/game/gadget.c")])
        self.assertNotIn("warning:", result.stdout)
        os.makedirs(os.path.join(WORK, "src", "game"))
        source = os.path.join(WORK, "src", "game", "gadget.c")
        with open(source, "w", encoding="utf-8") as f:
            f.write("int q;\n")
        later = time.time() + 10
        os.utime(source, (later, later))
        result = subprocess.run(
            [sys.executable, SCRIPT, "--declarations", os.path.join(WORK, "declarations.tsv"),
             "--exceptions", os.path.join(WORK, "exceptions.txt"), "--root", WORK],
            capture_output=True, text=True)
        self.assertEqual(result.returncode, 0)
        self.assertIn("warning:", result.stdout)


class Check(unittest.TestCase):
    """--check: the scope guard, exit 1 on any refused declaration, 2 when
    the extraction is missing or older than the sources."""

    def test_check_fails_on_a_refused_declaration_and_passes_without_one(self):
        result = run([decl("global", "g", "src/game/gadget.c")], "--check")
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertEqual(table(result)["total"]["total"], 1)
        result = run([decl("global", "gadget", "src/game/gadget.c"),
                      decl("field", "x", "src/engine/widget.h", "widget_vec_t")], "--check")
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(table(result)["total"]["total"], 0)

    def test_check_exits_two_on_a_stale_or_missing_extraction(self):
        run([])
        os.makedirs(os.path.join(WORK, "src", "game"))
        source = os.path.join(WORK, "src", "game", "gadget.c")
        with open(source, "w", encoding="utf-8") as f:
            f.write("int gadget;\n")
        later = time.time() + 10
        os.utime(source, (later, later))
        args = [sys.executable, SCRIPT, "--exceptions", os.path.join(WORK, "exceptions.txt"),
                "--root", WORK, "--check", "--declarations"]
        result = subprocess.run(args + [os.path.join(WORK, "declarations.tsv")],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertIn("make rename-extract", result.stderr)
        result = subprocess.run(args + [os.path.join(WORK, "none.tsv")],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stdout)
        self.assertIn("make rename-extract", result.stderr)

    def test_check_is_refused_with_cryptic(self):
        result = run([], "--check", "--cryptic", "--table", os.path.join(WORK, "none.tsv"))
        self.assertEqual(result.returncode, 2)
        self.assertIn("--check", result.stderr)


TABLE = ("# Statut : validé\n"
         "ancien\tnouveau\tfichier\tcatégorie\tsorte\tstatut\tmodule\tconcept\tnote\tidentité\n"
         "jo_i\tplanet_index\tsrc/game/gadget.c\tidentifiant\tlocale\trenommé\tgame/gadget\t\t\tc:x\n"
         "q\tq\tsrc/game/gadget.c\tidentifiant\tlocale\tconservé\tgame/gadget\t\t\tc:y\n"
         "zz\tcount_total\tsrc/game/gadget.c\tidentifiant\tlocale\tconservé\tgame/gadget\t\t\tc:z\n")
TABLE_PATH = os.path.join(REPO, "tmp", "test_scope_count_table.tsv")


def cryptic(rows, *args):
    """The candidate lines and the per-module totals of --cryptic."""
    result = run(rows, "--cryptic", "--table", TABLE_PATH, *args)
    candidates, totals = [], {}
    for line in result.stdout.splitlines():
        cells = line.split()
        if line.startswith("warning:") or not cells or cells[0] in ("module", "candidate"):
            continue
        if cells[0] == "total" or len(cells) == 2:
            totals[cells[0]] = int(cells[1])
        else:
            candidates.append(tuple(cells))
    return result, candidates, totals


class Cryptic(unittest.TestCase):
    """--cryptic: the locals and params of scope > 40 lines whose name has
    three characters or less, or a `_` part that no renamed new name has."""

    @classmethod
    def setUpClass(cls):
        os.makedirs(os.path.dirname(TABLE_PATH), exist_ok=True)
        with open(TABLE_PATH, "w", encoding="utf-8") as f:
            f.write(TABLE)

    def test_a_short_name_is_a_candidate_past_forty_lines(self):
        result, candidates, totals = cryptic([decl("local", "abc", "src/game/gadget.c", "run", 41),
                                              decl("param", "pos", "src/game/gadget.c", "run", 40)])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(candidates, [("game/gadget", "local", "abc", "run", "41")], result.stdout)
        self.assertEqual(totals, {"game/gadget": 1, "total": 1}, result.stdout)

    def test_a_name_with_a_part_no_new_name_has_is_a_candidate(self):
        _, candidates, _ = cryptic([decl("local", "planet_index", "src/game/gadget.c", "run", 90),
                                    decl("local", "Planet_INDEX", "src/game/gadget.c", "run", 90),
                                    decl("local", "planet_tally", "src/game/gadget.c", "run", 90),
                                    decl("param", "count_total", "src/game/gadget.c", "run", 90)])
        self.assertEqual(sorted(c[2] for c in candidates), ["count_total", "planet_tally"])

    def test_fields_statics_and_function_pointer_params_are_not_candidates(self):
        _, candidates, totals = cryptic([decl("field", "abc", "src/game/gadget.c", "run", 90),
                                         decl("static", "abc", "src/game/gadget.c", "run", 90),
                                         decl("param", "abc", "src/game/gadget.h",
                                              "src/game/gadget.c", 90)])
        self.assertEqual(candidates, [])
        self.assertEqual(totals, {"total": 0})

    def test_a_static_of_a_function_past_forty_lines_is_not_a_candidate(self):
        _, candidates, totals = cryptic([decl("static", "ab", "tests/test_widget.c", "fold", 41),
                                         decl("local", "ab", "tests/test_widget.c", "fold", 41)])
        self.assertEqual(candidates, [("tests/test_widget", "local", "ab", "fold", "41")])
        self.assertEqual(totals, {"tests/test_widget": 1, "total": 1})

    def test_argc_and_argv_of_main_are_not_candidates(self):
        _, candidates, _ = cryptic([decl("param", "argc", "src/cmd/gadget.c", "main", 90),
                                    decl("param", "argv", "src/cmd/gadget.c", "main", 90),
                                    decl("param", "argc", "src/cmd/gadget.c", "run", 90),
                                    decl("local", "argv", "src/cmd/gadget.c", "main", 90)])
        self.assertEqual(sorted(c[1:4] for c in candidates),
                         [("local", "argv", "main"), ("param", "argc", "run")])

    def test_the_exceptions_file_is_needed_by_the_count_only(self):
        run([decl("local", "abc", "src/game/gadget.c", "run", 41)])
        args = [sys.executable, SCRIPT, "--declarations", os.path.join(WORK, "declarations.tsv"),
                "--root", WORK]
        result = subprocess.run(args + ["--cryptic", "--table", TABLE_PATH],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("game/gadget\tlocal\tabc\trun\t41", result.stdout)
        result = subprocess.run(args, capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn("--exceptions", result.stderr)

    def test_modules_and_substep_select_the_candidates(self):
        rows = [decl("local", "abc", "src/engine/widget.c", "run", 90),
                decl("local", "abc", "src/game/gadget.c", "run", 90)]
        _, candidates, _ = cryptic(rows, "--module", "engine/*")
        self.assertEqual([c[0] for c in candidates], ["engine/widget"])
        _, candidates, _ = cryptic(rows, "--substep", "3b",
                                   "--substeps", os.path.join(WORK, "substeps.tsv"))
        self.assertEqual([c[0] for c in candidates], ["game/gadget"])


if __name__ == "__main__":
    unittest.main()
