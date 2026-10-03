"""The readers shared by the rename tools: a table of the docs/renommage.tsv
schema, the declarations of an extraction, the header line of either, and
the triples of an exceptions file.
Imports no tool, so that every tool can import it."""


def header_line(lines):
    """(index, columns) of the first line neither blank nor a `#` comment,
    or (None, None) if there is none."""
    for index, line in enumerate(lines):
        if line and not line.startswith("#"):
            return index, line.split("\t")
    return None, None


def read_table(path):
    """(leading `#` lines, rows) of a table of the docs/renommage.tsv schema
    (the table itself, or the remainder of `make rename-derive`): each row
    after the header line as a dict by that header, a missing cell as "";
    blank and other `#` lines are skipped."""
    with open(path, encoding="utf-8") as f:
        lines = f.read().splitlines()
    index, columns = header_line(lines)
    if index is None:
        return [line for line in lines if line.startswith("#")], []
    comments = [line for line in lines[:index] if line.startswith("#")]
    rows = []
    for line in lines[index + 1:]:
        if not line or line.startswith("#"):
            continue
        cells = line.split("\t")
        rows.append(dict(zip(columns, cells + [""] * (len(columns) - len(cells)))))
    return comments, rows


def read_declarations(path):
    """The declarations.tsv of `make rename-extract`, each row a dict by its
    header (identity, kind, name, parent, file, line, linkage, scope_lines,
    occurrences, flags); the `# partial` lines are skipped."""
    return read_table(path)[1]


def read_triples(path):
    """The lines of an exceptions file that are neither blank nor `#`
    comments, each cut on tabs into its first three cells, missing ones as
    "". Each file keeps its own schema (`column module word`,
    `kind parent name`)."""
    return [triple for _, triple in read_numbered_triples(path)]


def read_numbered_triples(path):
    """(line number, triple) of each line read_triples keeps."""
    found = []
    with open(path, encoding="utf-8") as f:
        for number, line in enumerate(f.read().splitlines(), 1):
            if line.strip() and not line.startswith("#"):
                found.append((number, tuple((line.split("\t") + ["", ""])[:3])))
    return found


def dead_lines(path, numbered, used, key=lambda triple: triple):
    """`<path>:<line>: removed nothing: <triple>` for each numbered triple
    whose key is not in used."""
    return ["%s:%d: removed nothing: %s" % (path, number, " ".join(triple))
            for number, triple in numbered if key(triple) not in used]
