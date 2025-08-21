# ----------------------------------------------------------------------------
# -                        tiny3d: www.tiny3d.org                            -
# ----------------------------------------------------------------------------
# Copyright (c) 2018-2024 www.tiny3d.org
# SPDX-License-Identifier: MIT
# ----------------------------------------------------------------------------

import os
import sys

os.environ["KMP_DUPLICATE_LIB_OK"] = "True"
os.environ["TCM_ENABLE"] = "1"
from pathlib import Path

# Attempt to load build configuration (may not exist for pure-python wheels)
try:
    from tiny3d._build_config import _build_config  # type: ignore
except Exception:  # noqa: BLE001
    _build_config = {"version": "0.0.0", "build_type": "pure-python"}

if sys.platform == "win32":  # pragma: no cover - platform specific
    _win32_dll_dir = os.add_dll_directory(str(Path(__file__).parent))

__DEVICE_API__ = "cpu"
_HAVE_NATIVE = False

if __DEVICE_API__ == "cpu":  # Try optional native bindings
    try:  # noqa: SIM105
        from tiny3d.cpu.pybind import (  # type: ignore
            geometry,  # noqa: F401
            io,        # noqa: F401
            pipelines, # noqa: F401
            utility,   # noqa: F401
        )
        from tiny3d.cpu import pybind as _pybind_root  # type: ignore # noqa: F401
        _HAVE_NATIVE = True
    except Exception as e:  # noqa: BLE001
        import warnings
        import traceback
        
        # Debug: Print detailed error information
        print(f"[tiny3d DEBUG] Native import failed with error: {type(e).__name__}: {e}")
        print(f"[tiny3d DEBUG] Traceback:")
        traceback.print_exc()
        
        # Check if the expected module structure exists
        import os
        package_dir = Path(__file__).parent
        print(f"[tiny3d DEBUG] Package directory: {package_dir}")
        print(f"[tiny3d DEBUG] Package contents: {list(package_dir.iterdir()) if package_dir.exists() else 'NOT FOUND'}")
        
        cpu_dir = package_dir / "cpu"
        if cpu_dir.exists():
            print(f"[tiny3d DEBUG] cpu directory contents: {list(cpu_dir.iterdir())}")
            pybind_files = list(cpu_dir.glob("pybind*"))
            print(f"[tiny3d DEBUG] pybind files found: {pybind_files}")
        else:
            print(f"[tiny3d DEBUG] cpu directory does not exist")
        
        warnings.warn(
            f"tiny3d native cpu bindings not found; running in pure-python mode. Error: {e}",
            RuntimeWarning,
        )
        
        # Create placeholder modules that raise informative errors
        class _PurePythonPlaceholder:
            def __getattr__(self, name):
                raise ImportError(
                    f"tiny3d.{self._module_name}.{name} is not available in pure-python mode. "
                    "Native bindings failed to load. This may be due to missing dependencies "
                    "or an incompatible wheel for your platform."
                )
            def __bool__(self):
                return False
        
        class _GeometryPlaceholder(_PurePythonPlaceholder):
            _module_name = "geometry"
        
        class _IOPlaceholder(_PurePythonPlaceholder):
            _module_name = "io"
            
        class _PipelinesPlaceholder(_PurePythonPlaceholder):
            _module_name = "pipelines"
            
        class _UtilityPlaceholder(_PurePythonPlaceholder):
            _module_name = "utility"
        
        geometry = _GeometryPlaceholder()  # type: ignore
        io = _IOPlaceholder()  # type: ignore
        pipelines = _PipelinesPlaceholder()  # type: ignore
        utility = _UtilityPlaceholder()  # type: ignore

__version__ = _build_config.get("version", "0.0.0")
__all__ = [
    "__version__",
    "geometry",
    "io",
    "pipelines",
    "utility",
    "_HAVE_NATIVE",
]

# Ensure _HAVE_NATIVE is always accessible at module level
globals()['_HAVE_NATIVE'] = _HAVE_NATIVE

if int(sys.version_info[0]) < 3:  # pragma: no cover
    raise Exception("tiny3d only supports Python 3.")

if sys.platform == "win32":  # pragma: no cover
    _win32_dll_dir.close()
