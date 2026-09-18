#ifndef CDROID_ESPRESSO_ESPRESSOKEY_H
#define CDROID_ESPRESSO_ESPRESSOKEY_H

/*
 * android.support.test.espresso.action.EspressoKey — wraps the key code and
 * meta state of the desired key press.
 */

#include <string>

#include <view/keyevent.h>

namespace cdroid {
namespace espresso {

class EspressoKey {
public:
    class Builder;

    int getKeyCode() const { return mKeyCode; }
    int getMetaState() const { return mMetaState; }

    std::string toString() const {
        return "keyCode: " + std::to_string(mKeyCode)
            + ", metaState: " + std::to_string(mMetaState);
    }

private:
    friend class Builder;
    explicit EspressoKey(const Builder& builder);
    int mKeyCode;
    int mMetaState;
};

class EspressoKey::Builder {
public:
    Builder& withKeyCode(int keyCode) {
        mBuilderKeyCode = keyCode;
        return *this;
    }

    /** Sets the SHIFT_ON meta state of the resulting key. */
    Builder& withShiftPressed(bool shiftPressed) {
        mIsShiftPressed = shiftPressed;
        return *this;
    }

    /** Sets the CTRL_ON meta state of the resulting key. */
    Builder& withCtrlPressed(bool ctrlPressed) {
        mIsCtrlPressed = ctrlPressed;
        return *this;
    }

    /** Sets the ALT_ON meta state of the resulting key. */
    Builder& withAltPressed(bool altPressed) {
        mIsAltPressed = altPressed;
        return *this;
    }

    int getMetaState() const {
        int metaState = 0;
        if (mIsShiftPressed) metaState |= KeyEvent::META_SHIFT_ON;
        if (mIsAltPressed) metaState |= KeyEvent::META_ALT_ON;
        if (mIsCtrlPressed) metaState |= KeyEvent::META_CTRL_ON;
        return metaState;
    }

    EspressoKey build();

private:
    friend class EspressoKey;
    int mBuilderKeyCode = -1;
    bool mIsShiftPressed = false;
    bool mIsAltPressed = false;
    bool mIsCtrlPressed = false;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ESPRESSOKEY_H*/
