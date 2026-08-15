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
// TypedArray implementation. See core/typedarray.h for the layering rationale
// (consumer-side typed view, split out of androidfw). The low-level getters are
// mechanical Res_value decoders; getDrawable/getColorStateList reach back into
// cdroid::Assets via the opaque mContext pointer.
//
#include <core/typedarray.h>
#include <androidfw/restable.h>        // ResTable (complete def for mTable use)
#include <androidfw/typedvalue.h>      // TypedValue (getResolved reference resolution)
#include <core/resources.h>            // Resources (mResources: getDrawable/loadComplexColor/getString)
#include <drawable/colordrawable.h>    // ColorDrawable
#include <drawable/colorstatelist.h>   // ColorStateList
#include <cstring>

// Re-resolve a style-sourced TYPE_STRING resource path (e.g.
// "res/drawable/ic_menu.xml", "res/color/primary_text.xml",
// "res/drawable-xxhdpi/foo.png") into a typed resource reference
// "@<package>:<type>/<name>" so Assets loads it from whichever pak owns it.
// aapt2 stores style-bag drawable/color values as the file path (TYPE_STRING),
// not as a reference id, so the high-level getters must turn the path back into
// a ref. The owning package is resolved through the ResTable (package="" searches
// every loaded pak, so it matches the framework pak "android" or an app pak as
// appropriate), removing the previous hard-coded "@android:" assumption.
// Returns empty when the path isn't under "res/" or the name isn't found in the
// table; density/config qualifiers on the directory ("-xxhdpi") are stripped,
// matching how a resource ref is spelled.
// Reverse-resolve a style-sourced file path (e.g. "res/drawable/foo.xml") to its
// resource id via the arsc, so TypedArray getters can route through the ID path
// (Resources.getDrawable/loadComplexColor) instead of the legacy string Assets
// lookup. Returns 0 if the path isn't a known resource.
static int pathToResourceId(const cdroid::ResTable& table, const std::string& path) {
    if (path.compare(0, 4, "res/") != 0) return 0;
    size_t sl = path.find_last_of('/');
    if (sl == std::string::npos || sl <= 4) return 0;
    std::string type = path.substr(4, sl - 4);  // "drawable" / "color" / "drawable-xxhdpi"
    size_t dash = type.find('-');
    if (dash != std::string::npos) type = type.substr(0, dash);  // drop qualifiers
    size_t dot = path.find_last_of('.');
    std::string base = path.substr(sl + 1,
        (dot != std::string::npos && dot > sl) ? dot - sl - 1 : std::string::npos);
    if (type.empty() || base.empty()) return 0;
    // package="" -> search all packages (framework "android" + app).
    return (int)table.getIdentifier(base, type, "");
}

namespace cdroid {

// --- Constructors (out-of-line so typedarray.h need not include restable.h) ---

TypedArray::TypedArray(const ResTable& table, const StyledAttr* vals, size_t count,
                       const ResXMLTree* xmlSrc, float density, const Resources* res)
    : mTable(table), mVals(vals), mCount(count), mXml(xmlSrc),
      mDensity(density), mResources(res) {}

TypedArray::TypedArray(const ResTable& table, std::vector<StyledAttr>&& vals,
                       const ResXMLTree* xmlSrc, float density, const Resources* res)
    : mTable(table), mOwned(std::move(vals)),
      mVals(mOwned.data()), mCount(mOwned.size()),
      mXml(xmlSrc), mDensity(density), mResources(res) {}

// --- Low-level typed getters (Res_value decoders) ---

bool TypedArray::getResolved(size_t idx, Res_value* out) const {
    Res_value v;
    if (!get(idx, &v)) return false;
    if ((v.dataType == Res_value::TYPE_REFERENCE || v.dataType == Res_value::TYPE_ATTRIBUTE ||
         v.dataType == Res_value::TYPE_DYNAMIC_REFERENCE || v.dataType == Res_value::TYPE_DYNAMIC_ATTRIBUTE)
        && mResources) {
        TypedValue tv;
        if (!mResources->getValue((int)v.data, &tv, true)) return false;
        v.dataType = tv.type;
        v.data = tv.data;
    }
    *out = v;
    return true;
}

int32_t TypedArray::getInt(size_t idx, int32_t def) const {
    Res_value v; if (!getResolved(idx, &v)) return def;
    return (v.dataType == Res_value::TYPE_INT_DEC || v.dataType == Res_value::TYPE_INT_HEX) ? (int32_t)v.data : def;
}

bool TypedArray::getBoolean(size_t idx, bool def) const {
    Res_value v; if (!getResolved(idx, &v)) return def;
    return v.dataType == Res_value::TYPE_INT_BOOLEAN ? (v.data != 0) : def;
}

uint32_t TypedArray::getColor(size_t idx, uint32_t def) const {
    Res_value v; if (!get(idx, &v)) return def;
    if (v.dataType >= Res_value::TYPE_FIRST_COLOR_INT && v.dataType <= Res_value::TYPE_LAST_COLOR_INT)
        return v.data;
    // Reference (@color/foo): resolve the referenced color resource.
    if (v.dataType == Res_value::TYPE_REFERENCE || v.dataType == Res_value::TYPE_ATTRIBUTE ||
        v.dataType == Res_value::TYPE_DYNAMIC_REFERENCE || v.dataType == Res_value::TYPE_DYNAMIC_ATTRIBUTE) {
        if (mResources) {
            int c = mResources->getColor((int)v.data);
            return c;   // getColor returns the packed ARGB color
        }
        return def;
    }
    // Style-sourced color stored as the file path (TYPE_STRING): load the
    // ColorStateList and return its default color, matching AOSP getColor.
    if (v.dataType == Res_value::TYPE_STRING) {
        auto csl = getColorStateList(idx);
        if (csl) return csl->getDefaultColor();
        return def;
    }
    return def;
}

float TypedArray::getDimension(size_t idx, float def) const {
    Res_value v; if (!getResolved(idx, &v)) return def;
    return v.dataType == Res_value::TYPE_DIMENSION ? complexToFloat(v.data) : def;
}

int32_t TypedArray::getDimensionPixelSize(size_t idx, int32_t def) const {
    Res_value v; if (!getResolved(idx, &v)) return def;
    if (v.dataType != Res_value::TYPE_DIMENSION) return def;
    float mag = complexToFloat(v.data);
    int unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
    float px = (unit == Res_value::COMPLEX_UNIT_PX) ? mag : mag * mDensity;
    return (int32_t)(px + 0.5f);
}

uint32_t TypedArray::getResourceId(size_t idx, uint32_t def) const {
    // AOSP TypedArray.getResourceId: TYPE_NULL/TYPE_STRING → def; otherwise the
    // STYLE_RESOURCE_ID column — the id the value came from, kept even after
    // reference flattening (resolver records it in StyledAttr::resourceId).
    // The ids are the aapt2/arsc space View::getId() uses now that idgen is
    // retired (R.h is dumped from the arsc).
    if (!hasValue(idx)) return def;
    const Res_value& v = mVals[idx].value;
    if (v.dataType == Res_value::TYPE_NULL || v.dataType == Res_value::TYPE_STRING) return def;
    if (mVals[idx].resourceId != 0) return mVals[idx].resourceId;
    return def;
}

std::string TypedArray::getString(size_t idx) const {
    Res_value v; if (!get(idx, &v) || v.dataType != Res_value::TYPE_STRING) return "";
    const ResStringPool* pool = nullptr;
    if (mVals[idx].stringBlock == -2 && mXml) {
        pool = &mXml->getStrings();  // element-sourced: AXML's own pool
    } else if (mVals[idx].stringBlock >= 0 && mVals[idx].stringBlock < (ssize_t)0 /*placeholder*/) {
        // style-sourced: owning arsc header pool (resolved via table on demand)
        // (kept simple: fall back to table's first pool)
        pool = &mTable.getStringPool();
    } else {
        pool = &mTable.getStringPool();
    }
    size_t len = 0;
    const char16_t* s = pool->stringAt(v.data, &len);
    std::string out;
    for (size_t i = 0; s && i < len; i++) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00) c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        if (c < 0x80) out += (char)c;
        else if (c < 0x800) { out += (char)(0xC0 | (c >> 6)); out += (char)(0x80 | (c & 0x3F)); }
        else { out += (char)(0xE0 | (c >> 12)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
    }
    return out;
}

// --- Android-aligned getters (Phase 2) ---

int32_t TypedArray::getInteger(size_t idx, int32_t def) const {
    return getInt(idx, def);
}

bool TypedArray::hasValueOrEmpty(size_t idx) const {
    if (!hasValue(idx)) return false;
    Res_value v = mVals[idx].value;
    // @empty is represented as TYPE_REFERENCE with data == 0.
    return !(v.dataType == Res_value::TYPE_REFERENCE && v.data == 0);
}

float TypedArray::getFloat(size_t idx, float def) const {
    Res_value v; if (!getResolved(idx, &v)) return def;
    if (v.dataType == Res_value::TYPE_FLOAT) {
        float f; memcpy(&f, &v.data, sizeof(f)); return f;
    }
    if (v.dataType == Res_value::TYPE_INT_DEC || v.dataType == Res_value::TYPE_INT_HEX)
        return (float)(int32_t)v.data;
    if (v.dataType == Res_value::TYPE_DIMENSION)
        return complexToFloat(v.data);
    return def;
}

int32_t TypedArray::getDimensionPixelOffset(size_t idx, int32_t def) const {
    Res_value v; if (!getResolved(idx, &v)) return def;
    if (v.dataType != Res_value::TYPE_DIMENSION) return def;
    float mag = complexToFloat(v.data);
    int unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
    float px = (unit == Res_value::COMPLEX_UNIT_PX) ? mag : mag * mDensity;
    return (int32_t)px;  // truncate (offset), vs round (size)
}

int32_t TypedArray::getLayoutDimension(size_t idx, int32_t def) const {
    Res_value v; if (!get(idx, &v)) return def;
    // MATCH_PARENT(-1) / WRAP_CONTENT(-2) are passed through as-is.
    if (v.dataType == Res_value::TYPE_INT_DEC &&
        ((int32_t)v.data < 0)) return (int32_t)v.data;
    return getDimensionPixelSize(idx, def);
}

float TypedArray::getFraction(size_t idx, int base, int pbase, float def) const {
    Res_value v; if (!getResolved(idx, &v)) return def;
    if (v.dataType != Res_value::TYPE_FRACTION) return def;
    float f = complexToFloat(v.data);
    int unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
    return (unit == Res_value::COMPLEX_UNIT_FRACTION_PARENT) ? f * pbase : f * base;
}

std::string TypedArray::getText(size_t idx) const {
    Res_value v;
    if (!get(idx, &v)) return "";
    // AOSP TypedArray.getText resolves @string/foo references to their value
    // (TypedValue.coerceToString). getString only handles TYPE_STRING, so resolve
    // TYPE_REFERENCE / TYPE_DYNAMIC_REFERENCE through the owning Resources.
    if (v.dataType == Res_value::TYPE_REFERENCE ||
        v.dataType == Res_value::TYPE_DYNAMIC_REFERENCE) {
        if (mResources) return mResources->getString((int)v.data);
        return "";
    }
    return getString(idx);
}

int TypedArray::getType(size_t idx) const {
    if (!hasValue(idx)) return -1;
    return mVals[idx].value.dataType;
}

// AOSP TypedArray.peekValue(int): expose the typed value as a TypedValue
// (android.util container); the raw Res_value stays an androidfw internal.
bool TypedArray::peekValue(size_t idx, TypedValue* out) const {
    Res_value v;
    if (!get(idx, &v)) return false;
    *out = TypedValue::from(v);
    return true;
}

size_t TypedArray::getIndexCount() const {
    size_t n = 0;
    for (size_t i = 0; i < mCount; i++) if (mVals[i].set) n++;
    return n;
}

size_t TypedArray::getIndex(size_t n) const {
    for (size_t i = 0; i < mCount; i++) {
        if (mVals[i].set) {
            if (n == 0) return i;
            n--;
        }
    }
    return (size_t)-1;  // out of range
}

// --- High-level resource access (reach into cdroid::Assets via mContext) ---

Drawable* TypedArray::getDrawable(size_t idx) const {
    if (!mResources) return nullptr;
    Res_value v;
    if (!get(idx, &v)) return nullptr;
    // Inline color → ColorDrawable directly (no resource id).
    if (v.dataType >= Res_value::TYPE_FIRST_COLOR_INT && v.dataType <= Res_value::TYPE_LAST_COLOR_INT)
        return new ColorDrawable(v.data);
    // Resolve the resource id, then load through the owning Resources (AOSP
    // TypedArray -> mResources.loadDrawable). References carry the id in v.data;
    // style-sourced file paths (TYPE_STRING) are reverse-resolved via the arsc.
    int id = 0;
    if (v.dataType == Res_value::TYPE_REFERENCE || v.dataType == Res_value::TYPE_ATTRIBUTE ||
        v.dataType == Res_value::TYPE_DYNAMIC_REFERENCE || v.dataType == Res_value::TYPE_DYNAMIC_ATTRIBUTE) {
        id = (int)v.data;
    } else if (v.dataType == Res_value::TYPE_STRING) {
        id = pathToResourceId(mTable, getString(idx));
    }
    if (id != 0) return mResources->getDrawable(id);
    return nullptr;
}

std::shared_ptr<ColorStateList> TypedArray::getColorStateList(size_t idx) const {
    if (!mResources) return nullptr;
    Res_value v;
    if (!get(idx, &v)) return nullptr;
    // Inline color → single-color ColorStateList (valueOf caches it).
    if (v.dataType >= Res_value::TYPE_FIRST_COLOR_INT && v.dataType <= Res_value::TYPE_LAST_COLOR_INT)
        return ColorStateList::valueOf(v.data);
    // Load through the owning Resources.loadComplexColor (AOSP TypedArray ->
    // mResources.loadComplexColor), preserving shared_ptr ownership so the cached
    // instance is shared with the loader cache.
    int id = 0;
    if (v.dataType == Res_value::TYPE_REFERENCE || v.dataType == Res_value::TYPE_ATTRIBUTE ||
        v.dataType == Res_value::TYPE_DYNAMIC_REFERENCE || v.dataType == Res_value::TYPE_DYNAMIC_ATTRIBUTE) {
        id = (int)v.data;
    } else if (v.dataType == Res_value::TYPE_STRING) {
        id = pathToResourceId(mTable, getString(idx));
    }
    if (id != 0)
        return std::dynamic_pointer_cast<ColorStateList>(mResources->loadComplexColor(id));
    return nullptr;
}

} // namespace cdroid
