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
#include <content/decimalformatsymbols.h>
#include <gui_features.h>
#ifdef ENABLE_I18N
#include <content/i18nbridge.h>
#include <content/i18n/data_resource.h>
#include <content/i18n/str_util.h>
#endif

namespace cdroid{

// The pool split indices mirror NumberFormatImpl::Init (number-format.json:
// pattern / percent-pattern / dec / group / percent, '_' separated).
enum { DFS_SPLIT_SIZE = 5, DFS_DEC_SIGN = 2, DFS_GROUP_SIGN = 3, DFS_PERCENT_SIGN = 4 };

DecimalFormatSymbols::DecimalFormatSymbols()
    : DecimalFormatSymbols(Locale::getDefault()) {}

DecimalFormatSymbols::DecimalFormatSymbols(const Locale& locale) {
    // AOSP defaults (en): the engine data overrides per locale below.
    mZeroDigit = "0";
    mDecimalSeparator = ".";
    mGroupingSeparator = ",";
    mPercent = "%";
    mMinusSign = "-";
#ifdef ENABLE_I18N
    i18n::LocaleInfo localeInfo = I18nBridge::toLocaleInfo(locale);
    i18n::DataResource resource(&localeInfo);
    if (!resource.Init()) return;

    std::string unprocessedNumberFormat;
    resource.GetString(i18n::DataResourceType::NUMBER_FORMAT, unprocessedNumberFormat);
    if (!unprocessedNumberFormat.empty()) {
        std::string split[DFS_SPLIT_SIZE];
        i18n::Split(unprocessedNumberFormat, split, DFS_SPLIT_SIZE, '_');
        if (!split[DFS_DEC_SIGN].empty()) mDecimalSeparator = split[DFS_DEC_SIGN];
        if (!split[DFS_GROUP_SIGN].empty()) mGroupingSeparator = split[DFS_GROUP_SIGN];
        if (!split[DFS_PERCENT_SIGN].empty()) mPercent = split[DFS_PERCENT_SIGN];
    }

    // number-digit.json: the locale's digit set ("0;1;...;9" for ASCII
    // locales — absent from the data — native digits elsewhere, e.g. ar).
    std::string unprocessedNumberDigit;
    resource.GetString(i18n::DataResourceType::NUMBER_DIGIT, unprocessedNumberDigit);
    if (!unprocessedNumberDigit.empty()) {
        std::string splitDigit[10];
        i18n::Split(unprocessedNumberDigit, splitDigit, 10, ';');
        if (!splitDigit[0].empty()) mZeroDigit = splitDigit[0];
    }

    std::string minus;
    resource.GetString(i18n::DataResourceType::MINUS_SIGN, minus);
    if (!minus.empty()) mMinusSign = minus;
#endif
}

DecimalFormatSymbols DecimalFormatSymbols::getInstance() {
    return DecimalFormatSymbols();
}

DecimalFormatSymbols DecimalFormatSymbols::getInstance(const Locale& locale) {
    return DecimalFormatSymbols(locale);
}

std::string DecimalFormatSymbols::getZeroDigitString() const { return mZeroDigit; }
std::string DecimalFormatSymbols::getDigitString() const { return mZeroDigit; }
std::string DecimalFormatSymbols::getDecimalSeparatorString() const { return mDecimalSeparator; }
std::string DecimalFormatSymbols::getGroupingSeparatorString() const { return mGroupingSeparator; }
std::string DecimalFormatSymbols::getPercentString() const { return mPercent; }
std::string DecimalFormatSymbols::getMinusSignString() const { return mMinusSign; }

char DecimalFormatSymbols::getZeroDigit() const {
    return mZeroDigit.empty() ? '0' : mZeroDigit[0];
}
char DecimalFormatSymbols::getDecimalSeparator() const {
    return (mDecimalSeparator.size() == 1) ? mDecimalSeparator[0] : '\0';
}
char DecimalFormatSymbols::getGroupingSeparator() const {
    return (mGroupingSeparator.size() == 1) ? mGroupingSeparator[0] : '\0';
}
char DecimalFormatSymbols::getPercent() const {
    return (mPercent.size() == 1) ? mPercent[0] : '\0';
}
char DecimalFormatSymbols::getMinusSign() const {
    return (mMinusSign.size() == 1) ? mMinusSign[0] : '\0';
}

} // namespace cdroid
