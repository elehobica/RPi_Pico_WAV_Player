#!/bin/bash
#------------------------------------------------------
# Copyright (c) 2026, Elehobica
# Released under the BSD-2-Clause
# refer to https://opensource.org/licenses/BSD-2-Clause
#------------------------------------------------------
#
# Local Docker build script that mirrors .github/workflows/build-binaries.yml.
# Uses the same SDK image as CI (elehobica/pico-sdk-dev-docker:sdk-2.2.0)
# and runs cmake/make inside the container.
#
# Run it from anywhere; it builds this repository (the folder containing this
# script), e.g.:
#   ./build_docker.sh          # both targets -> build/ , build2/
#   ./build_docker.sh pico2    # rp2350 only  -> build2/

set -e

IMAGE="elehobica/pico-sdk-dev-docker:sdk-2.2.0"
SDK_PATH_IN_IMAGE="/home/rp2dev/pico/pico-sdk"   # PICO_SDK_PATH inside the container
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"    # repository root (has CMakeLists.txt)
PROJECT_NAME="$(basename "$PROJECT_ROOT")"

# Read the Pico SDK version from pico_sdk_version.cmake inside the container.
sdk_version() {
  docker run --rm "$IMAGE" bash -c '
    f="'"$SDK_PATH_IN_IMAGE"'/pico_sdk_version.cmake"
    maj=$(sed -n "s/^[[:space:]]*set(PICO_SDK_VERSION_MAJOR \([0-9]*\)).*/\1/p" "$f")
    min=$(sed -n "s/^[[:space:]]*set(PICO_SDK_VERSION_MINOR \([0-9]*\)).*/\1/p" "$f")
    rev=$(sed -n "s/^[[:space:]]*set(PICO_SDK_VERSION_REVISION \([0-9]*\)).*/\1/p" "$f")
    if [[ -n "$maj$min$rev" ]]; then echo "${maj}.${min}.${rev}"; else echo "unknown"; fi
  '
}

usage() {
  cat <<EOF
Usage: ./$(basename "$0") [target] [options]

Builds this repository (RPi_Pico_WAV_Player) inside the SDK Docker container.

Targets:
  pico    Build for Pico / Pico W       (rp2040, output: build/)
  pico2   Build for Pico 2 / Pico 2 W   (rp2350, output: build2/)
  all     Build both (default)

Options:
  -h, --help     Show this help
  -k, --keep     Keep build directory contents (incremental build)
EOF
}

TARGET=all
KEEP=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    pico|pico2|all) TARGET="$1" ;;
    -h|--help)      usage; exit 0 ;;
    -k|--keep)      KEEP=1 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
  esac
  shift
done

if [[ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]]; then
  echo "error: CMakeLists.txt not found at $PROJECT_ROOT" >&2
  exit 1
fi

echo "Project: $PROJECT_NAME"
echo "Pico SDK version: $(sdk_version) (from $SDK_PATH_IN_IMAGE)"

# Run cmake/make inside the SDK container.
# Args: $1=build_dir  $2=extra cmake options (may be empty)
run_build() {
  local build_dir="$1"
  local cmake_extra="$2"

  if [[ "$KEEP" -eq 0 ]]; then
    rm -rf "$PROJECT_ROOT/$build_dir"
  fi
  mkdir -p "$PROJECT_ROOT/$build_dir"

  docker run --rm \
    --user "$(id -u):$(id -g)" \
    -e HOME=/tmp \
    -e PICO_SDK_PATH="$SDK_PATH_IN_IMAGE" \
    -e PICO_EXTRAS_PATH=/home/rp2dev/pico/pico-extras \
    -e PICO_EXAMPLES_PATH=/home/rp2dev/pico/pico-examples \
    -v "$PROJECT_ROOT":/workspace \
    -w "/workspace/$build_dir" \
    "$IMAGE" \
    bash -c "cmake $cmake_extra /workspace && make -j\$(nproc)"
}

BUILT_DIRS=()

case "$TARGET" in
  pico|all)
    echo "===== Build Pico (rp2040) ====="
    run_build build ""
    BUILT_DIRS+=(build)
    ;;
esac
case "$TARGET" in
  pico2|all)
    echo "===== Build Pico 2 (rp2350) ====="
    run_build build2 "-DPICO_PLATFORM=rp2350 -DPICO_BOARD=pico2"
    BUILT_DIRS+=(build2)
    ;;
esac

echo ""
echo "===== Output ====="
for d in "${BUILT_DIRS[@]}"; do
  for uf2 in "$PROJECT_ROOT/$d"/*.uf2; do
    [[ -f "$uf2" ]] && echo "  $d/$(basename "$uf2")"
  done
done
