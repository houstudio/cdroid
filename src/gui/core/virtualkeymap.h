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
#ifndef __VIRTUAL_KEYMAP_H__
#define __VIRTUAL_KEYMAP_H__
#include <string>
#include <vector>
#include <memory>
namespace cdroid {

/* Describes a virtual key. */
struct VirtualKeyDefinition {
    int32_t scanCode;

    // configured position data, specified in display coords
    int32_t centerX;
    int32_t centerY;
    int32_t width;
    int32_t height;
};


/**
 * Describes a collection of virtual keys on a touch screen in terms of
 * virtual scan codes and hit rectangles.
 *
 * This object is immutable after it has been loaded.
 */
class VirtualKeyMap {
public:
    ~VirtualKeyMap();

    static std::unique_ptr<VirtualKeyMap> fromStream(std::istream& filename);

    inline const std::vector<VirtualKeyDefinition>& getVirtualKeys() const {
        return mVirtualKeys;
    }

private:
    class Parser {
        VirtualKeyMap* mMap;
        Tokenizer* mTokenizer;

    public:
        Parser(VirtualKeyMap* map, Tokenizer* tokenizer);
        ~Parser();
        int32_t parse();

    private:
        bool consumeFieldDelimiterAndSkipWhitespace();
        bool parseNextIntField(int32_t* outValue);
    };

    std::vector<VirtualKeyDefinition> mVirtualKeys;

    VirtualKeyMap();
};

} // namespace cdroid
#endif
