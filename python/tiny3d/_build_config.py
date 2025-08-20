"""Fallback build configuration for pure-Python tiny3d wheels.

Normally, a CMake build step would generate this file with detailed
feature flags and version info. For now we supply a static minimal
version so that importing tiny3d works directly from the built wheel.
"""

_build_config = {
    "version": "0.0.0",  # TODO: replace via release automation
    "build_type": "pure-python",
    "features": {},
}
