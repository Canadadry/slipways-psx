"""tools/rename_check.py, black box: a mini extraction, a mini table and a
mini tree written to tmp/, then the violations it prints and its exit code.
Run by `make rename-check-test`, part of `make test` (no libclang needed)."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "rename_check.py")
WORK = os.path.join(REPO, "tmp", "test_rename_check_work")

COLUMNS = ["ancien", "nouveau", "fichier", "catégorie", "sorte", "statut",
           "module", "concept", "note", "identité"]
DECL_HEADER = "identity\tkind\tname\tparent\tfile\tline\tlinkage\tscope_lines\toccurrences\tflags\n"
KIND_LABELS = {
    "function": "fonction", "field": "champ", "enum_constant": "constante",
    "macro": "macro", "static": "statique", "global": "globale",
    "param": "paramètre", "local": "locale", "type": "type", "tag": "tag",
}


class Decl:
    """One extracted declaration and the table row that names it."""

    def __init__(self, identity, kind, name, new, file, parent="-", linkage="-",
                 line=1, scope="-"):
        self.fields = [identity, kind, name, parent, file, str(line), linkage, str(scope), "1", "-"]
        status = "conservé" if new == name else "renommé"
        self.row = [name, new, file, "identifiant", KIND_LABELS[kind], status,
                    os.path.splitext(file)[0].replace("src/", "", 1), "", "", identity]


def other(old, new, file, category="chaîne", kind="clé", note="lecteur", identity=None):
    status = "conservé" if old == new else "renommé"
    return [old, new, file, category, kind, status, "m", "", note,
            identity or "%s:%s" % (category, old)]


def run(decls, extra_rows=(), drop_rows=(), remainder=(), tree=None, header="# Statut : à valider\n"):
    """Writes the inputs, runs the check; returns the completed process."""
    if os.path.isdir(WORK):
        shutil.rmtree(WORK)
    root = os.path.join(WORK, "root")
    os.makedirs(root)
    for path, text in (tree or {}).items():
        full = os.path.join(root, path)
        os.makedirs(os.path.dirname(full), exist_ok=True)
        with open(full, "w") as f:
            f.write(text)
    paths = {name: os.path.join(WORK, name) for name in ("decls.tsv", "table.tsv", "remainder.tsv")}
    with open(paths["decls.tsv"], "w") as f:
        f.write("# partial\tsome/unit.c\tdefault\tignored\n" + DECL_HEADER)
        for d in decls:
            f.write("\t".join(d.fields) + "\n")
    with open(paths["table.tsv"], "w") as f:
        f.write(header + "\t".join(COLUMNS) + "\n")
        for d in decls:
            if d.fields[0] not in drop_rows:
                f.write("\t".join(d.row) + "\n")
        for row in extra_rows:
            f.write("\t".join(row) + "\n")
    with open(paths["remainder.tsv"], "w") as f:
        f.write("\t".join(COLUMNS) + "\n")
        for row in remainder:
            f.write("\t".join(row) + "\n")
    return subprocess.run(
        [sys.executable, SCRIPT, "--table", paths["table.tsv"], "--declarations", paths["decls.tsv"],
         "--remainder", paths["remainder.tsv"], "--root", root],
        capture_output=True, text=True)


def fn(name, new, file="src/game/planet.h", linkage="extern"):
    return Decl("c:@F@" + name + "@" + file, "function", name, new, file, linkage=linkage)


class Check(unittest.TestCase):

    def assertPasses(self, result):
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def assertFails(self, result, *fragments):
        self.assertNotEqual(result.returncode, 0, result.stdout)
        for fragment in fragments:
            self.assertIn(fragment, result.stdout)

    def test_a_sound_table_passes(self):
        self.assertPasses(run([
            fn("pico8_jo_kj", "planet_draw"),
            Decl("c:@SA@pico8_jo_t@FI@p", "field", "p", "map_pos", "src/game/planet.h", parent="pico8_jo_t"),
            Decl("c:a.c@1@F@f@i", "local", "i", "i", "src/game/planet.c", parent="f", line=3, scope=4),
        ], extra_rows=[other("jo", "planet", "src/game/planet.c")]))

    # --- exhaustiveness ----------------------------------------------------

    def test_extracted_identity_without_a_row_fails(self):
        result = run([fn("pico8_jo_kj", "planet_draw"), fn("pico8_jo_ev", "planet_draw_overlay")],
                     drop_rows=["c:@F@pico8_jo_ev@src/game/planet.h"])
        self.assertFails(result, "c:@F@pico8_jo_ev@src/game/planet.h")

    def test_row_without_an_extracted_identity_fails(self):
        gone = fn("pico8_gone", "gone")
        result = run([fn("pico8_jo_kj", "planet_draw")], extra_rows=[gone.row])
        self.assertFails(result, "c:@F@pico8_gone@src/game/planet.h")

    def test_empty_new_name_and_nonempty_remainder_fail(self):
        result = run([fn("pico8_zz", "")], remainder=[fn("pico8_zz", "").row])
        self.assertFails(result, "pico8_zz", "remainder")

    def test_missing_status_marker_fails(self):
        self.assertFails(run([fn("pico8_jo_kj", "planet_draw")], header=""), "Statut")

    # --- collisions --------------------------------------------------------

    def test_two_identities_to_one_global_name_fail(self):
        result = run([fn("pico8_jo_kj", "planet_draw"),
                      fn("pico8_jo_draw", "planet_draw", file="src/game/planet_display.h")])
        self.assertFails(result, "planet_draw", "pico8_jo_kj", "pico8_jo_draw")

    def test_same_static_name_in_two_files_passes_in_one_file_fails(self):
        self.assertPasses(run([fn("pico8_a_measure", "measure", "src/game/a.c", "static"),
                               fn("pico8_b_measure", "measure", "src/game/b.c", "static")]))
        self.assertFails(run([fn("pico8_a_measure", "measure", "src/game/a.c", "static"),
                              Decl("c:a.c@g", "static", "g_a", "measure", "src/game/a.c",
                                   linkage="static")]), "measure")

    def test_static_name_meeting_an_exported_one_fails(self):
        self.assertFails(run([fn("pico8_a_measure", "measure", "src/game/a.c", "static"),
                              fn("pico8_measure_all", "measure")]), "measure")

    def test_fields_collide_only_within_their_struct(self):
        self.assertPasses(run([
            Decl("c:@SA@a_t@FI@p", "field", "p", "map_pos", "src/game/a.h", parent="a_t"),
            Decl("c:@SA@b_t@FI@mp", "field", "mp", "map_pos", "src/game/b.h", parent="b_t"),
            Decl("c:a.c@S@a.c@60@FI@px", "field", "px", "name", "src/game/a.c", parent="(anonymous)"),
            Decl("c:b.c@S@b.c@90@FI@name", "field", "name", "name", "tests/b.c", parent="(anonymous)"),
        ]))
        self.assertFails(run([
            Decl("c:@SA@a_t@FI@p", "field", "p", "map_pos", "src/game/a.h", parent="a_t"),
            Decl("c:@SA@a_t@FI@mp", "field", "mp", "map_pos", "src/game/a.h", parent="a_t"),
        ]), "map_pos")

    def test_locals_collide_only_where_their_scopes_overlap(self):
        self.assertPasses(run([
            Decl("c:a.c@1@F@f@jo", "local", "jo", "planet", "src/game/a.c", parent="f", line=10, scope=5),
            Decl("c:a.c@2@F@f@pl", "local", "pl", "planet", "src/game/a.c", parent="f", line=20, scope=5),
        ]))
        self.assertFails(run([
            Decl("c:a.c@1@F@f@jo", "local", "jo", "planet", "src/game/a.c", parent="f", line=10, scope=20),
            Decl("c:a.c@2@F@f@pl", "param", "pl", "planet", "src/game/a.c", parent="f", line=8, scope=30),
        ]), "planet")

    def test_names_equal_before_the_rename_are_not_a_new_collision(self):
        self.assertPasses(run([
            Decl("c:a.c@1@F@f@i", "local", "i", "i", "src/game/a.c", parent="f", line=10, scope=20),
            Decl("c:a.c@2@F@f@i", "local", "i", "i", "src/game/a.c", parent="f", line=12, scope=2),
        ]))

    def test_local_shadowing_a_global_fails(self):
        result = run([
            fn("pico8_jo", "planet"),
            Decl("c:a.c@1@F@f@jo", "local", "jo", "planet", "src/game/a.c", parent="f", line=10, scope=50),
        ])
        self.assertFails(result, "planet", "shadow")

    def test_macro_meeting_a_field_fails(self):
        self.assertFails(run([
            Decl("macro:src/game/a.h:PICO8_X", "macro", "PICO8_X", "x", "src/game/a.h"),
            Decl("c:@SA@a_t@FI@xx", "field", "xx", "x", "src/game/a.h", parent="a_t"),
        ]), "PICO8_X")

    def test_c_keywords_and_library_names_fail_but_class_is_allowed(self):
        self.assertFails(run([Decl("c:@SA@a_t@FI@df", "field", "df", "default", "src/game/a.h",
                                   parent="a_t")]), "default")
        self.assertFails(run([fn("pico8_free", "free")]), "free")
        self.assertPasses(run([Decl("c:@SA@a_t@FI@cls", "field", "cls", "class", "src/engine/a.h",
                                    parent="a_t")]))

    # --- strings and files -------------------------------------------------

    def test_one_string_name_must_have_one_new_name(self):
        self.assertFails(run([], extra_rows=[other("jo", "planet", "src/a.c"),
                                             other("jo", "body", "src/b.c")]), "chaîne:jo")

    def test_two_keys_of_one_kind_to_one_new_name_fail(self):
        self.assertFails(run([], extra_rows=[other("jo", "planet", "src/a.c", kind="clé d'étiquette"),
                                             other("pl", "planet", "src/b.c", kind="clé d'étiquette")]),
                         "planet")
        self.assertPasses(run([], extra_rows=[other("jo", "planet", "src/a.c", kind="clé d'étiquette"),
                                              other("pj", "planet", "src/b.c", kind="nom de classe")]))

    def test_two_files_to_one_target_or_onto_an_existing_name_fail(self):
        self.assertFails(run([], extra_rows=[
            other("tmp/pico8_a", "tmp/a", "Makefile", "fichier", "binaire"),
            other("tmp/main_a", "tmp/a", "Makefile", "fichier", "binaire")]), "tmp/a")
        self.assertFails(run([], extra_rows=[
            other("tmp/pico8_frames", "tmp/frames", "Makefile", "fichier", "binaire")],
            tree={"Makefile": "FRAMES_DIR := $(TMP_DIR)/frames\n"}), "tmp/frames")
        self.assertFails(run([], extra_rows=[
            other("tools/pico8_probe.p8", "tools/probe.p8", "tools/x.sh", "fichier", "fixture")],
            tree={"tools/probe.p8": ""}), "tools/probe.p8")

    # --- form --------------------------------------------------------------

    def test_section_4_form_is_a_warning_only(self):
        result = run([Decl("c:@T@pico8_jo_t", "type", "pico8_jo_t", "planet", "src/game/planet.h"),
                      Decl("macro:src/game/a.h:PICO8_A", "macro", "PICO8_A", "p8_a", "src/game/a.h")])
        self.assertPasses(result)
        self.assertIn("warning", result.stdout)
        self.assertIn("planet", result.stdout)
        self.assertIn("p8_a", result.stdout)


if __name__ == "__main__":
    unittest.main()
