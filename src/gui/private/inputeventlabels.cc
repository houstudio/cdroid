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
#include <private/inputeventlabels.h>
#include <view/keycodes.h>
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <core/eventcodes.h>
#include <string.h>
//#include <linux/input.h>

#define DEFINE_KEYCODE(key) { #key, KeyEvent::KEYCODE_##key }
#define DEFINE_AXIS(axis) { #axis, MotionEvent::AXIS_##axis }
#define DEFINE_LED(led)   { #led, LED_##led }
#define DEFINE_FLAG(flag) { #flag, POLICY_FLAG_##flag }

namespace cdroid{

// NOTE: If you add a new keycode here you must also add it to several other files.
//       Refer to frameworks/base/core/java/android/view/KeyEvent.java for the full list.
#define KEYCODES_SEQUENCE \
    DEFINE_KEYCODE(UNKNOWN), \
    DEFINE_KEYCODE(SOFT_LEFT), \
    DEFINE_KEYCODE(SOFT_RIGHT), \
    DEFINE_KEYCODE(HOME), \
    DEFINE_KEYCODE(BACK), \
    DEFINE_KEYCODE(CALL), \
    DEFINE_KEYCODE(ENDCALL), \
    DEFINE_KEYCODE(0), \
    DEFINE_KEYCODE(1), \
    DEFINE_KEYCODE(2), \
    DEFINE_KEYCODE(3), \
    DEFINE_KEYCODE(4), \
    DEFINE_KEYCODE(5), \
    DEFINE_KEYCODE(6), \
    DEFINE_KEYCODE(7), \
    DEFINE_KEYCODE(8), \
    DEFINE_KEYCODE(9), \
    DEFINE_KEYCODE(STAR), \
    DEFINE_KEYCODE(POUND), \
    DEFINE_KEYCODE(DPAD_UP), \
    DEFINE_KEYCODE(DPAD_DOWN), \
    DEFINE_KEYCODE(DPAD_LEFT), \
    DEFINE_KEYCODE(DPAD_RIGHT), \
    DEFINE_KEYCODE(DPAD_CENTER), \
    DEFINE_KEYCODE(VOLUME_UP), \
    DEFINE_KEYCODE(VOLUME_DOWN), \
    DEFINE_KEYCODE(POWER), \
    DEFINE_KEYCODE(CAMERA), \
    DEFINE_KEYCODE(CLEAR), \
    DEFINE_KEYCODE(A), \
    DEFINE_KEYCODE(B), \
    DEFINE_KEYCODE(C), \
    DEFINE_KEYCODE(D), \
    DEFINE_KEYCODE(E), \
    DEFINE_KEYCODE(F), \
    DEFINE_KEYCODE(G), \
    DEFINE_KEYCODE(H), \
    DEFINE_KEYCODE(I), \
    DEFINE_KEYCODE(J), \
    DEFINE_KEYCODE(K), \
    DEFINE_KEYCODE(L), \
    DEFINE_KEYCODE(M), \
    DEFINE_KEYCODE(N), \
    DEFINE_KEYCODE(O), \
    DEFINE_KEYCODE(P), \
    DEFINE_KEYCODE(Q), \
    DEFINE_KEYCODE(R), \
    DEFINE_KEYCODE(S), \
    DEFINE_KEYCODE(T), \
    DEFINE_KEYCODE(U), \
    DEFINE_KEYCODE(V), \
    DEFINE_KEYCODE(W), \
    DEFINE_KEYCODE(X), \
    DEFINE_KEYCODE(Y), \
    DEFINE_KEYCODE(Z), \
    DEFINE_KEYCODE(COMMA), \
    DEFINE_KEYCODE(PERIOD), \
    DEFINE_KEYCODE(ALT_LEFT), \
    DEFINE_KEYCODE(ALT_RIGHT), \
    DEFINE_KEYCODE(SHIFT_LEFT), \
    DEFINE_KEYCODE(SHIFT_RIGHT), \
    DEFINE_KEYCODE(TAB), \
    DEFINE_KEYCODE(SPACE), \
    DEFINE_KEYCODE(SYM), \
    DEFINE_KEYCODE(EXPLORER), \
    DEFINE_KEYCODE(ENVELOPE), \
    DEFINE_KEYCODE(ENTER), \
    DEFINE_KEYCODE(DEL), \
    DEFINE_KEYCODE(GRAVE), \
    DEFINE_KEYCODE(MINUS), \
    DEFINE_KEYCODE(EQUALS), \
    DEFINE_KEYCODE(LEFT_BRACKET), \
    DEFINE_KEYCODE(RIGHT_BRACKET), \
    DEFINE_KEYCODE(BACKSLASH), \
    DEFINE_KEYCODE(SEMICOLON), \
    DEFINE_KEYCODE(APOSTROPHE), \
    DEFINE_KEYCODE(SLASH), \
    DEFINE_KEYCODE(AT), \
    DEFINE_KEYCODE(NUM), \
    DEFINE_KEYCODE(HEADSETHOOK), \
    DEFINE_KEYCODE(FOCUS), \
    DEFINE_KEYCODE(PLUS), \
    DEFINE_KEYCODE(MENU), \
    DEFINE_KEYCODE(NOTIFICATION), \
    DEFINE_KEYCODE(SEARCH), \
    DEFINE_KEYCODE(MEDIA_PLAY_PAUSE), \
    DEFINE_KEYCODE(MEDIA_STOP), \
    DEFINE_KEYCODE(MEDIA_NEXT), \
    DEFINE_KEYCODE(MEDIA_PREVIOUS), \
    DEFINE_KEYCODE(MEDIA_REWIND), \
    DEFINE_KEYCODE(MEDIA_FAST_FORWARD), \
    DEFINE_KEYCODE(MUTE), \
    DEFINE_KEYCODE(PAGE_UP), \
    DEFINE_KEYCODE(PAGE_DOWN), \
    DEFINE_KEYCODE(PICTSYMBOLS), \
    DEFINE_KEYCODE(SWITCH_CHARSET), \
    DEFINE_KEYCODE(BUTTON_A), \
    DEFINE_KEYCODE(BUTTON_B), \
    DEFINE_KEYCODE(BUTTON_C), \
    DEFINE_KEYCODE(BUTTON_X), \
    DEFINE_KEYCODE(BUTTON_Y), \
    DEFINE_KEYCODE(BUTTON_Z), \
    DEFINE_KEYCODE(BUTTON_L1), \
    DEFINE_KEYCODE(BUTTON_R1), \
    DEFINE_KEYCODE(BUTTON_L2), \
    DEFINE_KEYCODE(BUTTON_R2), \
    DEFINE_KEYCODE(BUTTON_THUMBL), \
    DEFINE_KEYCODE(BUTTON_THUMBR), \
    DEFINE_KEYCODE(BUTTON_START), \
    DEFINE_KEYCODE(BUTTON_SELECT), \
    DEFINE_KEYCODE(BUTTON_MODE), \
    DEFINE_KEYCODE(ESCAPE), \
    DEFINE_KEYCODE(FORWARD_DEL), \
    DEFINE_KEYCODE(CTRL_LEFT), \
    DEFINE_KEYCODE(CTRL_RIGHT), \
    DEFINE_KEYCODE(CAPS_LOCK), \
    DEFINE_KEYCODE(SCROLL_LOCK), \
    DEFINE_KEYCODE(META_LEFT), \
    DEFINE_KEYCODE(META_RIGHT), \
    DEFINE_KEYCODE(FUNCTION), \
    DEFINE_KEYCODE(SYSRQ), \
    DEFINE_KEYCODE(BREAK), \
    DEFINE_KEYCODE(MOVE_HOME), \
    DEFINE_KEYCODE(MOVE_END), \
    DEFINE_KEYCODE(INSERT), \
    DEFINE_KEYCODE(FORWARD), \
    DEFINE_KEYCODE(MEDIA_PLAY), \
    DEFINE_KEYCODE(MEDIA_PAUSE), \
    DEFINE_KEYCODE(MEDIA_CLOSE), \
    DEFINE_KEYCODE(MEDIA_EJECT), \
    DEFINE_KEYCODE(MEDIA_RECORD), \
    DEFINE_KEYCODE(F1), \
    DEFINE_KEYCODE(F2), \
    DEFINE_KEYCODE(F3), \
    DEFINE_KEYCODE(F4), \
    DEFINE_KEYCODE(F5), \
    DEFINE_KEYCODE(F6), \
    DEFINE_KEYCODE(F7), \
    DEFINE_KEYCODE(F8), \
    DEFINE_KEYCODE(F9), \
    DEFINE_KEYCODE(F10), \
    DEFINE_KEYCODE(F11), \
    DEFINE_KEYCODE(F12), \
    DEFINE_KEYCODE(NUM_LOCK), \
    DEFINE_KEYCODE(NUMPAD_0), \
    DEFINE_KEYCODE(NUMPAD_1), \
    DEFINE_KEYCODE(NUMPAD_2), \
    DEFINE_KEYCODE(NUMPAD_3), \
    DEFINE_KEYCODE(NUMPAD_4), \
    DEFINE_KEYCODE(NUMPAD_5), \
    DEFINE_KEYCODE(NUMPAD_6), \
    DEFINE_KEYCODE(NUMPAD_7), \
    DEFINE_KEYCODE(NUMPAD_8), \
    DEFINE_KEYCODE(NUMPAD_9), \
    DEFINE_KEYCODE(NUMPAD_DIVIDE), \
    DEFINE_KEYCODE(NUMPAD_MULTIPLY), \
    DEFINE_KEYCODE(NUMPAD_SUBTRACT), \
    DEFINE_KEYCODE(NUMPAD_ADD), \
    DEFINE_KEYCODE(NUMPAD_DOT), \
    DEFINE_KEYCODE(NUMPAD_COMMA), \
    DEFINE_KEYCODE(NUMPAD_ENTER), \
    DEFINE_KEYCODE(NUMPAD_EQUALS), \
    DEFINE_KEYCODE(NUMPAD_LEFT_PAREN), \
    DEFINE_KEYCODE(NUMPAD_RIGHT_PAREN), \
    DEFINE_KEYCODE(VOLUME_MUTE), \
    DEFINE_KEYCODE(INFO), \
    DEFINE_KEYCODE(CHANNEL_UP), \
    DEFINE_KEYCODE(CHANNEL_DOWN), \
    DEFINE_KEYCODE(ZOOM_IN), \
    DEFINE_KEYCODE(ZOOM_OUT), \
    DEFINE_KEYCODE(TV), \
    DEFINE_KEYCODE(WINDOW), \
    DEFINE_KEYCODE(GUIDE), \
    DEFINE_KEYCODE(DVR), \
    DEFINE_KEYCODE(BOOKMARK), \
    DEFINE_KEYCODE(CAPTIONS), \
    DEFINE_KEYCODE(SETTINGS), \
    DEFINE_KEYCODE(TV_POWER), \
    DEFINE_KEYCODE(TV_INPUT), \
    DEFINE_KEYCODE(STB_POWER), \
    DEFINE_KEYCODE(STB_INPUT), \
    DEFINE_KEYCODE(AVR_POWER), \
    DEFINE_KEYCODE(AVR_INPUT), \
    DEFINE_KEYCODE(PROG_RED), \
    DEFINE_KEYCODE(PROG_GREEN), \
    DEFINE_KEYCODE(PROG_YELLOW), \
    DEFINE_KEYCODE(PROG_BLUE), \
    DEFINE_KEYCODE(APP_SWITCH), \
    DEFINE_KEYCODE(BUTTON_1), \
    DEFINE_KEYCODE(BUTTON_2), \
    DEFINE_KEYCODE(BUTTON_3), \
    DEFINE_KEYCODE(BUTTON_4), \
    DEFINE_KEYCODE(BUTTON_5), \
    DEFINE_KEYCODE(BUTTON_6), \
    DEFINE_KEYCODE(BUTTON_7), \
    DEFINE_KEYCODE(BUTTON_8), \
    DEFINE_KEYCODE(BUTTON_9), \
    DEFINE_KEYCODE(BUTTON_10), \
    DEFINE_KEYCODE(BUTTON_11), \
    DEFINE_KEYCODE(BUTTON_12), \
    DEFINE_KEYCODE(BUTTON_13), \
    DEFINE_KEYCODE(BUTTON_14), \
    DEFINE_KEYCODE(BUTTON_15), \
    DEFINE_KEYCODE(BUTTON_16), \
    DEFINE_KEYCODE(LANGUAGE_SWITCH), \
    DEFINE_KEYCODE(MANNER_MODE), \
    DEFINE_KEYCODE(3D_MODE), \
    DEFINE_KEYCODE(CONTACTS), \
    DEFINE_KEYCODE(CALENDAR), \
    DEFINE_KEYCODE(MUSIC), \
    DEFINE_KEYCODE(CALCULATOR), \
    DEFINE_KEYCODE(ZENKAKU_HANKAKU), \
    DEFINE_KEYCODE(EISU), \
    DEFINE_KEYCODE(MUHENKAN), \
    DEFINE_KEYCODE(HENKAN), \
    DEFINE_KEYCODE(KATAKANA_HIRAGANA), \
    DEFINE_KEYCODE(YEN), \
    DEFINE_KEYCODE(RO), \
    DEFINE_KEYCODE(KANA), \
    DEFINE_KEYCODE(ASSIST), \
    DEFINE_KEYCODE(BRIGHTNESS_DOWN), \
    DEFINE_KEYCODE(BRIGHTNESS_UP), \
    DEFINE_KEYCODE(MEDIA_AUDIO_TRACK), \
    DEFINE_KEYCODE(SLEEP), \
    DEFINE_KEYCODE(WAKEUP), \
    DEFINE_KEYCODE(PAIRING), \
    DEFINE_KEYCODE(MEDIA_TOP_MENU), \
    DEFINE_KEYCODE(11), \
    DEFINE_KEYCODE(12), \
    DEFINE_KEYCODE(LAST_CHANNEL), \
    DEFINE_KEYCODE(TV_DATA_SERVICE), \
    DEFINE_KEYCODE(VOICE_ASSIST), \
    DEFINE_KEYCODE(TV_RADIO_SERVICE), \
    DEFINE_KEYCODE(TV_TELETEXT), \
    DEFINE_KEYCODE(TV_NUMBER_ENTRY), \
    DEFINE_KEYCODE(TV_TERRESTRIAL_ANALOG), \
    DEFINE_KEYCODE(TV_TERRESTRIAL_DIGITAL), \
    DEFINE_KEYCODE(TV_SATELLITE), \
    DEFINE_KEYCODE(TV_SATELLITE_BS), \
    DEFINE_KEYCODE(TV_SATELLITE_CS), \
    DEFINE_KEYCODE(TV_SATELLITE_SERVICE), \
    DEFINE_KEYCODE(TV_NETWORK), \
    DEFINE_KEYCODE(TV_ANTENNA_CABLE), \
    DEFINE_KEYCODE(TV_INPUT_HDMI_1), \
    DEFINE_KEYCODE(TV_INPUT_HDMI_2), \
    DEFINE_KEYCODE(TV_INPUT_HDMI_3), \
    DEFINE_KEYCODE(TV_INPUT_HDMI_4), \
    DEFINE_KEYCODE(TV_INPUT_COMPOSITE_1), \
    DEFINE_KEYCODE(TV_INPUT_COMPOSITE_2), \
    DEFINE_KEYCODE(TV_INPUT_COMPONENT_1), \
    DEFINE_KEYCODE(TV_INPUT_COMPONENT_2), \
    DEFINE_KEYCODE(TV_INPUT_VGA_1), \
    DEFINE_KEYCODE(TV_AUDIO_DESCRIPTION), \
    DEFINE_KEYCODE(TV_AUDIO_DESCRIPTION_MIX_UP), \
    DEFINE_KEYCODE(TV_AUDIO_DESCRIPTION_MIX_DOWN), \
    DEFINE_KEYCODE(TV_ZOOM_MODE), \
    DEFINE_KEYCODE(TV_CONTENTS_MENU), \
    DEFINE_KEYCODE(TV_MEDIA_CONTEXT_MENU), \
    DEFINE_KEYCODE(TV_TIMER_PROGRAMMING), \
    DEFINE_KEYCODE(HELP), \
    DEFINE_KEYCODE(NAVIGATE_PREVIOUS), \
    DEFINE_KEYCODE(NAVIGATE_NEXT), \
    DEFINE_KEYCODE(NAVIGATE_IN), \
    DEFINE_KEYCODE(NAVIGATE_OUT), \
    DEFINE_KEYCODE(STEM_PRIMARY), \
    DEFINE_KEYCODE(STEM_1), \
    DEFINE_KEYCODE(STEM_2), \
    DEFINE_KEYCODE(STEM_3), \
    DEFINE_KEYCODE(DPAD_UP_LEFT), \
    DEFINE_KEYCODE(DPAD_DOWN_LEFT), \
    DEFINE_KEYCODE(DPAD_UP_RIGHT), \
    DEFINE_KEYCODE(DPAD_DOWN_RIGHT), \
    DEFINE_KEYCODE(MEDIA_SKIP_FORWARD), \
    DEFINE_KEYCODE(MEDIA_SKIP_BACKWARD), \
    DEFINE_KEYCODE(MEDIA_STEP_FORWARD), \
    DEFINE_KEYCODE(MEDIA_STEP_BACKWARD), \
    DEFINE_KEYCODE(SOFT_SLEEP), \
    DEFINE_KEYCODE(CUT), \
    DEFINE_KEYCODE(COPY), \
    DEFINE_KEYCODE(PASTE), \
    DEFINE_KEYCODE(SYSTEM_NAVIGATION_UP), \
    DEFINE_KEYCODE(SYSTEM_NAVIGATION_DOWN), \
    DEFINE_KEYCODE(SYSTEM_NAVIGATION_LEFT), \
    DEFINE_KEYCODE(SYSTEM_NAVIGATION_RIGHT), \
    DEFINE_KEYCODE(ALL_APPS), \
    DEFINE_KEYCODE(REFRESH), \
    DEFINE_KEYCODE(THUMBS_UP), \
    DEFINE_KEYCODE(THUMBS_DOWN), \
    DEFINE_KEYCODE(PROFILE_SWITCH), \
    DEFINE_KEYCODE(VIDEO_APP_1), \
    DEFINE_KEYCODE(VIDEO_APP_2), \
    DEFINE_KEYCODE(VIDEO_APP_3), \
    DEFINE_KEYCODE(VIDEO_APP_4), \
    DEFINE_KEYCODE(VIDEO_APP_5), \
    DEFINE_KEYCODE(VIDEO_APP_6), \
    DEFINE_KEYCODE(VIDEO_APP_7), \
    DEFINE_KEYCODE(VIDEO_APP_8), \
    DEFINE_KEYCODE(FEATURED_APP_1), \
    DEFINE_KEYCODE(FEATURED_APP_2), \
    DEFINE_KEYCODE(FEATURED_APP_3), \
    DEFINE_KEYCODE(FEATURED_APP_4), \
    DEFINE_KEYCODE(DEMO_APP_1), \
    DEFINE_KEYCODE(DEMO_APP_2), \
    DEFINE_KEYCODE(DEMO_APP_3), \
    DEFINE_KEYCODE(DEMO_APP_4), \
    DEFINE_KEYCODE(KEYBOARD_BACKLIGHT_DOWN), \
    DEFINE_KEYCODE(KEYBOARD_BACKLIGHT_UP), \
    DEFINE_KEYCODE(KEYBOARD_BACKLIGHT_TOGGLE), \
    DEFINE_KEYCODE(STYLUS_BUTTON_PRIMARY), \
    DEFINE_KEYCODE(STYLUS_BUTTON_SECONDARY), \
    DEFINE_KEYCODE(STYLUS_BUTTON_TERTIARY), \
    DEFINE_KEYCODE(STYLUS_BUTTON_TAIL), \
    DEFINE_KEYCODE(RECENT_APPS), \
    DEFINE_KEYCODE(MACRO_1), \
    DEFINE_KEYCODE(MACRO_2), \
    DEFINE_KEYCODE(MACRO_3), \
    DEFINE_KEYCODE(MACRO_4), \
    DEFINE_KEYCODE(EMOJI_PICKER), \
    DEFINE_KEYCODE(SCREENSHOT), \
    DEFINE_KEYCODE(DICTATE), \
    DEFINE_KEYCODE(NEW), \
    DEFINE_KEYCODE(CLOSE), \
    DEFINE_KEYCODE(DO_NOT_DISTURB), \
    DEFINE_KEYCODE(PRINT), \
    DEFINE_KEYCODE(LOCK), \
    DEFINE_KEYCODE(FULLSCREEN), \
    DEFINE_KEYCODE(F13), \
    DEFINE_KEYCODE(F14), \
    DEFINE_KEYCODE(F15), \
    DEFINE_KEYCODE(F16), \
    DEFINE_KEYCODE(F17), \
    DEFINE_KEYCODE(F18), \
    DEFINE_KEYCODE(F19),\
    DEFINE_KEYCODE(F20), \
    DEFINE_KEYCODE(F21), \
    DEFINE_KEYCODE(F22), \
    DEFINE_KEYCODE(F23), \
    DEFINE_KEYCODE(F24)

// NOTE: If you add a new axis here you must also add it to several other files.
//       Refer to frameworks/base/core/java/android/view/MotionEvent.java for the full list.
#define AXES_SEQUENCE \
    DEFINE_AXIS(X), \
    DEFINE_AXIS(Y), \
    DEFINE_AXIS(PRESSURE), \
    DEFINE_AXIS(SIZE), \
    DEFINE_AXIS(TOUCH_MAJOR), \
    DEFINE_AXIS(TOUCH_MINOR), \
    DEFINE_AXIS(TOOL_MAJOR), \
    DEFINE_AXIS(TOOL_MINOR), \
    DEFINE_AXIS(ORIENTATION), \
    DEFINE_AXIS(VSCROLL), \
    DEFINE_AXIS(HSCROLL), \
    DEFINE_AXIS(Z), \
    DEFINE_AXIS(RX), \
    DEFINE_AXIS(RY), \
    DEFINE_AXIS(RZ), \
    DEFINE_AXIS(HAT_X), \
    DEFINE_AXIS(HAT_Y), \
    DEFINE_AXIS(LTRIGGER), \
    DEFINE_AXIS(RTRIGGER), \
    DEFINE_AXIS(THROTTLE), \
    DEFINE_AXIS(RUDDER), \
    DEFINE_AXIS(WHEEL), \
    DEFINE_AXIS(GAS), \
    DEFINE_AXIS(BRAKE), \
    DEFINE_AXIS(DISTANCE), \
    DEFINE_AXIS(TILT), \
    DEFINE_AXIS(SCROLL), \
    DEFINE_AXIS(RELATIVE_X), \
    DEFINE_AXIS(RELATIVE_Y), \
    {"RESERVED_29", 29}, \
    {"RESERVED_30", 30}, \
    {"RESERVED_31", 31}, \
    DEFINE_AXIS(GENERIC_1), \
    DEFINE_AXIS(GENERIC_2), \
    DEFINE_AXIS(GENERIC_3), \
    DEFINE_AXIS(GENERIC_4), \
    DEFINE_AXIS(GENERIC_5), \
    DEFINE_AXIS(GENERIC_6), \
    DEFINE_AXIS(GENERIC_7), \
    DEFINE_AXIS(GENERIC_8), \
    DEFINE_AXIS(GENERIC_9), \
    DEFINE_AXIS(GENERIC_10), \
    DEFINE_AXIS(GENERIC_11), \
    DEFINE_AXIS(GENERIC_12), \
    DEFINE_AXIS(GENERIC_13), \
    DEFINE_AXIS(GENERIC_14), \
    DEFINE_AXIS(GENERIC_15), \
    DEFINE_AXIS(GENERIC_16), \
    DEFINE_AXIS(GESTURE_X_OFFSET), \
    DEFINE_AXIS(GESTURE_Y_OFFSET), \
    DEFINE_AXIS(GESTURE_SCROLL_X_DISTANCE), \
    DEFINE_AXIS(GESTURE_SCROLL_Y_DISTANCE), \
    DEFINE_AXIS(GESTURE_PINCH_SCALE_FACTOR), \
    DEFINE_AXIS(GESTURE_SWIPE_FINGER_COUNT)

// NOTE: If you add new LEDs here, you must also add them to Input.h
#define LEDS_SEQUENCE \
    DEFINE_LED(NUM_LOCK), \
    DEFINE_LED(CAPS_LOCK), \
    DEFINE_LED(SCROLL_LOCK), \
    DEFINE_LED(COMPOSE), \
    DEFINE_LED(KANA), \
    DEFINE_LED(SLEEP), \
    DEFINE_LED(SUSPEND), \
    DEFINE_LED(MUTE), \
    DEFINE_LED(MISC), \
    DEFINE_LED(MAIL), \
    DEFINE_LED(CHARGING), \
    DEFINE_LED(CONTROLLER_1), \
    DEFINE_LED(CONTROLLER_2), \
    DEFINE_LED(CONTROLLER_3), \
    DEFINE_LED(CONTROLLER_4)

#define FLAGS_SEQUENCE \
    DEFINE_FLAG(VIRTUAL), \
    DEFINE_FLAG(FUNCTION), \
    DEFINE_FLAG(GESTURE), \
    DEFINE_FLAG(WAKE), \
    DEFINE_FLAG(FALLBACK_USAGE_MAPPING)

// clang-format on

// --- InputEventLookup ---

InputEventLookup::InputEventLookup()
      : KEYCODES({KEYCODES_SEQUENCE}),
        KEY_NAMES({KEYCODES_SEQUENCE}),
        AXES({AXES_SEQUENCE}),
        AXES_NAMES({AXES_SEQUENCE})/*,
        LEDS({LEDS_SEQUENCE}),
        FLAGS({FLAGS_SEQUENCE})*/ {}

int InputEventLookup::lookupValueByLabel(
        const std::unordered_map<std::string, int>& map, const char* literal) {
    std::string str(literal);
    auto it = map.find(str);
    return it != map.end() ? it->second : -1;
}

const char* InputEventLookup::lookupLabelByValue(const std::vector<InputEventLabel>& vec,
                                                 int value) {
    if (static_cast<size_t>(value) < vec.size()) {
        return vec[value].literal;
    }
    return nullptr;
}

int InputEventLookup::getKeyCodeByLabel(const char* label) {
    const auto& self = get();
    return self.lookupValueByLabel(self.KEYCODES, label);
}

const char* InputEventLookup::getLabelByKeyCode(int32_t keyCode) {
    const auto& self = get();
    if (keyCode >= 0 && static_cast<size_t>(keyCode) < self.KEYCODES.size()) {
        return get().lookupLabelByValue(self.KEY_NAMES, keyCode);
    }
    return nullptr;
}

int InputEventLookup::getKeyFlagByLabel(const char* label) {
    const auto& self = get();
    return lookupValueByLabel(self.FLAGS, label);
}

int InputEventLookup::getAxisByLabel(const char* label) {
    const auto& self = get();
    return lookupValueByLabel(self.AXES, label);
}

const char* InputEventLookup::getAxisLabel(int32_t axisId) {
    const auto& self = get();
    return lookupLabelByValue(self.AXES_NAMES, axisId);
}

int InputEventLookup::getLedByLabel(const char* label) {
    const auto& self = get();
    return lookupValueByLabel(self.LEDS, label);
}

namespace {

struct label {
    const char* name;
    int value;
};

#define LABEL(constant) { #constant, constant }
#define LABEL_END       { nullptr, -1 }

static struct label ev_key_value_labels[] = {
        {"UP", 0},
        {"DOWN", 1},
        {"REPEAT", 2},
        LABEL_END,
};

#include "private/input.h-labels.h"

#undef LABEL
#undef LABEL_END

std::string getLabel(const label* labels, int value) {
    if (labels == nullptr) return std::to_string(value);
    while (labels->name != nullptr && value != labels->value) {
        labels++;
    }
    return labels->name != nullptr ? labels->name : std::to_string(value);
}

int getValue(const label* labels, const char* searchLabel) {
    if (labels == nullptr) return {};
    while (labels->name != nullptr && ::strcasecmp(labels->name, searchLabel) != 0) {
        labels++;
    }
    return labels->name != nullptr ? labels->value : -1;
}

const label* getCodeLabelsForType(int32_t type) {
    switch (type) {
        case EV_SYN:  return syn_labels;
        case EV_KEY:  return key_labels;
        case EV_REL:  return rel_labels;
        case EV_ABS:  return abs_labels;
        case EV_SW:   return sw_labels;
        case EV_MSC:  return msc_labels;
        case EV_LED:  return led_labels;
        case EV_REP:  return rep_labels;
        case EV_SND:  return snd_labels;
        case EV_FF:   return ff_labels;
        case EV_FF_STATUS:
            return ff_status_labels;
        default:      return nullptr;
    }
}

const label* getValueLabelsForTypeAndCode(int32_t type, int32_t code) {
    if (type == EV_KEY) {
        return ev_key_value_labels;
    }
    if (type == EV_ABS && code == ABS_MT_TOOL_TYPE) {
        return mt_tool_labels;
    }
    return nullptr;
}

} // namespace

EvdevEventLabel InputEventLookup::getLinuxEvdevLabel(int32_t type, int32_t code, int32_t value) {
    return {
            .type = getLabel(ev_labels, type),
            .code = getLabel(getCodeLabelsForType(type), code),
            .value = getLabel(getValueLabelsForTypeAndCode(type, code), value),
    };
}

int InputEventLookup::getLinuxEvdevEventTypeByLabel(const char* label) {
    return getValue(ev_labels, label);
}

int InputEventLookup::getLinuxEvdevEventCodeByLabel(int32_t type, const char* label) {
    return getValue(getCodeLabelsForType(type), label);
}

int InputEventLookup::getLinuxEvdevInputPropByLabel(const char* label) {
    return getValue(input_prop_labels, label);
}
}/*endof namespace*/
