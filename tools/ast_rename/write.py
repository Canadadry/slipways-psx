from collections import defaultdict


def apply_rewrites(project, rewrites):
    """Every new content is computed before the first file is written."""
    by_file = defaultdict(list)
    for rewrite in rewrites:
        by_file[rewrite.file].append(rewrite)
    contents = {}
    for relative, file_rewrites in by_file.items():
        with open(project.absolute(relative), "rb") as f:
            data = f.read()
        for rewrite in sorted(file_rewrites, key=lambda r: r.offset, reverse=True):
            end = rewrite.offset + len(rewrite.old.encode())
            assert data[rewrite.offset:end] == rewrite.old.encode(), \
                "%s:%d changed since the plan" % (relative, rewrite.offset)
            data = data[:rewrite.offset] + rewrite.new.encode() + data[end:]
        contents[relative] = data
    for relative, data in sorted(contents.items()):
        with open(project.absolute(relative), "wb") as f:
            f.write(data)
    return sorted(contents)
