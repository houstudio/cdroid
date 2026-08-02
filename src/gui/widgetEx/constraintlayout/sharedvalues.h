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
 * is told the new + old value. Listeners are CallbackBase values owned by the registry; a caller
 * that wants to unregister keeps its handle and calls removeListener (identity by shared functor).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_SHARED_VALUES_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_SHARED_VALUES_H

#include <unordered_map>
#include <vector>

#include <core/callbackbase.h> // CallbackBase (SharedValuesListener value type)

namespace cdroid {

class SharedValues {
public:
    static constexpr int UNSET = -1;

    // SharedValuesListener: a single-callback listener (androidx
    // SharedValues.SharedValuesListener#onNewValue). Expressed as a CallbackBase value (CDROID
    // style): addListener/removeListener take it by const ref and the registry owns its listeners
    // (stored by value), so no caller new/delete and no "must removeListener before dying" contract.
    // Identity for removeListener is the CallbackBase shared-functor pointer — a copy compares equal
    // to its original, so a caller that kept its handle can remove exactly its registration.
    using SharedValuesListener = CallbackBase<void, int /*key*/, int /*newValue*/, int /*oldValue*/>;

    // Register `listener` for changes to `key`.
    void addListener(int key, const SharedValuesListener& listener);
    // Remove `listener` from a single key.
    void removeListener(int key, const SharedValuesListener& listener);
    // Remove `listener` from every key.
    void removeListener(const SharedValuesListener& listener);
    void clearListeners();

    int getValue(int key) const;

    // Store `value` under `key` and notify this key's listeners with (newValue, oldValue). A no-op
    // (no notification) if the value is unchanged.
    void fireNewValue(int key, int value);

private:
    std::unordered_map<int, int> mValues;
    std::unordered_map<int, std::vector<SharedValuesListener>> mListeners;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_SHARED_VALUES_H
