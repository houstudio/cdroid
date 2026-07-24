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
 */

#ifndef MINIKIN_BUFFER_H
#define MINIKIN_BUFFER_H

#include <cstring>
#include <cstdint>
#include <string>//string_view>
#include <type_traits>
#include <utility>

namespace minikin {

// This is a helper class to read data from a memory buffer.
// This class does not copy memory, and may return pointers to parts of the memory buffer.
// Thus the memory buffer should outlive objects created using this class.
//
// Note on alignment:
// Some CPU archs (e.g. arm32) do not allow misaligned memory access.
// Therefore, BufferReader and BufferWriter automatically insert paddings
// to align data records.
// For the padding to be deterministic, the following conditions must be met:
// (1) Alignment and size of each data record must be fixed regardless of
//     CPU arch.
// (2) Alignment for each data record must be a power of 2 (2^n) and
//     must be less than or equal to kMaxAlignment.
// (3) The head address of the buffer must be aligned at kMaxAlignment.
//
// The condition (2) and (3) ensures that 'headAddress % align == 0'
// and the padding is determined only by the current position.
// I.e. mCurrent % align == (mCurrent - headAddress) % align.
class BufferReader {
public:
    static constexpr size_t kMaxAlignment = 8;

    explicit BufferReader(const void* buffer) : BufferReader(buffer, 0) {}
    BufferReader(const void* buffer, uint32_t pos)
            : mCurrent(reinterpret_cast<const uint8_t*>(buffer) + pos) {}

    // align() adds padding if necessary so that the returned pointer is aligned
    // at 'align' template parameter (i.e. align<T, _align>(p) % _align == 0).
    //
    // By default we align to sizeof(T) instead of alignof(T), because the
    // buffer may be shared between 32-bit processes and 64-bit processes.
    // The value of alignof(T) may change between the two.
    //
    // If T is a large struct or class, you would need to specify 'align'
    // template parameter manually.
    template <typename T, size_t _align = sizeof(T)>
    static const uint8_t* align(const uint8_t* p) {
        static_assert(_align <= kMaxAlignment);
        static_assert(__builtin_popcount(_align) == 1, "align must be a power of 2");
        constexpr size_t mask = _align - 1;
        intptr_t i = reinterpret_cast<intptr_t>(p);
        intptr_t aligned = (i + mask) & ~mask;
        return reinterpret_cast<const uint8_t*>(aligned);
    }

    template <typename T, size_t _align = sizeof(T)>
    const T& read() {
        const T* data = map<T, _align>(sizeof(T));
        return *data;
    }

    template <typename T, size_t _align = sizeof(T)>
    const T* map(uint32_t size) {
        static_assert(std::is_pod<T>::value, "T must be a POD");
        mCurrent = BufferReader::align<T, _align>(mCurrent);
        const T* data = reinterpret_cast<const T*>(mCurrent);
        mCurrent += size;
        return data;
    }

    template <typename T, size_t _align = sizeof(T)>
    void skip() {
        static_assert(std::is_pod<T>::value, "T must be a POD");
        mCurrent = BufferReader::align<T, _align>(mCurrent);
        mCurrent += sizeof(T);
    }

    // Return a pointer to an array and its number of elements.
    template <typename T, size_t _align = sizeof(T)>
    std::pair<const T*, uint32_t> readArray() {
        static_assert(std::is_pod<T>::value, "T must be a POD");
        static_assert(sizeof(T) % _align == 0);
        uint32_t size = read<uint32_t>();
        mCurrent = BufferReader::align<T, _align>(mCurrent);
        const T* data = reinterpret_cast<const T*>(mCurrent);
        mCurrent += size * sizeof(T);
        return std::make_pair(data, size);
    }

    template <typename T, size_t _align = sizeof(T)>
    void skipArray() {
        static_assert(std::is_pod<T>::value, "T must be a POD");
        uint32_t size = read<uint32_t>();
        mCurrent = BufferReader::align<T, _align>(mCurrent);
        mCurrent += size * sizeof(T);
    }

    /*std::string_view readString() {
        auto [data, size] = readArray<char>();
        return std::string_view(data, size);
    }*//*comment by zhhou*/

    void skipString() { skipArray<char>(); }

    const void* current() const { return mCurrent; }

private:
    const uint8_t* mCurrent;
};

// This is a helper class to write data to a memory buffer.
//
// BufferWriter does NOT allocate the memory.
// The typical usage is to use BufferWriter twice; in the first pass, write
// data with a fake BufferWriter (BufferWriter(nullptr)) to calculate the buffer
// size. In the second pass, allocate a memory buffer and use a real
// BufferWriter to write the data.
// Pseudo code:
//     BufferWriter fakeWriter(nullptr);
//     myData.writeTo(&fakeWriter);
//     void* buffer = malloc(fakeWriter.size());
//     BufferWriter realWriter(buffer);
//     myData.writeTo(&realWriter);
class BufferWriter {
public:
    // Create a buffer writer. Passing nullptr creates a fake writer,
    // which can be used to measure the buffer size needed.
    explicit BufferWriter(void* buffer) : BufferWriter(buffer, 0) {}
    BufferWriter(void* buffer, uint32_t pos)
            : mData(reinterpret_cast<uint8_t*>(buffer)), mPos(pos) {}

    BufferWriter(BufferWriter&&) = default;
    BufferWriter& operator=(BufferWriter&&) = default;

    // Write a single data of type T.
    // Please always specify T explicitly using <>. std::common_type_t<T> resolves to T, but
    // disables template argument deduction.
    // TODO: use std::type_identity_t when C++20 is available.
    template <typename T, size_t _align = sizeof(T)>
    void write(const std::common_type_t<T>& data) {
        T* buf = reserve<T, _align>(sizeof(T));
        if (buf != nullptr) {
            memcpy(buf, &data, sizeof(T));
        }
    }

    // Reserve a region and return a pointer to the reserved region.
    // The reserved region is not initialized.
    template <typename T, size_t _align = sizeof(T)>
    T* reserve(uint32_t size) {
        static_assert(std::is_pod<T>::value, "T must be a POD");
        mPos = BufferWriter::align<T, _align>(mPos);
        uint32_t pos = mPos;
        mPos += size;
        return mData == nullptr ? nullptr : reinterpret_cast<T*>(mData + pos);
    }

    // Write an array of type T.
    // Please always specify T explicitly using <>. std::common_type_t<T> resolves to T, but
    // disables template argument deduction.
    // TODO: use std::type_identity_t when C++20 is available.
    template <typename T, size_t _align = sizeof(T)>
    void writeArray(const std::common_type_t<T>* data, uint32_t size) {
        static_assert(std::is_pod<T>::value, "T must be a POD");
        static_assert(sizeof(T) % _align == 0);
        write<uint32_t>(size);
        mPos = BufferWriter::align<T, _align>(mPos);
        if (mData != nullptr) {
            memcpy(mData + mPos, data, size * sizeof(T));
        }
        mPos += size * sizeof(T);
    }

    void writeString(const std::string& string) { writeArray<char>(string.c_str(), string.size()); }

    // Return the number of bytes written.
    size_t size() const { return mPos; }

private:
    uint8_t* mData;
    size_t mPos;

    template <typename T, size_t _align>
    size_t align(size_t pos) const {
        return BufferReader::align<T, _align>(mData + pos) - mData;
    }

    // Forbid copy and assign.
    BufferWriter(const BufferWriter&) = delete;
    void operator=(const BufferWriter&) = delete;
};

}  // namespace minikin

#endif  // MINIKIN_BUFFER_H
