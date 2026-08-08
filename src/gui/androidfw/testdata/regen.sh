#!/usr/bin/env bash
# Regenerate the androidfw test fixtures from source.
#
# Each subdirectory (axml/, arsc/, arsc_verify/) holds a res/ tree +
# AndroidManifest.xml. This script runs aapt2 (build-tools/36) to compile+link
# each into an apk, extracts the binary artifact (the AXML for layouts, or
# resources.arsc for value tables), and writes both the raw binary and the
# `xxd -i` C header consumed by the tests.
#
#   AAPT2=/path/to/aapt2 ANDROID_JAR=/path/to/android.jar ./regen.sh
#
# Defaults point at the SDK on this machine. The committed *_fixture.h headers
# are canonical; this script reproduces them from the source trees here.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
AAPT2="${AAPT2:-/opt/android-sdk/build-tools/36.0.0/aapt2}"
ANDROID_JAR="${ANDROID_JAR:-/opt/android-sdk/platforms/android-36/android.jar}"
OUT="${OUT:-$HERE/_gen}"
mkdir -p "$OUT"

# build_arsc <name> <header_sym>
# Links <name>/res into an apk, extracts resources.arsc, emits binary + header.
build_arsc() {
    local name="$1"; local sym="$2"; local src="$HERE/$name"
    local work="$OUT/$name"; rm -rf "$work"; mkdir -p "$work"
    ( cd "$work" && "$AAPT2" compile --dir "$src/res" -o compiled.zip >/dev/null
      "$AAPT2" link -I "$ANDROID_JAR" --manifest "$src/AndroidManifest.xml" -o out.apk compiled.zip >/dev/null )
    unzip -o "$work/out.apk" resources.arsc -d "$work" >/dev/null
    cp "$work/resources.arsc" "$HERE/$name/resources.arsc"
    ( cd "$HERE/$name" && cp resources.arsc "$sym.bin" && xxd -i "$sym.bin" \
        | sed "s/${sym}_bin/${sym}/g" > "$OUT/${name}.h" )
    rm -f "$HERE/$name/$sym.bin"
    echo "  $name -> $HERE/$name/resources.arsc (+ $OUT/${name}.h)"
}

# build_axml: links the layout apk, extracts res/layout/*.xml (binary AXML).
build_axml() {
    local name="axml"; local sym="kAXML"; local src="$HERE/$name"
    local work="$OUT/$name"; rm -rf "$work"; mkdir -p "$work"
    ( cd "$work" && "$AAPT2" compile --dir "$src/res" -o compiled.zip >/dev/null
      "$AAPT2" link -I "$ANDROID_JAR" --manifest "$src/AndroidManifest.xml" -o out.apk compiled.zip >/dev/null )
    local bin="$HERE/$name/test.xml"
    unzip -o "$work/out.apk" "res/layout/test.xml" -d "$work" >/dev/null
    cp "$work/res/layout/test.xml" "$bin"
    ( cd "$HERE/$name" && cp test.xml "$sym.bin" && xxd -i "$sym.bin" \
        | sed "s/${sym}_bin/${sym}/g" > "$OUT/${name}.h" )
    rm -f "$HERE/$name/$sym.bin"
    echo "  $name -> $bin (+ $OUT/${name}.h)"
}

echo "Regenerating fixtures into $HERE (binaries) + $OUT (headers):"
build_axml
build_arsc arsc         kARS
build_arsc arsc_verify  kARSV
echo "Done. Compare $OUT/*.h against the committed *_fixture.h."
