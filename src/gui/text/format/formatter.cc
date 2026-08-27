/*********************************************************************************
 * Port of android.text.format.Formatter, android_12 line (Apache 2.0).
 * See formatter.h for the version choice and locale conventions.
 *
 * KNOWN DEVIATIONS vs AOSP:
 *  - bidiWrap: android.text.BidiFormatter is not ported (see the text API-gap
 *    records), so RTL locales get the unwrapped string.
 *  - formatShortElapsedTime's MeasureFormat SHORT units are an inline en-US
 *    table (day/days, hr, min, sec, joined by ", "); other locales get the
 *    English units until an i18n measure-word table exists (FormatterTest's
 *    FR/RU assertions cover this gap).
 *  - String.format("%.Nf")'s HALF_UP is reproduced by pre-rounding through
 *    llround(result * roundFactor) — exactly the arithmetic AOSP uses for
 *    roundedBytes — before handing the value to NumberFormat, so the printed
 *    digits and roundedBytes can never disagree (9123 → "9.12", not
 *    half-even "9.12" vs "9.13" drift on ties).
 *********************************************************************************/
#include <text/format/formatter.h>
#include <content/resources.h>
#include <content/Locale.h>
#include <content/numberformat.h>
#include <core/context.h>
#include <widget/internal_R.h>
#include <text/textutils.h>
#include <cmath>
#include <cstdio>

namespace cdroid {

using namespace cdroid::internal;   // framework R namespace (suffix string ids)

// AOSP localeFromContext(); CDROID convention is Locale::getDefault() (see
// header note) — the tests drive it with Locale::setDefault().
static Locale localeFromContext(Context& context) {
    return context.getResources().getConfiguration().getLocales().get(0);
}

// AOSP bidiWrap(); BidiFormatter is not ported — return the source as-is.
static std::string bidiWrap(Context& context, std::string source) {
    // AOSP consults the locale's layout direction and wraps through
    // BidiFormatter in RTL locales; BidiFormatter is not ported, so the source
    // comes back unchanged (recorded deviation, file header).
    (void)context;
    return source;
}

std::string Formatter::formatFileSize(Context* context, int64_t sizeBytes) {
    return formatFileSize(context, sizeBytes, FLAG_SI_UNITS);
}

std::string Formatter::formatFileSize(Context* context, int64_t sizeBytes, int flags) {
    if (context == nullptr) {
        return "";
    }
    const BytesResult res = formatBytes(context->getResources(), sizeBytes, flags);
    return bidiWrap(*context, context->getResources().getString(R::string::fileSizeSuffix,
            { res.value, res.units }));
}

std::string Formatter::formatShortFileSize(Context* context, int64_t sizeBytes) {
    if (context == nullptr) {
        return "";
    }
    const BytesResult res = formatBytes(context->getResources(), sizeBytes,
            FLAG_SI_UNITS | FLAG_SHORTER);
    return bidiWrap(*context, context->getResources().getString(R::string::fileSizeSuffix,
            { res.value, res.units }));
}

Formatter::BytesResult Formatter::formatBytes(Resources& res, int64_t sizeBytes, int flags) {
    const int unit = ((flags & FLAG_IEC_UNITS) != 0) ? 1024 : 1000;
    const bool isNegative = (sizeBytes < 0);
    float result = isNegative ? (float)-sizeBytes : (float)sizeBytes;
    int suffix = R::string::byteShort;
    int64_t mult = 1;
    if (result > 900) {
        suffix = R::string::kilobyteShort;
        mult = unit;
        result = result / unit;
    }
    if (result > 900) {
        suffix = R::string::megabyteShort;
        mult *= unit;
        result = result / unit;
    }
    if (result > 900) {
        suffix = R::string::gigabyteShort;
        mult *= unit;
        result = result / unit;
    }
    if (result > 900) {
        suffix = R::string::terabyteShort;
        mult *= unit;
        result = result / unit;
    }
    if (result > 900) {
        suffix = R::string::petabyteShort;
        mult *= unit;
        result = result / unit;
    }
    // Note we calculate the rounded long by ourselves, but still let the
    // number formatter compute the printed value (AOSP: String.format; here a
    // pre-rounded double through NumberFormat, see the file header).
    int roundFactor;
    int fractionDigits;
    if (mult == 1 || result >= 100) {
        roundFactor = 1;
        fractionDigits = 0;
    } else if (result < 1) {
        roundFactor = 100;
        fractionDigits = 2;
    } else if (result < 10) {
        if ((flags & FLAG_SHORTER) != 0) {
            roundFactor = 10;
            fractionDigits = 1;
        } else {
            roundFactor = 100;
            fractionDigits = 2;
        }
    } else { // 10 <= result < 100
        if ((flags & FLAG_SHORTER) != 0) {
            roundFactor = 1;
            fractionDigits = 0;
        } else {
            roundFactor = 100;
            fractionDigits = 2;
        }
    }

    if (isNegative) {
        result = -result;
    }

    // The HALF_UP pre-round AOSP performs for roundedBytes, applied to the
    // printed value too (llround rounds halves away from zero = Java's
    // Math.round for these magnitudes).
    const float rounded = (float)std::llround(result * roundFactor) / roundFactor;

    // Note this might overflow if abs(result) >= Long.MAX_VALUE / 100, but
    // that's like 80PB so it's okay (for now)...
    const int64_t roundedBytes =
            (flags & FLAG_CALCULATE_ROUNDED) == 0 ? 0
            : (((int64_t) std::llround(result * roundFactor)) * mult / roundFactor);

    const std::string units = res.getString(suffix);

    const Locale locale = res.getConfiguration().getLocales().get(0);
    std::unique_ptr<NumberFormat> numberFormatter = NumberFormat::getInstance(locale);
    numberFormatter->setMinimumFractionDigits(fractionDigits);
    numberFormatter->setMaximumFractionDigits(fractionDigits);
    return BytesResult(numberFormatter->format(rounded), units, roundedBytes);
}

std::string Formatter::formatIpAddress(int ipv4Address) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d",
            ipv4Address & 0xFF, (ipv4Address >> 8) & 0xFF,
            (ipv4Address >> 16) & 0xFF, (ipv4Address >> 24) & 0xFF);
    return buf;
}

static constexpr int SECONDS_PER_MINUTE = 60;
static constexpr int SECONDS_PER_HOUR = 60 * 60;
static constexpr int SECONDS_PER_DAY = 24 * 60 * 60;
static constexpr int64_t MILLIS_PER_MINUTE = 1000 * 60;

// ICU MeasureFormat SHORT (en-US) stand-in: unit words for the value counts,
// pluralized the way ICU's short units read ("1 day, 23 hr", "3 days").
static std::string measureShort(int64_t value, const char* single, const char* plural) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld %s", (long long)value,
            value == 1 ? single : plural);
    return buf;
}

static std::string joinMeasures(const std::string& a, const std::string& b) {
    return a + ", " + b;
}

std::string Formatter::formatShortElapsedTime(Context& /*context*/, int64_t millis) {
    // AOSP builds a locale MeasureFormat here; the en-US unit-word table
    // (file header) needs no locale, so the parameter goes unused.
    int64_t secondsLong = millis / 1000;

    int days = 0, hours = 0, minutes = 0;
    if (secondsLong >= SECONDS_PER_DAY) {
        days = (int)(secondsLong / SECONDS_PER_DAY);
        secondsLong -= days * SECONDS_PER_DAY;
    }
    if (secondsLong >= SECONDS_PER_HOUR) {
        hours = (int)(secondsLong / SECONDS_PER_HOUR);
        secondsLong -= hours * SECONDS_PER_HOUR;
    }
    if (secondsLong >= SECONDS_PER_MINUTE) {
        minutes = (int)(secondsLong / SECONDS_PER_MINUTE);
        secondsLong -= minutes * SECONDS_PER_MINUTE;
    }
    const int seconds = (int)secondsLong;

    if (days >= 2 || (days > 0 && hours == 0)) {
        days += (hours + 12) / 24;
        return measureShort(days, "day", "days");
    } else if (days > 0) {
        return joinMeasures(measureShort(days, "day", "days"), measureShort(hours, "hr", "hr"));
    } else if (hours >= 2 || (hours > 0 && minutes == 0)) {
        hours += (minutes + 30) / 60;
        return measureShort(hours, "hr", "hr");
    } else if (hours > 0) {
        return joinMeasures(measureShort(hours, "hr", "hr"), measureShort(minutes, "min", "min"));
    } else if (minutes >= 2 || (minutes > 0 && seconds == 0)) {
        minutes += (seconds + 30) / 60;
        return measureShort(minutes, "min", "min");
    } else if (minutes > 0) {
        return joinMeasures(measureShort(minutes, "min", "min"), measureShort(seconds, "sec", "sec"));
    } else {
        return measureShort(seconds, "sec", "sec");
    }
}

std::string Formatter::formatShortElapsedTimeRoundingUpToMinutes(Context& context,
        int64_t millis) {
    const int64_t minutesRoundedUp = (millis + MILLIS_PER_MINUTE - 1) / MILLIS_PER_MINUTE;

    if (minutesRoundedUp == 0 || minutesRoundedUp == 1) {
        return measureShort(minutesRoundedUp, "min", "min");
    }

    return formatShortElapsedTime(context, minutesRoundedUp * MILLIS_PER_MINUTE);
}

} // namespace cdroid
