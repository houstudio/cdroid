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
#include <content/dateformat.h>
#include <content/simpledateformat.h>
#include <stdexcept>

namespace cdroid{

DateFormat::DateFormat() {
    // AOSP leaves the calendar null until the concrete formatter initializes
    // it; CDROID always formats through a calendar, so create the default
    // one up front (SimpleDateFormat keeps it unless replaced).
    calendar = Calendar::getInstance();
}

std::string DateFormat::format(int64_t dateMillis) {
    std::string toAppendTo;
    format(dateMillis, toAppendTo);
    return toAppendTo;
}

Calendar& DateFormat::getCalendar() const { return *calendar; }

void DateFormat::setCalendar(std::unique_ptr<Calendar> newCalendar) {
    if (newCalendar) calendar = std::move(newCalendar);
}

void DateFormat::setLenient(bool lenient) { calendar->setLenient(lenient); }
bool DateFormat::isLenient() const { return calendar->isLenient(); }
void DateFormat::setTimeZone(int offsetSeconds) { calendar->setTimeZone(offsetSeconds); }
int DateFormat::getTimeZone() const { return calendar->getTimeZone(); }

bool DateFormat::operator==(const DateFormat& that) const {
    return getTimeZone() == that.getTimeZone() && isLenient() == that.isLenient();
}

// AOSP private get(timeStyle, dateStyle, flags, loc): flags bit0 = time
// requested, bit1 = date requested; the other half becomes -1 and the
// requested halves must be 0..3.
static DateFormat* get(int timeStyle, int dateStyle, int flags, const Locale& loc) {
    if ((flags & 1) != 0) {
        if (timeStyle < 0 || timeStyle > 3) {
            throw std::invalid_argument("Illegal time style " + std::to_string(timeStyle));
        }
    } else {
        timeStyle = -1;
    }
    if ((flags & 2) != 0) {
        if (dateStyle < 0 || dateStyle > 3) {
            throw std::invalid_argument("Illegal date style " + std::to_string(dateStyle));
        }
    } else {
        dateStyle = -1;
    }
    return new SimpleDateFormat(timeStyle, dateStyle, loc);
}

DateFormat* DateFormat::getTimeInstance() {
    return get(DEFAULT, 0, 1, Locale::getDefault());
}

DateFormat* DateFormat::getTimeInstance(int style) {
    return get(style, 0, 1, Locale::getDefault());
}

DateFormat* DateFormat::getTimeInstance(int style, const Locale& aLocale) {
    return get(style, 0, 1, aLocale);
}

DateFormat* DateFormat::getDateInstance() {
    return get(0, DEFAULT, 2, Locale::getDefault());
}

DateFormat* DateFormat::getDateInstance(int style) {
    return get(0, style, 2, Locale::getDefault());
}

DateFormat* DateFormat::getDateInstance(int style, const Locale& aLocale) {
    return get(0, style, 2, aLocale);
}

DateFormat* DateFormat::getDateTimeInstance() {
    return get(DEFAULT, DEFAULT, 3, Locale::getDefault());
}

DateFormat* DateFormat::getDateTimeInstance(int dateStyle, int timeStyle) {
    return get(timeStyle, dateStyle, 3, Locale::getDefault());
}

DateFormat* DateFormat::getDateTimeInstance(int dateStyle, int timeStyle, const Locale& aLocale) {
    return get(timeStyle, dateStyle, 3, aLocale);
}

}//endof namespace
