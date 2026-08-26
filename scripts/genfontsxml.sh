#!/bin/bash
# Generate an Android-format fonts.xml from the build host's fontconfig.
#
# Desktop runs then follow the product path (curated families + ordered
# fallback) instead of enumerating every installed font at startup. The
# output uses absolute file paths, so it is only valid on the machine that
# built it — regenerate (rerun build.sh) after installing fonts.
#
# Usage: genfontsxml.sh <output-fonts.xml>

set -u

# App-fonts mode: scan a directory of bundled font files. Dispatched first
# so the host-mode positional parsing below never sees --dir.
# ---- App-fonts mode ----------------------------------------------------
# Usage: genfontsxml.sh --dir <fontdir> <output-fonts.xml>
# Scans an app's bundled font files with fc-scan (no fonts.conf needed — the
# legacy app confs hardcode build-machine paths) and emits one <family> per
# typeface family with the real weight/slant per file. The first family also
# backs sans-serif/serif/monospace aliases: the app's own fonts are its whole
# font world, matching the FONTCONFIG_FILE confinement these apps used.
if [ "${1:-}" = "--dir" ]; then
    FONTDIR="${2:?usage: genfontsxml.sh --dir <fontdir> <out>}"
    OUT2="${3:?usage: genfontsxml.sh --dir <fontdir> <out>}"
    command -v fc-scan >/dev/null 2>&1 || { echo "genfontsxml: fc-scan not found" >&2; exit 1; }
    FONTDIR=$(realpath "$FONTDIR")
    [ -d "$FONTDIR" ] || { echo "genfontsxml: no such dir: $FONTDIR" >&2; exit 1; }
    python3 "$(dirname "$0")/genfontsxml_dir.py" "$FONTDIR" "$OUT2"
    exit $?
fi
