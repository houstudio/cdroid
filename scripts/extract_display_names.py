#!/usr/bin/env python3
"""Generate language/region display-name data for i18n.dat (native names).

The upstream OHOS i18n_lite resource set (tools/i18n-dat-tool/resources, 26
json files — mirrored in src/gui/i18n/resource) carries NO language/territory
display names (that lives in OHOS's standard-system ICU i18n, not the lite
profile). This script fills the gap for cdroid::Locale's getDisplayName()
family by mining the NATIVE names from the host glibc locale database
(/usr/share/i18n/locales/*): each glibc file's LC_ADDRESS section carries
lang_name / country_name in the locale's own language (zh_CN → 中文 /
中华人民共和国), encoded either literally or as <UXXXX> escapes.

Output: two json files beside the other i18n resources, keyed by the SAME
locale ids locales.json declares (unknown ids are dropped — the .dat lookup
walks its fallback mask chain anyway):
  languages-display.json   {locale-id: native language name}
  territories-display.json {locale-id: native country/region name}

Usage: extract_display_names.py [glibc-dir]   (default /usr/share/i18n/locales)
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
RESOURCE_DIR = os.path.join(HERE, "..", "src", "gui", "i18n", "resource")

GLIBC_DIR = sys.argv[1] if len(sys.argv) > 1 else "/usr/share/i18n/locales"

_FIELD = re.compile(r'^(lang_name|country_name)\s+"(.*)"\s*(?:%.*)?$')
_UESC = re.compile(r'<U([0-9A-Fa-f]{4,6})>')


def decode_value(raw):
    """glibc value → UTF-8 text: <UXXXX> escapes become code points, the rest
    stays literal. Returns '' when nothing decodable."""
    out = []
    pos = 0
    for m in _UESC.finditer(raw):
        out.append(raw[pos:m.start()])
        out.append(chr(int(m.group(1), 16)))
        pos = m.end()
    out.append(raw[pos:])
    text = "".join(out).strip()
    return text if text and not text.startswith("<") else ""


def parse_glibc(path):
    """→ (lang_name, country_name) native strings, either may be ''."""
    lang = region = ""
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            for line in fh:
                m = _FIELD.match(line.strip())
                if not m:
                    continue
                val = decode_value(m.group(2))
                if m.group(1) == "lang_name" and not lang:
                    lang = val
                elif m.group(1) == "country_name" and not region:
                    region = val
                if lang and region:
                    break
    except OSError:
        pass
    return lang, region


def glibc_candidates(locale_id):
    """dat locale id ('zh-Hans-CN') → (candidate filenames, convention index).
    The convention candidate ({lang}_{LANG}, e.g. de_DE) is the language's
    "home" country file — its country_name is the fallback territory for
    region-less dat ids (the DataResource mask chain lands there when the
    region-bearing id itself has no table entry)."""
    parts = locale_id.split("-")
    lang = parts[0]
    region = parts[-1] if len(parts[-1]) == 2 and parts[-1].isupper() else ""
    cands = []
    if region:
        cands.append("%s_%s" % (lang, region))
    cands.append("%s_%s" % (lang, lang.upper()))
    cands.append(lang)
    n_convention = len(cands)
    # Languages with no {lang}_{LANG} file (ar, am, ...): any {lang}_* sibling
    # carries the same native language name — but NOT a usable home country
    # (ar_AE's UAE is not "the" Arabic country), so no territory from there.
    for f in sorted(os.listdir(GLIBC_DIR)):
        if f.startswith(lang + "_") and "@" not in f:
            cands.append(f)
    return cands, n_convention


def main():
    with open(os.path.join(RESOURCE_DIR, "locales.json"), encoding="utf-8") as fh:
        known = set(json.load(fh)["locales"])

    languages = {}
    territories = {}
    missed = []
    for locale_id in sorted(known):
        parts = locale_id.split("-")
        has_region = len(parts[-1]) == 2 and parts[-1].isupper()
        lang = region = ""
        by_convention = False
        cands, n_convention = glibc_candidates(locale_id)
        for i, cand in enumerate(cands):
            path = os.path.join(GLIBC_DIR, cand)
            if os.path.exists(path):
                lang, region = parse_glibc(path)
                by_convention = (i < n_convention)
                break
        if lang:
            languages[locale_id] = lang
        # Territory entries: region-bearing ids take their own file's country;
        # region-less ids (de, ja) take the CONVENTION file's home country so
        # the DataResource fallback chain (de-DE → de) still resolves; ids
        # matched only through an arbitrary sibling (ar) keep no territory.
        if region and (has_region or by_convention):
            territories[locale_id] = region
        if not lang and not (region and (has_region or by_convention)):
            missed.append(locale_id)

    for name, data in (("languages-display.json", languages),
                       ("territories-display.json", territories)):
        out = os.path.join(RESOURCE_DIR, name)
        with open(out, "w", encoding="utf-8") as fh:
            json.dump(data, fh, ensure_ascii=False, indent=1, sort_keys=True)
        print("%s: %d entries" % (name, len(data)))
    print("locales with neither name (fallback to codes): %d %s"
          % (len(missed), missed[:8]))


if __name__ == "__main__":
    main()
