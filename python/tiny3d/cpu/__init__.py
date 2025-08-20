# Namespace package for compiled CPU bindings.
# The pybind extension module 'pybind' will be placed here by the CMake build.
# Import side-effects (like registering geometry classes) occur when importing submodules.
from importlib import import_module as _imp

try:
    # Attempt to import the compiled pybind module.
    _pybind = _imp(__name__ + '.pybind')  # noqa: F401
except Exception:  # noqa: BLE001
    # Leave silent; top-level tiny3d will warn if native bindings absent.
    _pybind = None

__all__ = ['_pybind']
