#ifndef __INPUT_EVENT_LABE_H__
#define __INPUT_EVENT_LABE_H__
#include <view/keyevent.h>
#include <view/motionevent.h>
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
#endif
