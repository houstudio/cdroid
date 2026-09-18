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
#ifndef __DECIMAL_FORMAT_SYMBOLS_H__
#define __DECIMAL_FORMAT_SYMBOLS_H__
#include <string>
#include <memory>
#include <content/Locale.h>

namespace cdroid{

// Port of java.text.DecimalFormatSymbols: the locale's number-formatting
// symbols. AOSP fields covering currency/exponential/NaN/grouping-size and
// the pattern getters/setters are not carried (nothing in-tree consumes them;
// the engine's number_format drives the actual formatting) — this is the
// read-only symbol face over the same CLDR data, as UTF-8 strings because the
// separators can be multi-byte (ar decimal "٫", fr grouping U+202F).
class DecimalFormatSymbols{
public:
    DecimalFormatSymbols();
    explicit DecimalFormatSymbols(const Locale& locale);

    // AOSP getInstance()s: default-locale / explicit-locale factories.
    static DecimalFormatSymbols getInstance();
    static DecimalFormatSymbols getInstance(const Locale& locale);

    // --- AOSP getters (string forms where CLDR symbols are multi-byte) ------
    std::string getZeroDigitString() const;     // first char of the digit set ("0" / "٠")
    std::string getDigitString() const;         // AOSP getDigit(): the locale digit char
    std::string getDecimalSeparatorString() const;
    std::string getGroupingSeparatorString() const;
    std::string getPercentString() const;       // AOSP getPercentString() (Android-added)
    std::string getMinusSignString() const;

    // Char getters (AOSP originals): the FIRST UTF-8 code point of the symbol,
    // or '\0' when the symbol is multi-byte (use the String form then).
    char getZeroDigit() const;
    char getDecimalSeparator() const;
    char getGroupingSeparator() const;
    char getPercent() const;
    char getMinusSign() const;

private:
    std::string mZeroDigit;
    std::string mDecimalSeparator;
    std::string mGroupingSeparator;
    std::string mPercent;
    std::string mMinusSign;
};

} // namespace cdroid
#endif/*__DECIMAL_FORMAT_SYMBOLS_H__*/
