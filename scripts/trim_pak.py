#!/usr/bin/env python3
# ----------------------------------------------------------------------------
# trim_pak.py — post-process a cdroid.pak blueprint to drop density/locale
# resource dirs not needed by a product.
#
# Product config (json):
#   {"densities": ["hdpi"], "locales": ["zh", "en"]}
#
# Drops entries under drawable-*/mipmap-*/values-* whose density/locale
# qualifier is NOT in the keep list. resources.arsc and other types (anim,
# layout, color, ...) are untouched.
#
# CDROID embedded devices run with a fixed density/locale that the product
# declares in the keep list, so ResTable always resolves to a kept variant
# (whose file is still in the pak). Only a device whose config is NOT in the
# keep list would hit a deleted variant — which won't happen for a released
# product with a fixed config.
#
# Usage:
#   trim_pak.py <blueprint.pak> --config <product.json> -o <product.pak>
# ----------------------------------------------------------------------------
import sys
import os
import json
import zipfile
import argparse

DENSITY_QUALS = {"ldpi", "mdpi", "hdpi", "tvdpi", "xhdpi", "xxhdpi", "xxxhdpi"}
# qualifiers that look like short alpha tokens but are NOT locales
NON_LOCALE_QUALS = {"v", "w", "sw", "tv", "car"}

# density qualifiers that are density-independent and always kept
DENSITY_INDEPENDENT = {"nodpi", "anydpi"}


def split_qualifiers(dirseg):
    """'drawable-hdpi-v4' -> ('drawable', ['hdpi', 'v4'])."""
    parts = dirseg.split("-")
    return parts[0], parts[1:]


def find_density(quals):
    for q in quals:
        if q in DENSITY_QUALS:
            return q
    return None


def find_locale(quals):
    """First qual that is a 2-3 letter language code (optionally -rXX / -vN).

    'zh', 'en', 'zh-rCN' → 'zh'/'en'; 'land'/'round'/'v4' → not locale."""
    for q in quals:
        base = q.split("-r")[0].split("-v")[0]
        if (base and base.isalpha() and 2 <= len(base) <= 3
                and base.lower() not in NON_LOCALE_QUALS):
            return base.lower()
    return None


def should_drop(entry_path, keep_dens, keep_loc, exclude):
    # exclude by resource basename (e.g. default_wallpaper, watch.png)
    fname = entry_path.split("/")[-1]
    base = fname.rsplit(".", 1)[0]  # strip extension (.png / .9.png handled: foo.9 → foo.9, see note)
    if base in exclude or fname in exclude:
        return True
    parts = entry_path.split("/")
    if len(parts) < 2:
        return False
    base_dir, quals = split_qualifiers(parts[0])
    if base_dir not in ("drawable", "mipmap", "values"):
        return False
    dens = find_density(quals)
    if dens and dens not in keep_dens:
        return True
    loc = find_locale(quals)
    if loc and loc not in keep_loc:
        return True
    return False


def main():
    ap = argparse.ArgumentParser(description="Trim density/locale dirs from a cdroid.pak blueprint.")
    ap.add_argument("input", help="blueprint cdroid.pak (full density/locale)")
    ap.add_argument("--config", required=True, help="product json: {densities:[...], locales:[...]}")
    ap.add_argument("-o", "--output", required=True, help="output product cdroid.pak")
    args = ap.parse_args()

    cfg = json.load(open(args.config))
    keep_dens = set(cfg.get("densities", [])) | DENSITY_INDEPENDENT
    keep_loc = set(l.lower() for l in cfg.get("locales", []))
    exclude = set(cfg.get("exclude", []))

    in_size = os.path.getsize(args.input)
    removed = 0
    kept = 0
    with zipfile.ZipFile(args.input) as zin, \
         zipfile.ZipFile(args.output, "w", zipfile.ZIP_DEFLATED) as zout:
        for item in zin.infolist():
            name = item.filename
            if name.endswith("/"):
                continue
            if should_drop(name, keep_dens, keep_loc, exclude):
                removed += 1
                continue
            zout.writestr(item, zin.read(name))
            kept += 1

    out_size = os.path.getsize(args.output)
    sys.stderr.write("trim_pak: removed %d entries; %d -> %d bytes (saved %d)\n"
                     % (removed, in_size, out_size, in_size - out_size))


if __name__ == "__main__":
    main()
