set -e

# The bindings' major version must match the system libclang's (checked by the
# tool at startup).
CLANG_MAJOR="${CLANG_MAJOR:?set CLANG_MAJOR, the libclang major version: run make rename-tool-setup}"
BINDINGS_DIR="third_party/clang-python"

python3 -m pip install --no-deps --upgrade --target "$BINDINGS_DIR" "clang==$CLANG_MAJOR.*"

echo "clang Python bindings $CLANG_MAJOR.* installed in $BINDINGS_DIR"
