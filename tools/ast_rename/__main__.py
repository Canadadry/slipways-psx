import argparse
import os
import re
import sys

from ast_rename import analysis, bindings, plan, resolution, write


def parse_args(argv):
    parser = argparse.ArgumentParser(
        prog="ast_rename",
        description="Rename C declarations at the token positions libclang gives.")
    parser.add_argument("--libclang", default=os.environ.get("LIBCLANG"),
                        help="path to libclang (default: $LIBCLANG)")
    commands = parser.add_subparsers(dest="command", required=True)

    analysis = argparse.ArgumentParser(add_help=False)
    analysis.add_argument("--root", required=True,
                          help="project root; only declarations written under it count")
    analysis.add_argument("--exclude-dir", action="append", default=[],
                          help="directory under the root that is not project code")
    analysis.add_argument("--cflags", default="", help="flags shared by every configuration")
    analysis.add_argument("--config", action="append", default=[], metavar="NAME=FLAGS",
                          help="one build configuration; every unit is parsed in each")
    analysis.add_argument("--include-on-demand", action="append", default=[],
                          metavar="HEADER=DIR",
                          help="add -idirafter DIR to a unit that fails to find HEADER")
    analysis.add_argument("units", nargs="+", help="the .c files to parse")

    extract = commands.add_parser(
        "extract", parents=[analysis],
        help="write every declaration and its occurrences; fails if a unit does not parse")
    extract.add_argument("-o", "--output", required=True)
    extract.add_argument("--occurrences", help="also write every occurrence here")

    apply = commands.add_parser(
        "apply", parents=[analysis],
        help="rename the table's entries; writes nothing if any check fails")
    apply.add_argument("--table", required=True,
                       help="TSV: old, new, declaration file[, identity]")
    apply.add_argument("--report", help="write the per-entry report here")
    return parser.parse_args(argv)


def main(argv):
    args = parse_args(argv)
    try:
        cindex = bindings.load(args.libclang)
    except bindings.BindingsError as error:
        print("ast_rename: %s" % error, file=sys.stderr)
        return 2
    project = analysis.Project(args.root, args.exclude_dir)
    sources = analysis.Sources(project)
    model = analyze(cindex, args, project, sources)
    if args.command == "extract":
        write_extract(model, args.output, args.occurrences)
        for label, failures in (("unparsed", model.unparsed), ("partial", model.partial)):
            for failure in failures:
                print("ast_rename: %s %s [%s]: %s"
                      % (label, failure.unit, failure.config, failure.message),
                      file=sys.stderr)
        return 1 if model.unparsed else 0
    return run_apply(args, project, sources, model)


def run_apply(args, project, sources, model):
    with open(args.table) as f:
        entries = plan.read_table(f.read())
    checked = set(model.files) | {project.relative(unit) for unit in args.units}
    include_dirs = _include_dirs(args.cflags)
    for failure in model.unparsed + model.partial:
        checked |= _include_closure(project, failure.unit, include_dirs)
    files = {relative: sources.data(relative) for relative in sorted(checked)
             if relative is not None}
    result = plan.build(entries, model, files)
    for warning in result.warnings:
        print("ast_rename: warning: %s" % warning, file=sys.stderr)
    if result.errors:
        for error in result.errors:
            print("ast_rename: refused: %s" % error, file=sys.stderr)
        print("ast_rename: nothing written", file=sys.stderr)
        return 1
    written = write.apply_rewrites(project, result.rewrites)
    report = result.report + ["%d occurrence(s) in %d file(s)" % (len(result.rewrites),
                                                                 len(written))]
    if args.report:
        with open(args.report, "w") as f:
            f.write("\n".join(report) + "\n")
    print("\n".join(report))
    return 0


def _include_dirs(cflags):
    flags = analysis.split_flags(cflags)
    dirs = []
    for index, flag in enumerate(flags):
        if flag == "-I" and index + 1 < len(flags):
            dirs.append(flags[index + 1])
        elif flag.startswith("-I") and len(flag) > 2:
            dirs.append(flag[2:])
    return dirs


def _include_closure(project, unit, include_dirs):
    """Project files a unit reaches by `#include "..."`, read as text: what a
    unit that does not parse would have shown the analysis."""
    seen, stack = set(), [unit]
    while stack:
        relative = stack.pop()
        if relative in seen:
            continue
        seen.add(relative)
        path = project.absolute(relative)
        with open(path, "rb") as f:
            text = f.read()
        for name in re.findall(rb'^\s*#\s*include\s*"([^"]+)"', text, re.M):
            for base in [os.path.dirname(path)] + include_dirs:
                candidate = os.path.join(base, name.decode())
                if os.path.exists(candidate):
                    found = project.relative(candidate)
                    if found is not None:
                        stack.append(found)
                    break
    return seen


def analyze(cindex, args, project, sources):
    configs = []
    for config in args.config or ["default="]:
        name, _, flags = config.partition("=")
        configs.append((name, analysis.split_flags(args.cflags + " " + flags)))
    on_demand = [tuple(item.split("=", 1)) for item in args.include_on_demand]
    index = cindex.Index.create()
    builder = resolution.Builder(sources)
    unparsed = []
    for unit in args.units:
        relative = project.relative(unit)
        for name, flags in configs:
            tu, error = analysis.parse_unit(cindex, index, unit, flags, on_demand)
            if tu is None:
                unparsed.append(analysis.Unparsed(relative, name, error))
                continue
            builder.add_unit(analysis.collect(cindex, tu, sources))
    model = builder.finish()
    failures = {}
    for failure in unparsed:
        failures.setdefault(failure.unit, []).append(failure)
    for unit, unit_failures in failures.items():
        if len(unit_failures) == len(configs):
            model.unparsed += unit_failures
        else:
            model.partial += unit_failures
    return model


DECLARATION_COLUMNS = ["identity", "kind", "name", "parent", "file", "line", "linkage",
                       "scope_lines", "occurrences", "flags"]


def write_extract(model, output, occurrences_output):
    """Problems first as `#` lines, then one row per identity, sorted."""
    counts = {}
    for occurrence in model.occurrences.values():
        counts[occurrence.usr] = counts.get(occurrence.usr, 0) + 1
    by_name = {}
    for identity in model.identities.values():
        by_name.setdefault(identity.name, []).append(identity)
    conflicted = {usr for usrs in model.conflicts.values() for usr in usrs}
    unresolved = {item[0] for item in model.unresolved}
    lines = []
    for label, failures in (("unparsed", model.unparsed), ("partial", model.partial)):
        for failure in sorted(failures, key=lambda u: (u.unit, u.config)):
            lines.append("# %s\t%s\t%s\t%s" % (label, failure.unit, failure.config,
                                                failure.message))
    for (relative, offset), usrs in sorted(model.conflicts.items()):
        lines.append("# conflict\t%s\t%d\t%s" % (relative, offset,
                                                  " ".join(sorted(map(str, usrs)))))
    for usr, relative, offset, reason in sorted(model.unresolved, key=str):
        lines.append("# unresolved\t%s\t%s\t%d\t%s" % (usr, relative, offset, reason))
    lines.append("\t".join(DECLARATION_COLUMNS))
    rows = []
    for identity in model.identities.values():
        flags = []
        if len(identity.name) <= 2:
            flags.append("short")
        if len(by_name[identity.name]) > 1:
            flags.append("homonym")
        if identity.define_count > 1:
            flags.append("multidef")
        if identity.usr in conflicted:
            flags.append("conflict")
        if identity.usr in unresolved:
            flags.append("unresolved")
        rows.append((identity.file, identity.line, identity.kind, identity.name, identity.usr,
                     [identity.usr, identity.kind, identity.name, identity.parent,
                      identity.file, str(identity.line), identity.linkage,
                      str(identity.scope_lines) if identity.scope_lines >= 0 else "-",
                      str(counts.get(identity.usr, 0)), ",".join(flags) or "-"]))
    for row in sorted(rows, key=lambda r: r[:5]):
        lines.append("\t".join(row[5]))
    with open(output, "w") as f:
        f.write("\n".join(lines) + "\n")
    if occurrences_output:
        with open(occurrences_output, "w") as f:
            f.write("identity\tfile\toffset\tlength\torigin\n")
            for key in sorted(model.occurrences):
                occurrence = model.occurrences[key]
                if occurrence.usr is None:
                    continue
                f.write("%s\t%s\t%d\t%d\t%s\n" % (occurrence.usr, occurrence.file,
                                                  occurrence.offset, occurrence.length,
                                                  occurrence.origin))


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
