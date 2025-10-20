#!/usr/bin/env bash
set -euo pipefail

# Source repo (public): contains systemctrl.h and libpspsystemctrl_kernel.a
BASE_URL="https://raw.githubusercontent.com/Operation-DITTO/ctrlHook/main"

# Destination inside this repo
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEST_ROOT="${ROOT_DIR}/third_party/psp_systemctrl"
DEST_INC="${DEST_ROOT}/include"
DEST_LIB="${DEST_ROOT}/lib"

# Flags
FORCE=0
while (( $# )); do
  case "$1" in
    -f|--force) FORCE=1; shift ;;
    *) echo "Usage: $0 [-f|--force]"; exit 1 ;;
  esac
done

echo "==> Installing PSP SystemControl stubs into: ${DEST_ROOT}"
mkdir -p "${DEST_INC}" "${DEST_LIB}"

fetch() {
  local url="$1" out="$2"
  if [[ -f "${out}" && ${FORCE} -eq 0 ]]; then
    echo "    - Skipping (exists): ${out}"
    return
  fi
  echo "    - Downloading ${url}"
  curl -fsSL --retry 3 --retry-delay 1 -o "${out}.tmp" "${url}"
  mv "${out}.tmp" "${out}"
  if [[ ! -s "${out}" ]]; then
    echo "ERROR: Empty download: ${out}" >&2
    exit 1
  fi
}

# Required header + kernel import stub (.a)
fetch "${BASE_URL}/systemctrl.h"                           "${DEST_INC}/systemctrl.h"
fetch "${BASE_URL}/libs/libpspsystemctrl_kernel.a"         "${DEST_LIB}/libpspsystemctrl_kernel.a"

echo "==> Done."
echo "    Include path: ${DEST_INC}"
echo "    Lib path:     ${DEST_LIB}"
echo "    Tip: re-run with --force to re-download."
