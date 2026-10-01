"""tools/rename_strings.py, black box: a mini source tree, a mini table of
identifier rows, a mini vocabulary and overrides written to tmp/, then the
`chaîne` and `fichier` rows it adds to the table, its flags and its exit
code. Run by `make rename-strings-test`, part of `make test` (no libclang
needed)."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "rename_strings.py")
WORK = os.path.join(REPO, "tmp", "test_rename_strings_work")

COLUMNS = ["ancien", "nouveau", "fichier", "catégorie", "sorte", "statut",
           "module", "concept", "note", "identité"]

VOCABULARY = """# Vocabulary

Statut : validé

## Concepts

### Core

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `jo` | planet | `planet` | x | |
| `mo` | camera singleton | `main_camera` | x | |
| `qy` (`pico8_qy_t`) | particle | `particle_t` / `particles` | x | |

## Vtable

| membre actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `kj` | draw | `draw` | x | |

## Arborescence

### `tests/` (2)

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `tests/test_pico8_jo.c` | `tests/` | `test_planet.c` |  |
| `tests/test_pico8_zone_choisie.c` | `tests/` | `test_allocator.c` |  |
"""


def ident(old, new, file, kind="constante", identity=None, concept=""):
    status = "conservé" if old == new else "renommé"
    module = os.path.splitext(file)[0].replace("src/", "", 1)
    return [old, new, file, "identifiant", kind, status, module, concept, "",
            identity or "c:@%s" % old]


BASE_IDENTIFIERS = [
    ident("PICO8_TAG_JO", "TAG_PLANET", "src/engine/entity_class.h", identity="c:@E@t@PICO8_TAG_JO"),
    ident("PICO8_TAG_QH", "TAG_STRUCTURE", "src/engine/entity_class.h", identity="c:@E@t@PICO8_TAG_QH"),
    ident("PICO8_STATE_PH", "ENTITY_STATE_NORMAL", "src/engine/entity_class.h",
          identity="c:@E@s@PICO8_STATE_PH"),
    ident("PICO8_STATE_LY", "ENTITY_STATE_SETTLED", "src/engine/entity_class.h",
          identity="c:@E@s@PICO8_STATE_LY"),
    ident("kj", "draw", "src/engine/entity_class.h", kind="champ",
          identity="c:@S@pico8_struct_class@FI@kj"),
    ident("pico8_jo_struct_class", "planet_class", "src/game/planet.c", kind="fonction",
          identity="c:@F@pico8_jo_struct_class"),
    ident("pico8_cg", "entities_by_tag", "src/engine/entity_registry.c", kind="fonction",
          identity="c:@F@pico8_cg"),
]

ENTITY_CLASS_H = """static const char *const states[] = {
    [PICO8_STATE_PH] = "ph", [PICO8_STATE_LY] = "ly",
};
static const char *const names[] = {
    [PICO8_TAG_QH] = "qh", [PICO8_TAG_JO] = "jo",
};
"""


def run(tree, identifiers=None, overrides="", vocabulary=VOCABULARY, extra_rows=()):
    """Writes the tree and the inputs, runs the script; returns (result,
    rows other than identifiers, flags text, table lines)."""
    if os.path.isdir(WORK):
        shutil.rmtree(WORK)
    root = os.path.join(WORK, "root")
    for path, text in tree.items():
        full = os.path.join(root, path)
        os.makedirs(os.path.dirname(full), exist_ok=True)
        with open(full, "w") as f:
            f.write(text)
    paths = {name: os.path.join(WORK, name)
             for name in ("table.tsv", "vocab.md", "overrides.tsv", "flags.tsv")}
    rows = (BASE_IDENTIFIERS if identifiers is None else identifiers) + list(extra_rows)
    with open(paths["table.tsv"], "w") as f:
        f.write("# Statut : à valider\n" + "\t".join(COLUMNS) + "\n")
        for row in rows:
            f.write("\t".join(row) + "\n")
    with open(paths["vocab.md"], "w") as f:
        f.write(vocabulary)
    with open(paths["overrides.tsv"], "w") as f:
        f.write("kind\tkey\tnew\tconcept\n" + overrides)
    result = subprocess.run(
        [sys.executable, SCRIPT, "--root", root, "--table", paths["table.tsv"],
         "--vocabulary", paths["vocab.md"], "--overrides", paths["overrides.tsv"],
         "--flags", paths["flags.tsv"]],
        capture_output=True, text=True)
    with open(paths["table.tsv"]) as f:
        lines = f.read().splitlines()
    data = [l for l in lines if not l.startswith("#")]
    others = [dict(zip(COLUMNS, l.split("\t"))) for l in data[1:]]
    others = [r for r in others if r["catégorie"] != "identifiant"]
    flags = ""
    if os.path.exists(paths["flags.tsv"]):
        with open(paths["flags.tsv"]) as f:
            flags = f.read()
    return result, others, flags, lines


def sites(rows, old):
    """{file: note} of the rows of one old text."""
    return {r["fichier"]: r["note"] for r in rows if r["ancien"] == old}


def only(rows, old, column):
    values = {r[column] for r in rows if r["ancien"] == old}
    assert len(values) == 1, (old, column, values)
    return values.pop()


class StringKeys(unittest.TestCase):

    def test_tag_key_takes_its_tag_constant_name_with_writer_and_reader(self):
        result, rows, flags, _ = run({
            "src/engine/entity_class.h": ENTITY_CLASS_H,
            "src/game/economy.c": 'void f(void) { pico8_cg("jo"); }\n',
        })
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(only(rows, "jo", "nouveau"), "planet")
        self.assertEqual(only(rows, "jo", "catégorie"), "chaîne")
        self.assertEqual(only(rows, "jo", "statut"), "renommé")
        self.assertEqual(sites(rows, "jo"), {"src/engine/entity_class.h": "écrivain",
                                             "src/game/economy.c": "lecteur"})
        self.assertEqual({r["module"] for r in rows if r["ancien"] == "jo"},
                         {"engine/entity_class", "game/economy"})
        self.assertNotIn("\tjo\t", flags)

    def test_state_names_setters_write_and_comparisons_read(self):
        result, rows, flags, _ = run({
            "src/engine/entity_class.h": ENTITY_CLASS_H,
            "src/game/game_data.c": 'static const def_t d = {.me = "ph"};\n',
            "src/game/probe.c": 'void f(e_t *e) { pico8_ke_ks(e, "ly"); if (strcmp(e->me, "ph")) g(); }\n',
            "tests/test_probe.c": 'int main(void) { assert(strcmp(m, "ly") == 0 && "not \\"ly\\""); }\n',
        })
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(only(rows, "ph", "nouveau"), "normal")
        self.assertEqual(only(rows, "ly", "nouveau"), "settled")
        self.assertEqual(only(rows, "ph", "sorte"), "nom d'état")
        self.assertEqual(sites(rows, "ph"), {"src/engine/entity_class.h": "écrivain",
                                             "src/game/game_data.c": "écrivain",
                                             "src/game/probe.c": "lecteur"})
        self.assertEqual(sites(rows, "ly"), {"src/engine/entity_class.h": "écrivain",
                                             "src/game/probe.c": "écrivain",
                                             "tests/test_probe.c": "lecteur"})

    def test_message_and_class_keys_and_a_key_with_two_meanings(self):
        result, rows, flags, _ = run({
            "src/engine/entity_class.h": ENTITY_CLASS_H,
            "src/engine/entity_registry.c": 'static const char *const g_bm_names[1] = {"kj"};\n',
            "src/game/cart.c": 'void f(void) { pico8_dc("kj"); }\n',
            "src/game/planet.c": 'void g(void) { pico8_class_coverage_record("jo"); }\n',
            "tools/class_coverage_net_expected.txt": "# comment jo\njo  12  3/26\nmo  0  0/26\ncouverture: OK\n",
            "src/cmd/class_coverage.c": 'static const char *const KNOWN[] = {"jo", "mo"};\n',
        })
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(only(rows, "kj", "nouveau"), "draw")
        self.assertEqual(only(rows, "kj", "sorte"), "clé de message")
        self.assertEqual(sites(rows, "kj"), {"src/engine/entity_registry.c": "écrivain",
                                             "src/game/cart.c": "lecteur"})
        self.assertEqual(only(rows, "jo", "nouveau"), "planet")
        self.assertEqual(only(rows, "jo", "sorte"), "clé d'étiquette + nom de classe")
        self.assertEqual(sites(rows, "jo"), {"src/engine/entity_class.h": "écrivain",
                                             "src/game/planet.c": "écrivain",
                                             "src/cmd/class_coverage.c": "écrivain",
                                             "tools/class_coverage_net_expected.txt": "lecteur"})
        self.assertEqual(only(rows, "jo", "concept"), "jo")
        self.assertEqual(only(rows, "mo", "sorte"), "nom de classe + clé")
        self.assertEqual(sites(rows, "mo"), {"src/cmd/class_coverage.c": "écrivain",
                                             "tools/class_coverage_net_expected.txt": "lecteur"})
        self.assertNotIn("couverture", [r["ancien"] for r in rows])

    def test_other_cart_names_in_strings_translate_through_the_vocabulary(self):
        result, rows, flags, _ = run({
            "src/cmd/replay.c": 'static const char *const N[2] = {"qy", "e"};\n'
                                'void f(void) { puts("jo is here"); }\n',
            "tests/test_live_slots.c": 'static const char *const N[] = {"qy", "mo"};\n',
        })
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(only(rows, "qy", "nouveau"), "particles")
        self.assertEqual(only(rows, "qy", "sorte"), "clé")
        self.assertEqual(only(rows, "qy", "concept"), "qy")
        self.assertEqual(sites(rows, "qy"), {"src/cmd/replay.c": "écrivain",
                                             "tests/test_live_slots.c": "lecteur"})
        self.assertEqual(only(rows, "mo", "nouveau"), "main_camera")
        self.assertEqual(sites(rows, "e"), {})
        self.assertEqual(sites(rows, "jo is here"), {})

    def test_string_override_wins_and_disagreeing_sources_are_flagged(self):
        tree = {
            "src/engine/entity_class.h": ENTITY_CLASS_H,
            "src/game/planet.c": 'void g(void) { pico8_class_coverage_record("jo"); pico8_cg("jo"); }\n',
        }
        identifiers = [r for r in BASE_IDENTIFIERS if r[0] != "pico8_jo_struct_class"]
        identifiers.append(ident("pico8_jo_struct_class", "body_class", "src/game/planet.c",
                                 kind="fonction", identity="c:@F@pico8_jo_struct_class"))
        result, rows, flags, _ = run(tree, identifiers)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(only(rows, "jo", "nouveau"), "")
        self.assertIn("chaîne:jo\t", flags)
        self.assertIn("body (", flags)
        result, rows, flags, _ = run(tree, identifiers, overrides="string\tjo\tplanet\tjo\n")
        self.assertEqual(only(rows, "jo", "nouveau"), "planet")
        self.assertNotIn("chaîne:jo\t", flags)

    def test_labels_hashed_labels_and_texts_that_are_not_names(self):
        result, rows, flags, _ = run({
            "tests/test_ui.c": 'void t(void) {\n'
                               '    check("qy", h, QY_HASH);\n'
                               '    h = fnv(fnv_str(h, "mo"), 1u);\n'
                               '    pico8_print(&gfx, &mem, "jo", 0, 1, 2);\n'
                               '    write_file(P, "qy_" "pose(self);\\n");\n'
                               '    FILE *f = fopen(path, "mo");\n'
                               '}\n'
                               'static const char *f(void) { return "jo"; }\n',
        })
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(sites(rows, "qy"), {"tests/test_ui.c": "libellé"})
        self.assertEqual(sites(rows, "mo"), {"tests/test_ui.c": "libellé haché"})
        self.assertEqual(sites(rows, "jo"), {})
        self.assertEqual(sites(rows, "qy_"), {})
        self.assertEqual(flags.count("\n"), 1)

    def test_case_table_of_a_test_writes_its_case_names(self):
        result, rows, flags, _ = run({
            "tests/test_game_zone.c": 'static const case_t CASES[] = {\n'
                                      '    {"mo_survives", case_mo_survives},\n'
                                      '};\n'
                                      'void t(void) { expect_clean("mo_survives"); }\n',
        })
        self.assertEqual(only(rows, "mo_survives", "nouveau"), "main_camera_survives")
        self.assertEqual(sites(rows, "mo_survives"), {"tests/test_game_zone.c": "écrivain, lecteur"})

    def test_class_names_come_from_src_and_tag_families_keep_trailing_underscores(self):
        identifiers = BASE_IDENTIFIERS + [
            ident("PICO8_TAG_G_", "TAG_PROBE", "src/engine/entity_class.h", identity="c:@E@t@PICO8_TAG_G_"),
            ident("pico8_g__struct_class", "probe_class", "src/game/probe.c", kind="fonction",
                  identity="c:@F@pico8_g__struct_class")]
        result, rows, flags, _ = run({
            "src/engine/entity_class.h": ENTITY_CLASS_H.replace(
                '[PICO8_TAG_JO] = "jo",', '[PICO8_TAG_JO] = "jo", [PICO8_TAG_G_] = "g_",'),
            "src/game/probe.c": 'void g(void) { pico8_class_coverage_record("g_"); }\n',
            "tests/test_class_coverage.c": 'void t(void) { pico8_class_coverage_record("aa"); }\n',
        }, identifiers)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(only(rows, "g_", "nouveau"), "probe")
        self.assertEqual(sites(rows, "aa"), {})

    def test_test_names_and_their_tmp_paths_follow_the_renamed_test_file(self):
        result, rows, flags, _ = run({
            "tests/test_allocator.c": '#define LOG "tmp/test_pico8_zone_choisie_stderr.txt"\n'
                                      'int main(void) {\n'
                                      '    printf("test_pico8_zone_choisie: cas inconnu %s\\n", c);\n'
                                      '    printf("test_pico8_zone_choisie: all tests passed\\n");\n'
                                      '    printf("test_pico8_gone: ok\\n");\n'
                                      '}\n',
        })
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(only(rows, "test_pico8_zone_choisie", "nouveau"), "test_allocator")
        self.assertEqual(only(rows, "test_pico8_zone_choisie", "sorte"), "nom de test")
        self.assertEqual(sites(rows, "test_pico8_zone_choisie"), {"tests/test_allocator.c": "libellé"})
        path = "tmp/test_pico8_zone_choisie_stderr.txt"
        self.assertEqual(only(rows, path, "nouveau"), "tmp/test_allocator_stderr.txt")
        self.assertEqual(only(rows, path, "sorte"), "chemin tmp")
        self.assertEqual(sites(rows, path), {"tests/test_allocator.c": "écrivain, lecteur"})
        self.assertEqual(only(rows, "test_pico8_gone", "nouveau"), "")
        self.assertIn("chaîne:test_pico8_gone\t", flags)

    def test_identifiers_cited_by_tools_makefile_and_strings(self):
        identifiers = BASE_IDENTIFIERS + [
            ident("pico8_dc", "draw_layers", "src/engine/entity_registry.c", kind="fonction",
                  identity="c:@F@pico8_dc"),
            ident("pico8_jo_t", "planet_t", "src/game/planet.h", kind="type", identity="c:@T@pico8_jo_t"),
            ident("jy", "needs", "src/game/planet.h", kind="champ", identity="c:@SA@pico8_jo_t@FI@jy"),
            ident("PICO8_CLASS_COVERAGE", "CLASS_COVERAGE", "src/devtools/class_coverage.h",
                  kind="macro", identity="macro:src/devtools/class_coverage.h:PICO8_CLASS_COVERAGE"),
            ident("pico8_draw", "planet_draw", "src/game/planet.c", kind="fonction",
                  identity="c:planet.c@F@pico8_draw"),
            ident("pico8_draw", "toast_draw", "src/game/message_box.c", kind="fonction",
                  identity="c:message_box.c@F@pico8_draw"),
            ident("jo_list", "planet_list", "src/game/economy.c", kind="locale",
                  identity="c:economy.c@12@F@f@jo_list"),
        ]
        result, rows, flags, _ = run({
            "tools/composition_residue_sites.txt": "# pico8_jo_t\npico8_dc\tsrc/engine/entity_registry.c\n",
            "tools/composition_residue.sh": "# pico8_dc in a comment\nPTRS=\"pico8_jo_t.jy jo_list\"\n",
            "Makefile": "COVERAGE_CFLAGS := -DPICO8_CLASS_COVERAGE\n",
            "tests/test_composition_residue.c": 'void t(void) { w(P, "pico8_draw(x);\\n"); }\n',
            "src/game/cart.c": 'void f(void) { assert(x && "see "\n  "pico8_dc and pico8_jo_t"); }\n',
        }, identifiers)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(only(rows, "pico8_dc", "nouveau"), "draw_layers")
        self.assertEqual(only(rows, "pico8_dc", "sorte"), "symbole cité")
        self.assertEqual(sites(rows, "pico8_dc"), {"src/engine/entity_registry.c": "écrivain",
                                                   "tools/composition_residue_sites.txt": "lecteur"})
        self.assertEqual(only(rows, "pico8_jo_t.jy", "nouveau"), "planet_t.needs")
        self.assertEqual(sites(rows, "pico8_jo_t.jy"), {"src/game/planet.h": "écrivain",
                                                        "tools/composition_residue.sh": "lecteur"})
        self.assertEqual(sites(rows, "PICO8_CLASS_COVERAGE")["Makefile"], "lecteur")
        self.assertEqual(sites(rows, "jo_list"), {})
        self.assertEqual(only(rows, "pico8_draw", "nouveau"), "")
        self.assertIn("chaîne:pico8_draw\t", flags)

    def test_tool_binaries_and_outputs_of_tmp(self):
        makefile = ("$(TMP_DIR)/pico8_replay: $(call objs,$(REPLAY_SRCS)) | $(TMP_DIR)\n"
                    "\t$(CC) -o $@ $^\n"
                    "# a comment on tmp/pico8_gif\n"
                    "$(TMP_DIR)/pico8_frames: $(call objs,src/cmd/frames.c) | $(TMP_DIR)\n"
                    "FRAMES_DIR := $(TMP_DIR)/frames\n"
                    "$(TMP_DIR)/main_raylib: src/cmd/game.c src/p8/gfx.c\n"
                    "$(TMP_DIR)/test_planet: tests/test_planet.c\n"
                    "replay: $(TMP_DIR)/pico8_replay\n"
                    "\t./$(TMP_DIR)/pico8_replay $(EVENTS)\n")
        result, rows, flags, _ = run({
            "Makefile": makefile,
            "tools/frames_ref.sh": "# usage: <outil pico8_frames>\nmake tmp/pico8_replay\n",
            "tests/test_frames_tool.c": '#define DRIVER "./tmp/pico8_replay"\n'
                                        '#define DUMP "tmp/pico8_replay_dump.txt"\n',
            "src/cmd/replay.c": 'void f(int c, char **v) {\n'
                                '    const char *d = c > 3 ? v[3] : "tmp/pico8_replay_dump.txt";\n'
                                '    fprintf(stderr, "usage: pico8_replay <events>\\n");\n}\n',
            "tests/test_demo.c": 'void t(void) { FILE *o = fopen("tmp/pico8_demo_dump.txt", "w"); }\n',
        }, overrides="file\ttmp/pico8_frames\ttmp/frames_tool\t\n"
                     "site\ttmp/pico8_replay@tools/frames_ref.sh\tkeeps the old name too\t\n")
        self.assertEqual(result.returncode, 0, result.stderr)
        files = [r for r in rows if r["catégorie"] == "fichier"]
        self.assertEqual(only(files, "tmp/pico8_replay", "nouveau"), "tmp/replay")
        self.assertEqual(only(files, "tmp/pico8_replay", "sorte"), "binaire")
        self.assertEqual(sites(files, "tmp/pico8_replay"), {
            "Makefile": "écrivain, lecteur", "tools/frames_ref.sh": "lecteur ; keeps the old name too",
            "tests/test_frames_tool.c": "lecteur", "src/cmd/replay.c": "lecteur"})
        self.assertEqual(only(files, "tmp/pico8_frames", "nouveau"), "tmp/frames_tool")
        self.assertEqual(only(files, "tmp/main_raylib", "nouveau"), "tmp/game")
        dump = "tmp/pico8_replay_dump.txt"
        self.assertEqual(only(files, dump, "nouveau"), "tmp/replay_dump.txt")
        self.assertEqual(only(files, dump, "sorte"), "sortie")
        self.assertEqual(sites(files, dump), {"src/cmd/replay.c": "écrivain",
                                              "tests/test_frames_tool.c": "lecteur"})
        self.assertEqual(sites(files, "tmp/pico8_demo_dump.txt"), {"tests/test_demo.c": "écrivain"})
        self.assertIn("fichier:tmp/pico8_demo_dump.txt\t", flags)
        self.assertEqual({r["ancien"] for r in files},
                         {"tmp/pico8_replay", "tmp/pico8_frames", "tmp/main_raylib", dump,
                          "tmp/pico8_demo_dump.txt"})

    def test_key_with_one_side_only_is_flagged(self):
        result, rows, flags, _ = run({
            "src/engine/entity_class.h": ENTITY_CLASS_H,
            "src/game/economy.c": 'void f(void) { pico8_cg("jo"); }\n',
        })
        self.assertIn("chaîne:qh\t", flags)
        self.assertIn("sans lecteur", flags)
        self.assertNotIn("chaîne:jo\t", flags)


if __name__ == "__main__":
    unittest.main()
