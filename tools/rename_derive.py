"""Derives an English name for every declaration that `make rename-extract`
found, from docs/vocabulaire.md, the naming rules of docs/plan-5.md section 4
and a hand-written overrides table. Writes the `identifiant` rows of
docs/renommage.tsv (rows of other categories already in the file are kept) and
a remainder: the declarations no rule could name, with an empty `nouveau`."""

import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import rename_tables  # noqa: E402

COLUMNS = ["ancien", "nouveau", "fichier", "catégorie", "sorte", "statut",
           "module", "concept", "note", "identité"]

KIND_LABELS = {
    "function": "fonction", "field": "champ", "enum_constant": "constante",
    "macro": "macro", "static": "statique", "global": "globale",
    "param": "paramètre", "local": "locale", "type": "type", "tag": "tag",
}

DEFAULT_HEADER = [
    "# Table de renommage du plan 5 (docs/plan-5.md, étape 2b).",
    "# Statut : à valider",
    "# Lignes identifiant écrites par `make rename-derive` (tools/rename_derive.py),",
    "# lignes chaîne et fichier par `make rename-strings` (tools/rename_strings.py) :",
    "# corriger docs/vocabulaire.md ou tools/rename_overrides.tsv, pas ces lignes.",
]


class Declaration:
    def __init__(self, row):
        """row: a declaration as rename_tables.read_declarations gives it."""
        self.identity, self.kind, self.name = row["identity"], row["kind"], row["name"]
        self.parent, self.file, self.line = row["parent"], row["file"], row["line"]
        self.linkage, self.occurrences, self.flags = (row["linkage"], row["occurrences"],
                                                      row["flags"])
        self.scope = int(row["scope_lines"]) if row["scope_lines"] != "-" else None


def declarations_of(path):
    return [Declaration(row) for row in rename_tables.read_declarations(path)]


# --- vocabulary ------------------------------------------------------------

class Lexicon:
    """What the vocabulary and the overrides say: whole-name mappings,
    token mappings, and the concept each mapping comes from."""

    def __init__(self):
        self.identities = {}  # identity -> (new, concept), from overrides
        self.members = {}  # (parent, name) -> (new, concept), from overrides
        self.names = {}    # name -> (new, concept), from overrides
        self.exact = {}    # old full name -> (new, concept)
        self.fields = {}   # (parent, old field) -> (new, concept)
        self.tokens = {}   # tuple of old tokens -> (new tokens tuple, concept)
        self.vocabulary_tokens = set()  # token keys that come from the vocabulary
        self.suffix_only = set()  # token keys translated only as the last token
        self.entity_structs = set()  # structs that embed an entity header
        self.type_stems = set()  # new type names without `_t`: receiver types
        self.class_stems = set()  # new names of the classes that have a class table
        self.strings = {}  # old string key -> (new, concept), for tools/rename_strings.py
        self.files = {}    # old tmp/ path -> (new, concept), for tools/rename_strings.py
        self.site_notes = {}  # "old@file" -> (note, concept), for tools/rename_strings.py

    def add_token(self, old, new, concept):
        key = tuple(split_tokens(old))
        self.tokens.setdefault(key, (tuple(split_tokens(new)), concept))
        self.vocabulary_tokens.add(key)


BACKTICK = re.compile(r"`([^`]+)`")
HEADER_TYPE = "pico8_struct_header_t"
CLASS_TYPE = "pico8_struct_class_t"


def table_rows(text):
    """(section, cells) of every markdown table row under `## Concepts`,
    `## En-tête commun` and `## Vtable`."""
    section = None
    for line in text.splitlines():
        if line.startswith("## "):
            section = line[3:].strip()
            continue
        if section not in ("Concepts", "En-tête commun", "Vtable"):
            continue
        if not line.startswith("|") or line.startswith("|---"):
            continue
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) < 3 or cells[0] in ("nom actuel", "membre actuel"):
            continue
        yield section, cells


def is_whole_name(item):
    return "pico8_" in item.lower() or item.endswith("_t")


def strip_cart(item):
    """`pico8_jo_prise_*` -> `jo_prise`; `PICO8_STATE_PH` -> `state_ph`."""
    item = item.lower()
    if item.endswith("_*"):
        item = item[:-len("_*")]
    if item.startswith("pico8_"):
        item = item[len("pico8_"):]
    return item


def read_vocabulary(path, lexicon):
    with open(path, encoding="utf-8") as f:
        text = f.read()
    for section, cells in table_rows(text):
        old_items = BACKTICK.findall(cells[0])
        new_items = [i for i in BACKTICK.findall(cells[2]) if not i.startswith('"')]
        if not old_items:
            continue
        concept = old_items[0]
        if "conservé" in cells[2] and not new_items:
            new_items = [old_items[0]]
        if section in ("En-tête commun", "Vtable") and not is_whole_name(concept):
            owner = HEADER_TYPE if section == "En-tête commun" else CLASS_TYPE
            lexicon.fields[(owner, concept)] = (new_items[0], concept)
            if section == "Vtable" or len(concept) == 2:
                lexicon.add_token(concept, new_items[0], concept)
            continue
        pattern_new = [i for i in new_items if i.startswith("X_")]
        if pattern_new:
            # `pico8_X_def` / `_dispatch` -> `X_archetype` / `X_<méthode>_method`:
            # the suffix token after the class.
            old_suffix = re.sub(r"^(pico8)?_?(X_)?", "", concept)
            new_suffix = re.sub(r"^X_(<[^>]*>_)?", "", pattern_new[0])
            lexicon.add_token(old_suffix, new_suffix, concept)
            if concept.startswith("_"):
                lexicon.suffix_only.add((old_suffix,))
            continue
        bare_old = [i for i in old_items if not is_whole_name(i) and " " not in i]
        bare_new = [i for i in new_items if not is_whole_name(i) and not i.isupper()]
        whole_old = [i for i in old_items
                     if is_whole_name(i) and "X" not in i and "*" not in i]
        whole_new = [i for i in new_items if is_whole_name(i) or i.isupper()]
        if any(i.endswith("_") for i in new_items):
            continue  # the `pico8_` -> `p8_` prefix row: a rule of derive_name
        if not whole_new and len(new_items) == 1:
            whole_old = [i for i in whole_old if i.islower()]
            whole_new = new_items * len(whole_old)
        for old, new in zip(whole_old, whole_new):
            lexicon.exact[old] = (new, concept)
        for old, new in zip(bare_old, bare_new):
            lexicon.add_token(old, new, concept)
        for old, new in zip(whole_old, whole_new):
            phrase = strip_type(strip_cart(old))
            if any(len(token) <= 2 for token in split_tokens(phrase)):
                lexicon.add_token(phrase, strip_type(new.lower()), concept)


def strip_type(name):
    return name[:-len("_t")] if name.endswith("_t") else name


def read_overrides(path, lexicon):
    """Override rows: kind (identity, name, field, token; string and file for
    tools/rename_strings.py; site: a note on one file of a string
    or file name), key, new (`=` keeps the old name), concept.
    They win over the vocabulary."""
    with open(path, encoding="utf-8") as f:
        lines = f.read().splitlines()
    seen = {}
    for number, line in enumerate(lines, 1):
        if not line.strip() or line.startswith("#") or line.startswith("kind\t"):
            continue
        cells = line.split("\t")
        if len(cells) < 3:
            raise SystemExit("%s:%d: expected kind, key, new[, concept]" % (path, number))
        kind, key, new = cells[:3]
        if (kind, key) in seen:
            raise SystemExit("%s:%d: %s %s already given line %d" % (
                path, number, kind, key, seen[(kind, key)]))
        seen[(kind, key)] = number
        concept = cells[3] if len(cells) > 3 else ""
        if kind == "identity":
            lexicon.identities[key] = (new, concept)
        elif kind == "name":
            lexicon.names[key] = (new, concept)
        elif kind == "field":
            parent, _, name = key.rpartition(".")
            lexicon.members[(parent, name)] = (new, concept)
        elif kind == "token":
            lexicon.tokens[tuple(split_tokens(key))] = (tuple(split_tokens(new)), concept)
        elif kind == "string":
            lexicon.strings[key] = (new, concept)
        elif kind == "file":
            lexicon.files[key] = (new, concept)
        elif kind == "site":
            lexicon.site_notes[key] = (new, concept)
        else:
            raise SystemExit("%s:%d: unknown override kind %r" % (path, number, kind))


# --- names -----------------------------------------------------------------

def split_tokens(name):
    """Lower-case tokens of a name; a one-letter cart name keeps its trailing
    underscore (`pico8_g__t` -> pico8, g_, t)."""
    parts = name.lower().split("_")
    tokens = []
    i = 0
    while i < len(parts):
        part = parts[i]
        if len(part) == 1 and i + 1 < len(parts) and parts[i + 1] == "" and part.isalpha():
            tokens.append(part + "_")
            i += 2
            continue
        if part:
            tokens.append(part)
        i += 1
    return tokens


def module_of(path):
    """`src/game/planet.c` -> `game/planet`."""
    stem = os.path.splitext(path)[0]
    if stem.startswith("src/"):
        return stem[len("src/"):]
    return stem


def translate(tokens, lexicon):
    """New tokens and the first concept used, or None if a token is unknown.
    In a name of several tokens a single letter stands for itself."""
    out = []
    concept = ""
    i = 0
    while i < len(tokens):
        for length in range(len(tokens) - i, 0, -1):
            key = tuple(tokens[i:i + length])
            if key in lexicon.suffix_only and i + length < len(tokens):
                continue
            if key in lexicon.tokens:
                new, source = lexicon.tokens[key]
                out.extend(new)
                concept = concept or source
                i += length
                break
        else:
            single = len(tokens) > 1 and len(tokens[i]) == 1 and tokens[i].isalpha()
            if not (single or is_plain_word(tokens[i])):
                return None, ""
            out.append(tokens[i])
            i += 1
    return out, concept


def is_plain_word(token):
    """A token taken as English unless the lexicon says otherwise: three
    letters or more, or a number."""
    if re.match(r"^[a-z]{1,2}[0-9]+$", token):
        return len(token) == 2 or token[1].isdigit()  # x1, y2: a coordinate, not ha2
    return token.isdigit() or (len(token) >= 3 and token.isalnum())


def derive(decl, lexicon):
    """(new name or "", status, concept)."""
    new, concept = derive_name(decl, lexicon)
    if new is None:
        return "", "", ""
    if new == "=":
        new = decl.name
    return new, "conservé" if new == decl.name else "renommé", concept


def derive_name(decl, lexicon):
    """(new name or None, concept)."""
    for table, key in ((lexicon.identities, decl.identity),
                       (lexicon.members, (decl.parent, decl.name)),
                       (lexicon.names, decl.name),
                       (lexicon.exact, decl.name)):
        if key in table:
            return table[key]
    for owner in (decl.parent, decl.parent + "_t"):
        if (owner, decl.name) in lexicon.fields:
            return lexicon.fields[(owner, decl.name)]
    mirrors_header = decl.parent.endswith("_def_t") or decl.parent in lexicon.entity_structs
    if decl.kind == "field" and mirrors_header:
        if (HEADER_TYPE, decl.name) in lexicon.fields:
            return lexicon.fields[(HEADER_TYPE, decl.name)]
    if len(decl.name) <= 2:
        return derive_short(decl, lexicon)
    if decl.kind == "macro" and decl.file.endswith(".h") and decl.name.endswith("_H"):
        return include_guard(decl.file), ""
    upper = decl.name.isupper()
    tokens = split_tokens(decl.name)
    runtime = False
    prefix = []
    cart_named = tokens[:1] == ["pico8"]
    if cart_named:
        tokens = tokens[1:]
        runtime = decl.file.startswith("src/p8/") and not file_local(decl)
    elif decl.name.startswith("g_") and tokens[0] == "g" and len(tokens) > 1:
        prefix, tokens = ["g"], tokens[1:]
    suffix = []
    if decl.kind == "type":
        suffix = ["t"]
        if tokens[-1:] == ["t"]:
            tokens = tokens[:-1]
    new, concept = translate(tokens, lexicon)
    if new is None:
        return None, ""
    singleton = cart_named and len(tokens) == 1 and tuple(tokens) in lexicon.vocabulary_tokens
    if decl.kind == "function" and singleton:
        return "_".join(new), concept  # `pico8_mo`: the vocabulary names it whole
    if decl.kind == "function" and file_local(decl):
        if cart_named:
            new = drop_module_prefix(new, decl.file, lexicon)
    elif decl.kind == "function" and decl.file.startswith("src/") and not runtime:
        new = add_module_prefix(new, decl.file, lexicon)
    if runtime:
        new = ["p8"] + new
    new = prefix + new
    name = "_".join(new + suffix)
    return (name.upper() if upper else name), concept


def file_local(decl):
    """A static function or variable of a .c file: no prefix of any kind."""
    return decl.linkage == "static" and decl.file.endswith(".c")


def include_guard(path):
    """`src/engine/geom.h` -> `ENGINE_GEOM_H`."""
    stem = os.path.splitext(path)[0]
    if stem.startswith("src/"):
        stem = stem[len("src/"):]
    return re.sub(r"[^A-Za-z0-9]", "_", stem).upper() + "_H"


LOCAL_KINDS = ("local", "param")


def prototype_parameter(decl):
    """A parameter of a function pointer type or member: the extraction gives
    it no function as parent, and its scope is the declaration alone."""
    return decl.kind == "param" and (decl.scope is None or "/" in decl.parent)


def is_local(decl):
    """A local, a parameter, or a static declared inside a function."""
    return decl.kind in LOCAL_KINDS or (decl.kind == "static" and decl.parent != "-")


SCOPE_TOLERANCE = 40


def derive_short(decl, lexicon):
    """A name of one or two characters: a local or parameter whose scope fits
    the tolerance keeps it, cart name or not; otherwise a name the lexicon
    knows is translated, and only a vector's x/y field keeps it."""
    if prototype_parameter(decl):
        return decl.name, ""
    if is_local(decl) and decl.scope is not None and decl.scope <= SCOPE_TOLERANCE:
        return decl.name, ""
    key = tuple(split_tokens(decl.name))
    if key in lexicon.tokens:
        new, concept = lexicon.tokens[key]
        name = "_".join(new)
        if len(name) > 2:
            return (name.upper() if decl.name.isupper() else name), concept
    if decl.kind == "field" and decl.name in ("x", "y"):
        return decl.name, ""
    return None, ""


def add_module_prefix(tokens, path, lexicon):
    """An exported function starts with its module, or with the type it is a
    method of."""
    module = os.path.splitext(os.path.basename(path))[0].split("_")
    if tokens[:len(module)] == module:
        return tokens
    if starts_with_receiver(tokens, lexicon):
        return tokens
    return module + tokens


def starts_with_receiver(tokens, lexicon):
    """The name starts with a type or a class: a method of it."""
    stems = lexicon.type_stems | lexicon.class_stems
    return any(tuple(tokens[:len(stem)]) == stem for stem in stems)


METHOD_SUFFIXES = ("method", "construct")


def drop_module_prefix(tokens, path, lexicon):
    """A static function needs no module prefix, unless the module is named
    after a type, whose methods keep it."""
    module = os.path.splitext(os.path.basename(path))[0].split("_")
    if tuple(module) in lexicon.type_stems | lexicon.class_stems:
        return tokens
    if starts_with_receiver(tokens, lexicon) and tokens[:len(module)] != module:
        return tokens
    rest = tokens[len(module):]
    if tokens[:len(module)] != module or not rest or rest[-1] in METHOD_SUFFIXES:
        return tokens
    if len(rest) == 1 and len(rest[0]) < 4:
        return tokens
    return rest


# --- output ----------------------------------------------------------------

CLASS_TABLE = re.compile(r"^(?:pico8|g)_(.+)_struct_class$")


def class_stems(declarations, lexicon):
    """The translated X of every `pico8_X_struct_class` or `g_X_struct_class`."""
    stems = set()
    for decl in declarations:
        match = CLASS_TABLE.match(decl.name)
        if match:
            new, _ = translate(split_tokens(match.group(1)), lexicon)
            if new:
                stems.add(tuple(new))
    return stems


def read_existing(path):
    """The comment header and the non-identifier rows of a previous table;
    the default header if there is none."""
    if not os.path.exists(path):
        return DEFAULT_HEADER, []
    header, rows = rename_tables.read_table(path)
    kept = [[row.get(c, "") for c in COLUMNS] for row in rows
            if row.get("catégorie") != "identifiant"]
    return header or DEFAULT_HEADER, kept


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--declarations", required=True)
    parser.add_argument("--vocabulary", required=True)
    parser.add_argument("--overrides", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--remainder", required=True)
    args = parser.parse_args(argv)

    lexicon = Lexicon()
    read_vocabulary(args.vocabulary, lexicon)
    read_overrides(args.overrides, lexicon)
    declarations = declarations_of(args.declarations)
    lexicon.entity_structs = {d.parent for d in declarations
                              if d.kind == "field" and d.name == "header"}
    types = {}
    for decl in declarations:
        if decl.kind == "type":
            types[(decl.file, decl.name)] = derive(decl, lexicon)
    lexicon.class_stems = class_stems(declarations, lexicon)
    lexicon.type_stems = {tuple(split_tokens(strip_type(new))) for new, _, _ in types.values()
                          if new.endswith("_t")}
    rows = []
    for decl in declarations:
        typedef = types.get((decl.file, decl.name + "_t"))
        if decl.kind == "tag" and typedef and typedef[0]:
            new, status, concept = typedef[0][:-len("_t")], typedef[1], typedef[2]
        else:
            new, status, concept = derive(decl, lexicon)
        rows.append([decl.name, new, decl.file, "identifiant", KIND_LABELS[decl.kind],
                     status, module_of(decl.file), concept, "", decl.identity])
    header, kept = read_existing(args.output)
    table = rows + kept
    table.sort(key=lambda row: (row[6], row[3], row[4], row[0], row[9]))
    with open(args.output, "w", encoding="utf-8") as f:
        f.write("\n".join(header) + "\n")
        f.write("\t".join(COLUMNS) + "\n")
        for row in table:
            f.write("\t".join(row) + "\n")
    remainder = [row for row in rows if not row[1]]
    with open(args.remainder, "w", encoding="utf-8") as f:
        f.write("\t".join(COLUMNS) + "\n")
        for row in remainder:
            f.write("\t".join(row) + "\n")
    renamed = sum(1 for row in rows if row[5] == "renommé")
    print("rename_derive: %d declarations, %d renamed, %d kept, %d in the remainder (%s)" % (
        len(rows), renamed, len(rows) - renamed - len(remainder), len(remainder), args.remainder))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
