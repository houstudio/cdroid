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
#ifndef __NINEPATCH_CDROID_H__
#define __NINEPATCH_CDROID_H__
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <core/rect.h>
namespace cdroid {

struct alignas(uintptr_t) Res_png_9patch{
    Res_png_9patch() : wasDeserialized(false), xDivsOffset(0),
                       yDivsOffset(0), colorsOffset(0) { }

    int8_t wasDeserialized;
    uint8_t numXDivs;
    uint8_t numYDivs;
    uint8_t numColors;

    uint32_t xDivsOffset;
    uint32_t yDivsOffset;

    int32_t paddingLeft, paddingRight;
    int32_t paddingTop, paddingBottom;

    enum {
        // The 9 patch segment is not a solid color.
        NO_COLOR = 0x00000001,

        // The 9 patch segment is completely transparent.
        TRANSPARENT_COLOR = 0x00000000
    };

    // The offset (from the start of this structure) to the colors array
    // for this 9patch.
    uint32_t colorsOffset;

    // Convert data from device representation to PNG file representation.
    void deviceToFile();
    // Convert data from PNG file representation to device representation.
    void fileToDevice();

    // Serialize/Marshall the patch data into a newly malloc-ed block.
    static void* serialize(const Res_png_9patch& patchHeader, const int32_t* xDivs,
                           const int32_t* yDivs, const uint32_t* colors);
    // Serialize/Marshall the patch data into |outData|.
    static void serialize(const Res_png_9patch& patchHeader, const int32_t* xDivs,
                           const int32_t* yDivs, const uint32_t* colors, void* outData);
    // Deserialize/Unmarshall the patch data
    static Res_png_9patch* deserialize(void* data);
    // Compute the size of the serialized data structure
    size_t serializedSize() const;

    inline int32_t* getXDivs() const {
        return reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(this) + xDivsOffset);
    }
    inline int32_t* getYDivs() const {
        return reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(this) + yDivsOffset);
    }
    inline uint32_t* getColors() const {
        return reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(this) + colorsOffset);
    }
} __attribute__((packed));

struct Range {
    int32_t start = 0;
    int32_t end = 0;

    explicit Range() = default;
    inline explicit Range(int32_t s, int32_t e) : start(s), end(e) {
    }
};

inline bool operator==(const Range& left, const Range& right) {
  return left.start == right.start && left.end == right.end;
}

struct Bounds {
    int32_t left = 0;
    int32_t top = 0;
    int32_t right = 0;
    int32_t bottom = 0;

    explicit Bounds() = default;
    inline explicit Bounds(int32_t l, int32_t t, int32_t r, int32_t b)
        : left(l), top(t), right(r), bottom(b) {
    }
    bool nonZero() const;
};

inline bool Bounds::nonZero() const {
  return left != 0 || top != 0 || right != 0 || bottom != 0;
}

inline bool operator==(const Bounds& left, const Bounds& right) {
  return left.left == right.left && left.top == right.top && left.right == right.right &&
         left.bottom == right.bottom;
}

class NinePatch {
public:
    static std::unique_ptr<NinePatch> Create(uint8_t** rows, const int32_t width,
                        const int32_t height, std::string* err_out);
    static uint32_t PackRGBA(const uint8_t* pixel);

    Bounds padding;
    Bounds layout_bounds;
    Bounds outline;

    float outline_radius = 0.0f;
    uint32_t outline_alpha = 0x000000ffu;
    std::vector<Range> horizontal_stretch_regions;
    std::vector<Range> vertical_stretch_regions;
    std::vector<uint32_t> region_colors;
    std::unique_ptr<uint8_t[]> SerializeBase(size_t* out_len) const;
    std::unique_ptr<uint8_t[]> SerializeLayoutBounds(size_t* out_len) const;
    std::unique_ptr<uint8_t[]> SerializeRoundedRectOutline(size_t* out_len) const;
private:
    explicit NinePatch() = default;
    DISALLOW_COPY_AND_ASSIGN(NinePatch);
};

::std::ostream& operator<<(::std::ostream& out, const Range& range);
::std::ostream& operator<<(::std::ostream& out, const Bounds& bounds);
::std::ostream& operator<<(::std::ostream& out, const NinePatch& nine_patch);

}/*namespace cdroid*/
#endif/*__NINEPATCH_CDROID_H__*/
