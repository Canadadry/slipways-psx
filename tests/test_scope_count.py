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


if __name__ == "__main__":
    unittest.main()
