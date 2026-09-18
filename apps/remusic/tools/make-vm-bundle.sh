#!/bin/bash
# Assemble a self-sufficient VM verification bundle for the remusic port:
# binary + paks + vcpkg .so closure + fonts + test music + launcher. Run on
# the build host (from anywhere); the tarball lands in /tmp.
#
# Layout mirrors the out tree's relative structure (exe at apps/remusic/,
# paks at the root) so the pak/font walk-up discovery keeps working.
set -euo pipefail

OUT=${1:-/home/houzh/cdroidMaster/outX64-Debug}
VCPKG_TRI=${2:-x64-linux-dynamic}
B=/tmp/remusic-vm-stage
TARBALL=/tmp/remusic-vm-bundle.tar.gz

command -v ldd >/dev/null || { echo "need ldd"; exit 1; }
[ -x "$OUT/apps/remusic/remusic" ] || { echo "build $OUT first (make remusic)"; exit 1; }

rm -rf "$B" && mkdir -p "$B/lib" "$B/apps/remusic" "$B/fonts" "$B/music"

# --- app + resources (relative structure matters) ---
cp "$OUT/apps/remusic/remusic" "$B/apps/remusic/"
cp "$OUT/remusic.pak" "$OUT/cdroid.pak" "$OUT/widgetex.pak" "$B/" 2>/dev/null \
    || cp "$OUT/apps/remusic/remusic.pak" "$B/"
cp "$OUT/fonts.xml" "$B/fonts.xml.in"

# --- fonts referenced by fonts.xml (absolute host paths get rewritten) ---
mkdir -p ~/.fonts 2>/dev/null || true
cp -r ~/.fonts/sat63069/. "$B/fonts/" 2>/dev/null || echo "WARN: ~/.fonts/sat63069 missing — Chinese text will tofu"

# --- test music: real WAV (decodes, real duration) + fake mp3s + lrc + cover ---
cp "$OUT/music/"* "$B/music/" 2>/dev/null || echo "WARN: no $OUT/music"

# --- transitive .so closure: everything outside system lib dirs ---
collect() { ldd "$1" 2>/dev/null | awk '$3 ~ /^\// {print $3}'; }
declare -A seen=()
front=("$OUT/apps/remusic/remusic")
while [ ${#front[@]} -gt 0 ]; do
    next=()
    for so in "${front[@]}"; do
        for dep in $(collect "$so"); do
            case "$dep" in
                /lib/*|/usr/lib/*|/lib64/*) continue ;;
            esac
            base=$(basename "$dep")
            [ -n "${seen[$base]:-}" ] && continue
            seen[$base]=1
            cp -L "$dep" "$B/lib/$base"
            next+=("$B/lib/$base")
        done
    done
    front=("${next[@]}")
done
echo "bundled ${#seen[@]} non-system .so files"

# --- launcher: rewrites font paths, prints the sound-card state, runs the app ---
cat > "$B/start.sh" <<'EOF'
#!/bin/bash
# remusic VM verification launcher.
#   ./start.sh                     interactive run
#   ./start.sh --test-script v.dsl scripted run (each step ~3s)
HERE=$(cd "$(dirname "$0")" && pwd)
sed "s|/home/houzh/.fonts/sat63069|$HERE/fonts|g" "$HERE/fonts.xml.in" > "$HERE/fonts.xml"
echo "== sound cards visible to ALSA =="
if command -v aplay >/dev/null 2>&1; then aplay -l 2>&1 | sed 's/^/  /'
else echo "  (aplay not installed; install alsa-utils to inspect)"; fi
export LD_LIBRARY_PATH="$HERE/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$HERE"
SCREEN_SIZE="${SCREEN_SIZE:-480x800}" DISPLAY="${DISPLAY:-:0}" \
    exec ./apps/remusic/remusic "$@"
EOF
chmod +x "$B/start.sh"

# --- scripted verification: navigate, play the real WAV, open the player ---
cat > "$B/vm-verify.dsl" <<'EOF'
sleep 3000
click text=本地音乐
sleep 2000
click text=测试音
sleep 2500
click text=测试音
sleep 2000
EOF

tar -C /tmp -czf "$TARBALL" remusic-vm-stage
rm -rf /tmp/remusic-vm && mv "$B" /tmp/remusic-vm
echo "bundle: $TARBALL ($(du -h "$TARBALL" | cut -f1))"
sha256sum "$TARBALL"
