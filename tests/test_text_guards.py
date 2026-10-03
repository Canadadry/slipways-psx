"""old-names-guard and french-guard are alive: a reduced copy of the
working tree (the Makefile, the rename table, tools/*.py, the exception
files and the sub-step table), each violation planted in its own fresh copy,
then `make -C <copy> <guard>`, the real recipe. Each plant must turn its
column of its module red, and nothing else; the clean copy, and a plant in
the module an exception names, stay green. Run by `make text-guards-test`,
part of `make test`.

The plants are drawn from the table, the tools and the exception files at
run time and never written here: written out, they would turn the guards
red on this very file."""

import glob
import os
import re
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(REPO, "tmp", "test_text_guards_work")
sys.path.insert(0, os.path.join(REPO, "tools"))
import french_count  # noqa: E402
import rename_count  # noqa: E402
import rename_tables  # noqa: E402

RENAMED = "renomm\u00e9"
NAMES_EXCEPTIONS = "tools/cart_names_exceptions.txt"
FRENCH_EXCEPTIONS = "tools/french_exceptions.txt"
PLANTED = "src/game/planet.c"
MODULE = "game/planet"
NEUTRAL = "/* Counts the planets. */\nint planet_count(void) { return 0; }\n"
PARENT_MAKE_VARIABLES = ("MAKEFLAGS", "MFLAGS", "MAKELEVEL")


def table_path():
    """The rename table, as the Makefile names it."""
    with open(os.path.join(REPO, "Makefile"), encoding="utf-8") as f:
        match = re.search(r"^RENAME_TABLE\s*:?=\s*(\S+)\s*$", f.read(), re.M)
    assert match, "no RENAME_TABLE line in the Makefile"
    return match.group(1)


def copy_tree(work, extra=()):
    """The reduced copy: what the guard recipes read, plus `extra` paths
    (files or directories) relative to the repository."""
    shutil.rmtree(work, ignore_errors=True)
    paths = (["Makefile", table_path(), "tools/rename_substeps.tsv"]
             + sorted(os.path.relpath(p, REPO) for p in glob.glob(os.path.join(REPO, "tools", "*.py")))
             + sorted(os.path.relpath(p, REPO)
                      for p in glob.glob(os.path.join(REPO, "tools", "*_exceptions.txt")))
             + list(extra))
    for path in paths:
        source, target = os.path.join(REPO, path), os.path.join(work, path)
        os.makedirs(os.path.dirname(target), exist_ok=True)
        if os.path.isdir(source):
            shutil.copytree(source, target, ignore=shutil.ignore_patterns("__pycache__"))
        else:
            shutil.copy2(source, target)


def write(work, path, text):
    target = os.path.join(work, path)
    os.makedirs(os.path.dirname(target), exist_ok=True)
    with open(target, "w", encoding="utf-8") as f:
        f.write(text)


def run_make(work, target, *variables):
    """`make -C <copy> <target> [VAR=value...]`, without the jobserver and
    the command-line variables of a parent make."""
    env = {k: v for k, v in os.environ.items() if k not in PARENT_MAKE_VARIABLES}
    return subprocess.run(["make", "--no-print-directory", "-C", work, target] + list(variables),
                          capture_output=True, text=True, env=env, timeout=600)


def tables(stdout):
    """The tables a guard prints, each as {module: {column: count}}, the
    total row under its own name (TOTAL or total); the echoed recipe lines
    around them are skipped."""
    found, columns = [], None
    for line in stdout.splitlines():
        cells = line.split()
        if cells[:1] == ["module"]:
            columns = cells[1:]
            found.append({})
        elif columns and len(cells) == len(columns) + 1 and all(c.isdigit() for c in cells[1:]):
            found[-1][cells[0]] = dict(zip(columns, map(int, cells[1:])))
            if cells[0] in ("TOTAL", "total"):
                columns = None
    return found


def report(result):
    return "exit %d\nstdout:\n%s\nstderr:\n%s" % (result.returncode, result.stdout, result.stderr)


def module_of_file(module):
    """The one file of the repository whose module is `module`."""
    found = [p for p in glob.glob(os.path.join(REPO, module + ".*"))
             if os.path.splitext(p)[1] in rename_count.TEXT_FILES]
    assert len(found) == 1, "module %s: expected one file, found %s" % (module, found)
    return os.path.relpath(found[0], REPO)


def literal_line(path, text):
    """A line that holds `text` as a string: a C literal, or a data line of a
    script or data file."""
    if path.endswith(rename_count.C_FILES):
        return 'const char *label = "%s";\n' % text
    return 'label = "%s"\n' % text


class Plants:
    """The violations, drawn from the table, the tools and the exception
    files; each assumption checked, never skipped."""

    def __init__(self):
        _, rows = rename_tables.read_table(os.path.join(REPO, table_path()))
        names = rename_count.Names(rows)
        renamed = [r for r in rows if r["statut"] == RENAMED]
        kept = {r["ancien"] for r in rows if r["statut"] != RENAMED}
        new = {r["nouveau"] for r in renamed}
        identifiers = {r["ancien"] for r in renamed if r["cat\u00e9gorie"] == "identifiant"}
        candidates = sorted(n for n in names.distinctive_words
                            if n in identifiers and re.fullmatch(r"[A-Za-z_]\w*", n))
        assert candidates, "no distinctive old name of an identifier row in the table"
        self.old_name = candidates[0]
        assert self.old_name not in kept, "%s is also a kept name" % self.old_name
        assert self.old_name not in new, "%s is also a new name" % self.old_name

        name_triples = rename_tables.read_triples(os.path.join(REPO, NAMES_EXCEPTIONS))
        assert name_triples, "no line in %s" % NAMES_EXCEPTIONS
        self.name_exception = name_triples[0]
        assert self.name_exception[0] == "string", \
            "first line of %s is not a string exception: %s" % (NAMES_EXCEPTIONS,
                                                                self.name_exception)
        assert self.name_exception[1] != MODULE, "%s excepts %s" % (NAMES_EXCEPTIONS, MODULE)
        assert all(t[2] != self.old_name for t in name_triples), \
            "%s excepts %s" % (NAMES_EXCEPTIONS, self.old_name)

        french_triples = rename_tables.read_triples(os.path.join(REPO, FRENCH_EXCEPTIONS))
        excepted = {t[2].lower() for t in french_triples}
        words = sorted(w for w in french_count.FUNCTION_WORDS if len(w) >= 4)
        assert words, "no French function word of four letters or more"
        self.french_word = words[0]
        assert self.french_word not in excepted, \
            "%s is excepted in %s" % (self.french_word, FRENCH_EXCEPTIONS)
        self.accented_word = "pr" + chr(0xE9) + "sent"
        assert french_count.is_french(self.accented_word)
        assert self.accented_word not in excepted, \
            "%s is excepted in %s" % (self.accented_word, FRENCH_EXCEPTIONS)
        strings = [t for t in french_triples if t[0] == "string"]
        assert strings, "no string line in %s" % FRENCH_EXCEPTIONS
        self.french_exception = strings[0]
        assert self.french_exception[1] != MODULE, "%s excepts %s" % (FRENCH_EXCEPTIONS, MODULE)


PLANTS = Plants()


class TextGuards(unittest.TestCase):

    def setUp(self):
        copy_tree(WORK)
        write(WORK, PLANTED, NEUTRAL)

    def guard(self, target):
        return run_make(WORK, target)

    def assert_green(self, result):
        self.assertEqual(result.returncode, 0, report(result))
        found = tables(result.stdout)
        self.assertEqual(len(found), 1, report(result))
        self.assertEqual(list(found[0]), ["TOTAL"], report(result))
        self.assertFalse(any(found[0]["TOTAL"].values()), report(result))

    def assert_only(self, result, column, count=1):
        """Red, with one module row, MODULE, and only `column` lit."""
        self.assertNotEqual(result.returncode, 0, report(result))
        found = tables(result.stdout)
        self.assertEqual(len(found), 1, report(result))
        self.assertEqual(sorted(found[0]), sorted([MODULE, "TOTAL"]), report(result))
        row = found[0][MODULE]
        for name, value in row.items():
            expected = count if name in (column, "total") else 0
            self.assertEqual(value, expected, "%s\n%s" % (name, report(result)))

    def plant(self, text):
        write(WORK, PLANTED, NEUTRAL + text)

    # --- old-names-guard ----------------------------------------------------

    def test_old_names_guard_is_green_on_the_clean_copy(self):
        self.assert_green(self.guard("old-names-guard"))

    def test_an_old_name_as_an_identifier(self):
        self.plant("int %s(void) { return 0; }\n" % PLANTS.old_name)
        self.assert_only(self.guard("old-names-guard"), "identifier")

    def test_an_old_name_bare_in_a_comment(self):
        self.plant("/* %s */\n" % PLANTS.old_name)
        self.assert_only(self.guard("old-names-guard"), "comment")

    def test_an_old_name_between_backticks(self):
        self.plant("/* `%s` */\n" % PLANTS.old_name)
        self.assert_only(self.guard("old-names-guard"), "backtick")

    def test_an_old_name_in_a_string(self):
        self.plant(literal_line(PLANTED, PLANTS.old_name))
        self.assert_only(self.guard("old-names-guard"), "string")

    def test_an_excepted_old_name_in_another_module(self):
        self.plant(literal_line(PLANTED, PLANTS.name_exception[2]))
        self.assert_only(self.guard("old-names-guard"), "string")

    def test_an_excepted_old_name_in_its_module(self):
        path = module_of_file(PLANTS.name_exception[1])
        write(WORK, path, literal_line(path, PLANTS.name_exception[2]))
        self.assert_green(self.guard("old-names-guard"))

    # --- french-guard -------------------------------------------------------

    def test_french_guard_is_green_on_the_clean_copy(self):
        self.assert_green(self.guard("french-guard"))

    def test_a_french_word_in_a_comment(self):
        self.plant("/* %s */\n" % PLANTS.french_word)
        self.assert_only(self.guard("french-guard"), "comment")

    def test_an_accented_word_in_a_string(self):
        self.plant(literal_line(PLANTED, PLANTS.accented_word))
        self.assert_only(self.guard("french-guard"), "string")

    def test_an_excepted_french_word_in_another_module(self):
        self.plant(literal_line(PLANTED, PLANTS.french_exception[2]))
        self.assert_only(self.guard("french-guard"), "string")

    def test_an_excepted_french_word_in_its_module(self):
        path = module_of_file(PLANTS.french_exception[1])
        write(WORK, path, literal_line(path, PLANTS.french_exception[2]))
        self.assert_green(self.guard("french-guard"))


if __name__ == "__main__":
    unittest.main()
