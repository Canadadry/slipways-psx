"""Porting gap markers: every file of src/, tests/, tools/ and the Makefile
(without __pycache__), read without git. Reports each line that carries the
old marker prefix, and each mention of a gap number (marker or prose) whose
`## ` title the register (--gaps) lacks, as `<file>:<line>: <reason>`, then
a footer. Exit 0 when nothing is reported, 1 otherwise, 2 when the register
is missing or has no gap title: a check against nothing would pass for
clean."""

import argparse
import os
import re
import sys

OLD_MARKER = re.compile(r"\b" + "TRO" + r"U-")
MENTION = re.compile(r"\bGAP-(\d+)\b")
TITLE = re.compile(r"^## GAP-(\d+)\b", re.M)
TOPS = ("src", "tests", "tools")


def files_of(root):
    found = []
    for top in TOPS:
        for directory, dirs, names in os.walk(os.path.join(root, top)):
            dirs[:] = sorted(d for d in dirs if d != "__pycache__")
            found.extend(os.path.relpath(os.path.join(directory, n), root) for n in sorted(names))
    if os.path.isfile(os.path.join(root, "Makefile")):
        found.append("Makefile")
    return found


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", required=True)
    parser.add_argument("--gaps", required=True, help="the register of the porting gaps")
    args = parser.parse_args(argv)
    if not os.path.isfile(args.gaps):
        print("error: no register %s" % args.gaps, file=sys.stderr)
        return 2
    with open(args.gaps, encoding="utf-8") as f:
        titled = set(TITLE.findall(f.read()))
    if not titled:
        print("error: %s has no gap title" % args.gaps, file=sys.stderr)
        return 2

    findings = 0
    cited = set()
    paths = files_of(args.root)
    for path in paths:
        with open(os.path.join(args.root, path), encoding="utf-8", errors="replace") as f:
            lines = f.read().splitlines()
        for number, line in enumerate(lines, 1):
            if OLD_MARKER.search(line):
                print("%s:%d: old marker prefix" % (path, number))
                findings += 1
            for gap in MENTION.findall(line):
                cited.add(gap)
                if gap not in titled:
                    print("%s:%d: GAP-%s has no title in %s" % (path, number, gap, args.gaps))
                    findings += 1
    print("gap_check: %d finding(s); %d file(s) read, %d gap number(s) cited, %d titled"
          % (findings, len(paths), len(cited), len(titled)))
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
