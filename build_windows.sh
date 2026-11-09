#!/bin/env bash
# DisasterServer — Windows build (Ninja + MSVC, as in CMakePresets.json)
# Run from: "x64 Native Tools Command Prompt for VS" or Git Bash/MSYS2
# where cl.exe is already in PATH. For WSL use build_linux.sh.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

UNAME_S="$(uname -s 2>/dev/null || echo unknown)"
case "$UNAME_S" in
    MINGW*|MSYS*|CYGWIN*) ENV_KIND="windows-native" ;;
    Linux)
        if grep -qi microsoft /proc/version 2>/dev/null; then
            echo "Error: you are in WSL. This script is for a Windows environment." >&2
            echo "       For a Linux build use ./build_linux.sh" >&2
            exit 1
        fi
        echo "Error: this is native Linux, not Windows." >&2
        echo "       Use ./build_linux.sh" >&2
        exit 1
        ;;
    *) echo "Error: unknown environment ($UNAME_S)." >&2; exit 1 ;;
esac

PRESET="${PRESET:-windows-release}"
BUILD_DIR="build/${PRESET}"
JOBS="${NUMBER_OF_PROCESSORS:-4}"
CLEAN=0
INSTALL=0

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Environment: $ENV_KIND

Options:
  -c, --clean      Remove the build directory
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

if ! command -v cl.exe >/dev/null 2>&1; then
    cat >&2 <<EOF
Error: cl.exe not found in PATH.

Preset "$PRESET" uses Ninja + MSVC (cl.exe).
Run this script from:
  • "x64 Native Tools Command Prompt for VS"  (Start → Visual Studio → x64 Native Tools), or
  • Git Bash after running vcvars64.bat:
        cmd //c "call \"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\" && bash"
    (adjust the VS path to your installation)

Or use a Clang-based preset if configured (e.g. windows-clang-release).
EOF
    exit 1
fi

for cmd in git cmake ninja; do
    command -v "$cmd" >/dev/null 2>&1 \
        || { echo "Error: '$cmd' not found in PATH." >&2; exit 1; }
done

if [[ -z "${VCPKG_ROOT:-}" ]]; then
    if [[ -n "${USERPROFILE:-}" && -d "$USERPROFILE/vcpkg" ]]; then
        export VCPKG_ROOT="$USERPROFILE/vcpkg"
    else
        cat >&2 <<EOF
Error: VCPKG_ROOT is not set.

  git clone https://github.com/microsoft/vcpkg.git %USERPROFILE%\\vcpkg
  %USERPROFILE%\\vcpkg\\bootstrap-vcpkg.bat
  setx VCPKG_ROOT "%USERPROFILE%\\vcpkg"

Then open a new terminal and re-run.
EOF
        exit 1
    fi
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

echo
FOUND=0
for candidate in \
    "$BUILD_DIR/disasterserver/Release/DisasterServerCXX.exe" \
    "$BUILD_DIR/disasterserver/DisasterServerCXX.exe"
do
    if [[ -f "$candidate" ]]; then
        echo "Done: $SCRIPT_DIR/$candidate"
        FOUND=1
        break
    fi
done

if [[ "$FOUND" -eq 0 ]]; then
    echo "Build finished, but .exe not found at the expected paths."
    echo "  Searching in $BUILD_DIR:"
    find "$BUILD_DIR" -maxdepth 4 -type f -name 'DisasterServer*.exe' 2>/dev/null | head -20 || true
fi