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
