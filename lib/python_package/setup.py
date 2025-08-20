# ----------------------------------------------------------------------------
# -                        tiny3d: www.tiny3d.org                            -
# ----------------------------------------------------------------------------
# Copyright (c) 2018-2024 www.tiny3d.org
# SPDX-License-Identifier: MIT
# ----------------------------------------------------------------------------

import os
import sys
import platform
import ctypes
from setuptools import setup, find_packages
from setuptools.command.install import install as _install
from wheel.bdist_wheel import bdist_wheel as _bdist_wheel

data_files_spec = [
    ("share/jupyter/nbextensions/tiny3d", "tiny3d/nbextension", "*.*"),
    ("share/jupyter/labextensions/tiny3d", "tiny3d/labextension", "**"),
    ("share/jupyter/labextensions/tiny3d", ".", "install.json"),
    ("etc/jupyter/nbconfig/notebook.d", ".", "tiny3d.json"),
]

cmdclass = {}

# Force platform specific wheel.
class bdist_wheel(_bdist_wheel):
    def finalize_options(self):
        _bdist_wheel.finalize_options(self)
        self.root_is_pure = False
    def get_tag(self):
        python, abi, plat = _bdist_wheel.get_tag(self)
        if plat[:5] == "linux":
            plat = "manylinux2014_x86_64" if platform.machine() == "x86_64" else plat
        elif plat[:6] == "macosx":
            plat = plat.replace("universal2", platform.machine())
        return python, abi, plat
cmdclass["bdist_wheel"] = bdist_wheel

class install(_install):
    def finalize_options(self):
        _install.finalize_options(self)
        self.install_libbase = self.install_platlib
        self.install_lib = self.install_platlib
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
    cmdclass=cmdclass,
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
    provides_dist=["tiny3d"],
)

setup(**setup_args)
