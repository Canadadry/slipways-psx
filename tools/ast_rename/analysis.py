import os
import shlex
import sys
from dataclasses import dataclass, field

from ast_rename import bindings, lexer

DECL = "decl"
REF = "ref"
TYPE_REF = "type_ref"
DESIGNATOR = "designator"
MACRO_ARG = "macro_arg"
MACRO_BODY = "macro_body"
MACRO_DEF = "macro_def"
MACRO_USE = "macro_use"
MACRO_COND = "macro_cond"


@dataclass(frozen=True)
class Decl:
    usr: str
    kind: str
    name: str
    parent: str
    file: str
    line: int
    linkage: str
    scope_lines: int  # -1 when the kind has no scope
    is_definition: bool


@dataclass(frozen=True)
class RawOccurrence:
    usr: str  # None for a declaration outside the project
    name: str
    file: str
    offset: int  # spelling location; the expansion point for a #define body
    origin: str
    expansion: tuple  # (file, offset) of the macro expansion point


@dataclass
class UnitRecords:
    decls: list = field(default_factory=list)
    occurrences: list = field(default_factory=list)
    # (file, offset of the macro name at the expansion point) -> (file, offset
    # of the macro name in its #define), project macros only.
    expansions: dict = field(default_factory=dict)
    # (file, offset) of any macro expansion point -> offset where its call ends.
    expansion_ends: dict = field(default_factory=dict)
    # macro name -> (file, offset of the name in its #define), project macros.
    macros: dict = field(default_factory=dict)
    files: set = field(default_factory=set)


@dataclass(frozen=True)
class Unparsed:
    unit: str
    config: str
    message: str


class Project:
    """Paths are kept relative to the root; a file outside the root or under an
    excluded directory is not project code."""

    def __init__(self, root, exclude_dirs):
        self.root = os.path.abspath(root)
        self.excludes = [os.path.normpath(d) for d in exclude_dirs]
        self._cache = {}

    def relative(self, path):
        """Root-relative path of a project file, or None."""
        if path in self._cache:
            return self._cache[path]
        relative = os.path.relpath(os.path.abspath(path), self.root)
        if relative.startswith("..") or any(
                relative == d or relative.startswith(d + os.sep) for d in self.excludes):
            relative = None
        self._cache[path] = relative
        return relative

    def absolute(self, relative):
        return os.path.join(self.root, relative)


class Sources:

    def __init__(self, project):
        self.project = project
        self._bytes = {}
        self._tokens = {}
        self._by_offset = {}

    def data(self, relative):
        if relative not in self._bytes:
            with open(self.project.absolute(relative), "rb") as f:
                self._bytes[relative] = f.read()
        return self._bytes[relative]

    def tokens(self, relative):
        if relative not in self._tokens:
            self._tokens[relative] = lexer.tokenize(self.data(relative))
            self._by_offset[relative] = {t.offset: t for t in self._tokens[relative]}
        return self._tokens[relative]

    def token_at(self, relative, offset):
        self.tokens(relative)
        return self._by_offset[relative].get(offset)


_DECL_KINDS = {
    "TYPEDEF_DECL": "type",
    "STRUCT_DECL": "tag",
    "UNION_DECL": "tag",
    "ENUM_DECL": "tag",
    "FIELD_DECL": "field",
    "ENUM_CONSTANT_DECL": "enum_constant",
    "FUNCTION_DECL": "function",
    "VAR_DECL": "variable",
    "PARM_DECL": "param",
}
_REF_KINDS = {
    "DECL_REF_EXPR": REF,
    "MEMBER_REF_EXPR": REF,
    "TYPE_REF": TYPE_REF,
    "MEMBER_REF": DESIGNATOR,
    "MACRO_INSTANTIATION": MACRO_USE,
}
_SCOPE_KINDS = {"COMPOUND_STMT", "FOR_STMT"}
_TAG_KINDS = {"STRUCT_DECL", "UNION_DECL", "ENUM_DECL"}


def _is_keyword_at(sources, relative, offset):
    token = sources.token_at(relative, offset)
    return token is not None and token.text in ("struct", "union", "enum")


def split_flags(text):
    """Compiler flags for libclang; -Werror is dropped, a warning is not a
    failure to parse."""
    return [flag for flag in shlex.split(text) if not flag.startswith("-Werror")]


def parse_unit(cindex, index, unit, flags, on_demand):
    """(translation unit, None) or (None, first error message)."""
    tu, error = _parse(cindex, index, unit, flags)
    if error is None:
        return tu, None
    for header, directory in on_demand:
        if "'%s' file not found" % header in error:
            tu, error = _parse(cindex, index, unit, flags + ["-idirafter", directory])
            break
    return (tu, None) if error is None else (None, error)


def _parse(cindex, index, unit, flags):
    try:
        tu = index.parse(unit, args=flags,
                         options=cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
    except cindex.TranslationUnitLoadError as error:
        return None, "%s: %s" % (unit, error)
    for diagnostic in tu.diagnostics:
        if diagnostic.severity >= cindex.Diagnostic.Error:
            location = diagnostic.location
            where = "%s:%d" % (location.file.name, location.line) if location.file else unit
            return None, "%s: %s" % (where, diagnostic.spelling)
    return tu, None


def collect(cindex, tu, sources):
    project = sources.project
    records = UnitRecords()
    sys.setrecursionlimit(max(sys.getrecursionlimit(), 20000))
    for cursor in tu.cursor.get_children():
        location_file = cursor.location.file
        if location_file is None:
            continue
        relative = project.relative(location_file.name)
        if relative is None:
            continue
        _visit(cindex, cursor, [], sources, records)
    for include in tu.get_includes():
        relative = project.relative(include.include.name)
        if relative is not None:
            records.files.add(relative)
    records.files.add(project.relative(tu.spelling))
    return records


def _position(cindex, project, location):
    name, offset = bindings.spelling_location(cindex, location)
    if name is None:
        return None, 0
    return project.relative(name), offset


def _expansion(project, location):
    if location.file is None:
        return None
    relative = project.relative(location.file.name)
    return None if relative is None else (relative, location.offset)


def _visit(cindex, cursor, ancestors, sources, records):
    project = sources.project
    kind = cursor.kind.name
    if kind == "MACRO_DEFINITION":
        _macro_definition(cindex, cursor, project, records)
        return
    if kind in _DECL_KINDS:
        _declaration(cindex, cursor, ancestors, sources, records)
    if kind in _REF_KINDS:
        _reference(cindex, cursor, project, records)
    ancestors.append(cursor)
    for child in cursor.get_children():
        _visit(cindex, child, ancestors, sources, records)
    ancestors.pop()


def _macro_definition(cindex, cursor, project, records):
    relative, offset = _position(cindex, project, cursor.location)
    if relative is None:
        return
    name = cursor.spelling
    records.macros[name] = (relative, offset)
    usr = macro_usr(relative, name)
    records.decls.append(Decl(usr, "macro", name, "-", relative, cursor.location.line,
                              "-", -1, True))
    records.occurrences.append(RawOccurrence(usr, name, relative, offset, MACRO_DEF,
                                             (relative, offset)))


def macro_usr(relative, name):
    """Identity of a macro: all its #define lines in one file are one macro."""
    return "macro:%s:%s" % (relative, name)


def _declaration(cindex, cursor, ancestors, sources, records):
    project = sources.project
    name = cursor.spelling
    if not name or not name.isidentifier() or cursor.is_anonymous():
        return
    relative, offset = _position(cindex, project, cursor.location)
    if relative is None:
        return
    if cursor.kind.name in _TAG_KINDS and _is_keyword_at(sources, relative, offset):
        return  # `typedef struct { ... } name_t`: the tag has no name of its own
    kind, linkage, parent, scope = _classify(cursor, ancestors)
    usr = cursor.get_usr()
    records.decls.append(Decl(usr, kind, name, parent, relative, cursor.location.line,
                              linkage, scope, cursor.is_definition()))
    records.occurrences.append(RawOccurrence(usr, name, relative, offset, DECL,
                                             _expansion(project, cursor.location)))


def _classify(cursor, ancestors):
    """(kind, linkage, parent, scope lines) of a declaration cursor."""
    kind = _DECL_KINDS[cursor.kind.name]
    function = next((a for a in reversed(ancestors) if a.kind.name == "FUNCTION_DECL"), None)
    if kind in ("type", "tag"):
        return kind, "-", "-", -1
    if kind == "field":
        return kind, "-", _parent_name(cursor.semantic_parent), -1
    if kind == "enum_constant":
        return kind, "-", _parent_name(cursor.semantic_parent), -1
    if kind == "function":
        return kind, _linkage(cursor), "-", -1
    if kind == "param":
        owner = function if function is not None else cursor.semantic_parent
        parent = owner.spelling if owner is not None else "-"
        end = owner.extent.end.line if owner is not None else cursor.location.line
        return kind, "local", parent or "-", end - cursor.location.line + 1
    if function is None:
        linkage = _linkage(cursor)
        return ("global" if linkage == "extern" else "static"), linkage, "-", -1
    scope = next((a for a in reversed(ancestors) if a.kind.name in _SCOPE_KINDS), function)
    span = scope.extent.end.line - cursor.location.line + 1
    is_static = cursor.storage_class.name == "STATIC"
    return ("static" if is_static else "local"), "local", function.spelling, span


def _parent_name(cursor):
    if cursor is None or not cursor.spelling.isidentifier():
        return "(anonymous)"
    return cursor.spelling


def _linkage(cursor):
    name = cursor.linkage.name
    if name in ("EXTERNAL", "UNIQUE_EXTERNAL"):
        return "extern"
    if name == "INTERNAL":
        return "static"
    return "local"


def _reference(cindex, cursor, project, records):
    expansion = _expansion(project, cursor.location)
    if expansion is None:
        return
    relative, offset = _position(cindex, project, cursor.location)
    referenced = cursor.referenced
    if cursor.kind.name == "MACRO_INSTANTIATION":
        if referenced is None or referenced.location.file is None:
            return
        def_relative, def_offset = _position(cindex, project, referenced.location)
        usr = None
        records.expansion_ends[expansion] = cursor.extent.end.offset
        if def_relative is not None:
            records.expansions[expansion] = (def_relative, def_offset)
            usr = macro_usr(def_relative, referenced.spelling)
        records.occurrences.append(RawOccurrence(usr, cursor.spelling, relative, offset,
                                                 MACRO_USE, expansion))
        return
    if relative is None or referenced is None:
        return
    name = referenced.spelling
    if not name or not name.isidentifier():
        return
    usr = referenced.get_usr()
    location = referenced.location
    if location.file is None or project.relative(location.file.name) is None:
        usr = None
    records.occurrences.append(RawOccurrence(usr, name, relative, offset,
                                             _REF_KINDS[cursor.kind.name], expansion))
