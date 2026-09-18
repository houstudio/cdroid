// Extract duration-unit display words (ICU MeasureFormat SHORT) for the
// i18n measure-format-patterns table. Values are produced by the HOST ICU
// (the same library family android.text.format.Formatter delegates to on
// AOSP) — never hand-typed.
//
// Build & run:
//   g++ -std=c++17 scripts/extract_measure_units.cc -o /tmp/extract_units \
//       $(pkg-config --cflags --libs icu-io) && /tmp/extract_units
//
// Output: one line per (locale, unit, n):
//   locale unit n=<n> plural=<keyword> full=<formatted> num=<number-part>
//   sep=<separator codepoints, hex> word=<unit word>
// The sep/word split is derived by stripping the formatted number prefix;
// plural=<keyword> comes from ICU PluralRules (CLDR categories: zero/one/two/
// few/many/other). Assembly into resource/measure-format-patterns.json is
// manual from this output.

#include <unicode/measfmt.h>
#include <unicode/measunit.h>
#include <unicode/fmtable.h>
#include <unicode/measunit.h>
#include <unicode/numfmt.h>
#include <unicode/plurrule.h>
#include <unicode/unistr.h>
#include <cstdio>
#include <string>

static bool isSepChar(UChar32 c) {
    return c == 0x20 || c == 0xA0 || c == 0x202F || c == 0x2009; // space NBSP NNBSP thin
}

static std::string toUtf8(const icu::UnicodeString& u) {
    std::string s;
    u.toUTF8String(s);
    return s;
}

static std::string sepHex(const std::string& sep) {
    std::string out;
    char buf[16];
    for (size_t i = 0; i < sep.size();) {
        UChar32 c;
        U8_NEXT(sep.c_str(), i, (int)sep.size(), c);
        snprintf(buf, sizeof(buf), "%s%04X", out.empty() ? "" : ",", c);
        out += buf;
    }
    return out;
}

int main() {
    const char* locales[] = {"en-US", "fr", "ru", "zh-Hans"};
    const char* unitNames[] = {"day", "hour", "minute", "second"};
    const double ladder[] = {0, 1, 1.5, 2, 3, 5, 11, 21, 100, 1000000};

    for (const char* loc : locales) {
        icu::Locale locale(loc);
        UErrorCode status = U_ZERO_ERROR;
        icu::MeasureFormat mf(locale, UMEASFMT_WIDTH_SHORT, status);
        icu::NumberFormat* nf = icu::NumberFormat::createInstance(locale, status);
        icu::PluralRules* rules = icu::PluralRules::forLocale(locale, status);
        if (U_FAILURE(status)) { printf("locale %s init failed\n", loc); continue; }

        icu::MeasureUnit* units[4] = {
            icu::MeasureUnit::createDay(status), icu::MeasureUnit::createHour(status),
            icu::MeasureUnit::createMinute(status), icu::MeasureUnit::createSecond(status)};

        for (int u = 0; u < 4; u++) {
            for (double n : ladder) {
                icu::FieldPosition fp;
                icu::UnicodeString full;
                // Measure adopts the unit pointer; clone per call. ICU 70's
                // single-measure formatMeasure is private — use formatMeasures.
                icu::Measure measure(icu::Formattable(n), units[u]->clone(), status);
                mf.formatMeasures(&measure, 1, full, fp, status);
                if (U_FAILURE(status)) { printf("format failed\n"); status = U_ZERO_ERROR; continue; }
                std::string fullS = toUtf8(full);
                icu::UnicodeString numU;
                std::string numS = toUtf8(nf->format(n, numU));
                std::string sep, word;
                if (fullS.compare(0, numS.size(), numS) == 0) {
                    std::string rest = fullS.substr(numS.size());
                    size_t i = 0;
                    while (i < rest.size()) {
                        UChar32 c;
                        int prev = (int)i;
                        U8_NEXT(rest.c_str(), i, (int)rest.size(), c);
                        if (!isSepChar(c)) { i = prev; break; }
                    }
                    sep = rest.substr(0, i);
                    word = rest.substr(i);
                } else {
                    word = "<number-prefix-mismatch>";
                }
                printf("%s %s n=%g plural=%s full=[%s] num=[%s] sep=%s word=[%s]\n",
                        loc, unitNames[u], n, toUtf8(rules->select(n)).c_str(),
                        fullS.c_str(), numS.c_str(), sepHex(sep).c_str(), word.c_str());
            }
        }
        delete nf;
        delete rules;
    }
    return 0;
}
