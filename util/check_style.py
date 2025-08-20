#!/usr/bin/env python3
"""Check and optionally apply code style for Tiny3D.

Provides a minimal subset of Open3D's style tooling:
 - C++ / CUDA: clang-format handled via CMake target (check_cpp_style.cmake)
 - Python: yapf formatting check / apply

Usage:
  python util/check_style.py            # check only, non-zero exit on diff
  python util/check_style.py --apply    # apply fixes in-place
"""
from __future__ import annotations
import argparse
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent

EXCLUDE_DIRS = {"build", "_deps", "3rdparty", "extern", ".venv", "__pycache__"}

def run(cmd: list[str]) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, cwd=REPO_ROOT, check=False, text=True, capture_output=True)

def find_python_files() -> list[Path]:
    files: list[Path] = []
    for path in REPO_ROOT.rglob("*.py"):
        if any(part in EXCLUDE_DIRS for part in path.parts):
            continue
        files.append(path)
    return files

def check_python(apply: bool) -> int:
    py_files = find_python_files()
    if not py_files:
        return 0
    cmd = [sys.executable, "-m", "yapf", "-p"]
    if apply:
        cmd += ["-i"]
    else:
        cmd += ["--diff"]
    cmd += [str(p) for p in py_files]
    proc = run(cmd)
    stdout = proc.stdout or ""
    stderr = proc.stderr or ""
    if not apply:
        if proc.returncode != 0:
            print(stderr, file=sys.stderr)
            return proc.returncode
        if "@@" in stdout:
            # There are diffs to apply.
            print(stdout)
            return 1
    return 0

def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--apply", action="store_true", help="Apply fixes in place")
    args = parser.parse_args(argv)
    rc = check_python(args.apply)
    if rc == 0:
        print("Python code style OK" + (" (applied)" if args.apply else ""))
    else:
        print("Python style differences detected." if not args.apply else "Failed applying style.")
    return rc

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
