"""ast-guard is alive: the reduced copy of tests/test_text_guards.py plus
tools/ast_rename/, a few units planted in it, then
`make -C <copy> ast-guard AST_RENAME_UNITS="<the planted units>"`: the real
recipe and its rename-extract prerequisite, over those units only (the copy
has no git tree, and a full extraction would cost minutes). A short global
must turn the scope guard red, a renamed identity of an ambiguous old name
the AST half of the old-name guard; the clean units stay green, and without
libclang the guard fails before any count. Run by `make ast-guard-test`,
part of `make test-all`: needs libclang and `make rename-tool-setup`.

LIBCLANG and AST_RENAME_BINDINGS are read from the environment the recipe
gives and passed to the child make."""

import itertools
import os
import re
import string
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import test_text_guards as text  # noqa: E402

REPO = text.REPO
WORK = os.path.join(REPO, "tmp", "test_ast_guard_work")
import rename_check  # noqa: E402
import rename_count  # noqa: E402
import rename_derive  # noqa: E402
import rename_tables  # noqa: E402

PLANET = "src/game/planet.c"
CLEAN_PLANET = "int planet_count(void) { return 0; }\n"
CLEAN_MAIN = "int main(void) { return 0; }\n"
SCOPE_EXCEPTIONS = "tools/scope_exceptions.txt"


def child_variables(units, libclang=None):
    found = ["AST_RENAME_UNITS=%s" % " ".join(units),
             "AST_RENAME_BINDINGS=%s" % os.environ.get(
                 "AST_RENAME_BINDINGS", os.path.join(REPO, "third_party", "clang-python"))]
    libclang = libclang or os.environ.get("LIBCLANG")
    if libclang:
        found.append("LIBCLANG=%s" % libclang)
    return found


class Plants:
    """The short global and the old identity, drawn from the table and the
    tools; each assumption checked, never skipped."""

    def __init__(self):
        _, rows = rename_tables.read_table(os.path.join(REPO, text.table_path()))
        names = rename_count.Names(rows)
        taken = ({r["ancien"] for r in rows} | {r["nouveau"] for r in rows}
                 | rename_check.C_KEYWORDS)
        pairs = ("".join(p) for p in itertools.product(string.ascii_lowercase, repeat=2))
        self.short_global = next((p for p in pairs if p not in taken), None)
        assert self.short_global, "every two-letter lowercase name is taken"
        excepted = rename_tables.read_triples(os.path.join(REPO, SCOPE_EXCEPTIONS))
        assert all(t[2] != self.short_global for t in excepted), \
            "%s is excepted in %s" % (self.short_global, SCOPE_EXCEPTIONS)

        identity = "identit\u00e9"
        static_functions = sorted(
            (r["ancien"], r["fichier"]) for r in rows
            if r["statut"] == text.RENAMED and r["cat\u00e9gorie"] == "identifiant"
            and r["sorte"] == "fonction" and r["ancien"] in names.ambiguous
            and r["fichier"].endswith(".c")
            and r[identity] == "c:%s@F@%s" % (os.path.basename(r["fichier"]), r["ancien"]))
        assert static_functions, \
            "no renamed static function of an ambiguous old name in the table"
        self.old_function, self.old_file = static_functions[0]
        long_ones = [f for f in static_functions if len(f[0]) >= 3]
        assert long_ones, "no such function of three characters or more"
        self.old_long_function, self.old_long_file = long_ones[0]
        for name, path in (static_functions[0], long_ones[0]):
            assert path != PLANET, "the old identity lives in %s" % PLANET
            assert re.fullmatch(r"[A-Za-z_]\w*", name)


PLANTS = Plants()


class AstGuard(unittest.TestCase):

    def setUp(self):
        text.copy_tree(WORK, extra=["tools/ast_rename"])

    def guard(self, units, libclang=None):
        for path, body in units.items():
            text.write(WORK, path, body)
        return text.run_make(WORK, "ast-guard", *child_variables(sorted(units), libclang))

    def halves(self, result):
        """(old-name AST half, scope guard) tables."""
        found = text.tables(result.stdout)
        self.assertEqual(len(found), 2, text.report(result))
        return found

    def test_green_on_clean_units(self):
        result = self.guard({PLANET: CLEAN_PLANET, "tests/test_planet.c": CLEAN_MAIN})
        self.assertEqual(result.returncode, 0, text.report(result))
        names, scope = self.halves(result)
        self.assertEqual(list(names), ["TOTAL"], text.report(result))
        self.assertFalse(any(names["TOTAL"].values()), text.report(result))
        self.assertEqual(list(scope), ["total"], text.report(result))
        self.assertFalse(any(scope["total"].values()), text.report(result))
        self.assertNotIn("warning:", result.stdout + result.stderr)

    def test_a_two_letter_global_turns_the_scope_guard_red(self):
        planet = "int %s;\nint planet_count(void) { return %s; }\n" % (
            PLANTS.short_global, PLANTS.short_global)
        result = self.guard({PLANET: planet, "tests/test_planet.c": CLEAN_MAIN})
        self.assertNotEqual(result.returncode, 0, text.report(result))
        names, scope = self.halves(result)
        self.assertFalse(any(names["TOTAL"].values()), text.report(result))
        self.assertEqual(sorted(scope), ["game/planet", "total"], text.report(result))
        for column, value in scope["game/planet"].items():
            expected = 1 if column in ("global", "total") else 0
            self.assertEqual(value, expected, "%s\n%s" % (column, text.report(result)))

    def old_identity(self, name, path):
        plant = "static void %s(void) {}\nint main(void) { %s(); return 0; }\n" % (name, name)
        result = self.guard({PLANET: CLEAN_PLANET, path: plant})
        self.assertNotEqual(result.returncode, 0, text.report(result))
        names, scope = self.halves(result)
        module = rename_derive.module_of(path)
        self.assertEqual(sorted(names), sorted([module, "TOTAL"]), text.report(result))
        self.assertGreaterEqual(names[module]["identifier"], 1, text.report(result))
        return scope

    def test_an_old_identity_turns_the_ast_half_red(self):
        self.old_identity(PLANTS.old_function, PLANTS.old_file)

    def test_an_old_identity_alone_turns_the_guard_red(self):
        """Three characters or more: the scope guard is silent, the red
        comes from the old-name half alone."""
        scope = self.old_identity(PLANTS.old_long_function, PLANTS.old_long_file)
        self.assertEqual(list(scope), ["total"])
        self.assertFalse(any(scope["total"].values()))

    def test_without_libclang_it_fails_before_any_count(self):
        result = self.guard({PLANET: CLEAN_PLANET, "tests/test_planet.c": CLEAN_MAIN},
                            libclang="/nonexistent")
        self.assertNotEqual(result.returncode, 0, text.report(result))
        self.assertIn("libclang not found", result.stderr, text.report(result))
        self.assertEqual(text.tables(result.stdout), [], text.report(result))


if __name__ == "__main__":
    unittest.main()
