/*
 * Ported from Android android.text.method.MetaKeyKeyListener (Apache 2.0).
 * See metakeylistener.h for the span/long dual API. CDROID runs on full
 * (chorded) keyboards, so the KeyCharacterMap.MODIFIER_BEHAVIOR_CHORDED_OR_
 * TOGGLED branch (toggled keypads) never applies — the helper below always
 * reports chorded, which means span-stored meta is not combined into
 * getMetaState(text, event); the physical KeyEvent's metaState already carries
 * Shift/Alt/Ctrl for a desktop keyboard.
 */
#include <text/method/metakeylistener.h>
#include <text/editable.h>
#include <text/spannablestring.h>
#include <view/keyevent.h>
#include <view/view.h>

namespace cdroid {

// --- public meta flag aliases (KeyEvent values) ---
const int MetaKeyKeyListener::META_SHIFT_ON    = KeyEvent::META_SHIFT_ON;
const int MetaKeyKeyListener::META_ALT_ON      = KeyEvent::META_ALT_ON;
const int MetaKeyKeyListener::META_SYM_ON      = KeyEvent::META_SYM_ON;
const int MetaKeyKeyListener::META_CAP_LOCKED  = KeyEvent::META_CAP_LOCKED;
const int MetaKeyKeyListener::META_ALT_LOCKED  = KeyEvent::META_ALT_LOCKED;
const int MetaKeyKeyListener::META_SYM_LOCKED  = KeyEvent::META_SYM_LOCKED;
const int MetaKeyKeyListener::META_SELECTING   = KeyEvent::META_SELECTING;

// --- private long masks (outside int range, per Android) ---
const long MetaKeyKeyListener::META_CAP_USED    = 1L << 32;
const long MetaKeyKeyListener::META_ALT_USED    = 1L << 33;
const long MetaKeyKeyListener::META_SYM_USED    = 1L << 34;
const long MetaKeyKeyListener::META_CAP_PRESSED = 1L << 40;
const long MetaKeyKeyListener::META_ALT_PRESSED = 1L << 41;
const long MetaKeyKeyListener::META_SYM_PRESSED = 1L << 42;
const long MetaKeyKeyListener::META_CAP_RELEASED= 1L << 48;
const long MetaKeyKeyListener::META_ALT_RELEASED= 1L << 49;
const long MetaKeyKeyListener::META_SYM_RELEASED= 1L << 50;
const long MetaKeyKeyListener::META_SHIFT_MASK  = META_SHIFT_ON | META_CAP_LOCKED | META_CAP_USED | META_CAP_PRESSED | META_CAP_RELEASED;
const long MetaKeyKeyListener::META_ALT_MASK    = META_ALT_ON | META_ALT_LOCKED | META_ALT_USED | META_ALT_PRESSED | META_ALT_RELEASED;
const long MetaKeyKeyListener::META_SYM_MASK    = META_SYM_ON | META_SYM_LOCKED | META_SYM_USED | META_SYM_PRESSED | META_SYM_RELEASED;

// --- span markers (distinct objects; identity-compared) ---
const NoCopySpan* MetaKeyKeyListener::CAP       = new NoCopySpan();
const NoCopySpan* MetaKeyKeyListener::ALT       = new NoCopySpan();
const NoCopySpan* MetaKeyKeyListener::SYM       = new NoCopySpan();
const NoCopySpan* MetaKeyKeyListener::SELECTING = new NoCopySpan();

// --- span flag values: SPAN_MARK_MARK | (n << SPAN_USER_SHIFT) ---
const int MetaKeyKeyListener::PRESSED  = Spanned::SPAN_MARK_MARK | (1 << Spanned::SPAN_USER_SHIFT);
const int MetaKeyKeyListener::RELEASED = Spanned::SPAN_MARK_MARK | (2 << Spanned::SPAN_USER_SHIFT);
const int MetaKeyKeyListener::USED     = Spanned::SPAN_MARK_MARK | (3 << Spanned::SPAN_USER_SHIFT);
const int MetaKeyKeyListener::LOCKED   = Spanned::SPAN_MARK_MARK | (4 << Spanned::SPAN_USER_SHIFT);
const int MetaKeyKeyListener::PRESSED_RETURN_VALUE = 1;
const int MetaKeyKeyListener::LOCKED_RETURN_VALUE  = 2;

// CDROID modifier behavior: always chorded (full keyboard). The toggled-keypad
// value from Android's KeyCharacterMap.MODIFIER_BEHAVIOR_CHORDED_OR_TOGGLED.
static constexpr int MODIFIER_BEHAVIOR_CHORDED_OR_TOGGLED = 1;
static int modifierBehavior(const KeyEvent& /*event*/) { return 0; /* full/chorded keyboard */ }

// =====================================================================================
//  Span-based API
// =====================================================================================

void MetaKeyKeyListener::resetMetaState(Spannable& text) {
    text.removeSpan(CAP);
    text.removeSpan(ALT);
    text.removeSpan(SYM);
    text.removeSpan(SELECTING);
}

int MetaKeyKeyListener::getMetaState(CharSequence& text) {
    return getActive(text, CAP, META_SHIFT_ON, META_CAP_LOCKED)
         | getActive(text, ALT, META_ALT_ON, META_ALT_LOCKED)
         | getActive(text, SYM, META_SYM_ON, META_SYM_LOCKED)
         | getActive(text, SELECTING, META_SELECTING, META_SELECTING);
}

int MetaKeyKeyListener::getMetaState(CharSequence& text, const KeyEvent& event) {
    int metaState = event.getMetaState();
    if (modifierBehavior(event) == MODIFIER_BEHAVIOR_CHORDED_OR_TOGGLED) {
        metaState |= getMetaState(text);
    }
    return metaState;
}

int MetaKeyKeyListener::getMetaState(CharSequence& text, int meta) {
    switch (meta) {
        case META_SHIFT_ON:  return getActive(text, CAP, PRESSED_RETURN_VALUE, LOCKED_RETURN_VALUE);
        case META_ALT_ON:    return getActive(text, ALT, PRESSED_RETURN_VALUE, LOCKED_RETURN_VALUE);
        case META_SYM_ON:    return getActive(text, SYM, PRESSED_RETURN_VALUE, LOCKED_RETURN_VALUE);
        case META_SELECTING: return getActive(text, SELECTING, PRESSED_RETURN_VALUE, LOCKED_RETURN_VALUE);
        default: return 0;
    }
}

int MetaKeyKeyListener::getMetaState(CharSequence& text, int meta, const KeyEvent& event) {
    int metaState = event.getMetaState();
    if (modifierBehavior(event) == MODIFIER_BEHAVIOR_CHORDED_OR_TOGGLED) {
        metaState |= getMetaState(text);
    }
    if (META_SELECTING == meta) {
        if ((metaState & META_SELECTING) != 0) return 1;
        return 0;
    }
    return getMetaState((long)metaState, meta);
}

void MetaKeyKeyListener::adjustMetaAfterKeypress(Spannable& content) {
    adjust(content, CAP);
    adjust(content, ALT);
    adjust(content, SYM);
}

bool MetaKeyKeyListener::isMetaTracker(CharSequence& /*text*/, const ParcelableSpan* what) {
    return what == CAP || what == ALT || what == SYM || what == SELECTING;
}

bool MetaKeyKeyListener::isSelectingMetaTracker(CharSequence& /*text*/, const ParcelableSpan* what) {
    return what == SELECTING;
}

void MetaKeyKeyListener::resetLockedMeta(Spannable& content) {
    resetLock(content, CAP);
    resetLock(content, ALT);
    resetLock(content, SYM);
    resetLock(content, SELECTING);
}

void MetaKeyKeyListener::startSelecting(View& /*view*/, Spannable& content) {
    content.setSpan(SELECTING, 0, 0, PRESSED);
}

void MetaKeyKeyListener::stopSelecting(View& /*view*/, Spannable& content) {
    content.removeSpan(SELECTING);
}

void MetaKeyKeyListener::clearMetaKeyState(Editable& content, int states) {
    if ((states & META_SHIFT_ON) != 0) content.removeSpan(CAP);
    if ((states & META_ALT_ON) != 0) content.removeSpan(ALT);
    if ((states & META_SYM_ON) != 0) content.removeSpan(SYM);
    if ((states & META_SELECTING) != 0) content.removeSpan(SELECTING);
}

bool MetaKeyKeyListener::onKeyDown(View& /*view*/, Editable& content, int keyCode, const KeyEvent& /*event*/) {
    if (keyCode == KeyEvent::KEYCODE_SHIFT_LEFT || keyCode == KeyEvent::KEYCODE_SHIFT_RIGHT) {
        press(content, CAP);
        return true;
    }
    if (keyCode == KeyEvent::KEYCODE_ALT_LEFT || keyCode == KeyEvent::KEYCODE_ALT_RIGHT
            || keyCode == KeyEvent::KEYCODE_NUM) {
        press(content, ALT);
        return true;
    }
    if (keyCode == KeyEvent::KEYCODE_SYM) {
        press(content, SYM);
        return true;
    }
    return false; // no super to call through to
}

bool MetaKeyKeyListener::onKeyUp(View& /*view*/, Editable& content, int keyCode, const KeyEvent& event) {
    if (keyCode == KeyEvent::KEYCODE_SHIFT_LEFT || keyCode == KeyEvent::KEYCODE_SHIFT_RIGHT) {
        release(content, CAP, event);
        return true;
    }
    if (keyCode == KeyEvent::KEYCODE_ALT_LEFT || keyCode == KeyEvent::KEYCODE_ALT_RIGHT
            || keyCode == KeyEvent::KEYCODE_NUM) {
        release(content, ALT, event);
        return true;
    }
    if (keyCode == KeyEvent::KEYCODE_SYM) {
        release(content, SYM, event);
        return true;
    }
    return false; // no super to call through to
}

void MetaKeyKeyListener::clearMetaKeyState(View& /*view*/, Editable& content, int states) {
    clearMetaKeyState(content, states);
}

// --- private span helpers ---

int MetaKeyKeyListener::getActive(CharSequence& text, const ParcelableSpan* meta, int on, int lock) {
    Spanned* sp = dynamic_cast<Spanned*>(&text);
    if (sp == nullptr) return 0;

    const int flag = sp->getSpanFlags(meta);
    if (flag == LOCKED) return lock;
    else if (flag != 0) return on;
    else return 0;
}

void MetaKeyKeyListener::adjust(Spannable& content, const ParcelableSpan* what) {
    const int current = content.getSpanFlags(what);
    if (current == PRESSED)        content.setSpan(what, 0, 0, USED);
    else if (current == RELEASED)  content.removeSpan(what);
}

void MetaKeyKeyListener::resetLock(Spannable& content, const ParcelableSpan* what) {
    const int current = content.getSpanFlags(what);
    if (current == LOCKED) content.removeSpan(what);
}

void MetaKeyKeyListener::press(Editable& content, const ParcelableSpan* what) {
    const int state = content.getSpanFlags(what);
    if (state == PRESSED) {
        ; // repeat before use
    } else if (state == RELEASED) {
        content.setSpan(what, 0, 0, LOCKED);
    } else if (state == USED) {
        ; // repeat after use
    } else if (state == LOCKED) {
        content.removeSpan(what);
    } else {
        content.setSpan(what, 0, 0, PRESSED);
    }
}

void MetaKeyKeyListener::release(Editable& content, const ParcelableSpan* what, const KeyEvent& event) {
    const int current = content.getSpanFlags(what);
    switch (modifierBehavior(event)) {
        case MODIFIER_BEHAVIOR_CHORDED_OR_TOGGLED:
            if (current == USED)         content.removeSpan(what);
            else if (current == PRESSED) content.setSpan(what, 0, 0, RELEASED);
            break;
        default:
            // CDROID full keyboard: release always clears the meta span.
            content.removeSpan(what);
            break;
    }
}

// =====================================================================================
//  Long-bitmask API
// =====================================================================================

int MetaKeyKeyListener::getMetaState(long state) {
    int result = 0;
    if ((state & META_CAP_LOCKED) != 0)      result |= META_CAP_LOCKED;
    else if ((state & META_SHIFT_ON) != 0)   result |= META_SHIFT_ON;
    if ((state & META_ALT_LOCKED) != 0)      result |= META_ALT_LOCKED;
    else if ((state & META_ALT_ON) != 0)     result |= META_ALT_ON;
    if ((state & META_SYM_LOCKED) != 0)      result |= META_SYM_LOCKED;
    else if ((state & META_SYM_ON) != 0)     result |= META_SYM_ON;
    return result;
}

int MetaKeyKeyListener::getMetaState(long state, int meta) {
    switch (meta) {
        case META_SHIFT_ON:
            if ((state & META_CAP_LOCKED) != 0) return LOCKED_RETURN_VALUE;
            if ((state & META_SHIFT_ON) != 0)   return PRESSED_RETURN_VALUE;
            return 0;
        case META_ALT_ON:
            if ((state & META_ALT_LOCKED) != 0) return LOCKED_RETURN_VALUE;
            if ((state & META_ALT_ON) != 0)     return PRESSED_RETURN_VALUE;
            return 0;
        case META_SYM_ON:
            if ((state & META_SYM_LOCKED) != 0) return LOCKED_RETURN_VALUE;
            if ((state & META_SYM_ON) != 0)     return PRESSED_RETURN_VALUE;
            return 0;
        default:
            return 0;
    }
}

long MetaKeyKeyListener::adjustMetaAfterKeypress(long state) {
    if ((state & META_CAP_PRESSED) != 0)        state = (state & ~META_SHIFT_MASK) | META_SHIFT_ON | META_CAP_USED;
    else if ((state & META_CAP_RELEASED) != 0)  state &= ~META_SHIFT_MASK;
    if ((state & META_ALT_PRESSED) != 0)        state = (state & ~META_ALT_MASK) | META_ALT_ON | META_ALT_USED;
    else if ((state & META_ALT_RELEASED) != 0)  state &= ~META_ALT_MASK;
    if ((state & META_SYM_PRESSED) != 0)        state = (state & ~META_SYM_MASK) | META_SYM_ON | META_SYM_USED;
    else if ((state & META_SYM_RELEASED) != 0)  state &= ~META_SYM_MASK;
    return state;
}

long MetaKeyKeyListener::handleKeyDown(long state, int keyCode, const KeyEvent& /*event*/) {
    if (keyCode == KeyEvent::KEYCODE_SHIFT_LEFT || keyCode == KeyEvent::KEYCODE_SHIFT_RIGHT) {
        return press(state, META_SHIFT_ON, META_SHIFT_MASK, META_CAP_LOCKED, META_CAP_PRESSED, META_CAP_RELEASED, META_CAP_USED);
    }
    if (keyCode == KeyEvent::KEYCODE_ALT_LEFT || keyCode == KeyEvent::KEYCODE_ALT_RIGHT
            || keyCode == KeyEvent::KEYCODE_NUM) {
        return press(state, META_ALT_ON, META_ALT_MASK, META_ALT_LOCKED, META_ALT_PRESSED, META_ALT_RELEASED, META_ALT_USED);
    }
    if (keyCode == KeyEvent::KEYCODE_SYM) {
        return press(state, META_SYM_ON, META_SYM_MASK, META_SYM_LOCKED, META_SYM_PRESSED, META_SYM_RELEASED, META_SYM_USED);
    }
    return state;
}

long MetaKeyKeyListener::handleKeyUp(long state, int keyCode, const KeyEvent& event) {
    if (keyCode == KeyEvent::KEYCODE_SHIFT_LEFT || keyCode == KeyEvent::KEYCODE_SHIFT_RIGHT) {
        return release(state, META_SHIFT_ON, META_SHIFT_MASK, META_CAP_PRESSED, META_CAP_RELEASED, META_CAP_USED, event);
    }
    if (keyCode == KeyEvent::KEYCODE_ALT_LEFT || keyCode == KeyEvent::KEYCODE_ALT_RIGHT
            || keyCode == KeyEvent::KEYCODE_NUM) {
        return release(state, META_ALT_ON, META_ALT_MASK, META_ALT_PRESSED, META_ALT_RELEASED, META_ALT_USED, event);
    }
    if (keyCode == KeyEvent::KEYCODE_SYM) {
        return release(state, META_SYM_ON, META_SYM_MASK, META_SYM_PRESSED, META_SYM_RELEASED, META_SYM_USED, event);
    }
    return state;
}

long MetaKeyKeyListener::press(long state, int what, long mask,
        long locked, long pressed, long released, long used) {
    if ((state & pressed) != 0) {
        ; // repeat before use
    } else if ((state & released) != 0) {
        state = (state & ~mask) | what | locked;
    } else if ((state & used) != 0) {
        ; // repeat after use
    } else if ((state & locked) != 0) {
        state &= ~mask;
    } else {
        state |= what | pressed;
    }
    return state;
}

long MetaKeyKeyListener::release(long state, int what, long mask,
        long pressed, long released, long used, const KeyEvent& event) {
    switch (modifierBehavior(event)) {
        case MODIFIER_BEHAVIOR_CHORDED_OR_TOGGLED:
            if ((state & used) != 0) {
                state &= ~mask;
            } else if ((state & pressed) != 0) {
                state |= (long)what | released;
            }
            break;
        default:
            state &= ~mask;
            break;
    }
    return state;
}

long MetaKeyKeyListener::resetLockedMeta(long state) {
    if ((state & META_CAP_LOCKED) != 0) state &= ~META_SHIFT_MASK;
    if ((state & META_ALT_LOCKED) != 0) state &= ~META_ALT_MASK;
    if ((state & META_SYM_LOCKED) != 0) state &= ~META_SYM_MASK;
    return state;
}

long MetaKeyKeyListener::clearMetaKeyState(long state, int which) {
    if ((which & META_SHIFT_ON) != 0 && (state & META_CAP_LOCKED) != 0) state &= ~META_SHIFT_MASK;
    if ((which & META_ALT_ON) != 0 && (state & META_ALT_LOCKED) != 0)   state &= ~META_ALT_MASK;
    if ((which & META_SYM_ON) != 0 && (state & META_SYM_LOCKED) != 0)   state &= ~META_SYM_MASK;
    return state;
}

} // namespace cdroid
