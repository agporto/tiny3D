<p align="center">
<img src="https://raw.githubusercontent.com/isl-org/tiny3d/main/docs/_static/tiny3d_logo_horizontal.png" width="320" />
</p>

# tiny3d: A Modern Library for 3D Data Processing

<h4>
    <a href="https://www.tiny3d.org">Homepage</a> |
    <a href="https://www.tiny3d.org/docs">Docs</a> |
    <a href="https://www.tiny3d.org/docs/release/getting_started.html">Quick Start</a> |
    <a href="https://www.tiny3d.org/docs/release/compilation.html">Compile</a> |
    <a href="https://www.tiny3d.org/docs/release/index.html#python-api-index">Python</a> |
    <a href="https://www.tiny3d.org/docs/release/cpp_api.html">C++</a> |
    <a href="https://github.com/isl-org/tiny3d-ML">tiny3d-ML</a> |
    <a href="https://github.com/isl-org/tiny3d/releases">Viewer</a> |
    <a href="https://www.tiny3d.org/docs/release/contribute/contribute.html">Contribute</a> |
    <a href="https://www.youtube.com/channel/UCRJBlASPfPBtPXJSPffJV-w">Demo</a> |
    <a href="https://github.com/isl-org/tiny3d/discussions">Forum</a>
</h4>

tiny3d is an open-source library that supports rapid development of software
that deals with 3D data. The tiny3d frontend exposes a set of carefully selected
data structures and algorithms in both C++ and Python. The backend is highly
optimized and is set up for parallelization. We welcome contributions from
the open-source community.

[![Ubuntu CI](https://github.com/isl-org/tiny3d/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/isl-org/tiny3d/actions?query=workflow%3A%22Ubuntu+CI%22)
[![macOS CI](https://github.com/isl-org/tiny3d/actions/workflows/macos.yml/badge.svg)](https://github.com/isl-org/tiny3d/actions?query=workflow%3A%22macOS+CI%22)
[![Windows CI](https://github.com/isl-org/tiny3d/actions/workflows/windows.yml/badge.svg)](https://github.com/isl-org/tiny3d/actions?query=workflow%3A%22Windows+CI%22)

**Core features of tiny3d include:**

-   3D data structures
-   3D data processing algorithms
-   Scene reconstruction
-   Surface alignment
-   3D visualization
-   Physically based rendering (PBR)
-   3D machine learning support with PyTorch and TensorFlow
-   GPU acceleration for core 3D operations
-   Available in C++ and Python

Here's a brief overview of the different components of tiny3d and how they fit
together to enable full end to end pipelines:

![tiny3d_layers](https://github.com/isl-org/tiny3d/assets/41028320/e9b8645a-a823-4d78-8310-e85207bbc3e4)

For more, please visit the [tiny3d documentation](https://www.tiny3d.org/docs).

## Python quick start

Pre-built pip packages support Ubuntu 20.04+, macOS 10.15+ and Windows 10+
(64-bit) with Python 3.8-3.11.

```bash
# Install
pip install tiny3d       # or
pip install tiny3d-cpu   # Smaller CPU only wheel on x86_64 Linux (v0.17+)

# Verify installation
python -c "import tiny3d as o3d; print(o3d.__version__)"

# Python API
python -c "import tiny3d as o3d; \
           mesh = o3d.geometry.TriangleMesh.create_sphere(); \
           mesh.compute_vertex_normals(); \
           o3d.visualization.draw(mesh, raw_mode=True)"

# tiny3d CLI
tiny3d example visualization/draw
```

To get the latest features in tiny3d, install the
[development pip package](https://www.tiny3d.org/docs/latest/getting_started.html#development-version-pip).
To compile tiny3d from source, refer to
[compiling from source](https://www.tiny3d.org/docs/release/compilation.html).

## C++ quick start

Checkout the following links to get started with tiny3d C++ API

-   Download tiny3d binary package: [Release](https://github.com/isl-org/tiny3d/releases) or [latest development version](https://www.tiny3d.org/docs/latest/getting_started.html#c)
-   [Compiling tiny3d from source](https://www.tiny3d.org/docs/release/compilation.html)
-   [tiny3d C++ API](https://www.tiny3d.org/docs/release/cpp_api.html)

To use tiny3d in your C++ project, checkout the following examples

-   [Find Pre-Installed tiny3d Package in CMake](https://github.com/isl-org/tiny3d-cmake-find-package)
-   [Use tiny3d as a CMake External Project](https://github.com/isl-org/tiny3d-cmake-external-project)

## tiny3d-Viewer app

<img width="480" src="https://raw.githubusercontent.com/isl-org/tiny3d/main/docs/_static/tiny3d_viewer.png">

tiny3d-Viewer is a standalone 3D viewer app available on Debian (Ubuntu), macOS
and Windows. Download tiny3d Viewer from the
[release page](https://github.com/isl-org/tiny3d/releases).

## tiny3d-ML

<img width="480" src="https://raw.githubusercontent.com/isl-org/tiny3d-ML/main/docs/images/getting_started_ml_visualizer.gif">

tiny3d-ML is an extension of tiny3d for 3D machine learning tasks. It builds on
top of the tiny3d core library and extends it with machine learning tools for
3D data processing. To try it out, install tiny3d with PyTorch or TensorFlow and check out
[tiny3d-ML](https://github.com/isl-org/tiny3d-ML).

## Communication channels

-   [GitHub Issue](https://github.com/isl-org/tiny3d/issues): bug reports,
    feature requests, etc.
-   [Forum](https://github.com/isl-org/tiny3d/discussions): discussion on the usage of tiny3d.
-   [Discord Chat](https://discord.gg/D35BGvn): online chats, discussions,
    and collaboration with other users and developers.

## Citation

Please cite [our work](https://arxiv.org/abs/1801.09847) if you use tiny3d.

```bib
@article{Zhou2018,
    author    = {Qian-Yi Zhou and Jaesik Park and Vladlen Koltun},
    title     = {{tiny3d}: {A} Modern Library for {3D} Data Processing},
    journal   = {arXiv:1801.09847},
    year      = {2018},
}
```
