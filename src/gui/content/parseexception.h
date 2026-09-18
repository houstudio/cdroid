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
#ifndef __CDROID_PARSEEXCEPTION_H__
#define __CDROID_PARSEEXCEPTION_H__

#include <stdexcept>
#include <string>

namespace cdroid{

/* java.text.ParseException: thrown by SimpleDateFormat::parse(text) when
   the text cannot be parsed; carries the offset where parsing failed. */
class ParseException : public std::runtime_error{
private:
    int mErrorOffset;
public:
    ParseException(const std::string& message, int errorOffset)
        : std::runtime_error(message), mErrorOffset(errorOffset) {}

    int getErrorOffset() const { return mErrorOffset; }
};

}//endof namespace
#endif//__CDROID_PARSEEXCEPTION_H__
