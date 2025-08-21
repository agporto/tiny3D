# ----------------------------------------------------------------------------
# -                        tiny3d: www.tiny3d.org                            -
# ----------------------------------------------------------------------------
# Copyright (c) 2018-2024 www.tiny3d.org
# SPDX-License-Identifier: MIT
# ----------------------------------------------------------------------------

import os
import sys
import re
import subprocess
from pathlib import Path
from setuptools import setup, find_packages, Extension
from setuptools.command.install import install as _install
from setuptools.command.build_ext import build_ext

# Get the project root directory (one level up from python/)
project_root = Path(__file__).parent.parent.absolute()

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def build_extension(self, ext):
        # Some tooling might pass just the extension name string instead of an Extension object.
        ext_name = getattr(ext, 'name', ext)
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext_name)))
        
        # required for auto-detection & inclusion of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        # CMake lets you override the generator - we need to check this.
        # Can be set with Conda-Build, for example.
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        # Minimal CMake arguments; let CMake/pybind11 discover Python details to avoid partial state.
        import sysconfig
        python_prefix = sysconfig.get_config_var('prefix') or sys.prefix
        python_libdir = sysconfig.get_config_var('LIBDIR') or ''
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPython3_EXECUTABLE={sys.executable}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",  # backward compat for some 3rdparty scripts
            f"-DCMAKE_BUILD_TYPE={cfg}",
            f"-DBUILD_PYTHON_MODULE=ON",
            f"-DBUILD_UNIT_TESTS=OFF",
            f"-DBUILD_BENCHMARKS=OFF",
            # Build a self-contained extension (static core lib) to avoid separate DLL/.so runtime issues in wheels.
            "-DBUILD_SHARED_LIBS=OFF",
            "-DSKIP_CONFIGURE_SETUP_PY=ON",
        ]
        # Hint root (helps if multiple versions present)
        if python_prefix:
            cmake_args.append(f"-DPython3_ROOT_DIR={python_prefix}")
        # Attempt to locate an explicit libpython; only add if it exists to avoid confusing FindPython3
        python_version = f"{sys.version_info.major}.{sys.version_info.minor}"
        candidate_libs = []
        if python_libdir:
            for stem in ("", "m"):
                for ext in (".so", ".a"):
                    candidate_libs.append(os.path.join(python_libdir, f"libpython{python_version}{stem}{ext}"))
        for path in candidate_libs:
            if os.path.exists(path):
                cmake_args.append(f"-DPython3_LIBRARIES={path}")
                break
        build_args = []
        
        if "CMAKE_ARGS" in os.environ:
            cmake_args += [item for item in os.environ["CMAKE_ARGS"].split(" ") if item]

        cmake_args += [f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{cfg.upper()}={extdir}"]

        if sys.platform.startswith("darwin"):
            # Cross-compile support for macOS - respect ARCHFLAGS if set
            archs = re.findall(r"-arch (\S+)", os.environ.get("ARCHFLAGS", ""))
            if archs:
                cmake_args += ["-DCMAKE_OSX_ARCHITECTURES={}".format(";".join(archs))]

        # Set CMAKE_BUILD_PARALLEL_LEVEL to control the parallel build level
        # across all generators.
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            # self.parallel is a Python 3 only way to set parallel jobs by hand
            # using -j in the build_ext call, not supported by pip or PyPA-build.
            if hasattr(self, "parallel") and self.parallel:
                build_args += [f"-j{self.parallel}"]

        build_temp = Path(self.build_temp) / ext_name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)
        
        # Clean any existing CMake cache to avoid conflicts
        cache_file = build_temp / "CMakeCache.txt"
        if cache_file.exists():
            cache_file.unlink()
            
        # Also clean any cache files in the project root
        project_cache = project_root / "CMakeCache.txt"
        if project_cache.exists():
            project_cache.unlink()
        project_install = project_root / "cmake_install.cmake"
        if project_install.exists():
            project_install.unlink()

        # Configure
        subprocess.check_call(
            ["cmake", str(project_root)] + cmake_args, cwd=build_temp
        )
        
        # Build
        subprocess.check_call(
            ["cmake", "--build", ".", "--target", "pybind"] + build_args, cwd=build_temp
        )
        
        # Locate the built pybind module (which may have an ABI tag in its filename)
        # CMake currently places it under "<extdir>/Python/cpu" unless overridden.
        candidate_dirs = [Path(extdir) / "Python" / "cpu", Path(extdir)]
        built_candidates = []
        for d in candidate_dirs:
            if d.exists():
                built_candidates.extend(list(d.glob("pybind*.so")))
                built_candidates.extend(list(d.glob("pybind*.pyd")))
        # Fallback: search the entire build tree (last resort)
        if not built_candidates:
            for p in Path(build_temp).rglob("pybind*.so"):
                built_candidates.append(p)
            for p in Path(build_temp).rglob("pybind*.pyd"):
                built_candidates.append(p)
        if not built_candidates:
            raise RuntimeError("Could not locate built pybind extension after CMake build")
        # Prefer the longest filename (likely includes full ABI tag)
        built_candidates.sort(key=lambda p: len(p.name), reverse=True)
        selected = built_candidates[0]
        # Expected final path from setuptools (full path to extension)
        expected_fullpath = Path(self.get_ext_fullpath(ext_name))
        if os.environ.get("TINY3D_VERBOSE"):
            print("[tiny3d] Selected built module:", selected)
            print("[tiny3d] Target path:", expected_fullpath)
        # If CMake already placed the file exactly where setuptools expects it, skip copy.
        try:
            if selected.resolve() != expected_fullpath.resolve():
                expected_fullpath.parent.mkdir(parents=True, exist_ok=True)
                import shutil
                shutil.copy2(selected, expected_fullpath)
            elif os.environ.get("TINY3D_VERBOSE"):
                print("[tiny3d] Copy skipped: source and destination are identical.")
        except FileNotFoundError:
            # Fallback: ensure destination directory exists then attempt copy without resolve comparison
            expected_fullpath.parent.mkdir(parents=True, exist_ok=True)
            import shutil
            shutil.copy2(selected, expected_fullpath)

        # If shared libs were built (custom user build overriding our OFF), also copy sibling Tiny3D/lib deps.
        sibling_dir = selected.parent
        if any(p.name.startswith("Tiny3D") for p in sibling_dir.glob("*.dll")) or any(p.name.startswith("libTiny3D") for p in sibling_dir.glob("*.so*")):
            patterns = ["Tiny3D*.dll", "libTiny3D*.so*", "libTiny3D*.dylib"]
            for pat in patterns:
                for lib in sibling_dir.glob(pat):
                    dest = expected_fullpath.parent / lib.name
                    if lib.resolve() != dest.resolve():
                        try:
                            import shutil
                            shutil.copy2(lib, dest)
                            if os.environ.get("TINY3D_VERBOSE"):
                                print(f"[tiny3d] Copied runtime dependency {lib.name}")
                        except Exception as e:  # noqa: BLE001
                            print(f"[tiny3d] Warning: could not copy dependency {lib}: {e}")

# Define the extension
ext_modules = [
    CMakeExtension("tiny3d.cpu.pybind", sourcedir=str(project_root))
]

data_files_spec = [
    ("share/jupyter/nbextensions/tiny3d", "tiny3d/nbextension", "*.*"),
    ("share/jupyter/labextensions/tiny3d", "tiny3d/labextension", "**"),
    ("share/jupyter/labextensions/tiny3d", ".", "install.json"),
    ("etc/jupyter/nbconfig/notebook.d", ".", "tiny3d.json"),
]

cmdclass = {}

# Keep install class in case future native extensions are added.
class install(_install):  # noqa: D401
    def finalize_options(self):  # type: ignore[override]
        _install.finalize_options(self)
        # For pure python, leave root_is_pure True; if native code is added later,
        # reintroduce a custom bdist_wheel or switch to scikit-build-core.
cmdclass["install"] = install

with open("requirements.txt", "r") as f:
    install_requires = [line.strip() for line in f.readlines() if line]

entry_points = {
    "console_scripts": ["tiny3d = tiny3d.tools.cli:main",]
}

classifiers = [
    "Development Status :: 3 - Alpha",
    "Environment :: MacOS X",
    "Environment :: Win32 (MS Windows)",
    "Environment :: X11 Applications",
    "Intended Audience :: Developers",
    "Intended Audience :: Education",
    "Intended Audience :: Other Audience",
    "Intended Audience :: Science/Research",
    "License :: OSI Approved :: MIT License",
    "Natural Language :: English",
    "Operating System :: POSIX :: Linux",
    "Operating System :: MacOS :: MacOS X",
    "Operating System :: Microsoft :: Windows",
    "Programming Language :: C",
    "Programming Language :: C++",
    "Programming Language :: Python :: 3",
    "Programming Language :: Python :: 3.8",
    "Programming Language :: Python :: 3.9",
    "Programming Language :: Python :: 3.10",
    "Programming Language :: Python :: 3.11",
    "Programming Language :: Python :: 3.12",
    "Topic :: Education",
    "Topic :: Multimedia :: Graphics :: 3D Modeling",
    "Topic :: Multimedia :: Graphics :: 3D Rendering",
    "Topic :: Multimedia :: Graphics :: Capture",
    "Topic :: Multimedia :: Graphics :: Graphics Conversion",
    "Topic :: Multimedia :: Graphics :: Viewers",
    "Topic :: Scientific/Engineering",
    "Topic :: Scientific/Engineering :: Mathematics",
    "Topic :: Scientific/Engineering :: Visualization",
    "Topic :: Software Development :: Libraries :: Python Modules",
    "Topic :: Utilities",
]
name = "tiny3d"
with open("README.rst") as readme:
    long_description = readme.read()

setup_args = dict(
    name=name,
    version="0.0.0",
    python_requires=">=3.8",
    include_package_data=True,
    install_requires=install_requires,
    packages=find_packages(),
    entry_points=entry_points,
    zip_safe=False,
    cmdclass={"build_ext": CMakeBuild, "install": install},
    ext_modules=ext_modules,
    author="tiny3d Team",
    author_email="tiny3d@intel.com",
    url="https://www.tiny3d.org",
    project_urls={
        "Documentation": "https://www.tiny3d.org/docs",
        "Source code": "https://github.com/isl-org/Tiny3D",
        "Issues": "https://github.com/isl-org/Tiny3D/issues",
    },
    classifiers=classifiers,
    keywords="3D reconstruction point cloud mesh RGB-D visualization",
    license="MIT",
    description="Tiny3D: A Modern Library for 3D Data Processing.",
    long_description=long_description,
    long_description_content_type="text/x-rst",
)

setup(**setup_args)
