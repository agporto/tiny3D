together to enable full end to end pipelines:
<h1 align="center">tiny3D</h1>
<p align="center"><em>A lean, point‑cloud–centric subset of the Open3D library.</em></p>

---

> DISCLAIMER & ATTRIBUTION: This repository is an independent, reduced fork of the
> original <a href="https://github.com/isl-org/Open3D">Open3D</a> project. The design,
> architecture, and the majority of the original implementation were created by the
> Open3D team and contributors under the MIT License. This fork removes large
> subsystems to provide a smaller, faster‑building core focused on point cloud
> processing. All original copyrights remain intact.

---

## Why tiny3D?

| Goal | tiny3D Approach |
|------|-----------------|
| Smaller install size | Strip visualization, rendering, ML, heavy optional modules |
| Faster build & CI | Fewer dependencies; CPU‑only; simplified targets |
| Easier embedding | Minimal surface area & reduced transitive libs |
| Focus | Core point cloud + essential geometry ops |

## Retained Features

- Point cloud container & basic mesh / voxel helpers
- KD-tree (nanoflann) nearest neighbor queries
- Core registration (e.g. ICP classes / feature alignment)
- Common point cloud / mesh file I/O (e.g. PLY)
- Lightweight utilities (logging, filesystem, random, progress)

## Removed / Not Included

- Advanced visualization & GUI / viewer applications
- Rendering / PBR materials
- Machine learning integration (PyTorch / TensorFlow bindings)
- CUDA / GPU acceleration layer (CPU‑only currently)
- Extensive mesh / geometry algorithms not essential to point cloud workflows
- Jupyter widget integrations & rich notebook tooling

## Installation (Python)

Prebuilt experimental wheels (as CI matures) target Linux x86_64, Windows 64‑bit, and macOS (x86_64 & arm64).

```bash
pip install tiny3d
python -c "import tiny3d; print(tiny3d.__version__)"
```

### Build From Source (Python)

```bash
pip install --upgrade pip build
pip install -r python/requirements.txt
pip install ./python
```

Verbose build:

```bash
TINY3D_VERBOSE=1 pip install ./python
```

### C++ Core Only

```bash
cmake -S . -B build -DBUILD_PYTHON_MODULE=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Minimal Python Example

```python
import tiny3d as t3d

pcd = t3d.geometry.PointCloud()
print("Point count:", len(pcd.points))
```

## Differences vs Open3D

| Area | Open3D | tiny3D |
|------|--------|--------|
| Scope | Broad 3D (rendering, ML, visualization) | Point cloud–focused core |
| GPU | CPU + optional CUDA | CPU only |
| Visualization | Rich UI | Removed |
| ML bindings | PyTorch / TF | Removed |
| Build time | Larger | Reduced |
| Binary size | Larger | Smaller |

## Troubleshooting

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| `_HAVE_NATIVE` False | Native module import failed | Reinstall with `TINY3D_VERBOSE=1`; confirm platform tag matches Python |
| Windows DLL load fail | Missing OpenMP runtime (now disabled by default) | Use latest wheel; if enabling OpenMP set `TINY3D_WITH_OPENMP_WINDOWS=1` and ensure VC runtime installed |
| Version mismatch | Stale build artifacts | Clear pip cache & reinstall |


## Contributing

Scope is intentionally narrow. Please open an issue before large feature PRs. Welcomed:

- Performance or stability improvements
- Bug fixes & packaging robustness
- Documentation corrections

## License

MIT License – see `LICENSE`.

```
Copyright (c) Open3D authors
Copyright (c) tiny3D fork maintainers

Permission is hereby granted, free of charge, to any person obtaining a copy
... (full text in LICENSE)
```

### Attribution

Derived from the Open3D project (https://github.com/isl-org/Open3D). If you use this fork academically, please cite the original Open3D paper:

```bibtex
@article{Zhou2018,
    author  = {Qian-Yi Zhou and Jaesik Park and Vladlen Koltun},
    title   = {Open3D: A Modern Library for 3D Data Processing},
    journal = {arXiv:1801.09847},
    year    = {2018}
}
```

## Disclaimer

This fork is provided "as is". Missing functionality compared to upstream Open3D is by design.

