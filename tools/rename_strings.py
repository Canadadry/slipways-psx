"""Lists the names the compiler does not see, for docs/renommage.tsv: string
keys (tag, message, state and class names, cart names written as strings),
identifiers cited by text (tool data, scripts, Makefile, string literals),
test names and tmp/ paths, and the tool binaries and outputs of tmp/. Writes
them as `chaîne` and `fichier` rows, one per (name, file), each file marked
writer or reader; keeps the identifier rows and the header. A name with no
writer or no reader, or whose new name cannot be decided, goes to the flags."""

import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_derive  # noqa: E402
import rename_tables  # noqa: E402

COLUMNS = rename_derive.COLUMNS
WRITER, READER = "écrivain", "lecteur"
LABEL, HASHED = "libellé", "libellé haché"  # printed, or folded into a frozen hash: no counterpart
ROLES = (WRITER, READER, LABEL, HASHED)


# --- reading sources -------------------------------------------------------

def c_literals(text):
    """(offset, content) of every string literal outside comments and
    character constants."""
    i, n = 0, len(text)
    while i < n:
        if text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            i = n if j < 0 else j + 2
        elif text[i] in "'\"":
            quote = text[i]
            j = i + 1
            while j < n and text[j] != quote:
                j += 2 if text[j] == "\\" else 1
            if quote == '"':
                yield i, text[i + 1:j]
            i = j + 1
        else:
            i += 1


def assert_message(text, offset):
    """The `cond && "message"` idiom of an assert: its text is a comment for
    the reader, left to step 4."""
    return re.search(r"&&\s*$", text[max(0, offset - 20):offset]) is not None


class Literal:
    def __init__(self, path, text, offset, content):
        self.path = path
        self.content = content
        line_start = text.rfind("\n", 0, offset) + 1
        self.before = text[line_start:offset]
        self.after = text[offset + len(content) + 2:offset + len(content) + 40]
        self.fragment = (re.search(r'"\s*$', self.before) is not None
                         or re.match(r'\s*"', self.after) is not None)
        self.initializer = initializer_before(text, offset)


def initializer_before(text, offset):
    """For an offset at the top of a brace initializer, or inside one of its
    nested braces: (declaration text before the outer `{`, nested), else
    None."""
    depth = 0
    nested = False
    i = offset - 1
    while i >= 0:
        c = text[i]
        if c == "}":
            depth += 1
        elif c == "{":
            if depth > 0:
                depth -= 1
            else:
                head = text[max(0, i - 120):i]
                if re.search(r"[{,]\s*$", head):
                    nested = True
                else:
                    head = head[head.rfind(";") + 1:]
                    return (head, nested) if re.search(r"=\s*$", head) else None
        elif c == ";" and depth == 0:
            return None
        i -= 1
    return None


SKIPPED_PARTS = ("third_party", "__pycache__", "tests/e2e", "tests/fixtures/ast_rename",
                 "tools/ast_rename")
# The dictionary machinery names the old symbols on purpose.
SKIPPED_FILES = re.compile(r"^(tools/rename_[a-z_]+\.(py|tsv)|tests/test_(rename_[a-z_]+|ast_rename)\.py)$")


def source_files(root):
    """Paths relative to root of the files that can carry a name: C sources
    of src/ and tests/, scripts and data of tools/, the Makefile."""
    found = []
    for top in ("src", "tests", "tools"):
        for directory, dirs, files in os.walk(os.path.join(root, top)):
            rel_dir = os.path.relpath(directory, root)
            dirs[:] = sorted(d for d in dirs
                             if not any((rel_dir + "/" + d).startswith(p) for p in SKIPPED_PARTS))
            for name in sorted(files):
                path = os.path.join(rel_dir, name)
                if SKIPPED_FILES.match(path) or not re.search(r"\.(c|h|sh|txt)$", name):
                    continue
                found.append(path)
    if os.path.exists(os.path.join(root, "Makefile")):
        found.append("Makefile")
    return found


def read(root, path):
    with open(os.path.join(root, path), encoding="utf-8", errors="replace") as f:
        return f.read()


def text_lines(text):
    """Non-comment lines of a script, data file or Makefile."""
    return [l for l in text.splitlines() if not l.lstrip().startswith("#")]


# --- the table -------------------------------------------------------------

def write_table(path, header, rows):
    rows = sorted(rows, key=lambda r: (r[6], r[3], r[4], r[0], r[9], r[2], r[8]))
    with open(path, "w", encoding="utf-8") as f:
        f.write("\n".join(header) + "\n")
        f.write("\t".join(COLUMNS) + "\n")
        for row in rows:
            f.write("\t".join(row) + "\n")


# --- names -----------------------------------------------------------------

class Name:
    """A name the compiler does not see: its new text, what it is, and the
    files that write and read it."""

    def __init__(self, category, kind, old, identity):
        self.category = category
        self.kinds = [kind]
        self.old = old
        self.identity = identity
        self.new = None
        self.concept = ""
        self.candidates = {}  # new text -> where it comes from
        self.sites = {}  # file -> set of roles
        self.note = ""

    def propose(self, new, source, concept=""):
        self.candidates.setdefault(new, source)
        if not self.concept:
            self.concept = concept

    def add_site(self, path, role):
        self.sites.setdefault(path, set()).add(role)

    @property
    def kind(self):
        return " + ".join(sorted(self.kinds, key=lambda k: KIND_ORDER.index(k) if k in KIND_ORDER else 99))


class Names:
    def __init__(self):
        self.by_identity = {}

    def get(self, category, kind, old):
        identity = "%s:%s" % (category, old)
        if identity not in self.by_identity:
            self.by_identity[identity] = Name(category, kind, old, identity)
        name = self.by_identity[identity]
        if kind not in name.kinds:
            name.kinds.append(kind)
        return name


def common_prefix(names):
    tokens = [n.split("_") for n in names]
    prefix = []
    for parts in zip(*tokens):
        if len(set(parts)) != 1:
            break
        prefix.append(parts[0])
    return len(prefix)


DESIGNATOR = re.compile(r"\[([A-Z][A-Z0-9_]*)\]\s*=\s*$")


def designated_keys(literals, identifiers):
    """Keys of the arrays indexed by an enum: `[PICO8_TAG_JO] = "jo"`. The new
    key is the new constant name without the prefix it shares with its
    siblings, lower-cased: TAG_PLANET -> planet."""
    groups = {}
    for lit in literals:
        match = DESIGNATOR.search(lit.before)
        if not match or match.group(1) not in identifiers:
            continue
        constant = match.group(1)
        family = re.sub(r"_[A-Z0-9]+_?$", "", constant)
        groups.setdefault(family, {})[constant] = lit.content
    keys = {}
    for family, members in groups.items():
        new_names = {c: identifiers[c][0] for c in members}
        cut = common_prefix(new_names.values()) if len(new_names) > 1 else 0
        for constant, key in members.items():
            new, concept = identifiers[constant]
            keys[key] = ("_".join(new.split("_")[cut:]).lower(), concept, family, constant)
    return keys


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", required=True)
    parser.add_argument("--table", required=True)
    parser.add_argument("--vocabulary", required=True)
    parser.add_argument("--overrides", required=True)
    parser.add_argument("--flags", required=True)
    args = parser.parse_args(argv)

    header, rows = rename_tables.read_table(args.table)
    rows = [[row.get(c, "") for c in COLUMNS] for row in rows]
    identifier_rows = [r for r in rows if r[3] == "identifiant"]
    lexicon = rename_derive.Lexicon()
    rename_derive.read_vocabulary(args.vocabulary, lexicon)
    rename_derive.read_overrides(args.overrides, lexicon)
    files = source_files(args.root)
    texts = {path: read(args.root, path) for path in files}
    all_literals = collect_literals(files, texts)

    names = Names()
    name_keys(names, all_literals, identifier_rows, lexicon, files, texts)
    cite_symbols(names, identifier_rows, all_literals, files, texts)
    name_artifacts(names, all_literals, files, texts)
    name_tests(names, all_literals, renamed_tests(args.vocabulary))
    apply_overrides(names, lexicon)

    out, flags = emit(names, lexicon)
    kept = [r for r in rows if r[3] not in ("chaîne", "fichier")]
    write_table(args.table, header, kept + out)
    with open(args.flags, "w", encoding="utf-8") as f:
        f.write("identité\tsorte\tsignalement\n")
        for name, reason in flags:
            f.write("%s\t%s\t%s\n" % (name.identity, name.kind, reason))
    print("rename_strings: %d names, %d rows, %d flags (%s)" % (
        len(names.by_identity), len(out), len(flags), args.flags))
    return 0


def collect_literals(files, texts):
    """String literals of the C files, assert messages left out (with the
    literals that continue them)."""
    literals = []
    for path in files:
        if not path.endswith((".c", ".h")):
            continue
        text = texts[path]
        message_end = None
        for offset, content in c_literals(text):
            continued = message_end is not None and not text[message_end:offset].strip()
            if assert_message(text, offset) or continued:
                message_end = offset + len(content) + 2
                continue
            message_end = None
            literals.append(Literal(path, text, offset, content))
    return literals


def name_keys(names, all_literals, identifier_rows, lexicon, files, texts):
    """Whole-literal keys: their new name from the code that binds them (an
    enum-indexed table, the class vtable, a class table), else from the
    vocabulary."""
    identifiers = {}
    for r in identifier_rows:
        identifiers.setdefault(r[0], (r[1], r[7]))
    literals = [lit for lit in all_literals
                if not lit.fragment and not any(c.search(lit.before) for c in NOT_A_NAME)]
    for key, (new, concept, family, constant) in designated_keys(literals, identifiers).items():
        names.get("chaîne", designated_kind(family), key).propose(new, constant, concept)
    vtable = {r[0]: (r[1], r[7]) for r in identifier_rows if VTABLE_FIELD in r[9]}
    for lit in literals:
        if lit.content in vtable and vtable[lit.content][0] != lit.content:
            new, concept = vtable[lit.content]
            names.get("chaîne", "clé de message", lit.content).propose(new, "vtable", concept)
        if CLASS_RECORD.search(lit.before) and lit.content and lit.path.startswith("src/"):
            anchor = identifiers.get("pico8_%s_struct_class" % lit.content)
            name = names.get("chaîne", "nom de classe", lit.content)
            if anchor and anchor[0].endswith("_class"):
                _, concept = rename_derive.translate(rename_derive.split_tokens(lit.content), lexicon)
                name.propose(anchor[0][:-len("_class")], "class table", concept)
    for lit in literals:
        name = names.by_identity.get("chaîne:%s" % lit.content)
        if name is None or not name.candidates:
            translated = cart_name(lit.content, lexicon)
            if translated:
                name = names.get("chaîne", "clé", lit.content)
                name.propose(translated[0], "vocabulary", translated[1])
        if name is not None:
            name.add_site(lit.path, literal_role(lit))
    keys = {n.old for n in names.by_identity.values() if n.category == "chaîne"}
    for path in files:
        if re.match(r"tools/class_coverage_.*\.txt$", path):
            for line in text_lines(texts[path]):
                fields = line.split()
                if fields and fields[0] in keys:
                    names.get("chaîne", "nom de classe", fields[0]).add_site(path, READER)


def name_tests(names, literals, test_files):
    """`test_pico8_X` in a message, `tmp/test_pico8_X…` in a path: renamed
    after the test file."""
    for lit in literals:
        for match in TEST_NAME.finditer(lit.content):
            if match.group(1):
                name = names.get("chaîne", "chemin tmp", match.group(0))
                roles = (WRITER, READER)  # the test writes the file, then reads it
            else:
                name = names.get("chaîne", "nom de test", match.group(0))
                roles = (LABEL,)
            new = renamed_test(match.group(2), test_files)
            if new:
                name.propose((match.group(1) or "") + new + match.group(3), "vocabulary")
            for role in roles:
                name.add_site(lit.path, role)


def apply_overrides(names, lexicon):
    for name in names.by_identity.values():
        override = lexicon.strings.get(name.old) or lexicon.files.get(name.old)
        if override:
            new = name.old if override[0] == "=" else override[0]
            name.candidates = {new: "override"}
            name.concept = override[1] or name.concept


def emit(names, lexicon):
    """Table rows, one per (name, file), and the flags: a new name left
    undecided, a writer without reader or a reader without writer."""
    out = []
    flags = []
    for name in sorted(names.by_identity.values(), key=lambda n: n.identity):
        if len(name.candidates) == 1:
            name.new = next(iter(name.candidates))
        status = "conservé" if name.new == name.old else "renommé"
        roles = set().union(*name.sites.values()) if name.sites else set()
        if name.new is None:
            flags.append((name, "nouveau nom indécis : %s" % ", ".join(
                "%s (%s)" % kv for kv in sorted(name.candidates.items()))))
        if (WRITER in roles) != (READER in roles):
            flags.append((name, "sans %s" % (WRITER if WRITER not in roles else READER)))
        for path, site_roles in sorted(name.sites.items()):
            note = ", ".join(r for r in ROLES if r in site_roles)
            extra = lexicon.site_notes.get("%s@%s" % (name.old, path))
            if extra:
                note += " ; " + extra[0]
            out.append([name.old, name.new or "", path, name.category, name.kind, status,
                        rename_derive.module_of(path), name.concept, note, name.identity])
    return out, flags


VTABLE_FIELD = "@S@pico8_struct_class@FI@"
CLASS_RECORD = re.compile(r"\bpico8_class_coverage_record\(\s*$")
WRITER_CONTEXTS = [
    DESIGNATOR,
    re.compile(r"(\.\w+|\b\w+(\[\w*\])?)\s*=\s*$"),  # member, designated or variable init
    re.compile(r"\b\w*(_ks|_record|_init|_register)\s*\([^;]*$"),  # setters and registrations
]


CITED_KINDS = ("fonction", "type", "tag", "champ", "constante", "macro", "statique", "globale")
SYMBOL = re.compile(r"(?<![A-Za-z0-9_])([A-Za-z_][A-Za-z0-9_]*)(?:\.([A-Za-z_][A-Za-z0-9_]*))?")


def cite_symbols(names, identifier_rows, literals, files, texts):
    """Identifiers named by text: in a string literal, a script or data file
    of tools/, the Makefile. Only names with an underscore and at least four
    characters: a shorter one is a key or a word. The declaration file is
    the writer."""
    by_name = {}
    fields = {}
    for r in identifier_rows:
        if r[4] not in CITED_KINDS:
            continue
        by_name.setdefault(r[0], []).append(r)
        match = re.search(r"@(?:SA|S|U|UA)@([A-Za-z0-9_]+)@FI@([A-Za-z0-9_]+)$", r[9])
        if r[4] == "champ" and match:
            fields[(match.group(1), match.group(2))] = r
            fields[(match.group(1) + "_t", match.group(2))] = r

    def cited(text):
        for match in SYMBOL.finditer(text):
            yield match.group(0), match.group(1), match.group(2)

    sites = []
    for lit in literals:
        sites.append((lit.path, lit.content))
    for path in files:
        if path == "Makefile" or (path.startswith("tools/") and path.endswith((".sh", ".txt"))):
            for line in text_lines(texts[path]):
                sites.append((path, re.sub(r"-D([A-Z])", r"-D \1", line)))
    for path, text in sites:
        for whole, first, member in cited(text):
            if "chaîne:%s" % whole in names.by_identity:
                continue
            if member is not None:
                field = fields.get((first, member))
                owners = by_name.get(first, [])
                if field is None or not owners:
                    whole, member = first, None
                else:
                    if field[5] == "conservé" and all(o[5] == "conservé" for o in owners):
                        continue
                    name = names.get("chaîne", "symbole cité", whole)
                    for owner in owners:
                        name.propose("%s.%s" % (owner[1], field[1]), owner[9], field[7] or owner[7])
                    name.add_site(field[2], WRITER)
                    name.add_site(path, READER)
                    continue
            rows = by_name.get(whole)
            if not rows or "_" not in whole or len(whole) < 4:
                continue
            if all(r[5] == "conservé" for r in rows):
                continue
            name = names.get("chaîne", "symbole cité", whole)
            for r in rows:
                name.propose(r[1], r[9], r[7])
                name.add_site(r[2], WRITER)
            name.add_site(path, READER)


TMP_ARTIFACT = re.compile(r"(?:\$\(TMP_DIR\)|(?<![A-Za-z0-9_.-])tmp)/([A-Za-z0-9_.-]+)")


def artifact_new_name(name, makefile):
    """tmp/ name of a binary or output: a `main_X` binary takes the name of
    the cmd/ file it is built from, any other drops its `pico8_`."""
    if name.startswith("main_"):
        rule = re.search(r"^\$\(TMP_DIR\)/%s:.*?\bsrc/cmd/(\w+)\.c" % re.escape(name), makefile, re.M)
        return rule.group(1) if rule else None
    return re.sub(r"(^|_)pico8_", r"\1", name, count=1)


def name_artifacts(names, literals, files, texts):
    """The binaries and outputs of tmp/ whose name carries `pico8_` or
    `main_`: the Makefile that builds a binary, or the program that writes
    an output, is the writer; every other mention a reader."""
    makefile = "\n".join(text_lines(texts.get("Makefile", "")))
    segments = [("Makefile", line) for line in makefile.splitlines()]
    for path in files:
        if path.startswith("tools/") and path.endswith((".sh", ".txt")):
            segments += [(path, line) for line in text_lines(texts[path])]
    segments += [(lit.path, lit) for lit in literals]
    found = set()
    for path, segment in segments:
        text = segment if isinstance(segment, str) else segment.content
        for match in TMP_ARTIFACT.finditer(text):
            artifact = match.group(1)
            if re.search(r"(^|_)pico8_", artifact) or artifact.startswith("main_"):
                if not artifact.startswith("test_pico8_"):
                    found.add(artifact)
    for artifact in sorted(found):
        old = "tmp/" + artifact
        built = re.search(r"^\$\(TMP_DIR\)/%s:" % re.escape(artifact), makefile, re.M)
        name = names.get("fichier", "binaire" if built and "." not in artifact else "sortie", old)
        new = artifact_new_name(artifact, makefile)
        if new:
            name.propose("tmp/" + new, "rule")
        mention = re.compile(r"(?<![A-Za-z0-9_])%s(?![A-Za-z0-9_])" % re.escape(artifact))
        for path, segment in segments:
            text = segment if isinstance(segment, str) else segment.content
            if not mention.search(text):
                continue
            if path == "Makefile":
                role = WRITER if re.match(r"\$\(TMP_DIR\)/%s:" % re.escape(artifact), text) else READER
            elif isinstance(segment, str):
                role = READER
            elif path.startswith("src/") and "." in artifact:
                role = WRITER  # the program that writes this output
            elif re.search(r"\bfopen\(\s*$", segment.before) and re.match(r'\s*,\s*"[wa]', segment.after):
                role = WRITER
            else:
                role = READER
            name.add_site(path, role)


TEST_NAME = re.compile(r"(tmp/)?(test_pico8_[A-Za-z0-9_]*)((?(1)[A-Za-z0-9_.-]*))")
TEST_ROW = re.compile(r"^\|\s*`tests/(test_pico8_\w+)\.c`\s*\|\s*`tests/`\s*\|\s*`(\w+)\.c`")


def renamed_tests(vocabulary):
    """{old test stem: new test stem} from the tree of the vocabulary."""
    with open(vocabulary, encoding="utf-8") as f:
        return dict(m.groups() for m in map(TEST_ROW.match, f.read().splitlines()) if m)


def renamed_test(text, test_files):
    """`test_pico8_zone_choisie_stderr` -> `test_allocator_stderr`: the
    longest old test stem the text starts with, renamed; None if none."""
    best = None
    for old in test_files:
        if (text == old or text.startswith(old + "_")) and (best is None or len(old) > len(best)):
            best = old
    return None if best is None else test_files[best] + text[len(best):]


def cart_name(text, lexicon):
    """(new text, concept) when the whole string is a cart name the
    vocabulary translates (`qy`, `qy_fv`), else None."""
    if not re.match(r"^[a-z][a-z0-9]*_?(_[a-z0-9]+_?)*$", text) or len(text) > 24:
        return None
    tokens = rename_derive.split_tokens(text)
    if not any(len(t) <= 2 and (t,) in lexicon.tokens for t in tokens):
        return None
    new, concept = rename_derive.translate(tokens, lexicon)
    if not new or "_".join(new) == text:
        return None
    return "_".join(new), concept


NOT_A_NAME = [
    re.compile(r"\bpico8_print\("),  # text drawn for the player
    re.compile(r"\bstr\(\s*$"),
    re.compile(r"\breturn\s+$"),  # a status text
    re.compile(r"\bf?open\([^;]*,\s*$"),  # a file mode
]
LABEL_CONTEXTS = [re.compile(r"\bcheck\w*\(\s*$")]
HASHED_CONTEXTS = [re.compile(r"\b(fnv_str|fold_\w+)\([^;]*,\s*$")]


def literal_role(lit):
    """Writer where the name is defined or set: an enum-indexed table, an
    assignment, a setter call, a table; a label when it is only
    printed or hashed; reader everywhere else."""
    if any(c.search(lit.before) for c in LABEL_CONTEXTS):
        return LABEL
    if any(c.search(lit.before) for c in HASHED_CONTEXTS):
        return HASHED
    if any(c.search(lit.before) for c in WRITER_CONTEXTS):
        return WRITER
    if lit.initializer is not None and (lit.path.startswith("src/") or lit.initializer[1]):
        return WRITER  # a table of src/, or a table of records such as a test's cases
    return READER


KIND_ORDER = ["clé de message", "nom d'état", "clé d'étiquette", "nom de classe", "clé"]


def designated_kind(family):
    """`PICO8_STATE` -> nom d'état, `PICO8_TAG` -> clé d'étiquette."""
    if family.endswith("STATE"):
        return "nom d'état"
    if family.endswith("TAG"):
        return "clé d'étiquette"
    return "clé"


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
