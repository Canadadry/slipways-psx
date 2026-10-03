"""tools/gap_check.py, black box on throwaway trees, then `make gap-check`
through the real recipe in a reduced copy (the Makefile, the tool, a
minimal register). Run by `make gap-check-test`, part of `make test`.

The markers are built at run time and never written here: written out,
they would turn the check red on this very file."""

import glob
import os
import re
import shutil
import subprocess
import sys
import unittest

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPT = os.path.join(REPO, "tools", "gap_check.py")
WORK = os.path.join(REPO, "tmp", "test_gap_check_work")
PARENT_MAKE_VARIABLES = ("MAKEFLAGS", "MFLAGS", "MAKELEVEL")

OLD_MARKER = "TRO" + "U-"
GAP = "GA" + "P-"
REGISTER = "# Register\n\n## %s1 — first\n\ntext\n\n## %s3 — third\n\ntext\n" % (GAP, GAP)


def register_path():
    """The one register that the GAP_REGISTER pattern of the Makefile
    matches, relative to the repository."""
    with open(os.path.join(REPO, "Makefile"), encoding="utf-8") as f:
        match = re.search(r"^GAP_REGISTER := \$\(wildcard (\S+)\)$", f.read(), re.M)
    assert match, "no GAP_REGISTER line in the Makefile"
    found = glob.glob(os.path.join(REPO, match.group(1)))
    assert len(found) == 1, "GAP_REGISTER matches %s" % found
    return os.path.relpath(found[0], REPO)


def marker(number):
    return "%s%d" % (GAP, number)


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def run(files, register=REGISTER):
    shutil.rmtree(WORK, ignore_errors=True)
    root = os.path.join(WORK, "tree")
    write(os.path.join(root, "src", "clean.c"), "int main(void) { return 0; }\n")
    for path, text in files.items():
        write(os.path.join(root, path), text)
    gaps = os.path.join(WORK, "register.md")
    if register is not None:
        write(gaps, register)
    return subprocess.run([sys.executable, SCRIPT, "--root", root, "--gaps", gaps],
                          capture_output=True, text=True)


def report(result):
    return "exit %d\nstdout:\n%s\nstderr:\n%s" % (result.returncode, result.stdout, result.stderr)


class GapCheck(unittest.TestCase):

    def test_titled_markers_and_prose_pass(self):
        result = run({"src/game/planet.c": "/* %s */\nint a;\n" % marker(1),
                      "tests/test_planet.c": "/* as %s says */\n" % marker(3),
                      "Makefile": "# %s\n" % marker(1)})
        self.assertEqual(result.returncode, 0, report(result))

    def test_an_untitled_marker_fails_and_is_named(self):
        result = run({"src/game/planet.c": "int a;\n/* %s */\n" % marker(2)})
        self.assertEqual(result.returncode, 1, report(result))
        self.assertIn("src/game/planet.c:2:", result.stdout, report(result))
        self.assertIn(marker(2), result.stdout, report(result))

    def test_an_untitled_number_in_prose_fails(self):
        result = run({"tools/notes.txt": "see %s for the reason\n" % marker(30)})
        self.assertEqual(result.returncode, 1, report(result))
        self.assertIn("tools/notes.txt:1:", result.stdout, report(result))

    def test_a_longer_number_is_not_a_titled_one(self):
        result = run({"src/a.c": "/* %s */\n" % marker(13)})
        self.assertEqual(result.returncode, 1, report(result))

    def test_an_old_marker_fails_anywhere(self):
        for path in ("src/a.c", "tests/test_a.c", "tools/a.sh", "Makefile"):
            result = run({path: "x\n/* %s1 */\n" % OLD_MARKER})
            self.assertEqual(result.returncode, 1, "%s\n%s" % (path, report(result)))
            self.assertIn("%s:2:" % path, result.stdout, report(result))

    def test_outside_the_read_directories_nothing_is_read(self):
        result = run({"docs/a.md": "/* %s1 %s */\n" % (OLD_MARKER, marker(9)),
                      "tools/__pycache__/a.txt": "%s1\n" % OLD_MARKER})
        self.assertEqual(result.returncode, 0, report(result))

    def test_a_missing_or_empty_register_is_an_error(self):
        result = run({}, register=None)
        self.assertEqual(result.returncode, 2, report(result))
        result = run({}, register="# Register\n\nno entry\n")
        self.assertEqual(result.returncode, 2, report(result))


class RealTarget(unittest.TestCase):
    """`make -C <copy> gap-check`, the real recipe."""

    def copy(self, plant):
        work = os.path.join(REPO, "tmp", "test_gap_check_copy")
        shutil.rmtree(work, ignore_errors=True)
        for path in ("Makefile", "tools/gap_check.py"):
            os.makedirs(os.path.dirname(os.path.join(work, path)) or work, exist_ok=True)
            shutil.copy2(os.path.join(REPO, path), os.path.join(work, path))
        write(os.path.join(work, register_path()), REGISTER)
        write(os.path.join(work, "src", "game", "planet.c"), "int a;\n" + plant)
        env = {k: v for k, v in os.environ.items() if k not in PARENT_MAKE_VARIABLES}
        return subprocess.run(["make", "--no-print-directory", "-C", work, "gap-check"],
                              capture_output=True, text=True, env=env, timeout=300)

    def test_green_on_titled_markers(self):
        result = self.copy("/* %s */\n" % marker(3))
        self.assertEqual(result.returncode, 0, report(result))

    def test_red_on_an_old_marker(self):
        result = self.copy("/* %s3 */\n" % OLD_MARKER)
        self.assertNotEqual(result.returncode, 0, report(result))
        self.assertIn("src/game/planet.c:2:", result.stdout, report(result))

    def test_red_on_an_untitled_marker(self):
        result = self.copy("/* %s */\n" % marker(99))
        self.assertNotEqual(result.returncode, 0, report(result))
        self.assertIn("src/game/planet.c:2:", result.stdout, report(result))


if __name__ == "__main__":
    unittest.main()
