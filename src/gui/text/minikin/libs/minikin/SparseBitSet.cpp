/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include "minikin/SparseBitSet.h"

#include "MinikinInternal.h"

namespace minikin {

const uint32_t SparseBitSet::kNotFound;

uint32_t SparseBitSet::calcNumPages(const uint32_t* ranges, size_t nRanges) {
    bool haveZeroPage = false;
    uint32_t nonzeroPageEnd = 0;
    uint32_t nPages = 0;
    for (size_t i = 0; i < nRanges; i++) {
        uint32_t start = ranges[i * 2];
        uint32_t end = ranges[i * 2 + 1];
        uint32_t startPage = start >> kLogValuesPerPage;
        uint32_t endPage = (end - 1) >> kLogValuesPerPage;
        if (startPage >= nonzeroPageEnd) {
            if (startPage > nonzeroPageEnd) {
                if (!haveZeroPage) {
                    haveZeroPage = true;
                    nPages++;
                }
            }
            nPages++;
        }
        nPages += endPage - startPage;
        nonzeroPageEnd = endPage + 1;
    }
    return nPages;
}

void SparseBitSet::initFromRanges(const uint32_t* ranges, size_t nRanges) {
    if (nRanges == 0) {
        return;
    }
    const uint32_t maxVal = ranges[nRanges * 2 - 1];
    if (maxVal >= kMaximumCapacity) {
        return;
    }
    uint32_t indicesCount = (maxVal + kPageMask) >> kLogValuesPerPage;
    uint32_t nPages = calcNumPages(ranges, nRanges);
    uint32_t bitmapsCount = nPages << (kLogValuesPerPage - kLogBitsPerEl);
    MappableData* data = MappableData::allocate(indicesCount, bitmapsCount);
    mData.reset(data);
    data->mMaxVal = maxVal;
    uint16_t* indices = data->indices();
    element* bitmaps = data->bitmaps();
    memset(bitmaps, 0, sizeof(uint32_t) * bitmapsCount);
    data->mZeroPageIndex = noZeroPage;
    uint32_t nonzeroPageEnd = 0;
    uint32_t currentPage = 0;
    for (size_t i = 0; i < nRanges; i++) {
        uint32_t start = ranges[i * 2];
        uint32_t end = ranges[i * 2 + 1];
        MINIKIN_ASSERT(start <= end, "Range size must be nonnegative");
        uint32_t startPage = start >> kLogValuesPerPage;
        uint32_t endPage = (end - 1) >> kLogValuesPerPage;
        if (startPage >= nonzeroPageEnd) {
            if (startPage > nonzeroPageEnd) {
                if (data->mZeroPageIndex == noZeroPage) {
                    data->mZeroPageIndex = (currentPage++) << (kLogValuesPerPage - kLogBitsPerEl);
                }
                for (uint32_t j = nonzeroPageEnd; j < startPage; j++) {
                    indices[j] = data->mZeroPageIndex;
                }
            }
            indices[startPage] = (currentPage++) << (kLogValuesPerPage - kLogBitsPerEl);
        }

        size_t index = ((currentPage - 1) << (kLogValuesPerPage - kLogBitsPerEl)) +
                       ((start & kPageMask) >> kLogBitsPerEl);
        size_t nElements = (end - (start & ~kElMask) + kElMask) >> kLogBitsPerEl;
        if (nElements == 1) {
            bitmaps[index] |=
                    (kElAllOnes >> (start & kElMask)) & (kElAllOnes << ((~end + 1) & kElMask));
        } else {
            bitmaps[index] |= kElAllOnes >> (start & kElMask);
            for (size_t j = 1; j < nElements - 1; j++) {
                bitmaps[index + j] = kElAllOnes;
            }
            bitmaps[index + nElements - 1] |= kElAllOnes << ((~end + 1) & kElMask);
        }
        for (size_t j = startPage + 1; j < endPage + 1; j++) {
            indices[j] = (currentPage++) << (kLogValuesPerPage - kLogBitsPerEl);
        }
        nonzeroPageEnd = endPage + 1;
    }
}

void SparseBitSet::initFromBuffer(BufferReader* reader) {
    uint32_t size = reader->read<uint32_t>();
    if (size == 0) return;
    mData.reset(reader->map<MappableData, alignof(MappableData)>(size));
}

void SparseBitSet::writeTo(BufferWriter* writer) const {
    if (mData == nullptr) {
        // Write 0 for empty SparseBitSet.
        writer->write<uint32_t>(0);
        return;
    }
    size_t size = mData->size();
    writer->write<uint32_t>(size);
    static_assert(alignof(MappableData) == 4);
    MappableData* out = writer->reserve<MappableData, alignof(MappableData)>(size);
    if (out != nullptr) {
        memcpy(out, mData.get(), size);
        out->mIsMapped = 1;
    }
}

int SparseBitSet::CountLeadingZeros(element x) {
    // Note: GCC / clang builtin
    return sizeof(element) <= sizeof(int) ? __builtin_clz(x) : __builtin_clzl(x);
}

uint32_t SparseBitSet::nextSetBit(uint32_t fromIndex) const {
    if (mData == nullptr || fromIndex >= mData->mMaxVal) {
        return kNotFound;
    }
    uint32_t fromPage = fromIndex >> kLogValuesPerPage;
    const element* bitmap = mData->bitmaps() + mData->indices()[fromPage];
    uint32_t offset = (fromIndex & kPageMask) >> kLogBitsPerEl;
    element e = bitmap[offset] & (kElAllOnes >> (fromIndex & kElMask));
    if (e != 0) {
        return (fromIndex & ~kElMask) + CountLeadingZeros(e);
    }
    for (uint32_t j = offset + 1; j < (1 << (kLogValuesPerPage - kLogBitsPerEl)); j++) {
        e = bitmap[j];
        if (e != 0) {
            return (fromIndex & ~kPageMask) + (j << kLogBitsPerEl) + CountLeadingZeros(e);
        }
    }
    uint32_t maxPage = (mData->mMaxVal + kPageMask) >> kLogValuesPerPage;
    for (uint32_t page = fromPage + 1; page < maxPage; page++) {
        uint16_t index = mData->indices()[page];
        if (index == mData->mZeroPageIndex) {
            continue;
        }
        bitmap = mData->bitmaps() + index;
        for (uint32_t j = 0; j < (1 << (kLogValuesPerPage - kLogBitsPerEl)); j++) {
            e = bitmap[j];
            if (e != 0) {
                return (page << kLogValuesPerPage) + (j << kLogBitsPerEl) + CountLeadingZeros(e);
            }
        }
    }
    return kNotFound;
}

// static
SparseBitSet::MappableData* SparseBitSet::MappableData::allocate(uint32_t indicesCount,
                                                                 uint32_t bitmapsCount) {
    MappableData* data = reinterpret_cast<MappableData*>(
            malloc(MappableData::calcSize(indicesCount, bitmapsCount)));
    data->mIndicesCount = indicesCount;
    data->mBitmapsCount = bitmapsCount;
    data->mIsMapped = 0;
    return data;
}

}  // namespace minikin
