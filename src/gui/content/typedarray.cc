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
#include <content/typedarray.h>
#include <content/androidfw/restable.h>        // ResTable (complete def for mTable use)
#include <content/typedvalue.h>      // TypedValue (getResolved reference resolution)
#include <content/resources.h>            // Resources (mResources: getDrawable/loadComplexColor/getString)
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

// androidfw glue: fill a TypedValue from a raw Res_value. AOSP does exactly
// this fill in the JNI layer (android.util.TypedValue itself has no factory);
// this is CDROID's single androidfw→core value seam.
static void fillTypedValue(const Res_value& rv, TypedValue* out) {
    out->type = rv.dataType;
    out->data = rv.data;
}

// --- Constructors (out-of-line so typedarray.h needs no androidfw header) ---
// The Theme view is stack-side at every call site, so a shared value snapshot
// is kept (AOSP TypedArray.mTheme — there GC keeps the theme alive).

TypedArray::TypedArray(const ResTable& table, const StyledAttr* vals, size_t count,
                       const ResXMLTree* xmlSrc, float density, const Resources* res,
                       const Resources::Theme* theme)
    : mTable(table), mVals(vals), mCount(count), mXml(xmlSrc),
      mDensity(density), mResources(res),
      mTheme(theme ? std::make_shared<Resources::Theme>(*theme) : nullptr) {}

TypedArray::TypedArray(const ResTable& table, std::vector<StyledAttr>&& vals,
                       const ResXMLTree* xmlSrc, float density, const Resources* res,
                       const Resources::Theme* theme)
    : mTable(table), mOwned(new std::vector<StyledAttr>(std::move(vals))),
      mVals(mOwned->data()), mCount(mOwned->size()),
      mXml(xmlSrc), mDensity(density), mResources(res),
      mTheme(theme ? std::make_shared<Resources::Theme>(*theme) : nullptr) {}

TypedArray::~TypedArray() {
    delete mOwned;
}

bool TypedArray::hasValue(size_t idx) const {
    // AOSP hasValue: entry present AND not TYPE_NULL (@empty reads false here).
    return idx < mCount && mVals[idx].set && mVals[idx].value.dataType != Res_value::TYPE_NULL;
}

// --- Low-level typed getters (TypedValue decoders; AOSP getValueAt shape) ---

bool TypedArray::get(size_t idx, TypedValue* out) const {
    if (!hasValue(idx)) return false;
    fillTypedValue(mVals[idx].value, out);
    return true;
}

bool TypedArray::getValue(size_t idx, TypedValue* out) const {
    return get(idx, out);
}

bool TypedArray::getResolved(size_t idx, TypedValue* out) const {
    TypedValue v;
    if (!get(idx, &v)) return false;
    // AOSP: TYPE_ATTRIBUTE (?attr/name) resolves through the theme — the theme
    // answer already follows references (resolveRefs=true).
    if ((v.type == TypedValue::TYPE_ATTRIBUTE || v.type == TypedValue::TYPE_DYNAMIC_ATTRIBUTE)
        && mTheme) {
        TypedValue tv;
        if (mTheme->resolveAttribute((int)v.data, &tv, true)) {
            v.type = tv.type; v.data = tv.data; v.resourceId = 0;
        } else {
            return false;
        }
    }
    if ((v.type == TypedValue::TYPE_REFERENCE ||
         v.type == TypedValue::TYPE_DYNAMIC_REFERENCE)
        && mResources) {
        TypedValue tv;
        if (!mResources->getValue((int)v.data, &tv, true)) return false;
        tv.resourceId = v.data;   // keep the source id (AOSP column semantics)
        v = tv;
    }
    *out = v;
    return true;
}

int32_t TypedArray::getInt(size_t idx, int32_t def) const {
    // AOSP getInt: FIRST_INT..LAST_INT (includes color ints) → raw data;
    // TYPE_FLOAT coerces through the value's string form — numerically (int)f.
    TypedValue v; if (!getResolved(idx, &v)) return def;
    if (v.type >= TypedValue::TYPE_FIRST_INT && v.type <= TypedValue::TYPE_LAST_INT)
        return (int32_t)v.data;
    if (v.type == TypedValue::TYPE_FLOAT) return (int32_t)v.getFloat();
    return def;
}

bool TypedArray::getBoolean(size_t idx, bool def) const {
    TypedValue v; if (!getResolved(idx, &v)) return def;
    return v.type == TypedValue::TYPE_INT_BOOLEAN ? (v.data != 0) : def;
}

uint32_t TypedArray::getColor(size_t idx, uint32_t def) const {
    TypedValue v; if (!get(idx, &v)) return def;
    if (v.type >= TypedValue::TYPE_FIRST_COLOR_INT && v.type <= TypedValue::TYPE_LAST_COLOR_INT)
        return v.data;
    // Reference (@color/foo): resolve the referenced color resource.
    if (v.type == TypedValue::TYPE_ATTRIBUTE || v.type == TypedValue::TYPE_DYNAMIC_ATTRIBUTE) {
        // AOSP: resolve ?attr through the theme first.
        if (mTheme) {
            TypedValue tv;
            if (mTheme->resolveAttribute((int)v.data, &tv, true)) {
                if (tv.type >= TypedValue::TYPE_FIRST_COLOR_INT && tv.type <= TypedValue::TYPE_LAST_COLOR_INT)
                    return tv.data;
                if (tv.resourceId != 0 && mResources) {
                    // The chain landed on a COLOR resource — often a
                    // color-selector file flattened to its file path. The
                    // resolver kept the last reference id (AOSP TypedValue.
                    // resourceId); load it themed (a CSL yields its default).
                    return mResources->getColor((int)tv.resourceId, mTheme.get());
                }
                v.type = tv.type; v.data = tv.data;
            } else {
                return def;
            }
        }
    }
    if (v.type == TypedValue::TYPE_REFERENCE || v.type == TypedValue::TYPE_DYNAMIC_REFERENCE) {
        if (mResources) {
            int c = mResources->getColor((int)v.data);
            return c;   // getColor returns the packed ARGB color
        }
        return def;
    }
    // Style-sourced color stored as the file path (TYPE_STRING): load the
    // ColorStateList and return its default color, matching AOSP getColor.
    if (v.type == TypedValue::TYPE_STRING) {
        auto csl = getColorStateList(idx);
        if (csl) return csl->getDefaultColor();
        return def;
    }
    return def;
}

float TypedArray::getDimension(size_t idx, float def) const {
    TypedValue v; if (!getResolved(idx, &v)) return def;
    return v.type == TypedValue::TYPE_DIMENSION ? TypedValue::complexToFloat(v.data) : def;
}

int32_t TypedArray::getDimensionPixelSize(size_t idx, int32_t def) const {
    TypedValue v; if (!getResolved(idx, &v)) return def;
    if (v.type != TypedValue::TYPE_DIMENSION) return def;
    float mag = TypedValue::complexToFloat(v.data);
    int unit = (v.data >> TypedValue::COMPLEX_UNIT_SHIFT) & TypedValue::COMPLEX_UNIT_MASK;
    float px = (unit == TypedValue::COMPLEX_UNIT_PX) ? mag : mag * mDensity;
    return (int32_t)(px + 0.5f);
}

uint32_t TypedArray::getResourceId(size_t idx, uint32_t def) const {
    // AOSP TypedArray.getResourceId: the id the value came from. AOSP resolves
    // theme attributes lazily (mThemeAttrs + Theme.resolveAttributeReference),
    // so a ?attr value yields the id of the resource it RESOLVES TO — e.g.
    // android:textAppearance="?attr/textAppearanceLargePopupMenu" must return
    // the @style id, not the attr's own id (the raw StyledAttr column holds
    // the attr id for element-set ?attrs; TextView read it as a style id and
    // the whole appearance chain died → menu text fell back to hard white).
    if (!hasValue(idx)) return def;
    TypedValue v;
    if (!get(idx, &v)) return def;
    if (v.type == Res_value::TYPE_ATTRIBUTE || v.type == Res_value::TYPE_DYNAMIC_ATTRIBUTE) {
        if (mTheme) {
            TypedValue tv;
            if (mTheme->resolveAttribute((int)v.data, &tv, /*resolveRefs*/false)
                    && tv.resourceId != 0)
                return tv.resourceId;
        }
        return def;
    }
    if (v.type == Res_value::TYPE_NULL) return def;
    if (v.type == Res_value::TYPE_REFERENCE || v.type == Res_value::TYPE_DYNAMIC_REFERENCE)
        return v.data;   // resolved final reference
    // Non-reference value that CAME from a reference (e.g. a style item
    // pointing at a file resource flattens to TYPE_STRING while the column
    // keeps the @animator/... id) — the StyledAttr column is exactly that.
    if (mVals[idx].resourceId != 0) return mVals[idx].resourceId;
    return def;
}

std::string TypedArray::getString(size_t idx) const {
    TypedValue v; if (!get(idx, &v)) return "";
    // AOSP's AssetManager.retrieveAttributes resolves @string/foo references
    // before the TypedValue reaches Java, so TypedArray.getString returns the
    // referenced value. CDROID's XmlBlock keeps the raw TYPE_REFERENCE, so
    // resolve it here (same as getText) — android:contentDescription="@string/…"
    // and every other string attr read via getString otherwise read as empty.
    if (v.type == TypedValue::TYPE_REFERENCE || v.type == TypedValue::TYPE_DYNAMIC_REFERENCE) {
        if (mResources) return mResources->getString((int)v.data);
        return "";
    }
    if (v.type != TypedValue::TYPE_STRING) return "";
    const char16_t* s = nullptr;
    size_t len = 0;
    if (mVals[idx].stringBlock == -2 && mXml) {
        // element-sourced: the AXML's own string pool
        s = mXml->getStrings().stringAt(v.data, &len);
    } else if (mVals[idx].stringBlock >= 0) {
        // style/theme-sourced: the owning arsc header's pool (multi-pak table —
        // the index is into THAT pool, not the first one).
        s = mTable.stringAtBlock(mVals[idx].stringBlock, v.data, &len);
    } else {
        s = mTable.getStringPool().stringAt(v.data, &len);
    }
    std::string out;
    for (size_t i = 0; s && i < len; i++) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00) c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        if (c < 0x80) out += (char)c;
        else if (c < 0x800) { out += (char)(0xC0 | (c >> 6)); out += (char)(0x80 | (c & 0x3F)); }
        else if (c < 0x10000) { out += (char)(0xE0 | (c >> 12)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
        else { out += (char)(0xF0 | (c >> 18)); out += (char)(0x80 | ((c >> 12) & 0x3F)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
    }
    return out;
}

// --- Android-aligned getters (Phase 2) ---

int32_t TypedArray::getInteger(size_t idx, int32_t def) const {
    return getInt(idx, def);
}

bool TypedArray::hasValueOrEmpty(size_t idx) const {
    // AOSP hasValueOrEmpty: hasValue, or a TYPE_NULL @empty (DATA_NULL_EMPTY).
    if (idx >= mCount || !mVals[idx].set) return false;
    const Res_value& v = mVals[idx].value;
    return v.dataType != Res_value::TYPE_NULL || v.data == Res_value::DATA_NULL_EMPTY;
}

std::string TypedArray::getNonResourceString(size_t idx) const {
    if (idx >= mCount || !mVals[idx].set) return "";
    if (mVals[idx].stringBlock != -2 || mXml == nullptr) return "";
    return getString(idx);
}

std::vector<std::string> TypedArray::getTextArray(size_t idx) const {
    const uint32_t id = getResourceId(idx, 0);
    if (id == 0 || !mResources) return {};
    return mResources->getStringArray((int)id);
}

Typeface* TypedArray::getFont(size_t idx) const {
    const uint32_t id = getResourceId(idx, 0);
    if (id == 0 || !mResources) return nullptr;
    return mResources->getFont((int)id);
}

float TypedArray::getFloat(size_t idx, float def) const {
    TypedValue v; if (!getResolved(idx, &v)) return def;
    if (v.type == TypedValue::TYPE_FLOAT) return v.getFloat();
    if (v.type == TypedValue::TYPE_INT_DEC || v.type == TypedValue::TYPE_INT_HEX)
        return (float)(int32_t)v.data;
    if (v.type == TypedValue::TYPE_DIMENSION)
        return TypedValue::complexToFloat(v.data);
    return def;
}

int32_t TypedArray::getDimensionPixelOffset(size_t idx, int32_t def) const {
    TypedValue v; if (!getResolved(idx, &v)) return def;
    if (v.type != TypedValue::TYPE_DIMENSION) return def;
    float mag = TypedValue::complexToFloat(v.data);
    int unit = (v.data >> TypedValue::COMPLEX_UNIT_SHIFT) & TypedValue::COMPLEX_UNIT_MASK;
    float px = (unit == TypedValue::COMPLEX_UNIT_PX) ? mag : mag * mDensity;
    return (int32_t)px;  // truncate (offset), vs round (size)
}

int32_t TypedArray::getLayoutDimension(size_t idx, int32_t def) const {
    TypedValue v; if (!get(idx, &v)) return def;
    // MATCH_PARENT(-1) / WRAP_CONTENT(-2) are passed through as-is.
    if (v.type == TypedValue::TYPE_INT_DEC && ((int32_t)v.data < 0)) return (int32_t)v.data;
    return getDimensionPixelSize(idx, def);
}

float TypedArray::getFraction(size_t idx, int base, int pbase, float def) const {
    TypedValue v; if (!getResolved(idx, &v)) return def;
    if (v.type != TypedValue::TYPE_FRACTION) return def;
    float f = TypedValue::complexToFloat(v.data);
    int unit = (v.data >> TypedValue::COMPLEX_UNIT_SHIFT) & TypedValue::COMPLEX_UNIT_MASK;
    return (unit == TypedValue::COMPLEX_UNIT_FRACTION_PARENT) ? f * pbase : f * base;
}

std::string TypedArray::getText(size_t idx) const {
    TypedValue v;
    if (!get(idx, &v)) return "";
    // AOSP TypedArray.getText resolves @string/foo references to their value
    // (TypedValue.coerceToString). getString only handles TYPE_STRING, so resolve
    // TYPE_REFERENCE / TYPE_DYNAMIC_REFERENCE through the owning Resources.
    if (v.type == TypedValue::TYPE_REFERENCE || v.type == TypedValue::TYPE_DYNAMIC_REFERENCE) {
        if (mResources) return mResources->getString((int)v.data);
        return "";
    }
    return getString(idx);
}

int TypedArray::getType(size_t idx) const {
    // AOSP getType: TYPE_NULL when the entry is unset.
    if (idx >= mCount || !mVals[idx].set) return TypedValue::TYPE_NULL;
    return mVals[idx].value.dataType;
}

// AOSP TypedArray.peekValue(int) / getValue(int, TypedValue): the typed value
// as a TypedValue (android.util container); the raw Res_value stays an
// androidfw internal, converted once at the StyledAttr boundary.
bool TypedArray::peekValue(size_t idx, TypedValue* out) const {
    return get(idx, out);
}

// AOSP extractThemeAttrs(): scan for TYPE_ATTRIBUTE (?attr) entries and
// return their attr ids (one slot per entry, 0 elsewhere) so a later Theme
// pass can re-resolve them (Drawable.applyTheme). AOSP also nulls the scanned
// entries so subsequent typed getters return defaults; CDROID resolves ?attr
// lazily in getResolved/getters instead, so the entries stay live (see header).
std::vector<int> TypedArray::extractThemeAttrs() const {
    std::vector<int> noScrap;
    return extractThemeAttrs(noScrap);
}

std::vector<int> TypedArray::extractThemeAttrs(std::vector<int>& scrap) const {
    std::vector<int> attrs;
    const size_t N = length();
    for (size_t i = 0; i < N; i++) {
        if (!mVals[i].set) continue;
        const uint8_t type = mVals[i].value.dataType;
        if (type != Res_value::TYPE_ATTRIBUTE && type != Res_value::TYPE_DYNAMIC_ATTRIBUTE)
            continue;  // not an attribute, ignore
        const int attr = (int)mVals[i].value.data;
        if (attr == 0) continue;  // useless data, ignore
        // Ensure we have a usable attribute array (AOSP reuses scrap when
        // it matches length(), else allocates a fresh zeroed one).
        if (attrs.empty()) {
            if (scrap.size() == N) {
                scrap.assign(N, 0);
                attrs = std::move(scrap);
            } else {
                attrs.assign(N, 0);
            }
        }
        attrs[i] = attr;
    }
    return attrs;
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
    TypedValue v;
    if (!get(idx, &v)) return nullptr;
    // Inline color → ColorDrawable directly (no resource id).
    if (v.type >= TypedValue::TYPE_FIRST_COLOR_INT && v.type <= TypedValue::TYPE_LAST_COLOR_INT)
        return new ColorDrawable(v.data);
    // Resolve the resource id, then load through the owning Resources (AOSP
    // TypedArray -> mResources.loadDrawable). References carry the id in v.data;
    // style-sourced file paths (TYPE_STRING) are reverse-resolved via the arsc.
    // AOSP: ?attr resolves through the theme first (may land on a color).
    if ((v.type == TypedValue::TYPE_ATTRIBUTE || v.type == TypedValue::TYPE_DYNAMIC_ATTRIBUTE) && mTheme) {
        // AOSP loadDrawable(themeAttr): resolveRefs=false first — a theme
        // drawable lands on a RESOURCE whose id must reach getDrawable (with
        // the theme for nested ?attr); resolveRefs=true flattens to the
        // file-path string whose data is a pool index, not an id (the black
        // popup: background attr resolved to nothing → null → black).
        TypedValue tv;
        if (mTheme->resolveAttribute((int)v.data, &tv, /*resolveRefs*/false)) {
            if (tv.type >= TypedValue::TYPE_FIRST_COLOR_INT && tv.type <= TypedValue::TYPE_LAST_COLOR_INT)
                return new ColorDrawable(tv.data);
            if (tv.type == TypedValue::TYPE_REFERENCE || tv.type == TypedValue::TYPE_DYNAMIC_REFERENCE)
                return mResources->getDrawable((int)tv.data, mTheme.get());
        }
        if (mVals[idx].resourceId != 0)
            return mResources->getDrawable((int)mVals[idx].resourceId, mTheme.get());
        if (mTheme->resolveAttribute((int)v.data, &tv, true)) {
            v.type = tv.type; v.data = tv.data;
            if (v.type >= TypedValue::TYPE_FIRST_COLOR_INT && v.type <= TypedValue::TYPE_LAST_COLOR_INT)
                return new ColorDrawable(v.data);
        }
    }
    int id = 0;
    if (v.type == TypedValue::TYPE_REFERENCE || v.type == TypedValue::TYPE_DYNAMIC_REFERENCE) {
        id = (int)v.data;
    } else if (v.type == TypedValue::TYPE_STRING) {
        id = (int)mVals[idx].resourceId;   // column keeps the source ref id
        if (id == 0) id = pathToResourceId(mTable, getString(idx));
    }
    // AOSP TypedArray.getDrawable → mResources.getDrawable(id, mTheme): the
    // nested load resolves ?attr in the drawable/CSL XML against THIS theme.
    if (id != 0) return mResources->getDrawable(id, mTheme.get());
    return nullptr;
}

std::shared_ptr<ColorStateList> TypedArray::getColorStateList(size_t idx) const {
    if (!mResources) return nullptr;
    TypedValue v;
    if (!get(idx, &v)) return nullptr;
    // Inline color → single-color ColorStateList (valueOf caches it).
    if (v.type >= TypedValue::TYPE_FIRST_COLOR_INT && v.type <= TypedValue::TYPE_LAST_COLOR_INT)
        return ColorStateList::valueOf(v.data);
    // Load through the owning Resources.loadComplexColor (AOSP TypedArray ->
    // mResources.loadComplexColor), preserving shared_ptr ownership so the cached
    // instance is shared with the loader cache.
    // AOSP: ?attr resolves through the theme first (may land on a color).
    if ((v.type == TypedValue::TYPE_ATTRIBUTE || v.type == TypedValue::TYPE_DYNAMIC_ATTRIBUTE) && mTheme) {
        // AOSP loadColorStateList(themeAttr): resolve WITHOUT following the
        // reference chain — a theme color usually lands on a CSL resource whose
        // id must reach loadComplexColor (themed); only an inline color
        // short-circuits. resolveRefs=true flattens to the file-path string
        // and the id is lost (→ null → callers fall to hard colors).
        TypedValue tv;
        if (mTheme->resolveAttribute((int)v.data, &tv, /*resolveRefs*/false)) {
            if (tv.type >= TypedValue::TYPE_FIRST_COLOR_INT && tv.type <= TypedValue::TYPE_LAST_COLOR_INT)
                return ColorStateList::valueOf(tv.data);
            if (tv.resourceId != 0)
                return std::dynamic_pointer_cast<ColorStateList>(
                        mResources->loadComplexColor((int)tv.resourceId, mTheme.get()));
        }
        // Full-resolution chase: ?attr -> ?attr -> @color/x (possibly a
        // color-selector file). resolveRefs=true flattens the value to the
        // file path, but TypedValue.resourceId keeps the LAST reference id —
        // the themed loader still has something to load.
        if (mTheme->resolveAttribute((int)v.data, &tv, true)) {
            if (tv.type >= TypedValue::TYPE_FIRST_COLOR_INT && tv.type <= TypedValue::TYPE_LAST_COLOR_INT)
                return ColorStateList::valueOf(tv.data);
            if (tv.resourceId != 0)
                return std::dynamic_pointer_cast<ColorStateList>(
                        mResources->loadComplexColor((int)tv.resourceId, mTheme.get()));
        }
        if (mVals[idx].resourceId != 0) {
            // Column reference id — only valid when it names a COLOR resource;
            // for element-set ?attr it holds the ATTR id (not a resource), and
            // for a two-hop theme attr (?attr -> ?attr) resolve(false) yields
            // no id at all. Try it, but fall through instead of returning null
            // — the flattened resolve(true) below still carries the color.
            if ((mVals[idx].resourceId & 0xff0000u) != 0x010000u ||
                mResources->getValue((int)mVals[idx].resourceId, &tv, true)) {
                auto cc = mResources->loadComplexColor((int)mVals[idx].resourceId, mTheme.get());
                if (cc) return std::dynamic_pointer_cast<ColorStateList>(cc);
            }
        }
        if (mTheme->resolveAttribute((int)v.data, &tv, true)) {
            v.type = tv.type; v.data = tv.data;
            if (v.type >= TypedValue::TYPE_FIRST_COLOR_INT && v.type <= TypedValue::TYPE_LAST_COLOR_INT)
                return ColorStateList::valueOf(v.data);
        }
    }
    int id = 0;
    if (v.type == TypedValue::TYPE_REFERENCE || v.type == TypedValue::TYPE_DYNAMIC_REFERENCE) {
        id = (int)v.data;
    } else if (v.type == TypedValue::TYPE_STRING) {
        // A bag value flattened from a reference (style items and ?attr
        // resolutions land here as the file path): the resolver kept the
        // source reference id in the column — prefer it over re-parsing the
        // path (the string needs the owning pool block to fetch at all).
        id = (int)mVals[idx].resourceId;
        if (id == 0) id = pathToResourceId(mTable, getString(idx));
    }
    if (id != 0)
        return std::dynamic_pointer_cast<ColorStateList>(mResources->loadComplexColor(id, mTheme.get()));
    return nullptr;
}

} // namespace cdroid
