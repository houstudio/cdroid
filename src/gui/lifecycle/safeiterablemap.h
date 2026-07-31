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
#ifndef __SAFEITERABLEMAP_H__
#define __SAFEITERABLEMAP_H__
/*********************************************************************************
 * Port of androidx.lifecycle.FastSafeIterableMap (internal).
 *
 * Intrusive doubly-linked list backed by a hash map, tuned for the lifecycle
 * observer pattern where observers are added/removed during traversal. Removed
 * entries become "ghost nodes": they are unlinked from the live chain but keep
 * their next/prev pointers so an in-flight traversal can bridge back to the
 * remaining entries. New entries appended at the tail are visited by a running
 * forward traversal.
 *
 * Note: ceil(key) deliberately returns the PREDECESSOR of key (matches the
 * upstream method name/behavior), used by LifecycleRegistry.calculateTargetState.
 *
 * NOT thread-safe.
 *********************************************************************************/
#include <unordered_map>
#include <vector>
#include <functional>

namespace cdroid{
namespace lifecycle{

template<typename K, typename V>
class SafeIterableMap{
public:
    struct Entry{
        K key;
        V value;
        Entry* next = nullptr;
        Entry* prev = nullptr;
        bool removed = false;
        Entry(const K& k, const V& v) : key(k), value(v) {}
    };

    SafeIterableMap() = default;
    ~SafeIterableMap(){ clearAll(); }
    SafeIterableMap(const SafeIterableMap&) = delete;
    SafeIterableMap& operator=(const SafeIterableMap&) = delete;

    bool contains(const K& key) const { return mMap.find(key) != mMap.end(); }
    int size() const { return (int)mMap.size(); }

    // Returns the existing entry if key is already present, otherwise appends a
    // new entry at the tail and returns nullptr.
    Entry* putIfAbsent(const K& key, const V& value){
        auto it = mMap.find(key);
        if(it != mMap.end()) return it->second;
        Entry* e = new Entry(key, value);
        mMap[key] = e;
        if(!mTail){ mHead = e; mTail = e; }
        else { mTail->next = e; e->prev = mTail; mTail = e; }
        return nullptr;
    }

    // Unlinks the entry and marks it as a ghost node (next/prev retained).
    void remove(const K& key){
        auto it = mMap.find(key);
        if(it == mMap.end()) return;
        Entry* e = it->second;
        mMap.erase(it);
        if(!e->prev) mHead = e->next; else e->prev->next = e->next;
        if(!e->next) mTail = e->prev; else e->next->prev = e->prev;
        e->removed = true;
        mGarbage.push_back(e);
        if(mTraversalDepth == 0) flushGarbage();
    }

    Entry* first() const { return mHead; }
    Entry* last() const { return mTail; }

    // Returns the entry inserted immediately before key, or null.
    Entry* ceil(const K& key) const {
        auto it = mMap.find(key);
        return it == mMap.end() ? nullptr : it->second->prev;
    }

    // Forward traversal; safe to add/remove (including the current entry) inside
    // the action. Entries appended after the cursor are visited.
    void forEachWithAdditions(const std::function<void(Entry&)>& action){
        ++mTraversalDepth;
        Entry* cur = mHead;
        while(cur){
            Entry* next = cur->next; // capture before action may invalidate cur
            if(!cur->removed) action(*cur);
            cur = next;
        }
        if(--mTraversalDepth == 0) flushGarbage();
    }

    void forEachReversed(const std::function<void(Entry&)>& action){
        ++mTraversalDepth;
        Entry* cur = mTail;
        while(cur){
            Entry* prev = cur->prev;
            if(!cur->removed) action(*cur);
            cur = prev;
        }
        if(--mTraversalDepth == 0) flushGarbage();
    }

    void clearAll(){
        for(auto& kv : mMap) delete kv.second;
        mMap.clear();
        flushGarbage();
        mHead = mTail = nullptr;
    }

private:
    void flushGarbage(){
        for(Entry* e : mGarbage) delete e;
        mGarbage.clear();
    }
    std::unordered_map<K, Entry*> mMap;
    std::vector<Entry*> mGarbage;
    Entry* mHead = nullptr;
    Entry* mTail = nullptr;
    int mTraversalDepth = 0;
};

}//namespace lifecycle
}//namespace cdroid
#endif
