#!/bin/sh
# usage: psx_release.sh <major|minor>
# Tags origin/master psx-v<major>.<minor>, one step above the highest such tag
# on origin, and pushes it: .github/workflows/psx-release.yml then publishes
# the release. With no tag yet, major gives psx-v1.0 and minor psx-v0.1.
# Refuses unless HEAD is a clean master equal to origin/master. DRY_RUN=1
# prints the tag and changes nothing.

set -eu

fail() {
    echo "psx-release: $*" >&2
    exit 1
}

case ${1:-} in
    major|minor) bump=$1 ;;
    *) fail "usage: psx_release.sh <major|minor>" ;;
esac

remote=origin
branch=master

[ "$(git rev-parse --abbrev-ref HEAD)" = "$branch" ] || fail "not on $branch"
[ -z "$(git status --porcelain)" ] || fail "uncommitted changes"
git fetch --quiet "$remote" "$branch"
[ "$(git rev-parse HEAD)" = "$(git rev-parse "$remote/$branch")" ] \
    || fail "HEAD differs from $remote/$branch"

latest=$(git ls-remote --tags --refs "$remote" 'psx-v*' \
    | sed -n 's|.*refs/tags/psx-v\([0-9][0-9]*\)\.\([0-9][0-9]*\)$|\1 \2|p' \
    | sort -n -k1,1 -k2,2 | tail -n 1)

if [ -z "$latest" ]; then
    major=0
    minor=0
else
    major=${latest% *}
    minor=${latest#* }
fi

if [ "$bump" = major ]; then
    major=$((major + 1))
    minor=0
else
    minor=$((minor + 1))
fi

tag=psx-v$major.$minor
echo "psx-release: ${latest:+psx-v$(echo "$latest" | tr ' ' .) -> }$tag"

[ "${DRY_RUN:-0}" = 1 ] && exit 0

git tag -a "$tag" -m "$tag"
git push "$remote" "$tag"
