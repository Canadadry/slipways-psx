"""tools/cart_citations.py, black box: a mini docs/renommage.tsv, a mini
sub-step table and a mini source tree, then the sites and per-module counts
it prints. Run by `make cart-citations-test`, part of `make test`."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "cart_citations.py")
WORK = os.path.join(REPO, "tmp", "test_cart_citations_work")

COLUMNS = ["ancien", "nouveau", "fichier", "catégorie", "sorte", "statut",
           "module", "concept", "note", "identité"]
ROWS = [
    ["jo", "planet", "src/game/planet.h", "identifiant", "type", "renommé", "game/planet",
     "", "", "c:@T@jo"],
    ["p", "map_pos", "src/engine/widget.h", "identifiant", "champ", "renommé", "engine/widget",
     "", "", "c:@S@w@FI@p"],
    ["at", "anchor", "src/engine/widget.h", "identifiant", "champ", "renommé", "engine/widget",
     "", "", "c:@S@w@FI@at"],
    ["le", "ephemerals", "src/engine/widget.h", "identifiant", "champ", "renommé",
     "engine/widget", "", "", "c:@S@w@FI@le"],
    ["s", "sprite", "src/engine/widget.h", "identifiant", "champ", "renommé", "engine/widget",
     "", "", "c:@S@w@FI@s"],
    ["e", "entity", "src/engine/widget.h", "identifiant", "param", "renommé", "engine/widget",
     "", "", "c:w.h@e"],
    ["t", "state_time", "src/engine/widget.h", "identifiant", "champ", "renommé",
     "engine/widget", "", "", "c:@S@w@FI@t"],
    ["kv", "kv", "src/engine/widget.h", "identifiant", "param", "conservé", "engine/widget",
     "", "", "c:w.h@kv"],
    ["pico8_jo", "planet_long", "src/game/planet.h", "identifiant", "fonction", "renommé",
     "game/planet", "", "", "c:@F@pico8_jo"],
]
SUBSTEPS = ("substep\tbound\tmodules\n"
            "3a\t0\tengine/widget\n"
            "3b\t0\tgame/planet game/cart tools/gadget\n")


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def run(files, extra=()):
    shutil.rmtree(WORK, ignore_errors=True)
    root = os.path.join(WORK, "tree")
    for path, text in files.items():
        write(os.path.join(root, path), text)
    table = os.path.join(WORK, "renommage.tsv")
    write(table, "# Statut : validé\n" + "\t".join(COLUMNS) + "\n"
          + "".join("\t".join(r) + "\n" for r in ROWS))
    write(os.path.join(WORK, "substeps.tsv"), SUBSTEPS)
    return subprocess.run(
        [sys.executable, SCRIPT, "--table", table, "--root", root,
         "--substeps", os.path.join(WORK, "substeps.tsv")] + list(extra),
        capture_output=True, text=True)


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


class Citations(unittest.TestCase):

    def test_cart_is_counted_outside_the_cart_module_and_never_fails(self):
        result = run({
            "src/game/planet.c": "/* as the Cart does, then cart_load(); the cart's way */\n",
            "src/game/cart.c": "/* the cart */\n",
        })
        self.assertEqual(result.returncode, 0, result.stderr)
        table = counts(result)
        self.assertEqual(table["game/planet"], {"cart": 2, "lua": 0, "short": 0, "total": 2},
                         result.stdout)
        self.assertNotIn("game/cart", table, result.stdout)
        self.assertIn("src/game/planet.c:1 cart Cart", result.stdout)

    def test_lua_and_lua_lua_are_counted(self):
        result = run({"src/game/planet.c": "/* Lua's truth, see pico8/lua.lua; lua_State */\n"})
        self.assertEqual(counts(result)["game/planet"]["lua"], 2, result.stdout)

    def test_a_short_old_name_is_counted_bare_in_comments_and_messages_only(self):
        result = run({"src/engine/widget.c": (
            "/* jo and e->p, but not `jo` nor `p`; it's a jo's p */\n"
            "static int jo(int p) { return p; }\n"
            "static const char *k = \"jo p\";\n"
            "void f(void) { assert(k && \"jo is set\"); }\n"
            "/* kv is kept, pico8_jo is long, le stays French and counted */\n")})
        self.assertEqual(counts(result)["engine/widget"],
                         {"cart": 0, "lua": 0, "short": 6, "total": 6}, result.stdout)

    def test_extensions_compounds_and_word_pieces_are_not_names(self):
        result = run({"src/engine/widget.c": "/* see p.c and p.h; a jo-like re-p; dès; i.e. e.g. a\\tb */\n"
                                             "/* but w.le and w.p are */\n"})
        self.assertEqual(counts(result)["engine/widget"]["short"], 2, result.stdout)
        self.assertIn("src/engine/widget.c:2 short le", result.stdout)

    def test_english_homographs_are_not_counted(self):
        result = run({"src/engine/widget.c": "/* look at it */\n"})
        self.assertNotIn("engine/widget", counts(result), result.stdout)

    def test_script_comments_are_read_and_substep_selects(self):
        files = {"tools/gadget.sh": "# the cart\necho cart\n",
                 "src/engine/widget.c": "/* the cart */\n"}
        result = run(files)
        self.assertEqual(counts(result)["tools/gadget"]["cart"], 1, result.stdout)
        result = run(files, ["--substep", "3a"])
        self.assertEqual(sorted(counts(result)), ["TOTAL", "engine/widget"], result.stdout)
        result = run(files, ["--module", "tools/*"])
        self.assertEqual(sorted(counts(result)), ["TOTAL", "tools/gadget"], result.stdout)


class Exceptions(unittest.TestCase):
    """--exceptions: each `column<TAB>module<TAB>token` line removes exactly
    the citations of that token, any case, in that column and module; the
    file itself is not read."""

    FILES = {
        "Makefile": "# the cart, the Lua of lua.lua\n",
        "tools/gadget.sh": "# the cart and Lua\n",
        "src/engine/widget.c": "/* jo and p */\n",
    }
    LINES = ("# column\tmodule\ttoken\n"
             "cart\tMakefile\tcart\n"
             "lua\tMakefile\tlua\n"
             "lua\ttools/gadget\tcart\n"
             "short\tengine/widget\tjo\n")

    def run_with(self, lines, extra=()):
        files = dict(self.FILES, **{"tools/citations_exceptions.txt": "# the cart\n" + lines})
        return run(files, ["--exceptions",
                           os.path.join(WORK, "tree", "tools", "citations_exceptions.txt")]
                   + list(extra))

    def test_a_line_removes_its_triple_and_only_it_in_any_case(self):
        result = self.run_with(self.LINES)
        self.assertEqual(result.returncode, 0, result.stderr)
        table = counts(result)
        self.assertEqual(table["Makefile"], {"cart": 0, "lua": 1, "short": 0, "total": 1},
                         result.stdout)
        self.assertIn("Makefile:1 lua lua.lua", result.stdout)
        self.assertNotIn("Makefile:1 lua Lua", result.stdout)
        self.assertEqual(table["tools/gadget"], {"cart": 1, "lua": 1, "short": 0, "total": 2},
                         result.stdout)
        self.assertEqual(table["engine/widget"], {"cart": 0, "lua": 0, "short": 1, "total": 1},
                         result.stdout)

    def test_the_exceptions_file_is_not_read(self):
        result = self.run_with(self.LINES)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn("tools/citations_exceptions", counts(result), result.stdout)
        result = run(dict(self.FILES, **{"tools/citations_exceptions.txt": "# the cart\n"}))
        self.assertEqual(counts(result)["tools/citations_exceptions"]["cart"], 1, result.stdout)

    def test_a_line_that_removed_nothing_is_a_warning_after_the_table(self):
        result = self.run_with(self.LINES)
        self.assertEqual(result.returncode, 0, result.stderr)
        lines = result.stdout.splitlines()
        dead = [i for i, line in enumerate(lines) if "removed nothing" in line]
        self.assertEqual(len(dead), 1, result.stdout)
        self.assertTrue(lines[dead[0]].startswith("warning: "), result.stdout)
        self.assertIn("citations_exceptions.txt:5: removed nothing: lua tools/gadget cart",
                      lines[dead[0]])
        total = [i for i, line in enumerate(lines) if line.startswith("TOTAL")]
        self.assertLess(total[0], dead[0], result.stdout)
        result = self.run_with(self.LINES, ["--module", "engine/widget"])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn("removed nothing", result.stdout)

    def test_without_lines_the_output_is_unchanged(self):
        before = run(self.FILES).stdout
        empty = os.path.join(WORK, "..", "test_cart_citations_comments_only.txt")
        write(empty, "# column\tmodule\ttoken\n")
        self.assertEqual(run(self.FILES, ["--exceptions", empty]).stdout, before)
        self.assertEqual(counts(run(self.FILES))["TOTAL"],
                         {"cart": 2, "lua": 3, "short": 2, "total": 7}, before)


if __name__ == "__main__":
    unittest.main()
