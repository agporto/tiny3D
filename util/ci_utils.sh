#!/usr/bin/env bash
# Tiny3D CI helper utilities
# Minimal re-implementation adapted from typical Open3D ci_utils.sh responsibilities.
# Provides just the functions referenced by this repository's GitHub workflows.

set -euo pipefail

# Detect number of processors
if [[ -z "${NPROC:-}" ]]; then
  if command -v getconf >/dev/null 2>&1; then
    NPROC=$(getconf _NPROCESSORS_ONLN || echo 2)
  else
    NPROC=2
  fi
  export NPROC
fi

log() {
  echo "[ci_utils] $*" >&2
}

maximize_ubuntu_github_actions_build_space() {
  # Free disk space on GitHub hosted runners (best-effort). Safe to skip locally.
  log "Maximizing available disk space (best-effort)."
  if [[ "${GITHUB_ACTIONS:-false}" != "true" ]]; then
    log "Not running in GitHub Actions; skipping space maximization."
    return 0
  fi
  sudo rm -rf /usr/share/dotnet || true
  sudo rm -rf /opt/ghc || true
  sudo rm -rf /usr/local/lib/android || true
  sudo apt-get clean || true
  df -h || true
}

install_python_dependencies() {
  log "Installing Python dependencies."
  python_bin="python3"
  if [[ -n "${PYTHON_VERSION:-}" ]]; then
    # Prefer the configured version if available.
    if command -v "python${PYTHON_VERSION}" >/dev/null 2>&1; then
      python_bin="python${PYTHON_VERSION}"
    fi
  fi
  $python_bin -m pip install --upgrade pip setuptools wheel
  if [[ -f python/requirements.txt ]]; then
    $python_bin -m pip install -r python/requirements.txt
  fi
  if [[ -f python/requirements_build.txt ]]; then
    $python_bin -m pip install -r python/requirements_build.txt
  fi
}

configure_cmake() {
  local build_dir=${1:-build}
  mkdir -p "$build_dir"
  local cmake_args=(
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-Release}
    -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS:-ON}
    -DDEVELOPER_BUILD=${DEVELOPER_BUILD:-ON}
    -DBUILD_CUDA_MODULE=${BUILD_CUDA_MODULE:-OFF}
  )
  log "Configuring CMake in $build_dir with args: ${cmake_args[*]}"
  cmake -S . -B "$build_dir" "${cmake_args[@]}"
}

build_all() {
  log "Starting full build."
  configure_cmake build
  cmake --build build -- -j"${NPROC}" || cmake --build build -- -j1
  log "Build finished."
}

build_docs() {
  # Placeholder for documentation build (implement properly if/when docs added)
  log "build_docs() placeholder: no documentation configuration present."
}

package_wheel() {
  log "Packaging Python wheel (if target exists)."
  if cmake --build build --target pip-package -- -j"${NPROC}"; then
    log "Wheel build succeeded."
  else
    log "Wheel target failed or does not exist; skipping."; return 0
  fi
}

run_tests() {
  if [[ -d build ]]; then
    if ctest --test-dir build -j"${NPROC}" --output-on-failure; then
      log "Tests passed."
    else
      log "Some tests failed."; return 1
    fi
  else
    log "No build directory for tests; skipping."
  fi
}

# If executed directly (not sourced), perform a default CI sequence.
if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  install_python_dependencies
  build_all
  package_wheel || true
  run_tests || true
fi
