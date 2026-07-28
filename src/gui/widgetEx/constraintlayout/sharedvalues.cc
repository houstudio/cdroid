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
 */
#include <widgetEx/constraintlayout/sharedvalues.h>

#include <algorithm>

namespace cdroid {

void SharedValues::addListener(int key, SharedValuesListener* listener) {
    if (listener == nullptr) return;
    mListeners[key].push_back(listener);
}

void SharedValues::removeListener(int key, SharedValuesListener* listener) {
    auto it = mListeners.find(key);
    if (it == mListeners.end()) return;
    auto& vec = it->second;
    vec.erase(std::remove(vec.begin(), vec.end(), listener), vec.end());
}

void SharedValues::removeListener(SharedValuesListener* listener) {
    for (auto& kv : mListeners) {
        auto& vec = kv.second;
        vec.erase(std::remove(vec.begin(), vec.end(), listener), vec.end());
    }
}

void SharedValues::clearListeners() {
    mListeners.clear();
}

int SharedValues::getValue(int key) const {
    auto it = mValues.find(key);
    return (it != mValues.end()) ? it->second : UNSET;
}

void SharedValues::fireNewValue(int key, int value) {
    const int previous = getValue(key);
    if (previous == value) return; // unchanged -> no notification
    mValues[key] = value;
    auto it = mListeners.find(key);
    if (it == mListeners.end()) return;
    for (auto* listener : it->second) {
        if (listener != nullptr) listener->onNewValue(key, value, previous);
    }
}

} // namespace cdroid
