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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.SharedValues.
 *
 * A registry of named integer values that notifies listeners when a value changes. Used by
 * ViewTransition's sharedValueSet/sharedValueUnset triggers (and any cross-component coordination):
 * MotionLayout.setSharedValue(key, value) fires the value, and any registered listener for that key
 * is told the new + old value. The registry holds raw listener pointers (borrowed); a listener must
 * remove itself before it is destroyed.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_SHARED_VALUES_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_SHARED_VALUES_H

#include <unordered_map>
#include <vector>

namespace cdroid {

class SharedValues {
public:
    static constexpr int UNSET = -1;

    class SharedValuesListener {
    public:
        virtual ~SharedValuesListener() = default;
        // Called when the value for a listened key changes.
        virtual void onNewValue(int key, int newValue, int oldValue) = 0;
    };

    // Register `listener` for changes to `key` (borrowed; caller must removeListener before dying).
    void addListener(int key, SharedValuesListener* listener);
    // Remove `listener` from a single key.
    void removeListener(int key, SharedValuesListener* listener);
    // Remove `listener` from every key.
    void removeListener(SharedValuesListener* listener);
    void clearListeners();

    int getValue(int key) const;

    // Store `value` under `key` and notify this key's listeners with (newValue, oldValue). A no-op
    // (no notification) if the value is unchanged.
    void fireNewValue(int key, int value);

private:
    std::unordered_map<int, int> mValues;
    std::unordered_map<int, std::vector<SharedValuesListener*>> mListeners;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_SHARED_VALUES_H
