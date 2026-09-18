#!/usr/bin/env python3
"""Extend resource/measure-format-patterns.json with duration units.

The duration-day/hour/minute/second SHORT words below are extracted from the
HOST ICU 70 (scripts/extract_measure_units.cc) — the same library family
android.text.format.Formatter delegates to. Not hand-typed; re-run the
extractor to re-verify.

Notes:
 - fr: ICU uses U+202F (NNBSP) before day/hour/second but U+00A0 (NBSP)
   before minute. The engine's join pattern is per-LOCALE, so fr's pattern is
   "%s%s" and each separator is baked into the word slots.
 - ru: ICU 70 SHORT is "дн./ч/мин/с" joined by a plain space — pattern
   "%s %s" with bare words. (An older expectation of a spaceless "1мин"
   matches no ICU-70 width; the test expectation was corrected instead.)
 - zh-Hans: ICU joins with no separator ("2天"), but the locale's existing
   pattern "%s %s" serves the OHOS fitness units too and cannot change
   per-unit — zh duration renders as "2 天" for now (documented deviation,
   no test oracle).
 - Plural slots are CLDR order [zero, one, two, few, many, other]; the engine
   falls back to the "other" slot for an empty one.
"""
import json
import sys

RES = "src/gui/content/i18n/resource/measure-format-patterns.json"

def unit(words):
    """words: dict of plural-key -> string; returns the 4-width unit node."""
    slots = ["" for _ in range(6)]
    for key, val in words.items():
        slots[["zero", "one", "two", "few", "many", "other"].index(key)] = val
    short = list(slots)
    return {"short": short, "medium": ["" for _ in range(6)],
            "long": ["" for _ in range(6)], "full": ["" for _ in range(6)]}

def main():
    with open(RES, "r", encoding="utf-8") as f:
        data = json.load(f)

    # --- en-US: extend the fitness unit set with the four duration keys -----
    en = data["en-US"]
    assert en["unit_num"] == "16"
    en["unit_num"] = "20"
    en["unit_set"] += "|day|hour|minute|second"
    en["units"]["day"] = unit({"one": "day", "other": "days"})
    en["units"]["hour"] = unit({"one": "hr", "other": "hr"})
    en["units"]["minute"] = unit({"one": "min", "other": "min"})
    en["units"]["second"] = unit({"one": "sec", "other": "sec"})

    # --- zh-Hans: same extension (separator caveat in the header note) ------
    zh = data["zh-Hans"]
    assert zh["unit_num"] == "16"
    zh["unit_num"] = "20"
    zh["unit_set"] += "|day|hour|minute|second"
    zh["units"]["day"] = unit({"other": "天"})
    zh["units"]["hour"] = unit({"other": "小时"})
    zh["units"]["minute"] = unit({"other": "分钟"})
    zh["units"]["second"] = unit({"other": "秒"})

    # --- fr: new locale entry; separators baked into the words --------------
    NNBSP, NBSP = "\u202f", "\u00a0"
    data["fr"] = {
        "unit_num": "4",
        "unit_set": "day|hour|minute|second",
        "pattern": "%s%s",
        "order": "#",
        "units": {
            "day": unit({"one": NNBSP + "j", "many": NNBSP + "j",
                         "other": NNBSP + "j"}),
            "hour": unit({"one": NNBSP + "h", "many": NNBSP + "h",
                          "other": NNBSP + "h"}),
            "minute": unit({"one": NBSP + "min", "many": NBSP + "min",
                            "other": NBSP + "min"}),
            "second": unit({"one": NNBSP + "s", "many": NNBSP + "s",
                            "other": NNBSP + "s"}),
        },
    }

    # --- ru: new locale entry; plain-space join, bare words -----------------
    data["ru"] = {
        "unit_num": "4",
        "unit_set": "day|hour|minute|second",
        "pattern": "%s %s",
        "order": "#",
        "units": {
            "day": unit({"one": "дн.", "few": "дн.",
                         "many": "дн.", "other": "дн."}),
            "hour": unit({"one": "ч", "few": "ч",
                          "many": "ч", "other": "ч"}),
            "minute": unit({"one": "мин", "few": "мин",
                            "many": "мин", "other": "мин"}),
            "second": unit({"one": "с", "few": "с",
                            "many": "с", "other": "с"}),
        },
    }

    with open(RES, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
        f.write("\n")
    print("extended", RES)

if __name__ == "__main__":
    sys.exit(main())
