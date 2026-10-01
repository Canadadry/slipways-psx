from collections import defaultdict
from dataclasses import dataclass, field

from ast_rename import analysis, lexer


@dataclass
class Identity:
    usr: str
    kind: str
    name: str
    parent: str
    file: str
    line: int
    linkage: str
    scope_lines: int
    files: set = field(default_factory=set)  # every file that declares it
    define_count: int = 0  # #define lines of a macro in its file


@dataclass(frozen=True)
class Occurrence:
    usr: str  # None: a declaration outside the project, kept so its tokens count
    file: str
    offset: int
    length: int
    origin: str


@dataclass
class Model:
    identities: dict = field(default_factory=dict)  # usr -> Identity
    # (file, offset) -> Occurrence
    occurrences: dict = field(default_factory=dict)
    # (file, offset) -> set of usr claiming the position
    conflicts: dict = field(default_factory=dict)
    # (usr, file, offset, reason): an occurrence whose token was not found
    unresolved: set = field(default_factory=set)
    files: set = field(default_factory=set)
    # analysis.Unparsed: units failing in every configuration, then in some only
    unparsed: list = field(default_factory=list)
    partial: list = field(default_factory=list)


class Builder:
    def __init__(self, sources):
        self.sources = sources
        self.model = Model()
        self._decls = defaultdict(list)

    def add_unit(self, records):
        self.model.files |= records.files
        for decl in records.decls:
            self._decls[decl.usr].append(decl)
        pending = defaultdict(set)  # (expansion, name) -> usrs
        for raw in records.occurrences:
            if raw.file is None:
                continue
            if self._spelled_at(raw.file, raw.offset, raw.name):
                self._add(raw.usr, raw.file, raw.offset, len(raw.name),
                          self._origin(raw))
            elif raw.expansion is not None:
                pending[(raw.expansion, raw.name)].add(raw.usr)
            elif raw.usr is not None:
                self._unresolved(raw.usr, raw.file, raw.offset, "token is not %s" % raw.name)
        for (expansion, name), usrs in sorted(pending.items(), key=_pending_key):
            self._resolve_macro_body(records, expansion, name, usrs)

    def _spelled_at(self, relative, offset, name):
        token = self.sources.token_at(relative, offset)
        return token is not None and token.text == name

    def _origin(self, raw):
        token = self.sources.token_at(raw.file, raw.offset)
        if raw.origin in (analysis.MACRO_DEF, analysis.MACRO_USE):
            return raw.origin
        if token.role == lexer.DEFINE_BODY:
            return analysis.MACRO_BODY
        if raw.expansion is not None and raw.expansion != (raw.file, raw.offset):
            return analysis.MACRO_ARG
        return raw.origin

    def _resolve_macro_body(self, records, expansion, name, usrs):
        """libclang puts a name written in a #define body at the expansion point
        of the outermost macro. Look in that macro's body and the project macros
        it expands; then in the call's arguments, when the name went through a
        foreign macro; then in the bodies of the project macros called in
        those arguments (`assert(CELLS > 0)`)."""
        relative, offset = expansion
        definition = records.expansions.get(expansion)
        candidates, origin = [], analysis.MACRO_BODY
        if definition is not None:
            candidates = self._body_tokens(records, definition, name, set())
        end = records.expansion_ends.get(expansion)
        if not candidates and end is not None:
            candidates = [(relative, t.offset) for t in self.sources.tokens(relative)
                          if offset < t.offset < end and t.text == name
                          and t.role == lexer.CODE]
            origin = analysis.MACRO_ARG
            if not candidates:
                origin = analysis.MACRO_BODY
                seen = set()
                for (inner_file, inner_offset), inner in sorted(records.expansions.items()):
                    if inner_file == relative and offset < inner_offset < end:
                        candidates += self._body_tokens(records, inner, name, seen)
        if not candidates:
            for usr in usrs:
                self._unresolved(usr, relative, offset, "%s not found in the macro" % name)
            return
        for usr in sorted(usrs, key=str):
            for candidate_file, candidate_offset in candidates:
                self._add(usr, candidate_file, candidate_offset, len(name), origin)

    def _unresolved(self, usr, relative, offset, reason):
        """A foreign declaration expanded from a foreign macro is not ours to
        find."""
        if usr is not None:
            self.model.unresolved.add((usr, relative, offset, reason))

    def _body_tokens(self, records, definition, name, seen):
        if definition in seen:
            return []
        seen.add(definition)
        relative, define_offset = definition
        body = [t for t in self.sources.tokens(relative)
                if t.define_offset == define_offset and t.role == lexer.DEFINE_BODY
                and t.kind == lexer.IDENT]
        found = [(relative, t.offset) for t in body if t.text == name]
        if found:
            return found
        for token in body:
            nested = records.macros.get(token.text)
            if nested is not None:
                found = self._body_tokens(records, nested, name, seen)
                if found:
                    return found
        return []

    def _add(self, usr, relative, offset, length, origin):
        key = (relative, offset)
        current = self.model.occurrences.get(key)
        if current is None:
            self.model.occurrences[key] = Occurrence(usr, relative, offset, length, origin)
        elif current.usr != usr:
            self.model.conflicts.setdefault(key, {current.usr}).add(usr)

    def finish(self):
        """Identities from the declarations, then the directive tokens libclang
        does not report: every #define line of a project macro, and its names in
        #if lines and in other #define bodies."""
        for usr, decls in self._decls.items():
            primary = min(decls, key=lambda d: (not d.is_definition, d.file, d.line))
            self.model.identities[usr] = Identity(
                usr, primary.kind, primary.name, primary.parent, primary.file,
                primary.line, primary.linkage, primary.scope_lines,
                {d.file for d in decls})
        macros_by_name = defaultdict(list)
        for identity in self.model.identities.values():
            if identity.kind == "macro":
                macros_by_name[identity.name].append(identity)
        for relative in sorted(self.model.files):
            for token in self.sources.tokens(relative):
                if token.kind != lexer.IDENT:
                    continue
                if token.role == lexer.DEFINE_NAME:
                    usr = analysis.macro_usr(relative, token.text)
                    if usr in self.model.identities:
                        self.model.identities[usr].define_count += 1
                        self._add(usr, relative, token.offset, len(token.text),
                                  analysis.MACRO_DEF)
                elif token.role in (lexer.CONDITION, lexer.DEFINE_BODY):
                    owners = macros_by_name.get(token.text, [])
                    if len(owners) == 1 and (relative, token.offset) not in self.model.occurrences:
                        origin = (analysis.MACRO_COND if token.role == lexer.CONDITION
                                  else analysis.MACRO_BODY)
                        self._add(owners[0].usr, relative, token.offset,
                                  len(token.text), origin)
        return self.model


def _pending_key(item):
    (expansion, name), _ = item
    return (expansion, name)
