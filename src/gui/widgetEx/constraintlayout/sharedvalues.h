/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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
