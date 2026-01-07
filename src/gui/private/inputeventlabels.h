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
#ifndef __INPUT_EVENT_LABEL_H__
#define __INPUT_EVENT_LABEL_H__
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace cdroid{

struct InputEventLabel {
    const char *literal;
    int value;
};

struct EvdevEventLabel {
    std::string type;
    std::string code;
    std::string value;
};

//   NOTE: If you want a new key code, axis code, led code or flag code in keylayout file,
//   then you must add it to InputEventLabels.cpp.

class InputEventLookup {
    /**
     * This class is not purely static, but uses a singleton pattern in order to delay the
     * initialization of the maps that it contains. If it were purely static, the maps could be
     * created early, and would cause sanitizers to report memory leaks.
     */
public:
    InputEventLookup(InputEventLookup& other) = delete;

    void operator=(const InputEventLookup&) = delete;

    static int lookupValueByLabel(const std::unordered_map<std::string, int>& map,
                                                 const char* literal);

    static const char* lookupLabelByValue(const std::vector<InputEventLabel>& vec, int value);

    static int getKeyCodeByLabel(const char* label);

    static const char* getLabelByKeyCode(int32_t keyCode);

    static int getKeyFlagByLabel(const char* label);

    static int getAxisByLabel(const char* label);

    static const char* getAxisLabel(int32_t axisId);

    static int getLedByLabel(const char* label);

    static EvdevEventLabel getLinuxEvdevLabel(int32_t type, int32_t code, int32_t value);

    static int getLinuxEvdevEventTypeByLabel(const char* label);

    static int getLinuxEvdevEventCodeByLabel(int32_t type, const char* label);

    static int getLinuxEvdevInputPropByLabel(const char* label);

private:
    InputEventLookup();

    static const InputEventLookup& get() {
        static InputEventLookup sLookup;
        return sLookup;
    }

    const std::unordered_map<std::string, int> KEYCODES;

    const std::vector<InputEventLabel> KEY_NAMES;

    const std::unordered_map<std::string, int> AXES;

    const std::vector<InputEventLabel> AXES_NAMES;

    const std::unordered_map<std::string, int> LEDS;

    const std::unordered_map<std::string, int> FLAGS;
};
}/*endof namespace*/
#endif/*__INPUT_EVENT_LABEL_H__*/
