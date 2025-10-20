#!/usr/bin/env bash
# Enable/disable/status for ge_patch.prx in ux0:/pspemu/seplugins/vsh.txt via VitaShell USB.
# Usage: scripts/vsh_plugin.sh <enable|disable|status> [--volume /Volumes/PSVITA]
set -euo pipefail

ACTION="${1:-}"; shift || true
VOLUME="${VITA_VOL:-}"
while (( $# )); do
  case "$1" in
    --volume) shift; VOLUME="${1:-}"; shift || true ;;
    -h|--help) echo "Usage: $0 <enable|disable|status> [--volume /Volumes/PSVITA]"; exit 0 ;;
    *) echo "Unknown arg: $1"; exit 1 ;;
  esac
done

if [[ -z "${ACTION}" ]]; then
  echo "Usage: $0 <enable|disable|status> [--volume /Volumes/PSVITA]"; exit 1
fi

detect_volume() {
  local candidates=()
  while IFS= read -r -d '' vol; do
    [[ -d "$vol/pspemu" ]] && candidates+=("$vol")
  done < <(find /Volumes -maxdepth 1 -type d -print0 2>/dev/null)

  if (( ${#candidates[@]} == 0 )); then
    echo "ERROR: Could not auto-detect Vita volume."
    echo "  • In VitaShell: START → USB device = Memory Card (ux0:) → O to start USB."
    echo "  • Or pass --volume /Volumes/PSVITA"
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

VSH_TXT="${VOLUME}/pspemu/seplugins/vsh.txt"
LINE_ENABLED="ms0:/seplugins/ge_patch.prx 1"
LINE_DISABLED="ms0:/seplugins/ge_patch.prx 0"
mkdir -p "$(dirname "$VSH_TXT")"
touch "$VSH_TXT"

# Basic write test (macOS privacy or RO mount will fail here)
TMP_DIR="$(dirname "$VSH_TXT")"
TMP="${TMP_DIR}/.write_test.$$"
if ! ( echo ok > "$TMP" 2>/dev/null ); then
  echo "ERROR: Cannot write to: ${TMP_DIR}"
  echo "  • On VitaShell: USB device = Memory Card (ux0:), restart USB (O)."
  echo "  • Close Adrenaline before copying."
  echo "  • macOS: grant Terminal \"Removable Volumes\" (Privacy → Files & Folders), then relaunch Terminal."
  exit 1
fi
rm -f "$TMP" || true

status() {
  if grep -Eq '^ms0:/seplugins/ge_patch\.prx[[:space:]]+1[[:space:]]*$' "$VSH_TXT"; then
    echo "ge_patch in VSH: ENABLED"
  elif grep -Eq '^ms0:/seplugins/ge_patch\.prx[[:space:]]+0[[:space:]]*$' "$VSH_TXT"; then
    echo "ge_patch in VSH: DISABLED"
  else
    echo "ge_patch in VSH: NOT LISTED (treated as disabled)"
  fi
}

write_line() {
  local line="$1"
  local tmpfile
  tmpfile="$(mktemp)"
  # Remove any existing ge_patch lines, then append the desired one
  awk '!/^ms0:\/seplugins\/ge_patch\.prx([[:space:]]+([01]))?[[:space:]]*$/' "$VSH_TXT" > "$tmpfile"
  echo "$line" >> "$tmpfile"
  mv "$tmpfile" "$VSH_TXT"
}

case "$ACTION" in
  status)
    echo "Vita volume: $VOLUME"
    status
    ;;
  enable)
    echo "Vita volume: $VOLUME"
    write_line "$LINE_ENABLED"
    echo "ge_patch set to ENABLED in vsh.txt"
    ;;
  disable)
    echo "Vita volume: $VOLUME"
    write_line "$LINE_DISABLED"
    echo "ge_patch set to DISABLED in vsh.txt"
    ;;
  *)
    echo "Unknown action: $ACTION"; exit 1 ;;
esac
