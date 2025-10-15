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
#ifndef __HWUI_PATH_PARSER_H__
#define __HWUI_PATH_PARSER_H__
#include <drawable/hwvectordrawable.h>
#include <drawable/hwvectordrawableutils.h>
#include <core/path.h>
namespace cdroid{
namespace hwui{
class PathParser {
public:
    struct ParseResult {
        bool failureOccurred = false;
        std::string failureMessage;
    };
    /**
     * Parse the string literal and create a Skia Path. Return true on success.
     */
    static void parseAsciiStringForPath(Cairo::RefPtr<cdroid::Path>& outPath, ParseResult* result,const char* pathStr, size_t strLength);
    static void getPathDataFromAsciiString(PathData* outData, ParseResult* result,const char* pathStr, size_t strLength);
    static void dump(const PathData& data);
    static void validateVerbAndPoints(char verb, size_t points, ParseResult* result);
};

}
}
#endif/*__HWUI_PATH_PARSER_H__*/
