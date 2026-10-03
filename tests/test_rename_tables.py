"""tools/rename_tables.py, module API: the readers of the rename table, of
an extraction's declarations, of the header line and of exception triples,
on mini files. Run by `make rename-tables-test`, part of `make test`."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = os.path.join(REPO, "tools")
WORK = os.path.join(REPO, "tmp", "test_rename_tables_work")
sys.path.insert(0, TOOLS)
import rename_tables  # noqa: E402


def write(name, text):
    os.makedirs(WORK, exist_ok=True)
    path = os.path.join(WORK, name)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)
    return path


class Table(unittest.TestCase):

    def setUp(self):
        shutil.rmtree(WORK, ignore_errors=True)

    def test_leading_comment_lines_are_kept_and_rows_follow_the_file_header(self):
        path = write("table.tsv", "# Title\n# Status : ok\n\nold\tnew\tfile\n"
                                  "a\tb\tsrc/x.c\n\n# a later comment\nc\td\tsrc/y.c\n")
        comments, rows = rename_tables.read_table(path)
        self.assertEqual(comments, ["# Title", "# Status : ok"])
        self.assertEqual(rows, [{"old": "a", "new": "b", "file": "src/x.c"},
                                {"old": "c", "new": "d", "file": "src/y.c"}])

    def test_missing_cells_are_empty(self):
        path = write("table.tsv", "old\tnew\tfile\tnote\na\tb\n")
        _, rows = rename_tables.read_table(path)
        self.assertEqual(rows, [{"old": "a", "new": "b", "file": "", "note": ""}])

    def test_the_header_line_is_the_first_one_neither_blank_nor_a_comment(self):
        lines = ["# Title", "", "# Status", "old\tnew", "a\tb"]
        self.assertEqual(rename_tables.header_line(lines), (3, ["old", "new"]))
        self.assertEqual(rename_tables.header_line(["# only comments", ""]), (None, None))


class Declarations(unittest.TestCase):

    def test_declarations_are_read_by_their_header(self):
        path = write("declarations.tsv",
                     "# partial\tsrc/x.c\tdefault\tsome reason\n"
                     "identity\tkind\tname\tparent\tfile\tline\tlinkage\tscope_lines"
                     "\toccurrences\tflags\n"
                     "c:@F@run\tfunction\trun\t-\tsrc/x.c\t3\textern\t-\t2\t-\n")
        self.assertEqual(rename_tables.read_declarations(path), [{
            "identity": "c:@F@run", "kind": "function", "name": "run", "parent": "-",
            "file": "src/x.c", "line": "3", "linkage": "extern", "scope_lines": "-",
            "occurrences": "2", "flags": "-"}])


class Triples(unittest.TestCase):

    def test_exception_lines_are_three_cells_padded_with_empty_ones(self):
        path = write("exceptions.txt", "# kind\tparent\tname\n\n"
                                       "field\twidget_vec_t\tx\tnote\n  \nstring\tMakefile\n")
        self.assertEqual(rename_tables.read_triples(path),
                         [("field", "widget_vec_t", "x"), ("string", "Makefile", "")])


class Imports(unittest.TestCase):

    def test_the_substep_module_loads_without_the_counter(self):
        result = subprocess.run(
            [sys.executable, "-c", "import sys, rename_substeps; "
                                   "print('rename_count' in sys.modules)"],
            cwd=TOOLS, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.strip(), "False")


if __name__ == "__main__":
    unittest.main()
