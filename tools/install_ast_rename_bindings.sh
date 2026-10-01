set -e

# The bindings' major version must match the system libclang's (checked by the
# tool at startup).
CLANG_BINDINGS_VERSION=18.1.8
BINDINGS_DIR="third_party/clang-python"

python3 -m pip install --no-deps --upgrade --target "$BINDINGS_DIR" "clang==$CLANG_BINDINGS_VERSION"

echo "clang Python bindings $CLANG_BINDINGS_VERSION installed in $BINDINGS_DIR"
