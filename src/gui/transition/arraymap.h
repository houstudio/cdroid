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
#ifndef __CDROID_TRANSITION_ARRAYMAP_H__
#define __CDROID_TRANSITION_ARRAYMAP_H__

#include <vector>
#include <utility>

namespace cdroid {

/**
 * Lightweight ordered associative container backed by a std::vector of pairs.
 *
 * This mirrors only the subset of android.util.ArrayMap that android.transition
 * actually relies on — in particular the *indexed* access (keyAt / valueAt /
 * removeAt) used by Transition::matchInstances, which walks the map in reverse
 * and mutates it while iterating. std::unordered_map cannot express that, so a
 * small vector-backed map is provided here instead.
 *
 * It is an internal helper for the transition port, NOT a faithful port of
 * android.util.ArrayMap (no hashing, linear scan lookup — fine for the small
 * per-transition maps it backs).
 */
template<typename K, typename V>
class ArrayMap {
  public:
    using value_type = std::pair<K, V>;

    ArrayMap() = default;

    bool isEmpty() const {
        return mEntries.empty();
    }
    int  size() const {
        return (int)mEntries.size();
    }
    bool containsKey(const K& key) const {
        return indexOfKey(key) >= 0;
    }

    int indexOfKey(const K& key) const {
        for (size_t i = 0; i < mEntries.size(); ++i) {
            if (mEntries[i].first == key) return (int)i;
        }
        return -1;
    }

    // Returns a pointer to the stored value (nullptr if absent). Keeps android's
    // "V get(K) -> may be null" semantics without forcing a value copy.
    V* get(const K& key) {
        int i = indexOfKey(key);
        return i >= 0 ? &mEntries[i].second : nullptr;
    }
    const V* get(const K& key) const {
        int i = indexOfKey(key);
        return i >= 0 ? &mEntries[i].second : nullptr;
    }

    // Insert or overwrite. Returns the previous value (default-constructed if none),
    // matching android.util.ArrayMap#put(K, V) -> V.
    V put(const K& key, const V& value) {
        int i = indexOfKey(key);
        if (i >= 0) {
            V old = mEntries[i].second;
            mEntries[i].second = value;
            return old;
        }
        mEntries.emplace_back(key, value);
        return V();
    }

    // Removes the entry for key; returns true if it existed.
    bool remove(const K& key) {
        int i = indexOfKey(key);
        if (i < 0) return false;
        mEntries.erase(mEntries.begin() + i);
        return true;
    }

    const K& keyAt(int index) const {
        return mEntries.at(index).first;
    }
    V&       valueAt(int index)     {
        return mEntries.at(index).second;
    }
    const V& valueAt(int index) const {
        return mEntries.at(index).second;
    }

    // Pointer access (nullptr if out of range). Convenient for the "may be absent"
    // patterns in Transition (e.g. fetching AnimationInfo from the running-animators map).
    V*       valueAtPtr(int index)       {
        return (index >= 0 && index < (int)mEntries.size()) ? &mEntries[index].second : nullptr;
    }
    const V* valueAtPtr(int index) const {
        return (index >= 0 && index < (int)mEntries.size()) ? &mEntries[index].second : nullptr;
    }

    // Removes the entry at index, shifting later entries down (android ArrayMap#removeAt).
    void removeAt(int index) {
        mEntries.erase(mEntries.begin() + index);
    }

    void clear() {
        mEntries.clear();
    }

    // Range-for over (key,value) pairs.
    typename std::vector<value_type>::iterator begin() {
        return mEntries.begin();
    }
    typename std::vector<value_type>::iterator end()   {
        return mEntries.end();
    }
    typename std::vector<value_type>::const_iterator begin() const {
        return mEntries.begin();
    }
    typename std::vector<value_type>::const_iterator end()   const {
        return mEntries.end();
    }

  private:
    std::vector<value_type> mEntries;
};

} // namespace cdroid
#endif // __CDROID_TRANSITION_ARRAYMAP_H__
