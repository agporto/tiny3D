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
    except Exception:  # noqa: BLE001
        import warnings
        warnings.warn(
            "tiny3d native cpu bindings not found; running in pure-python mode.",
            RuntimeWarning,
        )
        geometry = io = pipelines = utility = None  # type: ignore

__version__ = _build_config.get("version", "0.0.0")
__all__ = [
    "__version__",
    "geometry",
    "io",
    "pipelines",
    "utility",
    "_HAVE_NATIVE",
]

if int(sys.version_info[0]) < 3:  # pragma: no cover
    raise Exception("tiny3d only supports Python 3.")

if sys.platform == "win32":  # pragma: no cover
    _win32_dll_dir.close()
