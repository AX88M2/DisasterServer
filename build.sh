#!/usr/bin/env bash
# ============================
#  DisasterServer build script
#  Usage: ./build.sh [options]
# ============================
set -euo pipefail

# colored output
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
BLUE='\033[0;34m'; NC='\033[0m'

info()  { echo -e "${BLUE}[*]${NC} $*"; }
ok()    { echo -e "${GREEN}[+]${NC} $*"; }
warn()  { echo -e "${YELLOW}[!]${NC} $*"; }
err()   { echo -e "${RED}[-]${NC} $*" >&2; }

# defaults
BUILD_DIR="build"
BUILD_TYPE="Release"
WITH_UI="OFF"
GENERATOR=""
CLEAN=0
JOBS="$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

# argument parsing
usage() {
    cat <<EOF
Usage: $0 [options]

Options:
  -u, --ui             Build with UI (BUILD_UI=ON)
  -d, --debug          Debug build (Release by default)
  -b, --build-dir DIR  Build directory (default: build)
  -g, --generator GEN  CMake generator, e.g. "MinGW Makefiles" or "Ninja"
  -j, --jobs N         Number of parallel jobs (default: auto)
  -c, --clean          Remove the build directory before building
  -h, --help           Show this help

Examples:
  $0                          # regular build without UI (Release)
  $0 --ui                     # build with UI (Ninja if available)
  $0 -u -g "MinGW Makefiles"  # UI + MinGW
  $0 -c -u                    # clean build with UI
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -u|--ui)          WITH_UI="ON"; shift ;;
        -d|--debug)       BUILD_TYPE="Debug"; shift ;;
        -b|--build-dir)   BUILD_DIR="$2"; shift 2 ;;
        -g|--generator)   GENERATOR="$2"; shift 2 ;;
        -j|--jobs)        JOBS="$2"; shift 2 ;;
        -c|--clean)       CLEAN=1; shift ;;
        -h|--help)        usage; exit 0 ;;
        *) err "Unknown argument: $1"; usage; exit 1 ;;
    esac
done

# environment checks
command -v cmake >/dev/null 2>&1 || { err "cmake not found in PATH"; exit 1; }
command -v git   >/dev/null 2>&1 || { err "git not found in PATH"; exit 1; }

info "cmake: $(cmake --version | head -n1)"
info "Mode: BUILD_TYPE=$BUILD_TYPE, BUILD_UI=$WITH_UI"
[[ -n "$GENERATOR" ]] && info "Generator: $GENERATOR"

# clean
if [[ $CLEAN -eq 1 && -d "$BUILD_DIR" ]]; then
    warn "Removing $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# configure
CMAKE_ARGS=(
    -S .
    -B "$BUILD_DIR"
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DBUILD_UI="$WITH_UI"
)

if [[ -n "$GENERATOR" ]]; then
    CMAKE_ARGS+=( -G "$GENERATOR" )
else
    # ninja
    if command -v ninja >/dev/null 2>&1; then
        info "Ninja found, using it"
        CMAKE_ARGS+=( -G Ninja )
    fi
fi

info "Configuring: cmake ${CMAKE_ARGS[*]}"
cmake "${CMAKE_ARGS[@]}"

# build
info "Building: cmake --build $BUILD_DIR --config $BUILD_TYPE -j $JOBS"
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j "$JOBS"

ok "Done. Artifacts in: $BUILD_DIR"