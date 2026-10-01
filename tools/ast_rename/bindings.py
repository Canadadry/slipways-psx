import ctypes
import importlib.metadata
import os
import re


class BindingsError(Exception):
    pass


SETUP_HINT = "run `make rename-tool-setup`"


def load(library_path):
    """Return the configured clang.cindex module, or raise BindingsError."""
    try:
        from clang import cindex
    except ImportError:
        raise BindingsError("clang Python bindings not found: " + SETUP_HINT)
    bindings_version = importlib.metadata.version("clang")
    if not library_path:
        raise BindingsError("no libclang given: set LIBCLANG or pass --libclang")
    if not os.path.exists(library_path):
        raise BindingsError("libclang not found: %s" % library_path)
    if not cindex.Config.loaded:
        cindex.Config.set_library_file(library_path)
    library_version = _library_version(cindex)
    if _major(bindings_version) != _major(library_version):
        raise BindingsError(
            "clang bindings %s do not match libclang %s (%s): %s"
            % (bindings_version, library_version, library_path, SETUP_HINT))
    _register_spelling_location(cindex)
    return cindex


def _library_version(cindex):
    get_version = cindex.conf.lib.clang_getClangVersion
    get_version.restype = cindex._CXString
    get_version.argtypes = []
    text = cindex._CXString.from_result(get_version())
    if isinstance(text, bytes):
        text = text.decode()
    match = re.search(r"version (\d+\.\d+\.\d+)", text)
    if match is None:
        raise BindingsError("cannot read libclang version from %r" % text)
    return match.group(1)


def _major(version):
    return version.split(".")[0]


def _register_spelling_location(cindex):
    function = cindex.conf.lib.clang_getSpellingLocation
    function.argtypes = [cindex.SourceLocation, ctypes.POINTER(cindex.c_object_p),
                         ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_uint),
                         ctypes.POINTER(ctypes.c_uint)]
    function.restype = None


def spelling_location(cindex, location):
    """(file name or None, byte offset) where the token is written; the bindings'
    own `location.file` / `.offset` give the macro expansion point instead."""
    handle = cindex.c_object_p()
    line, column, offset = ctypes.c_uint(), ctypes.c_uint(), ctypes.c_uint()
    cindex.conf.lib.clang_getSpellingLocation(
        location, ctypes.byref(handle), ctypes.byref(line), ctypes.byref(column),
        ctypes.byref(offset))
    if not handle:
        return None, 0
    return cindex.File(handle).name, offset.value
