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
#ifndef __CDROID_PARSEPOSITION_H__
#define __CDROID_PARSEPOSITION_H__

#include <string>

namespace cdroid{

/* java.text.ParsePosition: tracks the current position during parsing.
   errorIndex stays -1 until an error is reported. */
class ParsePosition{
private:
    int index = 0;
    int errorIndex = -1;
public:
    ParsePosition() = default;
    ParsePosition(int index) { this->index = index; }

    int getIndex() const { return index; }
    void setIndex(int index) { this->index = index; }

    void setErrorIndex(int ei) { errorIndex = ei; }
    int getErrorIndex() const { return errorIndex; }

    std::string toString() const {
        return "java.text.ParsePosition[index=" + std::to_string(index)
             + ",errorIndex=" + std::to_string(errorIndex) + "]";
    }
};

}//endof namespace
#endif//__CDROID_PARSEPOSITION_H__
