// Port of AOSP androidfw/ByteBucketArray.h (frameworks/base/libs/androidfw/
// include/androidfw/ByteBucketArray.h), namespace cdroid.
//
// Copyright (C) 2016 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.
#ifndef __CDROID_ANDROIDFW_BYTE_BUCKET_ARRAY_H__
#define __CDROID_ANDROIDFW_BYTE_BUCKET_ARRAY_H__

#include <cstdint>
#include <cstring>

#include <porting/cdlog.h>

namespace cdroid {

/**
 * Stores a sparsely populated array. Has a fixed size of 256
 * (number of entries that a byte can represent).
 */
template <typename T>
class ByteBucketArray {
public:
    ByteBucketArray() : default_() { memset(buckets_, 0, sizeof(buckets_)); }

    ~ByteBucketArray() {
        for (size_t i = 0; i < kNumBuckets; i++) {
            if (buckets_[i] != NULL) {
                delete[] buckets_[i];
            }
        }
        memset(buckets_, 0, sizeof(buckets_));
    }

    inline size_t size() const { return kNumBuckets * kBucketSize; }

    inline const T& get(size_t index) const { return (*this)[index]; }

    const T& operator[](size_t index) const {
        if (index >= size()) {
            return default_;
        }

        uint8_t bucket_index = static_cast<uint8_t>(index) >> 4;
        T* bucket = buckets_[bucket_index];
        if (bucket == NULL) {
            return default_;
        }
        return bucket[0x0f & static_cast<uint8_t>(index)];
    }

    T& editItemAt(size_t index) {
        FATAL_IF(index >= size(), "ByteBucketArray.editItemAt(index=%zu) with size=%zu",
                index, size());

        uint8_t bucket_index = static_cast<uint8_t>(index) >> 4;
        T* bucket = buckets_[bucket_index];
        if (bucket == NULL) {
            bucket = buckets_[bucket_index] = new T[kBucketSize]();
        }
        return bucket[0x0f & static_cast<uint8_t>(index)];
    }

    bool set(size_t index, const T& value) {
        if (index >= size()) {
            return false;
        }

        editItemAt(index) = value;
        return true;
    }

private:
    enum { kNumBuckets = 16, kBucketSize = 16 };

    T* buckets_[kNumBuckets];
    T default_;
};

}  // namespace cdroid

#endif  // __CDROID_ANDROIDFW_BYTE_BUCKET_ARRAY_H__
