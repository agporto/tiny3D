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
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        
        # required for auto-detection & inclusion of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        debug = int(os.environ.get("DEBUG", 0)) if self.debug is None else self.debug
        cfg = "Debug" if debug else "Release"

        # CMake lets you override the generator - we need to check this.
        # Can be set with Conda-Build, for example.
        cmake_generator = os.environ.get("CMAKE_GENERATOR", "")

        # Set Python_EXECUTABLE instead of PYTHON_EXECUTABLE to help CMake find the right Python
        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            f"-DCMAKE_BUILD_TYPE={cfg}",
            f"-DBUILD_PYTHON_MODULE=ON",
            f"-DBUILD_UNIT_TESTS=OFF",
            f"-DBUILD_BENCHMARKS=OFF",
        ]
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

        build_temp = Path(self.build_temp) / ext.name
        if not build_temp.exists():
            build_temp.mkdir(parents=True)

        # Configure
        subprocess.check_call(
            ["cmake", str(project_root)] + cmake_args, cwd=build_temp
        )
        
        # Build
        subprocess.check_call(
            ["cmake", "--build", ".", "--target", "pybind"] + build_args, cwd=build_temp
        )

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
