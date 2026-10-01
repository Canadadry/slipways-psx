"""Raw C tokens of a file, inactive `#if` branches included, without the
preprocessor: what libclang does not report (directive lines, macro bodies never
expanded) is read from here."""

import re
from dataclasses import dataclass

CODE = "code"
DEFINE_NAME = "define_name"
DEFINE_BODY = "define_body"
MACRO_PARAM = "macro_param"
CONDITION = "condition"
OTHER_DIRECTIVE = "other_directive"

IDENT = "ident"
PUNCT = "punct"


@dataclass(frozen=True)
class Token:
    kind: str
    text: str
    offset: int
    role: str
    # The macro whose `#define` line holds the token, for DEFINE_* and MACRO_PARAM.
    macro: str = ""
    define_offset: int = -1


_IDENT_RE = re.compile(rb"[A-Za-z_][A-Za-z0-9_]*")
_NUMBER_RE = re.compile(rb"\.?[0-9](?:[eEpP][+-]|[A-Za-z0-9_.])*")
_PUNCT_RE = re.compile(rb"##|#|->|\+\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\|\||\.\.\.|[-+*/%&|^!~=<>?:;,.()\[\]{}]")
_CONDITIONS = {"if", "ifdef", "ifndef", "elif", "undef", "elifdef", "elifndef"}


def tokenize(data):
    """Identifier and punctuation tokens of `data` (bytes); comments, strings,
    character constants, numbers and `#include` operands are skipped."""
    tokens = []
    i, n = 0, len(data)
    at_line_start = True
    directive = None  # name of the directive being read, None outside one
    define = None  # (macro name, offset of the name) inside a #define
    define_params = set()
    define_stage = None  # "name", "params?", "params", "body"
    while i < n:
        c = data[i:i + 1]
        if c == b"\\" and data[i + 1:i + 2] == b"\n":
            i += 2
            continue
        if c == b"\n":
            i += 1
            at_line_start = True
            directive, define, define_stage = None, None, None
            continue
        if c in b" \t\r\f\v":
            i += 1
            continue
        if data.startswith(b"/*", i):
            end = data.find(b"*/", i + 2)
            i = n if end < 0 else end + 2
            continue
        if data.startswith(b"//", i):
            while i < n and data[i:i + 1] != b"\n":
                i += 2 if data[i:i + 2] == b"\\\n" else 1
            continue
        if c in (b'"', b"'"):
            i = _skip_quoted(data, i, c)
            at_line_start = False
            if define_stage == "params?":
                define_stage = "body"
            continue
        if c == b"#" and at_line_start and directive is None and data[i:i + 2] != b"##":
            directive = ""
            at_line_start = False
            i += 1
            continue
        at_line_start = False
        match = _IDENT_RE.match(data, i)
        if match:
            text = match.group().decode()
            start = i
            i = match.end()
            if directive == "":
                directive = text
                if text == "include" or text == "include_next":
                    i = _end_of_line(data, i)
                elif text == "define":
                    define_stage = "name"
                continue
            if directive is None:
                tokens.append(Token(IDENT, text, start, CODE))
            elif directive == "define":
                if define_stage == "name":
                    define = (text, start)
                    define_params = set()
                    tokens.append(Token(IDENT, text, start, DEFINE_NAME, text, start))
                    define_stage = "params?" if data[i:i + 1] == b"(" else "body"
                elif define_stage == "params":
                    define_params.add(text)
                    tokens.append(Token(IDENT, text, start, MACRO_PARAM, define[0], define[1]))
                else:
                    role = MACRO_PARAM if text in define_params else DEFINE_BODY
                    tokens.append(Token(IDENT, text, start, role, define[0], define[1]))
            elif directive in _CONDITIONS:
                tokens.append(Token(IDENT, text, start, CONDITION))
            else:
                tokens.append(Token(IDENT, text, start, OTHER_DIRECTIVE))
            continue
        if c.isdigit() or (c == b"." and data[i + 1:i + 2].isdigit()):
            i = _NUMBER_RE.match(data, i).end()
            continue
        match = _PUNCT_RE.match(data, i)
        if match:
            text = match.group().decode()
            if directive == "define":
                if define_stage == "params?" and text == "(":
                    define_stage = "params"
                elif define_stage == "params" and text == ")":
                    define_stage = "body"
                    i = match.end()
                    continue
                elif define_stage == "params?":
                    define_stage = "body"
                if define_stage == "body":
                    tokens.append(Token(PUNCT, text, i, DEFINE_BODY, define[0], define[1]))
            i = match.end()
            continue
        i += 1  # stray byte (UTF-8 outside a literal, `@`, `$`)
    return tokens


def _skip_quoted(data, i, quote):
    i += 1
    n = len(data)
    while i < n:
        c = data[i:i + 1]
        if c == b"\\":
            i += 2
        elif c == quote or c == b"\n":
            return i + 1
        else:
            i += 1
    return i


def _end_of_line(data, i):
    n = len(data)
    while i < n and data[i:i + 1] != b"\n":
        i += 2 if data[i:i + 2] == b"\\\n" else 1
    return i
