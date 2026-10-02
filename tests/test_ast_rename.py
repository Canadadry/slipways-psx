"""tools/ast_rename, black box: run on a copy of tests/fixtures/ast_rename/ in
tmp/, then check the files it writes, the sources it rewrites and its exit code.
Run by `make rename-tool-test`, which sets PYTHONPATH and LIBCLANG."""

import hashlib
import os
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FIXTURE = os.path.join(REPO, "tests", "fixtures", "ast_rename")
EXPECTED = os.path.join(REPO, "tests", "fixtures", "ast_rename_expected")
WORK = os.path.join(REPO, "tmp", "test_ast_rename_work")
CFLAGS = "-std=c99 -Wall -Wextra -Werror -Isrc"
CONFIGS = ["default=", "coverage=-DSHAPES_COVERAGE"]
UNITS = ["src/main.c", "src/shapes.c"]


def run_tool(args, env=None, cwd=WORK):
    return subprocess.run(
        [sys.executable, "-m", "ast_rename"] + args,
        cwd=cwd, env=env if env is not None else os.environ.copy(),
        capture_output=True, text=True)


def analysis_args(units=UNITS):
    args = ["--root", ".", "--cflags", CFLAGS]
    for config in CONFIGS:
        args += ["--config", config]
    return args + units


def extract(units=UNITS):
    result = run_tool(["extract", "-o", "decls.tsv", "--occurrences", "occs.tsv"]
                      + analysis_args(units))
    return result


def read(path):
    with open(os.path.join(WORK, path), "rb") as f:
        return f.read()


def all_rows():
    """Declaration rows of decls.tsv as dicts; `#` lines skipped, the first
    other line is the header."""
    lines = [l for l in read("decls.tsv").decode().splitlines() if not l.startswith("#")]
    header = lines[0].split("\t")
    return [dict(zip(header, line.split("\t"))) for line in lines[1:]]


def rows():
    """all_rows() keyed by (name, parent), for names declared once."""
    return {(row["name"], row["parent"]): row for row in all_rows()}


def occurrences():
    """(identity, file, offset, length, origin) lines of occs.tsv."""
    lines = read("occs.tsv").decode().splitlines()[1:]
    return {tuple(line.split("\t")) for line in lines}


def offset_of(path, needle, nth=0, delta=0):
    """Byte offset of the nth `needle` in the fixture file, plus delta."""
    data = read(path)
    offset = -1
    for _ in range(nth + 1):
        offset = data.index(needle.encode(), offset + 1)
    return str(offset + delta)


def tree_digest():
    digest = {}
    for base, _, files in os.walk(os.path.join(WORK, "src")):
        for name in files:
            path = os.path.join(base, name)
            with open(path, "rb") as f:
                digest[os.path.relpath(path, WORK)] = hashlib.sha256(f.read()).hexdigest()
    return digest


def write_table(lines):
    with open(os.path.join(WORK, "table.tsv"), "w") as f:
        f.write("".join("\t".join(line) + "\n" for line in lines))


def apply(units=UNITS):
    return run_tool(["apply", "--table", "table.tsv", "--report", "report.txt"]
                    + analysis_args(units))


def compiles(defines=""):
    for unit in UNITS:
        cmd = "cc %s %s -c -o /dev/null %s" % (CFLAGS, defines, unit)
        if subprocess.run(cmd, shell=True, cwd=WORK).returncode != 0:
            return False
    return True


class Setup(unittest.TestCase):
    def setUp(self):
        shutil.rmtree(WORK, ignore_errors=True)
        shutil.copytree(FIXTURE, WORK)


class StartupTest(Setup):
    def test_imports_and_shows_usage(self):
        result = run_tool(["--help"])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("extract", result.stdout)
        self.assertIn("apply", result.stdout)

    def test_refuses_bindings_whose_major_differs_from_libclang(self):
        fake = os.path.join(WORK, "fake_bindings")
        import clang
        shutil.copytree(os.path.dirname(clang.__file__), os.path.join(fake, "clang"))
        dist = os.path.join(fake, "clang-17.0.6.dist-info")
        os.makedirs(dist)
        with open(os.path.join(dist, "METADATA"), "w") as f:
            f.write("Metadata-Version: 2.1\nName: clang\nVersion: 17.0.6\n")
        env = os.environ.copy()
        env["PYTHONPATH"] = fake + os.pathsep + env.get("PYTHONPATH", "")
        result = run_tool(["extract", "-o", "decls.tsv"] + analysis_args(), env=env)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("17.0.6", result.stderr)
        self.assertIn("18", result.stderr)
        self.assertFalse(os.path.exists(os.path.join(WORK, "decls.tsv")))


class ExtractTest(Setup):
    def test_lists_each_declaration_once_with_its_kind_parent_and_linkage(self):
        result = extract()
        self.assertEqual(result.returncode, 0, result.stderr)
        table = rows()
        expected = {
            ("shape_t", "-"): ("type", "src/shapes.h", "-"),
            ("p", "shape_t"): ("field", "src/shapes.h", "-"),
            ("shape_area", "-"): ("function", "src/shapes.c", "extern"),
            ("shape_count", "-"): ("global", "src/shapes.c", "extern"),
            ("sequence", "-"): ("static", "src/shapes.c", "static"),
            ("SHAPE_SCALE", "-"): ("macro", "src/shapes.h", "-"),
            ("SHAPE_AREA", "-"): ("macro", "src/shapes.h", "-"),
            ("ab", "shape_area"): ("local", "src/shapes.c", "local"),
            ("main", "-"): ("function", "src/main.c", "extern"),
        }
        for key, (kind, path, linkage) in expected.items():
            self.assertIn(key, table)
            self.assertEqual((table[key]["kind"], table[key]["file"], table[key]["linkage"]),
                             (kind, path, linkage), key)

    def test_same_field_name_in_two_structs_is_two_identities_flagged_short_homonym(self):
        extract()
        table = rows()
        shape_p, camera_p = table[("p", "shape_t")], table[("p", "camera_t")]
        self.assertNotEqual(shape_p["identity"], camera_p["identity"])
        for row in (shape_p, camera_p):
            self.assertEqual(row["flags"].split(","), ["short", "homonym"])
        self.assertEqual(table[("shape_area", "-")]["flags"], "-")

    def test_occurrences_carry_their_byte_position_and_origin(self):
        extract()
        table = rows()
        shape_p = table[("p", "shape_t")]["identity"]
        sequence = table[("sequence", "-")]["identity"]
        shape_t = table[("shape_t", "-")]["identity"]
        scale = table[("SHAPE_SCALE", "-")]["identity"]
        guard = table[("SHAPES_H", "-")]["identity"]
        occs = occurrences()
        expected = [
            (shape_p, "src/shapes.c", offset_of("src/shapes.c", ".p = 1", delta=1), "1",
             "designator"),
            (shape_p, "src/shapes.c", offset_of("src/shapes.c", "s->p >= 0", delta=3), "1",
             "macro_arg"),
            (shape_p, "src/shapes.h", offset_of("src/shapes.h", "->p", delta=2), "1",
             "macro_body"),
            (sequence, "src/shapes.c", offset_of("src/shapes.c", "sizeof sequence", delta=7),
             "8", "macro_body"),
            (sequence, "src/shapes.c", offset_of("src/shapes.c", "sequence[0]"), "8",
             "macro_body"),
            (shape_t, "src/main.c", offset_of("src/main.c", "(shape_t *)", delta=1), "7",
             "type_ref"),
            (shape_t, "src/shapes.c", offset_of("src/shapes.c", "sizeof(shape_t)", nth=1,
                                                delta=7), "7", "type_ref"),
            (scale, "src/shapes.h", offset_of("src/shapes.h", "SHAPE_SCALE"), "11",
             "macro_def"),
            (scale, "src/shapes.c", offset_of("src/shapes.c", "SHAPE_SCALE"), "11",
             "macro_use"),
            (guard, "src/shapes.h", offset_of("src/shapes.h", "SHAPES_H"), "8", "macro_cond"),
        ]
        for occurrence in expected:
            self.assertIn(occurrence, occs)

    def test_name_in_a_define_expanded_inside_assert_is_found_in_the_define(self):
        extract()
        shape_t = rows()[("shape_t", "-")]["identity"]
        self.assertIn((shape_t, "src/shapes.c", offset_of("src/shapes.c", "sizeof(shape_t)"
                                                          , delta=7), "7", "macro_body"),
                      occurrences())
        self.assertNotIn("# unresolved", read("decls.tsv").decode())

    def test_offsets_are_bytes_even_after_utf8_on_the_line(self):
        extract()
        count = rows()[("shape_count", "-")]["identity"]
        self.assertIn((count, "src/shapes.c", offset_of("src/shapes.c", "+ shape_count", delta=2),
                       "11", "ref"), occurrences())

    def test_comments_and_strings_are_not_occurrences(self):
        extract()
        shape_t = rows()[("shape_t", "-")]["identity"]
        positions = {o[2] for o in occurrences() if o[0] == shape_t and o[1] == "src/shapes.c"}
        self.assertNotIn(offset_of("src/shapes.c", "shape_t is mentioned"), positions)
        self.assertNotIn(offset_of("src/shapes.c", "llo shape_t", delta=4), positions)

    def test_header_included_by_two_units_counts_each_position_once(self):
        extract()
        row = rows()[("shape_count", "-")]
        self.assertEqual(row["occurrences"], "4")
        self.assertEqual(row["file"], "src/shapes.c")

    def test_declaration_present_in_one_configuration_only_is_extracted(self):
        extract()
        row = rows()[("shape_coverage_hits", "-")]
        self.assertEqual((row["kind"], row["occurrences"]), ("global", "2"))

    def test_scope_of_locals_and_params_is_measured_in_lines(self):
        extract()
        table = rows()
        self.assertEqual(table[("ab", "shape_area")]["scope_lines"], "7")
        params = {r["file"]: r["scope_lines"] for r in all_rows()
                  if (r["name"], r["parent"]) == ("s", "shape_area")}
        self.assertEqual(params, {"src/shapes.c": "10", "src/shapes.h": "1"})
        self.assertEqual(table[("shape_count", "-")]["scope_lines"], "-")

    def test_two_runs_give_identical_files(self):
        extract()
        first = read("decls.tsv"), read("occs.tsv")
        extract()
        self.assertEqual((read("decls.tsv"), read("occs.tsv")), first)

    def test_unit_that_does_not_parse_is_listed_first_and_fails(self):
        with open(os.path.join(WORK, "src", "broken.c"), "w") as f:
            f.write('#include "missing.h"\nint broken(void) { return shape_count; }\n')
        result = extract(UNITS + ["src/broken.c"])
        self.assertNotEqual(result.returncode, 0)
        lines = read("decls.tsv").decode().splitlines()
        self.assertTrue(lines[0].startswith("# unparsed\tsrc/broken.c\tcoverage\t"), lines[0])
        self.assertTrue(lines[1].startswith("# unparsed\tsrc/broken.c\tdefault\t"), lines[1])
        self.assertIn("missing.h", lines[0])
        self.assertIn(("shape_count", "-"), rows())

    def test_unit_failing_in_some_configurations_only_is_listed_as_partial(self):
        with open(os.path.join(WORK, "src", "coverage_only.c"), "w") as f:
            f.write("#ifndef SHAPES_COVERAGE\n#error coverage only\n#endif\n"
                    "int coverage_only;\n")
        result = extract(UNITS + ["src/coverage_only.c"])
        self.assertEqual(result.returncode, 0, result.stderr)
        lines = read("decls.tsv").decode().splitlines()
        self.assertTrue(lines[0].startswith("# partial\tsrc/coverage_only.c\tdefault\t"),
                        lines[0])
        self.assertIn(("coverage_only", "-"), rows())


FULL_TABLE = [
    ("shape_t", "figure_t", "src/shapes.h"),
    ("p", "position", "src/shapes.h", "c:@SA@shape_t@FI@p"),
    ("sequence", "button_sequence", "src/shapes.c"),
    ("SHAPE_SCALE", "FIGURE_SCALE", "src/shapes.h"),
    ("SHAPE_AREA", "FIGURE_AREA", "src/shapes.h"),
    ("shape_count", "figure_count", "src/shapes.h"),
    ("ab", "area_base", "src/shapes.c"),
    ("SHAPES_H", "FIGURES_H", "src/shapes.h"),
    ("shape_coverage_hits", "coverage_hits", "src/shapes.c"),
]


class ApplyTest(Setup):
    def assertRefused(self, result, before, message):
        self.assertNotEqual(result.returncode, 0)
        self.assertIn(message, result.stderr)
        self.assertEqual(tree_digest(), before)

    def test_renames_every_occurrence_and_nothing_else(self):
        write_table(FULL_TABLE)
        result = apply()
        self.assertEqual(result.returncode, 0, result.stderr)
        for name in ("shapes.h", "shapes.c", "main.c"):
            with open(os.path.join(EXPECTED, "src", name), "rb") as f:
                self.assertEqual(read("src/" + name).decode(), f.read().decode(), name)
        self.assertTrue(compiles())
        self.assertTrue(compiles("-DSHAPES_COVERAGE"))

    def test_report_counts_occurrences_per_origin(self):
        write_table(FULL_TABLE)
        apply()
        report = read("report.txt").decode()
        self.assertIn("sequence -> button_sequence: 3 (decl 1, macro_body 2)", report)
        self.assertIn("p -> position: 5 (decl 1, designator 1, macro_arg 1, macro_body 1, ref 1)",
                      report)

    def test_refuses_an_entry_that_matches_no_declaration(self):
        before = tree_digest()
        write_table([("shape_count", "figure_count", "src/shapes.h"),
                     ("nothing", "something", "src/shapes.h")])
        self.assertRefused(apply(), before, "nothing: no declaration")

    def test_refuses_an_ambiguous_entry_without_identity(self):
        before = tree_digest()
        write_table([("p", "position", "src/shapes.h")])
        self.assertRefused(apply(), before, "p: ambiguous")

    def test_refuses_a_new_name_already_declared_in_the_same_scope(self):
        before = tree_digest()
        write_table([("shape_count", "shape_area", "src/shapes.h")])
        self.assertRefused(apply(), before, "shape_count -> shape_area: collides")

    def test_refuses_a_local_named_like_a_static_of_its_own_file(self):
        before = tree_digest()
        write_table([("label", "unit_shape", "src/shapes.c")])
        self.assertRefused(apply(), before, "label -> unit_shape: collides")

    def test_accepts_a_local_named_like_a_static_of_another_file(self):
        write_table([("camera", "unit_shape", "src/main.c")])
        result = apply()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(b"camera_t unit_shape = { 0 };", read("src/main.c"))
        self.assertTrue(compiles())

    def plant_sibling_locals(self, nested):
        inner = b"        int low_part = n - 1;\n        return low_part;\n"
        with open(os.path.join(WORK, "src", "shapes.c"), "ab") as f:
            f.write(b"int shape_split(int n) {\n"
                    b"    if (n > 0) {\n"
                    b"        int high_part = n + 1;\n"
                    + (inner + b"    }\n" if nested else
                       b"        return high_part;\n    } else {\n        n = -n;\n" + inner + b"    }\n")
                    + b"    return 0;\n}\n")

    def test_accepts_two_locals_of_disjoint_blocks_given_the_same_name(self):
        self.plant_sibling_locals(nested=False)
        write_table([("high_part", "part", "src/shapes.c"),
                     ("low_part", "part", "src/shapes.c")])
        result = apply()
        self.assertEqual(result.returncode, 0, result.stderr)
        source = read("src/shapes.c")
        self.assertIn(b"int part = n + 1;", source)
        self.assertIn(b"int part = n - 1;", source)
        self.assertTrue(compiles())

    def test_refuses_two_locals_of_overlapping_scopes_given_the_same_name(self):
        self.plant_sibling_locals(nested=True)
        before = tree_digest()
        write_table([("high_part", "part", "src/shapes.c"),
                     ("low_part", "part", "src/shapes.c")])
        self.assertRefused(apply(), before, "collides")

    def plant_enum(self, path, constants):
        with open(os.path.join(WORK, "src", path), "ab") as f:
            f.write(b"enum { " + b", ".join(constants) + b" };\n")

    def test_accepts_enum_constants_of_two_c_files_given_the_same_name(self):
        self.plant_enum("main.c", [b"MAIN_NONE", b"MAIN_PLANET"])
        self.plant_enum("shapes.c", [b"SHAPE_NONE", b"SHAPE_PLANET"])
        write_table([("MAIN_PLANET", "SHAPE_PLANET", "src/main.c")])
        result = apply()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(b"enum { MAIN_NONE, SHAPE_PLANET };", read("src/main.c"))
        self.assertTrue(compiles())

    def test_refuses_an_enum_constant_of_a_c_file_named_like_one_of_a_header(self):
        header = read("src/shapes.h").replace(
            b"#endif", b"enum { SHAPE_NONE, SHAPE_PLANET };\n\n#endif")
        with open(os.path.join(WORK, "src", "shapes.h"), "wb") as f:
            f.write(header)
        self.plant_enum("main.c", [b"MAIN_NONE", b"MAIN_PLANET"])
        before = tree_digest()
        write_table([("MAIN_PLANET", "SHAPE_PLANET", "src/main.c")])
        self.assertRefused(apply(), before, "MAIN_PLANET -> SHAPE_PLANET: collides")

    def plant_main_macro(self):
        with open(os.path.join(WORK, "src", "main.c"), "ab") as f:
            f.write(b"#define MAIN_LIMIT 2\nint main_limit(void) { return MAIN_LIMIT; }\n")

    def test_accepts_a_macro_of_a_c_file_named_like_a_macro_of_another_c_file(self):
        self.plant_main_macro()
        write_table([("MAIN_LIMIT", "SHAPE_SIZE", "src/main.c")])
        result = apply()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(b"return SHAPE_SIZE; }", read("src/main.c"))
        self.assertTrue(compiles())

    def test_refuses_a_macro_of_a_c_file_named_like_a_name_of_a_header(self):
        self.plant_main_macro()
        before = tree_digest()
        write_table([("MAIN_LIMIT", "shape_count", "src/main.c")])
        self.assertRefused(apply(), before, "MAIN_LIMIT -> shape_count: collides")

    def test_refuses_two_entries_giving_the_same_new_name(self):
        before = tree_digest()
        write_table([("shape_count", "total", "src/shapes.h"),
                     ("sequence", "total", "src/shapes.c")])
        self.assertRefused(apply(), before, "collides")

    def test_refuses_a_renamed_name_pasted_with_hash_hash(self):
        with open(os.path.join(WORK, "src", "shapes.c"), "ab") as f:
            f.write(b"#define COUNT_PASTE(suffix) shape_count##suffix\n")
        before = tree_digest()
        write_table([("shape_count", "figure_count", "src/shapes.h")])
        self.assertRefused(apply(), before, "##")

    def test_leaves_a_field_name_pasted_outside_a_member_access(self):
        with open(os.path.join(WORK, "src", "shapes.c"), "ab") as f:
            f.write(b"#define WITH_SUFFIX(stem) (stem##t)\n"
                    b"static int count_t = 3;\n"
                    b"int shape_suffixed(void) { return WITH_SUFFIX(count_); }\n")
        write_table([("t", "turn", "src/shapes.h", "c:@SA@shape_t@FI@t")])
        result = apply()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(b"(stem##t)", read("src/shapes.c"))
        self.assertIn(b"int turn;", read("src/shapes.h"))
        self.assertTrue(compiles())

    def test_refuses_a_field_name_pasted_inside_a_member_access(self):
        with open(os.path.join(WORK, "src", "shapes.c"), "ab") as f:
            f.write(b"#define SHAPE_FIELD(s, pre) ((s)->pre##t)\n")
        before = tree_digest()
        write_table([("t", "turn", "src/shapes.h", "c:@SA@shape_t@FI@t")])
        self.assertRefused(apply(), before, "##")

    def test_refuses_when_an_unparsed_unit_contains_the_old_name(self):
        with open(os.path.join(WORK, "src", "broken.c"), "w") as f:
            f.write('#include "missing.h"\nint broken(void) { return shape_count; }\n')
        before = tree_digest()
        write_table([("shape_count", "figure_count", "src/shapes.h")])
        self.assertRefused(apply(UNITS + ["src/broken.c"]), before,
                           "src/broken.c does not parse")

    def test_applies_when_unparsed_units_do_not_contain_the_old_name(self):
        with open(os.path.join(WORK, "src", "broken.c"), "w") as f:
            f.write('#include "missing.h"\nint broken(void) { return 0; }\n')
        write_table([("sequence", "button_sequence", "src/shapes.c")])
        result = apply(UNITS + ["src/broken.c"])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("src/broken.c", result.stderr)
        self.assertIn(b"button_sequence[0]", read("src/shapes.c"))

    def test_refuses_an_old_name_left_in_an_inactive_branch(self):
        with open(os.path.join(WORK, "src", "shapes.c"), "ab") as f:
            f.write(b"#ifdef NEVER_DEFINED\nint stale(void) { return shape_count; }\n#endif\n")
        before = tree_digest()
        write_table([("shape_count", "figure_count", "src/shapes.h")])
        self.assertRefused(apply(), before, "not accounted for")


class PlanTest(unittest.TestCase):
    """The rewrite plan alone: data in, data out, no file."""

    def model(self, offsets):
        from ast_rename import resolution
        model = resolution.Model()
        model.identities["c:@count"] = resolution.Identity(
            "c:@count", "global", "count", "-", "a.c", 1, "extern", -1, {"a.c"})
        for offset in offsets:
            model.occurrences[("a.c", offset)] = resolution.Occurrence(
                "c:@count", "a.c", offset, 5, "ref")
        return model

    def build(self, offsets, data):
        from ast_rename import plan
        entries = [plan.Entry("count", "total", "a.c", "", 1)]
        return plan.build(entries, self.model(offsets), {"a.c": data})

    def test_rewrites_each_position_holding_the_old_name(self):
        result = self.build([4, 19], b"int count;\nint x = count;\n")
        self.assertEqual(result.errors, [])
        self.assertEqual([(r.file, r.offset, r.new) for r in result.rewrites],
                         [("a.c", 4, "total"), ("a.c", 19, "total")])

    def test_refuses_a_position_that_does_not_hold_the_old_name(self):
        result = self.build([4, 16], b"int count;\nint x = count;\n")
        self.assertEqual(result.rewrites, [])
        self.assertTrue(any("a.c:16 holds" in e for e in result.errors), result.errors)

    def test_refuses_overlapping_positions(self):
        result = self.build([4, 6], b"int countcount;\n")
        self.assertTrue(any("overlapping" in e for e in result.errors), result.errors)


if __name__ == "__main__":
    unittest.main()
