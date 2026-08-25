/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __CDROID_SIMPLEDATEFORMAT_H__
#define __CDROID_SIMPLEDATEFORMAT_H__

#include <string>
#include <memory>
#include <core/Locale.h>
#include <core/calendar.h>
#include <content/dateformat.h>
#include <content/dateformatsymbols.h>
#include <content/parseposition.h>

namespace cdroid{

/* java.text.SimpleDateFormat (android-36 port): formats and parses dates in
   a locale-sensitive manner after a date-time "pattern" such as
   "yyyy.MM.dd G 'at' HH:mm:ss z" (pattern letters GyMdkHmsSEDFwWahKzZYuXLcbB,
   same semantics as AOSP — quoted literals, '' escape, letter count selects
   number width / abbreviated / wide / narrow name forms).

   Locale data comes from the vendored i18n engine through DateFormatSymbols;
   that wiring lives inside the library and never leaks past these headers.

   Time zones: TimeZone is not ported; the calendar carries a raw UTC offset
   in seconds. 'z' therefore always formats as the GMT-offset fallback
   ("GMT+08:00") — the i18n engine has no zone display names — while 'Z'
   (RFC 822) and 'X' (ISO 8601) match AOSP exactly. 'Y' (week year) behaves
   as 'y': the Calendar libc exposes no week-date getters.

   Not ported (no counterpart in CDROID): AttributedCharacterIterator
   attributes / FieldPosition tracking (formatToCharacterIterator) and
   NumberFormat injection (setNumberFormat). */
class SimpleDateFormat : public DateFormat{
public:
    /* AOSP SimpleDateFormat(): default SHORT date+time pattern and symbols
       for the default FORMAT locale. */
    SimpleDateFormat();

    /* AOSP SimpleDateFormat(String). */
    explicit SimpleDateFormat(const std::string& pattern);

    /* AOSP SimpleDateFormat(String, Locale). */
    SimpleDateFormat(const std::string& pattern, const Locale& locale);

    /* AOSP SimpleDateFormat(String, DateFormatSymbols). */
    SimpleDateFormat(const std::string& pattern, const DateFormatSymbols& formatSymbols);

    /* AOSP package-private SimpleDateFormat(timeStyle, dateStyle, locale),
       used by the DateFormat factories (kept public: CDROID has no package
       privacy). A negative style skips that half, like AOSP. */
    SimpleDateFormat(int timeStyle, int dateStyle, const Locale& locale);

    /* AOSP format(Date, StringBuffer, FieldPosition) — FieldPosition is not
       ported; the formatted text is appended to toAppendTo. The input date
       is epoch milliseconds. */
    void format(int64_t dateMillis, std::string& toAppendTo) override;
    /* AOSP format(Object) -> String convenience (input = epoch millis). */
    std::string format(int64_t dateMillis);

    /* AOSP parse(String, ParsePosition): epoch millis, or 0 on failure with
       pos.setErrorIndex() at the failure point (pos index unchanged). */
    int64_t parse(const std::string& text, ParsePosition& pos) override;
    /* AOSP parse(String): throws ParseException on failure. */
    int64_t parse(const std::string& text) override;

    void applyPattern(const std::string& pattern);
    void applyLocalizedPattern(const std::string& pattern);
    std::string toPattern() const;
    std::string toLocalizedPattern() const;

    /* AOSP getDateFormatSymbols returns a defensive clone of the symbol
       tables; this value type returns a copy. */
    DateFormatSymbols getDateFormatSymbols() const;
    void setDateFormatSymbols(const DateFormatSymbols& newFormatSymbols);

    /* AOSP DateFormat overrides. */
    Calendar& getCalendar() const override;
    void setLenient(bool lenient) override;
    bool isLenient() const override;
    void setTimeZone(int offsetSeconds) override;
    int getTimeZone() const override;
    bool operator==(const DateFormat& that) const override;

    void set2DigitYearStart(int64_t startDateMillis);
    int64_t get2DigitYearStart() const;

    SimpleDateFormat(const SimpleDateFormat&) = delete;
    SimpleDateFormat& operator=(const SimpleDateFormat&) = delete;
private:
    void initialize(const Locale& loc);
    void initializeDefaultCentury();
    void parseAmbiguousDatesAsAfter(int64_t startDateMillis);
    std::string compile(const std::string& pattern) const;
    void applyPatternImpl(const std::string& pattern);
    std::string translatePattern(const std::string& pattern,
            const std::string& from, const std::string& to) const;
    void subFormat(int patternCharIndex, int count, std::string& buffer);
    void zeroPaddingNumber(long value, int minDigits, int maxDigits, std::string& buffer) const;
    int64_t parseInternal(const std::string& text, ParsePosition& pos);
    bool shouldObeyCount(int tag, int count) const;
    int subParse(const std::string& text, int start, int patternCharIndex, int count,
                 bool obeyCount, bool& ambiguousYear, ParsePosition& origPos,
                 bool useFollowingMinusSignAsDelimiter, class CalendarBuilder& calb);
    int matchString(const std::string& text, int start, int field,
                    const std::vector<std::string>& data, CalendarBuilder& calb) const;
    int subParseNumericZone(const std::string& text, int start, int sign, int count,
                            bool colon, CalendarBuilder& calb) const;
    int parseMonth(const std::string& text, int count, int value, int start,
                   int field, ParsePosition& pos, bool standalone, CalendarBuilder& out) const;
    int parseWeekday(const std::string& text, int start, int field,
                     bool standalone, CalendarBuilder& out) const;
    std::string formatWeekday(int count, int value, bool standalone) const;
    std::string formatMonth(int count, int value, int maxIntCount,
                            std::string& buffer, bool standalone) const;

    std::string pattern;               // always the non-localized pattern
    std::string compiledPattern;       // byte-stream compiled form, see .cc
    char zeroDigit = '0';              // AOSP: DecimalFormatSymbols zero digit
    char minusSign = '-';
    bool hasFollowingMinusSign = false;
    DateFormatSymbols formatData;      // week/month names etc. (i18n-backed)
    Locale locale;                     // the locale this formatter was built for
    int64_t defaultCenturyStart = 0;   // epoch millis
    int defaultCenturyStartYear = 0;
    bool useDateFormatSymbolsFlag = false; // set by the symbols constructor
};

}//endof namespace
#endif//__CDROID_SIMPLEDATEFORMAT_H__
