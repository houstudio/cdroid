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
#include <string.h>
#include <stdint.h>

namespace cdroid{

struct InputEventLabel {
    const char *literal;
    int value;
};

extern const InputEventLabel KEYCODES[];

extern const InputEventLabel AXES[];

extern const InputEventLabel LEDS[];

extern const InputEventLabel FLAGS[];

int lookupValueByLabel(const char* literal, const InputEventLabel *list);

const char* lookupLabelByValue(int value, const InputEventLabel* list);

int32_t getKeyCodeByLabel(const char* label);

const char* getLabelByKeyCode(int32_t keyCode);

uint32_t getKeyFlagByLabel(const char* label);

int32_t getAxisByLabel(const char* label);

const char* getAxisLabel(int32_t axisId);

int32_t getLedByLabel(const char* label);

}/*endof namespace*/
#endif/*__INPUT_EVENT_LABEL_H__*/
