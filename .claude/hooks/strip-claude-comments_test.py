#!/usr/bin/env python3
"""Table-driven tests for strip-claude-comments.py.

Run with: python3 .claude/hooks/strip-claude-comments_test.py
"""
import importlib.util
import json
import os
import subprocess
import sys
import tempfile
import unittest

HOOK_PATH = os.path.join(os.path.dirname(__file__), "strip-claude-comments.py")

spec = importlib.util.spec_from_file_location("strip_claude_comments", HOOK_PATH)
hook = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hook)


class StripCommentsTableTest(unittest.TestCase):
    """Table-driven: each case feeds strip_comments() and checks output + removed count."""

    CASES = [
        dict(
            name="js_full_line_comment_removed",
            ext=".js",
            input="// header\nconst x = 1;\n",
            expected="const x = 1;\n",
            removed=1,
        ),
        dict(
            name="js_trailing_comment_trimmed",
            ext=".js",
            input="const x = 1; // inline note\n",
            expected="const x = 1;\n",
            removed=1,
        ),
        dict(
            name="js_todo_comment_preserved",
            ext=".js",
            input="return 1; // TODO fix later\n",
            expected="return 1; // TODO fix later\n",
            removed=0,
        ),
        dict(
            name="js_todo_case_insensitive_preserved",
            ext=".js",
            input="return 1; //   todo: revisit\n",
            expected="return 1; //   todo: revisit\n",
            removed=0,
        ),
        dict(
            name="js_marker_inside_string_not_stripped",
            ext=".js",
            input='const s = "// not a comment";\n',
            expected='const s = "// not a comment";\n',
            removed=0,
        ),
        dict(
            name="js_block_comment_own_lines_removed",
            ext=".js",
            input="function f() {\n  /* block\n     comment */\n  return 1;\n}\n",
            expected="function f() {\n  return 1;\n}\n",
            removed=1,
        ),
        dict(
            name="js_inline_block_comment_removed_keeps_code",
            ext=".js",
            input="const x = /* explain */ 1;\n",
            expected="const x = 1;\n",
            removed=1,
        ),
        dict(
            name="js_block_comment_todo_preserved",
            ext=".js",
            input="/* TODO revisit this block */\nconst x = 1;\n",
            expected="/* TODO revisit this block */\nconst x = 1;\n",
            removed=0,
        ),
        dict(
            name="js_backtick_template_marker_not_stripped",
            ext=".js",
            input="const s = `has // inside template`;\n",
            expected="const s = `has // inside template`;\n",
            removed=0,
        ),
        dict(
            name="js_multiple_comments_all_counted",
            ext=".js",
            input="// one\nconst a = 1; // two\n// TODO three (kept)\nconst b = 2;\n",
            expected="const a = 1;\n// TODO three (kept)\nconst b = 2;\n",
            removed=2,
        ),
        dict(
            name="lua_line_comment_removed",
            ext=".lua",
            input="-- comment\nlocal x = 1\n",
            expected="local x = 1\n",
            removed=1,
        ),
        dict(
            name="lua_todo_comment_preserved",
            ext=".lua",
            input="-- TODO keep this one\nlocal x = 1\n",
            expected="-- TODO keep this one\nlocal x = 1\n",
            removed=0,
        ),
        dict(
            name="lua_block_comment_removed",
            ext=".lua",
            input="--[[ block comment\nspanning lines ]]\nlocal y = 2\n",
            expected="local y = 2\n",
            removed=1,
        ),
        dict(
            name="lua_marker_inside_string_not_stripped",
            ext=".lua",
            input='local url = "http://example.com"\n',
            expected='local url = "http://example.com"\n',
            removed=0,
        ),
        dict(
            name="py_line_comment_removed",
            ext=".py",
            input="# header\nx = 1\n",
            expected="x = 1\n",
            removed=1,
        ),
        dict(
            name="py_marker_inside_string_not_stripped",
            ext=".py",
            input='s = "not a # comment"\n',
            expected='s = "not a # comment"\n',
            removed=0,
        ),
        dict(
            name="py_marker_inside_triple_quoted_string_not_stripped",
            ext=".py",
            input='doc = """has a # inside"""\nx = 1\n',
            expected='doc = """has a # inside"""\nx = 1\n',
            removed=0,
        ),
        dict(
            name="gitignore_comment_removed_todo_kept",
            ext=".gitignore",
            input="# real comment\nnode_modules/\n*.log # trailing\n# TODO keep\ndist/\n",
            expected="node_modules/\n*.log\n# TODO keep\ndist/\n",
            removed=2,
        ),
        dict(
            name="css_block_comment_removed",
            ext=".css",
            input="/* comment */\nbody { color: red; }\n",
            expected="body { color: red; }\n",
            removed=1,
        ),
        dict(
            name="yaml_comment_removed_todo_kept",
            ext=".yaml",
            input="# drop this\nkey: value\n# TODO revisit\nother: 1\n",
            expected="key: value\n# TODO revisit\nother: 1\n",
            removed=1,
        ),
        dict(
            name="sql_line_and_block_comments_removed",
            ext=".sql",
            input="-- drop me\nSELECT 1; /* also drop */\n",
            expected="SELECT 1;\n",
            removed=2,
        ),
        dict(
            name="html_block_comment_removed",
            ext=".html",
            input="<!-- comment -->\n<p>hi</p>\n",
            expected="<p>hi</p>\n",
            removed=1,
        ),
        dict(
            name="no_comments_unchanged",
            ext=".js",
            input="const x = 1;\n",
            expected="const x = 1;\n",
            removed=0,
        ),
        dict(
            name="empty_file_unchanged",
            ext=".js",
            input="",
            expected="",
            removed=0,
        ),
        dict(
            name="js_escaped_quote_inside_string_not_stripped",
            ext=".js",
            input='const s = "she said \\"// not real\\" ok"; // real one\n',
            expected='const s = "she said \\"// not real\\" ok";\n',
            removed=1,
        ),
        dict(
            name="lua_escaped_quote_inside_string_not_stripped",
            ext=".lua",
            input='local s = "quote \\" here -- not real"\n',
            expected='local s = "quote \\" here -- not real"\n',
            removed=0,
        ),
        dict(
            name="js_todo_with_parens_and_name_preserved",
            ext=".js",
            input="doStuff(); // TODO(alice): revisit this\n",
            expected="doStuff(); // TODO(alice): revisit this\n",
            removed=0,
        ),
        dict(
            name="js_todo_substring_not_at_start_is_stripped",
            ext=".js",
            input="doStuff(); // this is not a todo, remove me\n",
            expected="doStuff();\n",
            removed=1,
        ),
        dict(
            name="js_block_comment_todo_only_exempt_on_opening_line",
            ext=".js",
            input="/* explanation\n   TODO revisit */\nx = 1;\n",
            expected="x = 1;\n",
            removed=1,
        ),
        dict(
            name="js_unclosed_block_comment_consumes_to_eof",
            ext=".js",
            input="const x = 1;\n/* oops never closed\nmore text\n",
            expected="const x = 1;\n",
            removed=1,
        ),
        dict(
            name="js_multiple_comment_only_lines_mixed_with_todo",
            ext=".js",
            input="// one\n// TODO two (kept)\n// three\ncode();\n",
            expected="// TODO two (kept)\ncode();\n",
            removed=2,
        ),
        dict(
            name="js_no_trailing_newline_trailing_comment_stripped",
            ext=".js",
            input="const x = 1; // comment",
            expected="const x = 1;",
            removed=1,
        ),
        dict(
            name="js_no_trailing_newline_comment_only_last_line_deleted",
            ext=".js",
            input="code();\n// trailing comment",
            expected="code();\n",
            removed=1,
        ),
        dict(
            name="lua_triple_dash_doc_comment_is_line_comment_not_block",
            ext=".lua",
            input="--- doc comment\nlocal x = 1\n",
            expected="local x = 1\n",
            removed=1,
        ),
        dict(
            name="go_backtick_raw_string_marker_not_stripped",
            ext=".go",
            input="s := `has // inside raw string`\n",
            expected="s := `has // inside raw string`\n",
            removed=0,
        ),
        dict(
            name="rb_line_comment_removed_todo_kept",
            ext=".rb",
            input="# drop\nx = 1\n# TODO keep\ny = 2\n",
            expected="x = 1\n# TODO keep\ny = 2\n",
            removed=1,
        ),
    ]

    def test_strip_comments_table(self):
        for case in self.CASES:
            with self.subTest(case=case["name"]):
                cfg = hook.config_for("sample" + case["ext"]) or hook.BASENAME_CONFIGS.get(case["ext"])
                self.assertIsNotNone(cfg, f"no config resolved for {case['ext']!r}")
                actual, removed = hook.strip_comments(case["input"], cfg)
                self.assertEqual(actual, case["expected"])
                self.assertEqual(removed, case["removed"])


class ConfigForTableTest(unittest.TestCase):
    """Table-driven: file_path -> whether a comment-syntax config is resolved."""

    CASES = [
        ("main.lua", True),
        ("src/App.tsx", True),
        ("styles/site.css", True),
        ("docs/notes.md", False),
        ("data.json", False),
        (".gitignore", True),
        ("Readme.MD", False),
        ("main.LUA", True),
        ("Makefile", False),
        (".GITIGNORE", False),  # basename match is case-sensitive: only ".gitignore" resolves
        ("archive.tar.gz", False),
    ]

    def test_config_for_table(self):
        for file_path, expect_resolved in self.CASES:
            with self.subTest(file_path=file_path):
                cfg = hook.config_for(file_path)
                self.assertEqual(cfg is not None, expect_resolved)


class MainHookIntegrationTest(unittest.TestCase):
    """End-to-end: invoke the hook as Claude Code would, via stdin JSON + subprocess."""

    def run_hook(self, payload):
        return subprocess.run(
            [sys.executable, HOOK_PATH],
            input=json.dumps(payload),
            capture_output=True,
            text=True,
            check=False,
        )

    def write_temp(self, name, content):
        path = os.path.join(self.tmpdir, name)
        with open(path, "w", encoding="utf-8") as f:
            f.write(content)
        return path

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmpdir = self._tmp.name

    def tearDown(self):
        self._tmp.cleanup()

    def test_strips_comments_from_written_file(self):
        path = self.write_temp("main.lua", "-- drop\nlocal x = 1\n")
        result = self.run_hook(
            {"tool_name": "Write", "tool_input": {"file_path": path, "content": "ignored"}}
        )
        self.assertEqual(result.returncode, 0)
        with open(path, encoding="utf-8") as f:
            self.assertEqual(f.read(), "local x = 1\n")

    def test_non_write_tool_is_ignored(self):
        path = self.write_temp("main.lua", "-- drop\nlocal x = 1\n")
        with open(path, encoding="utf-8") as f:
            original = f.read()
        result = self.run_hook(
            {"tool_name": "Edit", "tool_input": {"file_path": path, "content": "ignored"}}
        )
        self.assertEqual(result.returncode, 0)
        with open(path, encoding="utf-8") as f:
            self.assertEqual(f.read(), original)

    def test_unsupported_extension_is_left_untouched(self):
        path = self.write_temp("notes.md", "<!-- kept as-is, no config for .md -->\n")
        with open(path, encoding="utf-8") as f:
            original = f.read()
        result = self.run_hook(
            {"tool_name": "Write", "tool_input": {"file_path": path, "content": "ignored"}}
        )
        self.assertEqual(result.returncode, 0)
        with open(path, encoding="utf-8") as f:
            self.assertEqual(f.read(), original)

    def test_missing_file_path_does_not_crash(self):
        result = self.run_hook({"tool_name": "Write", "tool_input": {}})
        self.assertEqual(result.returncode, 0)

    def test_malformed_json_does_not_crash(self):
        result = subprocess.run(
            [sys.executable, HOOK_PATH],
            input="not json",
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
