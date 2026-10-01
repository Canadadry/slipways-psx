"""tools/rename_derive.py, black box: a mini vocabulary, a mini extraction and
a mini overrides table written to tmp/, then the table and the remainder it
writes, and its exit code. Run by `make rename-derive-test`, part of
`make test` (no libclang needed)."""

import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "rename_derive.py")
WORK = os.path.join(REPO, "tmp", "test_rename_derive_work")

VOCABULARY = """# Vocabulary

Statut : validé

## Concepts

### Core

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `ke` (`pico8_ke_*`) | root class | `entity` (module) | x | |
| `ks` (`pico8_ke_ks`) | sets the state | `entity_set_state` | x | |
| `pq` (suffixe `pico8_X_pq`) | constructor | `X_construct` | x | |
| `def` (`pico8_X_def`, `pico8_X_def_t`) | archetype | `X_archetype`, `X_archetype_t` | x | |
| `hv` (`pico8_hv`) | live entities | `live_entities` | x | |
| `_dispatch` (suffixe) | vtable adapter | `X_<méthode>_method` | x | |

### States

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `ph` (`PICO8_STATE_PH`) | tick | `ENTITY_STATE_NORMAL`, chaîne `"normal"` | x | |

### Memory

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `pico8_zone_choisie_t` | allocator handle | `allocator_t` | x | |
| `prise` (`pico8_jo_prise_*`, champ `prise`) | grab arena | `grab_arena` | x | |

### Planets

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `jo` | planet | `planet` | x | |
| `qh` (lignée `PICO8_LINEAGE_QH`) | structure | `structure` | x | |
| `qu`, `re` (champs de `jc`) | ends | `start`, `end` | x | |

### Runtime

| nom actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| préfixe `pico8_` des primitives émulées (`pico8_peek`, `pico8_spr`…) | runtime | préfixe `p8_` | x | |

## En-tête commun

| membre actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `pico8_struct_header_t` | header | `entity_t` | x | |
| `t` | frames in state | `state_time` | x | |
| `ra` | pixel position | `pos` | x | |
| `emitter` | emitter | conservé | x | |

## Vtable

| membre actuel | sens | nom anglais proposé | source | doute |
|---|---|---|---|---|
| `kj` | draw | `draw` | x | |
| `ldown` | left press | `left_press` | x | |

## Arborescence

| chemin actuel | répertoire cible | nom cible | doute |
|---|---|---|---|
| `src/pico8_jo.c` | `src/game/` | `planet.c` | |
"""

HEADER = "identity\tkind\tname\tparent\tfile\tline\tlinkage\tscope_lines\toccurrences\tflags\n"


def decl(identity, kind, name, file, parent="-", linkage="-", scope="-"):
    return "\t".join([identity, kind, name, parent, file, "1", linkage, scope, "1", "-"]) + "\n"


def run(declarations, overrides="", vocabulary=VOCABULARY, existing=None):
    """Writes the inputs, runs the script; returns (result, rows by identity,
    remainder lines). `existing` is a previous table left at the output."""
    if os.path.isdir(WORK):
        shutil.rmtree(WORK)
    os.makedirs(WORK)
    paths = {name: os.path.join(WORK, name)
             for name in ("decls.tsv", "vocab.md", "overrides.tsv", "table.tsv", "remainder.tsv")}
    with open(paths["decls.tsv"], "w") as f:
        f.write("# partial\tsome/unit.c\tdefault\tignored\n" + HEADER + "".join(declarations))
    with open(paths["vocab.md"], "w") as f:
        f.write(vocabulary)
    with open(paths["overrides.tsv"], "w") as f:
        f.write("kind\tkey\tnew\tconcept\n" + overrides)
    if existing is not None:
        with open(paths["table.tsv"], "w") as f:
            f.write(existing)
    result = subprocess.run(
        [sys.executable, SCRIPT, "--declarations", paths["decls.tsv"],
         "--vocabulary", paths["vocab.md"], "--overrides", paths["overrides.tsv"],
         "--output", paths["table.tsv"], "--remainder", paths["remainder.tsv"]],
        capture_output=True, text=True)
    table = {}
    lines = []
    if os.path.exists(paths["table.tsv"]):
        with open(paths["table.tsv"]) as f:
            lines = f.read().splitlines()
    data = [l for l in lines if not l.startswith("#")]
    if data:
        header = data[0].split("\t")
        for line in data[1:]:
            row = dict(zip(header, line.split("\t")))
            table[row["identité"]] = row
    remainder = []
    if os.path.exists(paths["remainder.tsv"]):
        with open(paths["remainder.tsv"]) as f:
            remainder = [l for l in f.read().splitlines()[1:] if l]
    return result, table, remainder, lines


class Derivation(unittest.TestCase):

    def new_name(self, declaration, overrides="", vocabulary=VOCABULARY):
        result, table, remainder, _ = run([declaration], overrides, vocabulary)
        self.assertEqual(result.returncode, 0, result.stderr)
        identity = declaration.split("\t")[0]
        return table[identity]

    def test_vocabulary_concept_replaces_cart_prefix(self):
        row = self.new_name(decl("c:@F@pico8_jo_kj", "function", "pico8_jo_kj",
                                 "src/game/planet.h", linkage="extern"))
        self.assertEqual(row["ancien"], "pico8_jo_kj")
        self.assertEqual(row["nouveau"], "planet_draw")
        self.assertEqual(row["statut"], "renommé")
        self.assertEqual(row["catégorie"], "identifiant")
        self.assertEqual(row["sorte"], "fonction")
        self.assertEqual(row["fichier"], "src/game/planet.h")
        self.assertEqual(row["module"], "game/planet")
        self.assertEqual(row["concept"], "jo")

    def test_dispatch_adapter_becomes_class_method(self):
        row = self.new_name(decl("c:planet.c@F@pico8_jo_kj_dispatch", "function",
                                 "pico8_jo_kj_dispatch", "src/game/planet.c", linkage="static"))
        self.assertEqual(row["nouveau"], "planet_draw_method")

    def test_archetype_and_constructor_suffixes(self):
        row = self.new_name(decl("c:@pico8_jo_def", "global", "pico8_jo_def",
                                 "src/game/game_data.c", linkage="extern"))
        self.assertEqual(row["nouveau"], "planet_archetype")
        row = self.new_name(decl("c:@T@pico8_jo_def_t", "type", "pico8_jo_def_t",
                                 "src/game/game_data.h"))
        self.assertEqual(row["nouveau"], "planet_archetype_t")
        row = self.new_name(decl("c:@F@pico8_jo_pq", "function", "pico8_jo_pq",
                                 "src/game/planet.h", linkage="extern"))
        self.assertEqual(row["nouveau"], "planet_construct")

    def test_static_function_drops_module_prefix(self):
        result, table, _, _ = run([
            decl("c:layout.c@F@pico8_layout_measure", "function", "pico8_layout_measure",
                 "src/game/layout.c", linkage="static"),
            decl("c:planet.c@F@pico8_jo_measure", "function", "pico8_jo_measure",
                 "src/game/planet.c", linkage="static"),
            decl("c:@T@pico8_jo_t", "type", "pico8_jo_t", "src/game/planet.h"),
        ])
        self.assertEqual(table["c:layout.c@F@pico8_layout_measure"]["nouveau"], "measure")
        self.assertEqual(table["c:planet.c@F@pico8_jo_measure"]["nouveau"], "planet_measure")
        row = self.new_name(decl("c:@F@pico8_layout_measure", "function",
                                 "pico8_layout_measure", "src/game/layout.h", linkage="extern"))
        self.assertEqual(row["nouveau"], "layout_measure")

    def test_unknown_digram_goes_to_remainder(self):
        result, table, remainder, _ = run([
            decl("c:@F@pico8_zz", "function", "pico8_zz", "src/game/planet.h", linkage="extern")])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(table["c:@F@pico8_zz"]["nouveau"], "")
        self.assertEqual(len(remainder), 1)
        self.assertIn("c:@F@pico8_zz", remainder[0])

    def test_overrides_win_over_rules(self):
        overrides = ("name\tpico8_jo_kj\tplanet_render\tjo\n"
                     "token\tzz\tbuzz\t\n"
                     "identity\tc:@F@pico8_jo_ks\tplanet_set_mood\t\n"
                     "field\tpico8_jo_t.zz\tbuzz_level\t\n")
        result, table, remainder, _ = run([
            decl("c:@F@pico8_jo_kj", "function", "pico8_jo_kj", "src/game/planet.h", linkage="extern"),
            decl("c:@F@pico8_zz_ldown", "function", "pico8_zz_ldown", "src/game/planet.h", linkage="extern"),
            decl("c:@F@pico8_jo_ks", "function", "pico8_jo_ks", "src/game/planet.h", linkage="extern"),
            decl("c:@SA@pico8_jo_t@FI@zz", "field", "zz", "src/game/planet.h", parent="pico8_jo_t"),
        ], overrides)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(table["c:@F@pico8_jo_kj"]["nouveau"], "planet_render")
        self.assertEqual(table["c:@F@pico8_zz_ldown"]["nouveau"], "planet_buzz_left_press")
        self.assertEqual(table["c:@F@pico8_jo_ks"]["nouveau"], "planet_set_mood")
        self.assertEqual(table["c:@SA@pico8_jo_t@FI@zz"]["nouveau"], "buzz_level")
        self.assertEqual(remainder, [])

    def test_override_can_keep_a_name(self):
        row = self.new_name(decl("c:@F@pico8_jo_kj", "function", "pico8_jo_kj",
                                 "src/game/planet.h", linkage="extern"),
                            overrides="name\tpico8_jo_kj\t=\t\n")
        self.assertEqual(row["nouveau"], "pico8_jo_kj")
        self.assertEqual(row["statut"], "conservé")

    def test_header_members_and_their_tokens(self):
        result, table, remainder, _ = run([
            decl("c:@SA@pico8_struct_header_t@FI@t", "field", "t", "src/engine/entity_class.h",
                 parent="pico8_struct_header_t"),
            decl("c:@SA@pico8_iv_keys_t@FI@ra", "field", "ra", "src/game/message_box.h",
                 parent="pico8_iv_keys_t"),
            decl("c:@SA@pico8_jo_t@FI@emitter", "field", "emitter", "src/game/planet.h",
                 parent="pico8_jo_t"),
        ])
        self.assertEqual(table["c:@SA@pico8_struct_header_t@FI@t"]["nouveau"], "state_time")
        self.assertEqual(table["c:@SA@pico8_iv_keys_t@FI@ra"]["nouveau"], "pos")
        self.assertEqual(table["c:@SA@pico8_jo_t@FI@emitter"]["statut"], "conservé")

    def test_short_names_kept_only_for_short_scoped_locals_and_vector_fields(self):
        result, table, remainder, _ = run([
            decl("c:a.c@1@F@f@i", "local", "i", "src/game/planet.c", parent="f", scope="40"),
            decl("c:a.c@2@F@f@dx", "param", "dx", "src/game/planet.c", parent="f", scope="12"),
            decl("c:a.c@3@F@g@i", "local", "i", "src/game/planet.c", parent="g", scope="41"),
            decl("c:@SA@pico8_vec_t@FI@x", "field", "x", "src/engine/vec.h", parent="pico8_vec_t"),
            decl("c:@SA@pico8_lq_t@FI@w", "field", "w", "src/game/layout.h", parent="pico8_lq_t"),
            decl("c:a.c@4@F@f@jo", "local", "jo", "src/game/planet.c", parent="f", scope="5"),
            decl("c:a.c@5@F@g@jo", "local", "jo", "src/game/planet.c", parent="g", scope="80"),
            decl("c:@SA@pico8_jo_keys_t@FI@id", "field", "id", "src/game/planet.h", parent="pico8_jo_keys_t"),
            decl("c:a.c@6@F@f@s", "static", "s", "src/game/planet.c", parent="f", scope="9"),
            decl("c:a.h@7@e", "param", "e", "src/engine/collect.h",
                 parent="src/engine/collect.c", scope="89"),
            decl("c:a.h@8@sw", "param", "sw", "src/engine/entity_class.h", parent="src/cmd/game.c"),
        ], "token\tid\tid\t\n")
        self.assertEqual(table["c:a.c@1@F@f@i"]["statut"], "conservé")
        self.assertEqual(table["c:a.c@2@F@f@dx"]["statut"], "conservé")
        self.assertEqual(table["c:a.c@3@F@g@i"]["nouveau"], "")
        self.assertEqual(table["c:@SA@pico8_vec_t@FI@x"]["statut"], "conservé")
        self.assertEqual(table["c:@SA@pico8_lq_t@FI@w"]["nouveau"], "")
        self.assertEqual(table["c:a.c@4@F@f@jo"]["statut"], "conservé")
        self.assertEqual(table["c:a.c@5@F@g@jo"]["nouveau"], "planet")
        self.assertEqual(table["c:@SA@pico8_jo_keys_t@FI@id"]["nouveau"], "")
        self.assertEqual(table["c:a.c@6@F@f@s"]["statut"], "conservé")
        self.assertEqual(table["c:a.h@7@e"]["statut"], "conservé")
        self.assertEqual(table["c:a.h@8@sw"]["statut"], "conservé")
        self.assertEqual(len(remainder), 3)

    def test_corrected_concept_propagates_to_derived_names(self):
        corrected = VOCABULARY.replace("| `jo` | planet | `planet` |", "| `jo` | planet | `body` |")
        result, table, _, _ = run([
            decl("c:@F@pico8_jo_kj", "function", "pico8_jo_kj", "src/game/planet.h", linkage="extern"),
            decl("c:@pico8_jo_def", "global", "pico8_jo_def", "src/game/game_data.c", linkage="extern"),
            decl("c:@T@pico8_jo_t", "type", "pico8_jo_t", "src/game/planet.h"),
        ], vocabulary=corrected)
        self.assertEqual(table["c:@F@pico8_jo_kj"]["nouveau"], "body_draw")
        self.assertEqual(table["c:@pico8_jo_def"]["nouveau"], "body_archetype")

    def test_runtime_symbols_take_p8_prefix(self):
        result, table, _, _ = run([
            decl("c:@F@pico8_spr", "function", "pico8_spr", "src/p8/gfx.h", linkage="extern"),
            decl("c:gfx.c@F@pico8_circ_plot8", "function", "pico8_circ_plot8", "src/p8/gfx.c",
                 linkage="static"),
            decl("macro:src/p8/fixed.h:PICO8_FIXED_ONE", "macro", "PICO8_FIXED_ONE", "src/p8/fixed.h"),
            decl("macro:src/game/planet.h:PICO8_JO_CAP", "macro", "PICO8_JO_CAP", "src/game/planet.h"),
            decl("c:@EA@pico8_state_t@PICO8_STATE_PH", "enum_constant", "PICO8_STATE_PH",
                 "src/engine/entity_class.h", parent="pico8_state_t"),
            decl("c:@EA@pico8_lineage_t@PICO8_LINEAGE_QH", "enum_constant", "PICO8_LINEAGE_QH",
                 "src/engine/entity_class.h", parent="pico8_lineage_t"),
            decl("c:@Ea@QH@QH", "enum_constant", "QH", "tests/test_planet.c"),
        ])
        self.assertEqual(table["c:@F@pico8_spr"]["nouveau"], "p8_spr")
        self.assertEqual(table["c:gfx.c@F@pico8_circ_plot8"]["nouveau"], "circ_plot8")
        self.assertEqual(table["macro:src/p8/fixed.h:PICO8_FIXED_ONE"]["nouveau"], "P8_FIXED_ONE")
        self.assertEqual(table["macro:src/game/planet.h:PICO8_JO_CAP"]["nouveau"], "PLANET_CAP")
        self.assertEqual(table["c:@EA@pico8_state_t@PICO8_STATE_PH"]["nouveau"],
                         "ENTITY_STATE_NORMAL")
        self.assertEqual(table["c:@EA@pico8_lineage_t@PICO8_LINEAGE_QH"]["nouveau"], "LINEAGE_STRUCTURE")
        self.assertEqual(table["c:@Ea@QH@QH"]["nouveau"], "STRUCTURE")

    def test_include_guard_follows_the_header_path(self):
        row = self.new_name(decl("macro:src/engine/geom.h:PICO8_GEOM_H", "macro",
                                 "PICO8_GEOM_H", "src/engine/geom.h"))
        self.assertEqual(row["nouveau"], "ENGINE_GEOM_H")

    def test_struct_tag_follows_its_typedef(self):
        result, table, _, _ = run([
            decl("c:@S@pico8_zone_choisie", "tag", "pico8_zone_choisie", "src/engine/allocator.h"),
            decl("c:@T@pico8_zone_choisie_t", "type", "pico8_zone_choisie_t", "src/engine/allocator.h"),
        ])
        self.assertEqual(table["c:@T@pico8_zone_choisie_t"]["nouveau"], "allocator_t")
        self.assertEqual(table["c:@S@pico8_zone_choisie"]["nouveau"], "allocator")

    def test_rerun_keeps_header_and_other_categories_and_sorts(self):
        columns = "ancien\tnouveau\tfichier\tcatégorie\tsorte\tstatut\tmodule\tconcept\tnote\tidentité"
        existing = ("# Statut : validé\n" + columns + "\n"
                    "jo\tplanet\tsrc/game/planet.c\tchaîne\tclé\trenommé\tgame/planet\tjo\t\t\n"
                    "old_x\tnew_x\tsrc/game/planet.c\tidentifiant\tfonction\trenommé"
                    "\tgame/planet\t\t\tc:@F@gone\n")
        result, table, _, lines = run([
            decl("c:@F@pico8_jo_kj", "function", "pico8_jo_kj", "src/game/planet.h", linkage="extern"),
            decl("c:@F@pico8_spr", "function", "pico8_spr", "src/p8/gfx.h", linkage="extern"),
        ], existing=existing)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(lines[0], "# Statut : validé")
        self.assertEqual(lines[1], columns)
        self.assertNotIn("c:@F@gone", table)
        data = [l.split("\t") for l in lines[2:]]
        self.assertEqual([row[0] for row in data], ["jo", "pico8_jo_kj", "pico8_spr"])

    def test_global_prefix_single_letters_and_inner_dispatch(self):
        result, table, remainder, _ = run([
            decl("c:planet.c@g_jo_prise", "static", "g_jo_prise", "src/game/planet.c", linkage="static"),
            decl("c:a.c@1@F@f@cel_x", "param", "cel_x", "src/p8/gfx.c", parent="f", scope="90"),
            decl("c:@F@pico8_dispatch_set_gfx", "function", "pico8_dispatch_set_gfx",
                 "src/engine/dispatch_context.h", linkage="extern"),
        ])
        self.assertEqual(table["c:planet.c@g_jo_prise"]["nouveau"], "g_planet_grab_arena")
        self.assertEqual(table["c:a.c@1@F@f@cel_x"]["statut"], "conservé")
        self.assertEqual(table["c:@F@pico8_dispatch_set_gfx"]["nouveau"], "dispatch_context_dispatch_set_gfx")
        self.assertEqual(remainder, [])

    def test_exported_function_takes_module_prefix_unless_it_names_its_type(self):
        overrides = "token\tdh\tdevelopment_tier\t\n"
        result, table, _, _ = run([
            decl("c:@F@pico8_dh", "function", "pico8_dh", "src/game/economy_formulas.h", linkage="extern"),
            decl("c:@F@pico8_jo_kj", "function", "pico8_jo_kj", "src/game/planet_display.h",
                 linkage="extern"),
            decl("c:@T@pico8_jo_t", "type", "pico8_jo_t", "src/game/planet.h"),
            decl("c:@F@pico8_hv", "function", "pico8_hv", "src/engine/entity_registry.h", linkage="extern"),
        ], overrides)
        self.assertEqual(table["c:@F@pico8_dh"]["nouveau"], "economy_formulas_development_tier")
        self.assertEqual(table["c:@F@pico8_jo_kj"]["nouveau"], "planet_draw")
        self.assertEqual(table["c:@F@pico8_hv"]["nouveau"], "live_entities")

    def test_header_members_mirrored_in_archetypes_and_entity_structs(self):
        result, table, _, _ = run([
            decl("c:@SA@pico8_bg_def_t@FI@t", "field", "t", "src/game/game_data.h", parent="pico8_bg_def_t"),
            decl("c:@SA@pico8_jo_t@FI@header", "field", "header", "src/game/planet.h", parent="pico8_jo_t"),
            decl("c:@SA@pico8_jo_t@FI@t", "field", "t", "src/game/planet.h", parent="pico8_jo_t"),
            decl("c:@SA@pico8_qy_t@FI@t", "field", "t", "src/game/particle.h", parent="pico8_qy_t"),
        ])
        self.assertEqual(table["c:@SA@pico8_bg_def_t@FI@t"]["nouveau"], "state_time")
        self.assertEqual(table["c:@SA@pico8_jo_t@FI@t"]["nouveau"], "state_time")
        self.assertEqual(table["c:@SA@pico8_qy_t@FI@t"]["nouveau"], "")

    def test_receiver_type_is_a_valid_function_prefix(self):
        result, table, _, _ = run([
            decl("c:@T@pico8_bascule_zone_t", "type", "pico8_bascule_zone_t", "src/engine/zone.h"),
            decl("c:@F@pico8_bascule_zone_alloc", "function", "pico8_bascule_zone_alloc",
                 "src/engine/zone.h", linkage="extern"),
        ], "token\tbascule_zone\tflip_zone\t\n")
        self.assertEqual(table["c:@F@pico8_bascule_zone_alloc"]["nouveau"], "flip_zone_alloc")

    def test_static_function_keeps_a_prefix_it_cannot_lose(self):
        result, table, _, _ = run([
            decl("c:zone.c@F@zone_full", "function", "zone_full", "src/engine/zone.c", linkage="static"),
            decl("c:geom.c@F@pico8_entity_geom_ra", "function", "pico8_entity_geom_ra",
                 "src/engine/entity_geom.c", linkage="static"),
        ])
        self.assertEqual(table["c:zone.c@F@zone_full"]["statut"], "conservé")
        self.assertEqual(table["c:geom.c@F@pico8_entity_geom_ra"]["nouveau"], "entity_geom_pos")

    def test_class_with_a_class_table_is_a_valid_function_prefix(self):
        result, table, _, _ = run([
            decl("c:@F@pico8_jo_struct_class", "function", "pico8_jo_struct_class",
                 "src/game/planet_family.h", linkage="extern"),
            decl("c:@F@pico8_jo_ldown", "function", "pico8_jo_ldown",
                 "src/game/planet_family.h", linkage="extern"),
            decl("c:planet.c@F@pico8_jo_ldown_fast", "function", "pico8_jo_ldown_fast",
                 "src/game/planet_display.c", linkage="static"),
        ])
        self.assertEqual(table["c:@F@pico8_jo_ldown"]["nouveau"], "planet_left_press")
        self.assertEqual(table["c:planet.c@F@pico8_jo_ldown_fast"]["nouveau"], "planet_left_press_fast")

    def test_singleton_function_takes_its_vocabulary_name(self):
        vocabulary = VOCABULARY.replace(
            "| `jo` | planet |", "| `mo` | camera singleton | `main_camera` | x | |\n| `jo` | planet |")
        row = self.new_name(decl("c:@F@pico8_mo", "function", "pico8_mo", "src/engine/entity_registry.h",
                                 linkage="extern"), vocabulary=vocabulary)
        self.assertEqual(row["nouveau"], "main_camera")

    def test_digram_with_digits_is_not_a_word(self):
        result, table, remainder, _ = run([
            decl("c:menu.c@F@pico8_ha2_open", "function", "pico8_ha2_open", "src/game/menu.c",
                 linkage="static"),
        ])
        self.assertEqual(table["c:menu.c@F@pico8_ha2_open"]["nouveau"], "")

    def test_every_type_name_ends_in_t(self):
        result, table, _, _ = run([
            decl("c:@T@pico8_state_fn", "type", "pico8_state_fn", "src/engine/entity_class.h"),
            decl("c:@T@giver_t", "type", "giver_t", "tests/test_ui.c"),
        ], "token\tfn\tfn\t\n")
        self.assertEqual(table["c:@T@pico8_state_fn"]["nouveau"], "state_fn_t")
        self.assertEqual(table["c:@T@giver_t"]["statut"], "conservé")

    def test_string_file_and_site_overrides_leave_identifiers_alone(self):
        row = self.new_name(decl("c:@F@pico8_jo_kj", "function", "pico8_jo_kj",
                                 "src/game/planet.h", linkage="extern"),
                            overrides="string\tpico8_jo_kj\tnope\t\n"
                                      "file\ttmp/pico8_jo_kj\ttmp/nope\t\n"
                                      "site\tpico8_jo_kj@Makefile\ta note\t\n")
        self.assertEqual(row["nouveau"], "planet_draw")

    def test_an_override_key_given_twice_is_refused(self):
        result, _, _, _ = run([decl("c:@F@pico8_jo_kj", "function", "pico8_jo_kj",
                                    "src/game/planet.h", linkage="extern")],
                              "name\tpico8_jo_kj\tplanet_a\t\nname\tpico8_jo_kj\tplanet_b\t\n")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("pico8_jo_kj", result.stderr)


if __name__ == "__main__":
    unittest.main()
