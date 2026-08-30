#!/usr/bin/env python3
"""
PostToolUse hook (matcher: Write): strips comments Claude just wrote to a
file, mirroring step 5 of the git-commit skill but applied immediately at
write time instead of at commit time.

Rules:
- Comment syntax is picked per file extension (see CONFIGS below).
- A comment whose text starts with "TODO" (case-insensitive, after skipping
  leading whitespace) is left untouched.
- A comment-only line is deleted entirely; a trailing comment on a code line
  has just the comment portion removed.
- String literals are tracked so `//`, `--`, `#`, etc. inside strings are
  never mistaken for comment markers.
"""
import json
import os
import sys

SQ = {"open": "'", "close": "'", "multiline": False, "escape": True}
DQ = {"open": '"', "close": '"', "multiline": False, "escape": True}
BACKTICK = {"open": "`", "close": "`", "multiline": True, "escape": True}
PY_TRIPLE_S = {"open": "'''", "close": "'''", "multiline": True, "escape": True}
PY_TRIPLE_D = {"open": '"""', "close": '"""', "multiline": True, "escape": True}
LUA_LONG_STR = {"open": "[[", "close": "]]", "multiline": True, "escape": False}

CONFIGS = {
    ".lua": dict(line=["--"], block=[("--[[", "]]")], strings=[SQ, DQ, LUA_LONG_STR]),
    ".js": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ, BACKTICK]),
    ".jsx": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ, BACKTICK]),
    ".ts": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ, BACKTICK]),
    ".tsx": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ, BACKTICK]),
    ".mjs": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ, BACKTICK]),
    ".cjs": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ, BACKTICK]),
    ".java": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".c": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".h": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".cpp": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".hpp": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".cc": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".cs": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".go": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ, BACKTICK]),
    ".rs": dict(line=["//"], block=[("/*", "*/")], strings=[DQ]),
    ".swift": dict(line=["//"], block=[("/*", "*/")], strings=[DQ]),
    ".kt": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".php": dict(line=["//", "#"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".css": dict(line=[], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".scss": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".less": dict(line=["//"], block=[("/*", "*/")], strings=[SQ, DQ]),
    ".py": dict(line=["#"], block=[], strings=[PY_TRIPLE_D, PY_TRIPLE_S, SQ, DQ]),
    ".rb": dict(line=["#"], block=[], strings=[SQ, DQ]),
    ".sh": dict(line=["#"], block=[], strings=[SQ, DQ]),
    ".bash": dict(line=["#"], block=[], strings=[SQ, DQ]),
    ".zsh": dict(line=["#"], block=[], strings=[SQ, DQ]),
    ".yaml": dict(line=["#"], block=[], strings=[SQ, DQ]),
    ".yml": dict(line=["#"], block=[], strings=[SQ, DQ]),
    ".toml": dict(line=["#"], block=[], strings=[SQ, DQ]),
    ".sql": dict(line=["--"], block=[("/*", "*/")], strings=[SQ]),
    ".html": dict(line=[], block=[("<!--", "-->")], strings=[SQ, DQ]),
    ".htm": dict(line=[], block=[("<!--", "-->")], strings=[SQ, DQ]),
    ".xml": dict(line=[], block=[("<!--", "-->")], strings=[SQ, DQ]),
    ".svg": dict(line=[], block=[("<!--", "-->")], strings=[SQ, DQ]),
}

BASENAME_CONFIGS = {
    ".gitignore": dict(line=["#"], block=[], strings=[]),
}


def config_for(file_path):
    base = os.path.basename(file_path)
    if base in BASENAME_CONFIGS:
        return BASENAME_CONFIGS[base]
    _, ext = os.path.splitext(base)
    return CONFIGS.get(ext.lower())


def find_spans(content, cfg):
    line_markers = sorted(cfg["line"], key=len, reverse=True)
    block_pairs = cfg["block"]
    strings = cfg["strings"]

    n = len(content)
    i = 0
    spans = []  # (start, end_exclusive, marker_len, is_block)
    active_string = None

    while i < n:
        if active_string is not None:
            close = active_string["close"]
            if content.startswith(close, i):
                i += len(close)
                active_string = None
                continue
            if active_string["escape"] and content[i] == "\\":
                i += 2
                continue
            if not active_string["multiline"] and content[i] == "\n":
                active_string = None
                continue
            i += 1
            continue

        started = False
        for spec in strings:
            if content.startswith(spec["open"], i):
                active_string = spec
                i += len(spec["open"])
                started = True
                break
        if started:
            continue

        for open_m, close_m in block_pairs:
            if content.startswith(open_m, i):
                end = content.find(close_m, i + len(open_m))
                span_end = n if end == -1 else end + len(close_m)
                spans.append((i, span_end, len(open_m), True))
                i = span_end
                started = True
                break
        if started:
            continue

        for marker in line_markers:
            if content.startswith(marker, i):
                nl = content.find("\n", i)
                span_end = n if nl == -1 else nl
                spans.append((i, span_end, len(marker), False))
                i = span_end
                started = True
                break
        if started:
            continue

        i += 1

    return spans


def is_todo(content, start, end, marker_len):
    after = content[start + marker_len:end]
    first_nl = after.find("\n")
    check_text = after if first_nl == -1 else after[:first_nl]
    return check_text.strip().lower().startswith("todo")


def strip_comments(content, cfg):
    spans = find_spans(content, cfg)
    keep = [
        (start, end)
        for start, end, marker_len, _is_block in spans
        if not is_todo(content, start, end, marker_len)
    ]
    if not keep:
        return content, 0

    result = content
    removed = 0
    for start, end in reversed(keep):
        line_start = result.rfind("\n", 0, start) + 1
        nl_after = result.find("\n", end)
        line_end = nl_after if nl_after != -1 else len(result)
        before = result[line_start:start]
        after = result[end:line_end]
        if before.strip() == "" and after.strip() == "":
            del_end = line_end + 1 if nl_after != -1 else line_end
            result = result[:line_start] + result[del_end:]
        else:
            result = result[:start] + result[end:]
            if (
                0 < start < len(result)
                and result[start - 1] in " \t"
                and result[start] in " \t"
            ):
                result = result[:start] + result[start + 1:]
            ls = result.rfind("\n", 0, start) + 1
            nl2 = result.find("\n", ls)
            le = nl2 if nl2 != -1 else len(result)
            line = result[ls:le]
            stripped = line.rstrip()
            if stripped != line:
                result = result[:ls] + stripped + result[le:]
        removed += 1
    return result, removed


def main():
    try:
        payload = json.load(sys.stdin)
    except (json.JSONDecodeError, ValueError):
        return 0

    if payload.get("tool_name") != "Write":
        return 0

    file_path = (payload.get("tool_input") or {}).get("file_path")
    if not file_path:
        return 0

    cfg = config_for(file_path)
    if cfg is None:
        return 0

    try:
        with open(file_path, "r", encoding="utf-8") as f:
            original = f.read()
    except (OSError, UnicodeDecodeError):
        return 0

    new_content, removed = strip_comments(original, cfg)
    if removed and new_content != original:
        with open(file_path, "w", encoding="utf-8") as f:
            f.write(new_content)
        print(
            f"{removed} commentaire(s) retire(s) de {file_path} (hook strip-claude-comments)",
            file=sys.stderr,
        )

    return 0


if __name__ == "__main__":
    sys.exit(main())
