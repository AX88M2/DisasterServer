#!/usr/bin/env bash
# DisasterServer — GNU/Linux build (Ninja + Clang, as in CMakePresets.json)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

PRESET="linux-release"
BUILD_DIR="build/${PRESET}"
JOBS="$(nproc 2>/dev/null || echo 4)"
CLEAN=0
INSTALL=0

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  -c, --clean      Remove the build directory before building
  -j, --jobs N     Parallel jobs (default: $JOBS)
      --install    Run cmake --install after building
  -h, --help       Show this help
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--clean)   CLEAN=1; shift ;;
        -j|--jobs)    JOBS="$2"; shift 2 ;;
        --install)    INSTALL=1; shift ;;
        -h|--help)    usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    esac
done

for cmd in git cmake ninja clang++; do
    command -v "$cmd" >/dev/null 2>&1 \
        || { echo "Error: '$cmd' not found. Install it (see CI dependencies in .github/workflows/build.yml)." >&2; exit 1; }
done

if [[ -z "${VCPKG_ROOT:-}" ]]; then
    cat >&2 <<EOF
Error: VCPKG_ROOT is not set.

  git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
  ~/vcpkg/bootstrap-vcpkg.sh
  export VCPKG_ROOT=~/vcpkg
EOF
    exit 1
fi

[[ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]] \
    || { echo "Error: vcpkg toolchain not found in $VCPKG_ROOT" >&2; exit 1; }

[[ -f .gitmodules ]] && { echo "==> Updating submodules"; git submodule update --init --recursive; }

if [[ "$CLEAN" -eq 1 && -d "$BUILD_DIR" ]]; then
    echo "==> Cleaning $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

echo "==> cmake --preset $PRESET"
cmake --preset "$PRESET"

echo "==> Building ($JOBS jobs)"
cmake --build "$BUILD_DIR" --parallel "$JOBS"

if [[ "$INSTALL" -eq 1 ]]; then
    echo "==> cmake --install $BUILD_DIR"
    cmake --install "$BUILD_DIR"
fi

BIN="$BUILD_DIR/disasterserver/DisasterServerCXX"
echo
if [[ -f "$BIN" ]]; then
    echo "Done: $SCRIPT_DIR/$BIN"
else
    echo "Build finished, but binary not found at the expected path."
    echo "  Searching in $BUILD_DIR:"
    find "$BUILD_DIR" -maxdepth 3 -type f -name 'DisasterServer*' 2>/dev/null | head -20 || true
fi