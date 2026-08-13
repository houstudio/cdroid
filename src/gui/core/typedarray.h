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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0211-1301  USA
 *********************************************************************************/
//
// TypedArray — the CDROID-facing typed attribute view over a resolved styleable
// set, the C++ counterpart of android.util.TypedArray. Split out of androidfw's
// resourcetypes.h into gui/core because it is the consumer-side API: its
// high-level getters (getDrawable/getColorStateList) reach back into
// cdroid::Assets/Context, so it belongs with the cdroid layer, not the pure
// androidfw resource-format library.
//
// The low-level resolver it wraps (ResTable + obtainStyledAttributes) stays in
// androidfw/restable.h; the binary-format value types (Res_value, StyledAttr,
// ResXMLTree) stay in androidfw/resourcetypes.h.
//
#ifndef __TYPED_ARRAY_H__
#define __TYPED_ARRAY_H__

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <memory>
#include <androidfw/resourcetypes.h>  // Res_value, StyledAttr, ResXMLTree

namespace cdroid {

class ResTable;      // defined in androidfw/restable.h (reference member only)
class Resources;     // cdroid::Resources — the AOSP mResources holder (loadDrawable/...)
class Drawable;
class ColorStateList;

class TypedArray {
public:
    // Non-owning (StyledAttr* must outlive this TypedArray).
    TypedArray(const ResTable& table, const StyledAttr* vals, size_t count,
               const ResXMLTree* xmlSrc = nullptr, float density = 1.0f,
               const Resources* res = nullptr);
    // Owning (StyledAttr vector moved in; mVals points into mOwned).
    TypedArray(const ResTable& table, std::vector<StyledAttr>&& vals,
               const ResXMLTree* xmlSrc = nullptr, float density = 1.0f,
               const Resources* res = nullptr);
    size_t size() const { return mCount; }
    bool hasValue(size_t idx) const { return idx < mCount && mVals[idx].set; }
    bool hasValueOrEmpty(size_t idx) const;
    // AOSP TypedArray pattern: iterate only over SET indices (not all COUNT).
    size_t getIndexCount() const;
    size_t getIndex(size_t n) const;  // nth set index (0..getIndexCount()-1)

    // Typed value getters (low-level; aligned with Android.util.TypedArray).
    int32_t  getInt(size_t idx, int32_t def) const;
    int32_t  getInteger(size_t idx, int32_t def) const;  // Android alias of getInt
    bool     getBoolean(size_t idx, bool def) const;
    float    getFloat(size_t idx, float def) const;
    uint32_t getColor(size_t idx, uint32_t def) const;
    float    getDimension(size_t idx, float def) const;
    int32_t  getDimensionPixelOffset(size_t idx, int32_t def) const;
    int32_t  getDimensionPixelSize(size_t idx, int32_t def) const;
    int32_t  getLayoutDimension(size_t idx, int32_t def) const;
    float    getFraction(size_t idx, int base, int pbase, float def) const;
    uint32_t getResourceId(size_t idx, uint32_t def) const;
    std::string getString(size_t idx) const;
    std::string getText(size_t idx) const;     // alias of getString for now
    int       getType(size_t idx) const;        // Res_value dataType, or -1
    bool      peekValue(size_t idx, Res_value* out) const;
    // High-level resource access — delegate to the owning Resources (AOSP
    // AOSP TypedArray.getResources(): the owning Resources (for openRawResource,
    // getValue, DisplayMetrics — everything updateStateFromTypedArray needs).
    const Resources& getResources() const { return *mResources; }
    // TypedArray holds Resources mResources; getters call mResources.loadDrawable/
    // loadComplexColor). getDrawable returns a raw Drawable* (freshly new'd,
    // caller takes ownership). getColorStateList returns a shared_ptr (RefPtr):
    // ColorStateList is a shared/cached resource, so the returned shared_ptr
    // shares ownership with the loader cache.
    Drawable* getDrawable(size_t idx) const;
    std::shared_ptr<ColorStateList> getColorStateList(size_t idx) const;
private:
    bool get(size_t idx, Res_value* v) const {
        if (!hasValue(idx)) return false;
        *v = mVals[idx].value;
        return true;
    }
    // Resolve TYPE_REFERENCE/ATTRIBUTE/DYNAMIC_* to the referenced resource's
    // final value via the owning Resources (AOSP TypedArray resolves refs in
    // getValue). Non-reference values pass through unchanged.
    bool getResolved(size_t idx, Res_value* out) const;
    const ResTable&         mTable;
    std::vector<StyledAttr> mOwned;  // empty for non-owning mode
    const StyledAttr*       mVals;
    size_t                  mCount;
    const ResXMLTree*       mXml;
    float                   mDensity;
    Resources const*        mResources; // owning Resources (AOSP mResources); nullable
};

} // namespace cdroid
#endif // __TYPED_ARRAY_H__
