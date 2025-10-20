#!/usr/bin/env bash
# Copy ge_patch.prx to ux0:pspemu/seplugins on a Vita volume mounted via VitaShell USB.
# Usage:
#   scripts/deploy_ge_patch.sh [--volume /Volumes/PSVITA] [--src pspemu_plugin/ge_patch.prx] [--enable-vsh]
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="${ROOT_DIR}/pspemu_plugin/ge_patch.prx"
DEST_FILE="ge_patch.prx"
VOLUME="${VITA_VOL:-}"         # env override: VITA_VOL=/Volumes/PSVITA
ENABLE_VSH=0

while (( $# )); do
  case "$1" in
    --volume) shift; VOLUME="${1:-}"; shift || true ;;
    --src)    shift; SRC="${1:-}";    shift || true ;;
    --enable-vsh) ENABLE_VSH=1; shift ;;
    -h|--help) echo "Usage: $0 [--volume /Volumes/PSVITA] [--src path/to/ge_patch.prx] [--enable-vsh]"; exit 0 ;;
    *) echo "Unknown arg: $1"; exit 1 ;;
  esac
done

if [[ ! -f "$SRC" ]]; then
  echo "ERROR: Source PRX not found: $SRC" >&2
  exit 1
fi

detect_volume() {
  local candidates=()
  while IFS= read -r -d '' vol; do
    [[ -d "$vol/pspemu" ]] && candidates+=("$vol")
  done < <(find /Volumes -maxdepth 1 -type d -print0 2>/dev/null)

  if (( ${#candidates[@]} == 0 )); then
    echo "ERROR: Could not auto-detect Vita volume."
    echo "  • Open VitaShell → START → USB device = Memory Card (ux0:) → O to start USB."
    echo "  • Then run: $0 --volume /Volumes/<YourVitaVolume>"
    exit 1
  elif (( ${#candidates[@]} > 1 )); then
    echo "ERROR: Multiple candidate volumes found:" >&2
    printf '  - %s\n' "${candidates[@]}" >&2
    echo "Pass --volume /Volumes/NAME (or set VITA_VOL) to choose." >&2
    exit 1
  else
    VOLUME="${candidates[0]}"
  fi
}

[[ -n "$VOLUME" ]] || detect_volume

DEST_DIR="${VOLUME}/pspemu/seplugins"
DEST_PATH="${DEST_DIR}/${DEST_FILE}"

echo "==> Vita volume:  $VOLUME"
echo "==> Source PRX:   $SRC"
echo "==> Dest path:    $DEST_PATH"

mkdir -p "$DEST_DIR" || true

# Writability check: try a tiny temp file
TMP="${DEST_DIR}/.write_test.$$"
if ! ( echo "ok" > "$TMP" 2>/dev/null ); then
  echo "ERROR: Destination appears READ-ONLY:"
  echo "  ${DEST_DIR}"
  echo
  echo "Fixes:"
  echo "  • On VitaShell: START → USB device = Memory Card (ux0:) (not game card/other)."
  echo "  • Stop and re-start USB connection (O)."
  echo "  • Close Adrenaline before copying."
  echo "  • If volume is /Volumes/Untitled and still RO, unplug/plug USB and re-enter USB mode."
  echo
  echo "Alternatively, use FTP:"
  echo "  make deploy-psp-ftp HOST=<Vita IP> [PORT=1337]"
  exit 1
fi
rm -f "$TMP" || true

# Backup existing file if present
if [[ -f "$DEST_PATH" ]]; then
  ts="$(date +%Y%m%d-%H%M%S)"
  cp -p "$DEST_PATH" "${DEST_PATH}.bak-${ts}" || true
  echo "    Backed up existing to ${DEST_PATH}.bak-${ts}"
fi

cp -f "$SRC" "$DEST_PATH"
sync
echo "==> Copied."

if (( ENABLE_VSH )); then
  VSH_TXT="${VOLUME}/pspemu/seplugins/vsh.txt"
  LINE="ms0:/seplugins/ge_patch.prx 1"
  touch "$VSH_TXT"
  if ! grep -Fxq "$LINE" "$VSH_TXT"; then
    echo "$LINE" >> "$VSH_TXT"
    echo "==> Appended to vsh.txt: $LINE"
  else
    echo "==> vsh.txt already contains the line."
  fi
fi

echo "Done. (Restart Adrenaline to apply.)"
