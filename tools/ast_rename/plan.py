"""Rename table + analysis model -> checked rewrites, or the reasons to refuse.
Touches no file: the caller passes the bytes of every file to check."""

import re
from collections import Counter, defaultdict
from dataclasses import dataclass, field

from ast_rename import lexer

C_KEYWORDS = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
    "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long",
    "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct",
    "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Bool",
    "_Complex", "_Imaginary", "_Alignas", "_Alignof", "_Atomic", "_Generic",
    "_Noreturn", "_Static_assert", "_Thread_local",
}
_IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")


@dataclass(frozen=True)
class Entry:
    old: str
    new: str
    file: str
    identity: str  # "" when the table leaves it out
    line: int  # in the table, for messages


@dataclass(frozen=True)
class Rewrite:
    file: str
    offset: int
    old: str
    new: str
    origin: str


@dataclass
class Plan:
    rewrites: list = field(default_factory=list)
    errors: list = field(default_factory=list)
    warnings: list = field(default_factory=list)
    report: list = field(default_factory=list)


def read_table(text):
    """Entries of a TSV table: old, new, declaration file[, identity]. Blank
    lines, `#` lines and a header line are skipped."""
    entries = []
    for number, line in enumerate(text.splitlines(), 1):
        if not line.strip() or line.startswith("#"):
            continue
        columns = line.split("\t")
        if columns[:3] in (["old", "new", "file"], ["ancien", "nouveau", "fichier"]):
            continue
        if len(columns) not in (3, 4):
            raise ValueError("table line %d: expected 3 or 4 columns" % number)
        entries.append(Entry(columns[0], columns[1], columns[2],
                             columns[3] if len(columns) == 4 else "", number))
    return entries


def build(entries, model, files):
    """`files`: path -> bytes for every file that must be checked (the files the
    analysis saw and those of units that did not parse)."""
    plan = Plan()
    renamed = _resolve(entries, model, plan)
    _check_names(renamed, model, plan)
    occurrences = defaultdict(list)
    for occurrence in model.occurrences.values():
        if occurrence.usr in renamed:
            occurrences[occurrence.usr].append(occurrence)
    _check_model_problems(renamed, model, plan)
    _check_positions(renamed, occurrences, files, plan)
    tokens = {path: lexer.tokenize(data) for path, data in sorted(files.items())}
    _check_pasting(renamed, model, tokens, files, plan)
    _check_unparsed(renamed, model, files, plan)
    _check_unaccounted(renamed, model, tokens, files, plan)
    if plan.errors:
        return plan
    for usr, entry in sorted(renamed.items(), key=lambda item: item[1].line):
        found = sorted(occurrences[usr], key=lambda o: (o.file, o.offset))
        for occurrence in found:
            plan.rewrites.append(Rewrite(occurrence.file, occurrence.offset, entry.old,
                                         entry.new, occurrence.origin))
        plan.report.append(_report_line(entry, found))
    return plan


def _resolve(entries, model, plan):
    """usr -> entry, one identity per entry."""
    renamed = {}
    for entry in entries:
        where = "table line %d: %s" % (entry.line, entry.old)
        if not _IDENTIFIER.match(entry.new) or entry.new in C_KEYWORDS:
            plan.errors.append("%s: new name %r is not a C identifier" % (where, entry.new))
            continue
        if entry.new == entry.old:
            plan.errors.append("%s: new name is the old one" % where)
            continue
        matches = [identity for identity in model.identities.values()
                   if identity.name == entry.old and entry.file in identity.files
                   and (not entry.identity or identity.usr == entry.identity)]
        if not matches:
            plan.errors.append("%s: no declaration in %s%s" % (
                where, entry.file, " with identity " + entry.identity if entry.identity else ""))
        elif len(matches) > 1:
            plan.errors.append("%s: ambiguous in %s, add the identity column: %s" % (
                where, entry.file, ", ".join(sorted(m.usr for m in matches))))
        elif matches[0].usr in renamed:
            plan.errors.append("%s: identity %s already renamed by table line %d" % (
                where, matches[0].usr, renamed[matches[0].usr].line))
        else:
            renamed[matches[0].usr] = entry
    return renamed


def _check_names(renamed, model, plan):
    """The new name must not meet another declaration where both are visible:
    a macro of a header meets every name, a macro of a .c file the names of a
    header, of external linkage or of its file; file-scope names meet each other when one is
    shared or both are in the same file (an enum constant or a type of a .c file
    is unseen elsewhere), and meet the locals they would shadow; fields
    meet the fields of their struct; locals the locals of their function whose
    scope overlaps theirs, and the file-scope names of a header, of external linkage or of their file."""
    final = {}
    for usr, identity in model.identities.items():
        final[usr] = renamed[usr].new if usr in renamed else identity.name
    by_name = defaultdict(list)
    for usr, name in final.items():
        by_name[name].append(model.identities[usr])
    for usr, entry in sorted(renamed.items(), key=lambda item: item[1].line):
        identity = model.identities[usr]
        for other in by_name[entry.new]:
            if other.usr != usr and _visible_together(identity, other):
                plan.errors.append("table line %d: %s -> %s: collides with %s %s (%s:%d)" % (
                    entry.line, entry.old, entry.new, other.kind, other.usr, other.file,
                    other.line))


def _visible_together(a, b):
    if a.kind == "macro" or b.kind == "macro":
        macro, other = (a, b) if a.kind == "macro" else (b, a)
        return _shared(macro) or _shared(other) or macro.file == other.file
    if a.kind == "field" or b.kind == "field":
        return a.kind == b.kind and _record_of(a) == _record_of(b)
    if a.kind == "tag" or b.kind == "tag":
        return a.kind == b.kind
    a_local, b_local = _is_local(a), _is_local(b)
    if a_local and b_local:
        return a.parent == b.parent and a.file == b.file and _scopes_overlap(a, b)
    if a_local or b_local:
        outer = b if a_local else a
        local = a if a_local else b
        return _shared(outer) or outer.file == local.file
    if a.linkage == "static" and b.linkage == "static":
        return a.file == b.file
    return _shared(a) or _shared(b) or a.file == b.file


def _scopes_overlap(a, b):
    """Lines from the declaration to its scope end; an unknown scope runs to
    the end of the file (the rule of overlap() in tools/rename_check.py)."""
    def lines(identity):
        end = identity.line + identity.scope_lines if identity.scope_lines >= 0 else 10 ** 9
        return identity.line, end
    (a0, a1), (b0, b1) = lines(a), lines(b)
    return a0 <= b1 and b0 <= a1


def _shared(identity):
    """Seen outside its file: declared in a header, or with external linkage
    (the same rule as tools/rename_check.py)."""
    return identity.file.endswith(".h") or identity.linkage == "extern"


def _is_local(identity):
    return identity.kind in ("local", "param") or (
        identity.kind == "static" and identity.parent != "-")


def _record_of(identity):
    return identity.usr.rsplit("@FI@", 1)[0]


def _check_model_problems(renamed, model, plan):
    for (path, offset), usrs in sorted(model.conflicts.items()):
        for usr in sorted(usrs & renamed.keys()):
            plan.errors.append("%s: position %s:%d is claimed by several declarations: %s" % (
                renamed[usr].old, path, offset, ", ".join(sorted(map(str, usrs)))))
    for usr, path, offset, reason in sorted(model.unresolved, key=str):
        if usr in renamed:
            plan.errors.append("%s: occurrence at %s:%d not found: %s" % (
                renamed[usr].old, path, offset, reason))


def _check_positions(renamed, occurrences, files, plan):
    spans = defaultdict(list)
    for usr, found in occurrences.items():
        old = renamed[usr].old.encode()
        for occurrence in found:
            data = files.get(occurrence.file)
            at = None if data is None else data[occurrence.offset:occurrence.offset + len(old)]
            if at != old:
                plan.errors.append("%s: %s:%d holds %r, not the old name" % (
                    renamed[usr].old, occurrence.file, occurrence.offset, at))
            spans[occurrence.file].append((occurrence.offset, occurrence.offset + len(old)))
    for path, ranges in sorted(spans.items()):
        ranges.sort()
        for (start, end), (next_start, _) in zip(ranges, ranges[1:]):
            if next_start < end:
                plan.errors.append("%s:%d: overlapping rewrites" % (path, next_start))


def _check_pasting(renamed, model, tokens, files, plan):
    """`##` and `#` are not handled: a renamed name next to one in a #define
    body is refused. A renamed name passed as the argument of such a parameter
    is not a reference for clang, so _check_unaccounted refuses it. Exception:
    a name renamed only as a field, pasted outside a member access (`raw##u`),
    cannot name the field; it is left untouched."""
    olds = {entry.old for entry in renamed.values()}
    fields = _field_only_olds(renamed, model)
    for path, file_tokens in tokens.items():
        for index, token in enumerate(file_tokens):
            if token.kind != lexer.IDENT or token.text not in olds \
                    or token.role != lexer.DEFINE_BODY:
                continue
            if token.text in fields and _pasted_outside_member_access(file_tokens, index):
                continue
            before = file_tokens[index - 1] if index else None
            after = file_tokens[index + 1] if index + 1 < len(file_tokens) else None
            if (after is not None and after.define_offset == token.define_offset
                    and after.text == "##") or (
                    before is not None and before.define_offset == token.define_offset
                    and before.text in ("##", "#")):
                plan.errors.append("%s: %s:%d is an operand of # or ## in a #define" % (
                    token.text, path, _line(files[path], token.offset)))


def _check_unparsed(renamed, model, files, plan):
    units = sorted({failure.unit for failure in model.unparsed})
    for unit in units:
        text = files.get(unit, b"")
        hits = [entry.old for entry in renamed.values()
                if re.search(rb"\b%s\b" % re.escape(entry.old.encode()), text)]
        if hits:
            plan.errors.append("%s does not parse and contains %s" % (
                unit, ", ".join(sorted(set(hits)))))
        else:
            plan.warnings.append("%s does not parse; it does not contain any old name" % unit)


def _check_unaccounted(renamed, model, tokens, files, plan):
    """Every token spelled like an old name must be a known occurrence of some
    declaration: otherwise it sits where clang did not look (inactive branch,
    macro never expanded, unit that did not parse)."""
    olds = {entry.old for entry in renamed.values()}
    fields = _field_only_olds(renamed, model)
    for path, file_tokens in tokens.items():
        for index, token in enumerate(file_tokens):
            if token.kind == lexer.IDENT and token.text in olds \
                    and token.role != lexer.MACRO_PARAM \
                    and (path, token.offset) not in model.occurrences \
                    and not (token.text in fields
                             and _pasted_outside_member_access(file_tokens, index)):
                plan.errors.append("%s: %s:%d not accounted for by the analysis" % (
                    token.text, path, _line(files[path], token.offset)))


def _field_only_olds(renamed, model):
    kinds = defaultdict(set)
    for usr, entry in renamed.items():
        kinds[entry.old].add(model.identities[usr].kind)
    return {old for old, found in kinds.items() if found == {"field"}}


def _pasted_outside_member_access(file_tokens, index):
    """True for a #define body token joined by `##` to a neighbour, whose
    pasted chain does not follow `.` or `->`."""
    token = file_tokens[index]

    def same_define(i):
        return 0 <= i < len(file_tokens) and \
            file_tokens[i].define_offset == token.define_offset
    if not ((same_define(index - 1) and file_tokens[index - 1].text == "##")
            or (same_define(index + 1) and file_tokens[index + 1].text == "##")):
        return False
    start = index
    while same_define(start - 1) and file_tokens[start - 1].text == "##":
        start -= 2
    return not (same_define(start - 1) and file_tokens[start - 1].text in (".", "->"))


def _line(data, offset):
    return data.count(b"\n", 0, offset) + 1


def _report_line(entry, found):
    origins = Counter(o.origin for o in found)
    by_file = Counter(o.file for o in found)
    return "%s -> %s: %d (%s) in %s" % (
        entry.old, entry.new, len(found),
        ", ".join("%s %d" % item for item in sorted(origins.items())),
        ", ".join("%s %d" % item for item in sorted(by_file.items())))
