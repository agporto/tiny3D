#!/usr/bin/env bash
set -euo pipefail

# Source environment and utility settings
source "$(dirname "$0")/ci_utils.sh"

echo "nproc = $(getconf _NPROCESSORS_ONLN) NPROC = ${NPROC}"

# Install minimal Python dependencies (no unit test deps, no purge)
install_python_dependencies

# Configure, build, and install Tiny3D
build_all

# Build the Python pip wheel
make VERBOSE=1 -j"$NPROC" pip-package

echo "✅ Build and packaging completed."
echo "Wheel located at: build/lib/python_package/pip_package/tiny3d*.whl"

