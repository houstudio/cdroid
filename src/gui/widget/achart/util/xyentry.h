/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
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
 */
#ifndef __XY_ENTRY_H__
#define __XY_ENTRY_H__
namespace cdroid{
template<typename K, typename V>
class XYEntry {
private:
    K key;
    V value;

public:
    XYEntry(const K& k, const V& v) : key(k), value(v) {}
    K getKey() const {
        return key;
    }
    V getValue() const {
        return value;
    }
    V setValue(const V& newValue) {
        value = newValue;
        return value;
    }
};
}/*endof namespace*/
#endif/*__XY_ENTRY_H__*/
