/*********************************************************************************
 * Port of android.text.format.Formatter (android_12 line, Apache 2.0).
 *
 * The android-36 rewrite routes everything through ICU MeasureFormat/
 * MeasureUnit; the bundled ICU subset (minikin/include/myicu) has no
 * formatting family, so this port keeps the android_12 semantics the AOSP
 * coretests (FormatterTest) were written against: suffix resource strings +
 * String.format-style rounding. The measure-unit half of android-36
 * (formatMeasureShort/RoundedBytesResult) is intentionally not ported.
 *
 * Locale note: AOSP resolves the locale from the Resources configuration;
 * CDROID's DateUtils subset reads Locale::getDefault() (see
 * format/dateutils.cc), and this port follows that convention so tests can
 * drive es-ES via Locale::setDefault().
 *********************************************************************************/
#ifndef __CDROID_TEXT_FORMAT_FORMATTER_H__
#define __CDROID_TEXT_FORMAT_FORMATTER_H__

#include <string>
#include <cstdint>

namespace cdroid {

class Context;
class Resources;

/** android.text.format.Formatter — utility class to aid in formatting common
    values that are not covered by the java.util.Formatter class. */
class Formatter {
public:
    /** @hide */
    static constexpr int FLAG_SHORTER = 1 << 0;
    /** @hide */
    static constexpr int FLAG_CALCULATE_ROUNDED = 1 << 1;
    /** @hide */
    static constexpr int FLAG_SI_UNITS = 1 << 2;
    /** @hide */
    static constexpr int FLAG_IEC_UNITS = 1 << 3;

    /** @hide */
    struct BytesResult {
        const std::string value;
        const std::string units;
        const int64_t roundedBytes;

        BytesResult(const std::string& value, const std::string& units, int64_t roundedBytes)
            : value(value), units(units), roundedBytes(roundedBytes) {}
    };

    /** Formats a content size to be in the form of bytes, kilobytes, megabytes,
        etc. As of O the prefixes are SI: kB = 1000 bytes, MB = 1,000,000... */
    static std::string formatFileSize(Context* context, int64_t sizeBytes);
    /** @hide */
    static std::string formatFileSize(Context* context, int64_t sizeBytes, int flags);
    /** Like formatFileSize, but trying to generate shorter numbers (showing
        fewer digits of precision). */
    static std::string formatShortFileSize(Context* context, int64_t sizeBytes);

    /** @hide */
    static BytesResult formatBytes(Resources* res, int64_t sizeBytes, int flags);

    /** Returns a string in the canonical IPv4 format ###.###.###.### from a
        packed integer containing the IP address, little-endian (LSB first):
        0x01020304 returns "4.3.2.1". @deprecated */
    static std::string formatIpAddress(int ipv4Address);

    /** Returns elapsed time for the given millis, in the following format:
        1 day, 5 hr; will include at most two units, can go down to seconds
        precision. @hide */
    static std::string formatShortElapsedTime(Context* context, int64_t millis);
    /** Returns elapsed time for the given millis, in the following format:
        1 day, 5 hr; will include at most two units, can go down to minutes
        precision. @hide */
    static std::string formatShortElapsedTimeRoundingUpToMinutes(Context* context,
            int64_t millis);

private:
    Formatter() = delete;   // static utility
};

} // namespace cdroid

#endif /* __CDROID_TEXT_FORMAT_FORMATTER_H__ */
