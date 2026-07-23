#ifndef MINIKIN_UTILS_TYPEHELPERS_H
#define MINIKIN_UTILS_TYPEHELPERS_H

// Original: https://android.googlesource.com/platform/system/core/+/refs/heads/main/libutils/binder/include/utils/TypeHelpers.h
// Commit 007036bbdb9c09ebe245c01074a12b25c803ff54
/*
 * Copyright (C) 2005 The Android Open Source Project
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

#include <cstdint>
#include <cstring>
namespace android {

/*
 * Hash codes.
 */
typedef uint32_t hash_t;
template <typename TKey>
hash_t hash_type(const TKey& key);
/* Built-in hash code specializations */
#define ANDROID_INT32_HASH(T) \
        template <> inline hash_t hash_type(const T& value) { return hash_t(value); }
#define ANDROID_INT64_HASH(T) \
        template <> inline hash_t hash_type(const T& value) { \
                return hash_t((value >> 32) ^ value); }
#define ANDROID_REINTERPRET_HASH(T, R) \
        template <> inline hash_t hash_type(const T& value) { \
            R newValue; \
            static_assert(sizeof(newValue) == sizeof(value), "size mismatch"); \
            memcpy(&newValue, &value, sizeof(newValue)); \
            return hash_type(newValue); \
        }
ANDROID_INT32_HASH(bool)
ANDROID_INT32_HASH(int8_t)
ANDROID_INT32_HASH(uint8_t)
ANDROID_INT32_HASH(int16_t)
ANDROID_INT32_HASH(uint16_t)
ANDROID_INT32_HASH(int32_t)
ANDROID_INT32_HASH(uint32_t)
ANDROID_INT64_HASH(int64_t)
ANDROID_INT64_HASH(uint64_t)
ANDROID_REINTERPRET_HASH(float, uint32_t)
ANDROID_REINTERPRET_HASH(double, uint64_t)
template <typename T> inline hash_t hash_type(T* const & value) {
    return hash_type(uintptr_t(value));
}

} // namespace android

#endif // MINIKIN_UTILS_TYPEHELPERS_H
