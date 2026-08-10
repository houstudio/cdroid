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
// ResTable: the resources.arsc table reader (port of the ResTable class from
// AOSP frameworks/base/libs/androidfw's ResourceTypes.{h,cpp}). Split out of
// resourcetypes.h so the resource-table engine has its own translation unit.
// Foundational binary-format types (Res_value, ResTable_config, ResStringPool,
// ResXMLTree, DynamicRefTable, the ResTable_* chunk structs, StyledAttr) remain
// in resourcetypes.h; this header only declares the ResTable engine + the
// obtainStyledAttributes resolver built on top of it.
//
#ifndef __CDROID_ANDROIDFW_RESTABLE_H__
#define __CDROID_ANDROIDFW_RESTABLE_H__

#include "resourcetypes.h"

namespace cdroid {

class ResTable {
public:
    ResTable();
    ~ResTable();

    // Load a resources.arsc blob (appAsLib=false). Multiple add() calls
    // accumulate into one table (multi-package); the blockIndex getResource
    // returns identifies the owning Header. Returns NO_ERROR / BAD_TYPE.
    status_t add(const void* data, size_t size, int32_t cookie = -1, bool copyData = false);
    // Load a blob as a shared library (appAsLib=true): a package built with the
    // app id (0x7f) is reassigned a runtime id and its self-references are
    // translated via its DynamicRefTable (the IME / add-on package case).
    status_t add(const void* data, size_t size, bool appAsLib, int32_t cookie, bool copyData);
    status_t getError() const;
    void uninit();

    // Set the request configuration used by getResource() (host-endian; copies).
    // Returns NO_ERROR (matches AOSP signature).
    status_t setParameters(const ResTable_config* params);
    void getParameters(ResTable_config* params) const;

    // The global value string pool of the FIRST loaded arsc (TYPE_STRING values
    // index into their owning arsc's pool; for the common single-arsc case this
    // is it). For multi-arsc, resolve via getResource.
    const ResStringPool& getStringPool() const;

    // Resolve resID to a value for the current parameters. Faithful to AOSP:
    // returns the owning asset's blockIndex (>= 0) on success, or a negative
    // error (BAD_INDEX / BAD_VALUE / NAME_NOT_FOUND). `density` (>0) overrides
    // mParams.density for this call; `outSpecFlags` receives the aggregated
    // TypeSpec flags; `outConfig` receives the matched variant. A complex (bag)
    // entry yields BAD_VALUE unless mayBeBag is set (it only silences the log).
    ssize_t getResource(uint32_t resID, Res_value* outValue, bool mayBeBag = false,
                        uint16_t density = 0, uint32_t* outSpecFlags = nullptr,
                        ResTable_config* outConfig = nullptr) const;

    // Follow TYPE_REFERENCE/ATTRIBUTE chains (bounded, count < 20 — matches
    // AOSP). Returns the final blockIndex (>= 0) or a negative error; on a
    // reference into a non-resolvable (e.g. style) target, returns the last good
    // blockIndex and leaves *value as the reference.
    ssize_t resolveReference(Res_value* value, ssize_t blockIndex,
                             uint32_t* outLastRef = nullptr,
                             uint32_t* inoutTypeSpecFlags = nullptr,
                             ResTable_config* outConfig = nullptr) const;

    // Convenience: resolve a string resource to its UTF-16 text (follows refs).
    // Returns nullptr if the resource is not a (string-resolvable) string.
    const char16_t* getResourceString(uint32_t resId, size_t* outLen) const;

    // Read a TYPE_STRING bag value: a style/array bag entry's value.data is an
    // index into the OWNING header's global string pool (the header that holds
    // this bag, given by getBag's outBlock = package headerIndex). getResourceString
    // resolves by resId, but bag string values are pool indices, not resIds — so
    // expose the (block, index) lookup directly.
    const char16_t* stringAtBlock(ssize_t block, uint32_t index, size_t* outLen) const;

    // Read a bag (complex) entry: fills outCount and returns a pointer to the
    // first ResTable_map (count entries), or nullptr if not a bag / not found.
    const ResTable_map* getBag(uint32_t resId, size_t* outCount,
                               ResTable_config* outConfig = nullptr,
                               ssize_t* outBlock = nullptr,
                               uint32_t* outSpecFlags = nullptr) const;

    // Look up a package's type/key name pools for diagnostics.
    bool getResourceName(uint32_t resId, std::string* outPackage,
                         std::string* outType, std::string* outKey) const;

    // --- AssetManager2 management surface ---
    // Resolve a resource name to its id (0 if not found). package=="" searches
    // all packages. e.g. getIdentifier("hello","string","com.example.verify").
    uint32_t getIdentifier(const std::string& name, const std::string& type,
                           const std::string& package) const;
    // Every distinct configuration present in the table (for introspection).
    void getConfigurations(std::vector<ResTable_config>* out) const;
    // Names of all loaded packages.
    std::vector<std::string> listPackageNames() const;

    // Enumerate EVERY existing resource in the table — the runtime equivalent of
    // dumping the R class from an arsc. For each present entry yields its id
    // (0xpptteeee), type name and key name. Returns the count.
    struct ResourceRef {
        uint32_t    resId;
        uint8_t     packageId;
        std::string type;
        std::string key;
    };
    size_t listAllResources(std::vector<ResourceRef>* out) const;

    // Style parent resource id of a bag (style) entry, or 0 if none / not a bag.
    uint32_t getBagParent(uint32_t resId, ResTable_config* outConfig = nullptr) const;

    // A Theme holds a set of applied styles; attributes resolve against it (with
    // parent inheritance + force/TYPE_NULL override semantics). Port of AOSP
    // ResTable::Theme, minus the per-type bag_set locking cache (a plain map is
    // semantically equivalent). applyStyle follows the style's parent chain so a
    // child style overrides its parent.
    class Theme {
    public:
        explicit Theme(const ResTable& table);
        ~Theme();

        const ResTable& getResTable() const { return mTable; }

        status_t applyStyle(uint32_t resID, bool force = false);
        status_t setTo(const Theme& other);
        status_t clear();

        // Retrieve a themed attribute. Returns the owning-asset blockIndex (>= 0)
        // and fills outValue, or a negative error if not set. Does NOT follow
        // references — call resolveAttributeReference() for that.
        ssize_t getAttribute(uint32_t resID, Res_value* outValue,
                             uint32_t* outTypeSpecFlags = nullptr) const;

        // AOSP Resources.Theme.resolveAttribute(resid, outValue, resolveRefs):
        // resolve a single attribute against this theme. Returns false if unset;
        // when resolveRefs is true, REFERENCE/ATTRIBUTE chains are followed.
        bool resolveAttribute(uint32_t resID, Res_value* outValue, bool resolveRefs) const;

        // Like ResTable::resolveReference, but TYPE_ATTRIBUTE is resolved via
        // this theme (getAttribute) rather than the table.
        ssize_t resolveAttributeReference(Res_value* inOutValue, ssize_t blockIndex,
                                         uint32_t* outLastRef = nullptr,
                                         uint32_t* inoutTypeSpecFlags = nullptr,
                                         ResTable_config* inoutConfig = nullptr) const;

        // Bit mask of CONFIG_* changes that would impact this theme (so it must
        // be rebuilt when the configuration changes).
        uint32_t getChangingConfigurations() const;

    private:
        const ResTable& mTable;
        struct ThemedItem {
            Res_value value;
            ssize_t   stringBlock;   // owning header index (for string values)
            uint32_t  typeSpecFlags;
            bool      set;
        };
        std::map<uint32_t, ThemedItem> mEntries;  // attr resID -> item
        uint32_t mTypeSpecFlags = 0;
        status_t applyStyleChain(uint32_t resID, bool force, int depth);
    };

private:
    // One loaded resources.arsc blob (AOSP Header). Owns its global value string
    // pool — TYPE_STRING values index into the pool of the arsc they came from.
    struct Header {
        int32_t              index = 0;
        int32_t              cookie = -1;
        const uint8_t*       data = nullptr;
        size_t               size = 0;
        const uint8_t*       dataEnd = nullptr;
        void*                ownedData = nullptr;  // malloc'd copy when copyData
        ~Header() { free(ownedData); }
        Header() = default;
        Header(Header&& o) noexcept : index(o.index), cookie(o.cookie),
            data(o.data), size(o.size), dataEnd(o.dataEnd), ownedData(o.ownedData),
            values(std::move(o.values)) { o.ownedData = nullptr; o.data = nullptr; }
        Header& operator=(Header&& o) noexcept {
            if (this != &o) {
                free(ownedData);
                index = o.index; cookie = o.cookie; data = o.data; size = o.size;
                dataEnd = o.dataEnd; ownedData = o.ownedData; values = std::move(o.values);
                o.ownedData = nullptr; o.data = nullptr;
            }
            return *this;
        }
        ResStringPool        values;      // this arsc's global value string pool
    };
    // A config variant group for one type id (from a TypeSpec + its type chunks).
    struct TypeGroup {
        uint32_t             entryCount = 0;
        const uint32_t*      specFlags = nullptr;
        std::vector<const ResTable_type*> configs;
    };
    // One ResTable_package chunk (AOSP Package). Belongs to a Header + a
    // (runtime) id; carries its DynamicRefTable for build->runtime id translation.
    struct Package {
        size_t               headerIndex = 0;  // index into mHeaders (NOT a pointer:
                                               // mHeaders reallocs on each add())
        uint8_t              id = 0;          // RUNTIME package id
        uint8_t              buildId = 0;     // id as declared in the arsc
        bool                 isDynamic = false;
        std::string          name;            // package name (UTF-8)
        DynamicRefTable      dynamicRefTable;
        ResStringPool        typeStrings;
        ResStringPool        keyStrings;
        std::vector<TypeGroup> types;         // indexed [typeId - 1]
    };

    // addInternal mirrors AOSP: appAsLib makes an app-id (0x7f) package be
    // reassigned a runtime id (shared-library / IME / add-on package case).
    status_t addInternal(const void* data, size_t size, bool appAsLib,
                         int32_t cookie, bool copyData);
    status_t parsePackage(const ResTable_package* pkg, Header* header,
                          bool appAsLib, uint8_t* outRuntimeId);
    Package* packageForId(uint32_t pkgId);
    const Package* packageForId(uint32_t pkgId) const;
    // Core selection (mirrors AOSP getEntry): over configs of (typeId) pick the
    // one matching `desired` that isBetterThan the rest. Fills outEntry.
    const ResTable_entry* getBestEntry(const Package& pkg, uint8_t typeId, uint32_t entryId,
                                       const ResTable_config& desired,
                                       ResTable_config* outConfig,
                                       uint32_t* outSpecFlags) const;
    // First present entry for (typeId, entryId) across any config (for enumeration).
    const ResTable_entry* anyEntry(const Package& pkg, uint8_t typeId, uint32_t entryId) const;
    // Decode a string-pool entry to UTF-8 regardless of pool encoding.
    static std::string poolString(const ResStringPool& pool, uint32_t idx);
    // Find a string index in a pool by content (linear).
    static int32_t poolIndexOf(const ResStringPool& pool, const std::string& needle);

    status_t              mError;
    int32_t               mCookie;
    std::vector<Header>   mHeaders;
    std::vector<Package>  mPackages;  // Package holds ResStringPool with raw
                                      // pointers into mHeaders[x].owned; mHeaders
                                      // MUST NOT reallocate after Packages are parsed
                                      // (use reserve or unique_ptr if multi-add needed)
    std::vector<int>      mPackageMap;     // runtime package id -> index+1 into mPackages
    uint8_t               mNextPackageId;  // next runtime id for dynamic packages (starts at 2)
    ResTable_config       mParams;         // current request config (host-endian)
};

// ---------------------------------------------------------------------------
// Full attribute resolution (AssetManager.applyStyle merge) + TypedArray.
// obtainStyledAttributes resolves each attr in a styleable set through the
// Android priority chain: the XML element's own value, then the element's
// style=, then defStyleAttr (via the theme), then defStyleRes, then the theme's
// direct value. The result is consumed by cdroid::TypedArray (core/typedarray.h).
// ---------------------------------------------------------------------------

// One resolved attribute. stringBlock is the owning header index for TYPE_STRING
// values sourced from a style/theme (element-sourced strings use the AXML pool).
// (Declared in resourcetypes.h as part of the foundational types.)

void obtainStyledAttributes(const ResXMLTree& xml, const ResTable& table,
                            const ResTable::Theme* theme,
                            const uint32_t* attrs, size_t attrCount,
                            uint32_t defStyleAttr, uint32_t defStyleRes,
                            StyledAttr* out);

// Theme-only obtainStyledAttributes (no XML element): resolves attrs purely
// against the style/theme fallback chain (defStyleRes -> defStyleAttr via theme
// -> theme base values). The AOSP Resources.Theme.obtainStyledAttributes
// counterpart (AttributeSet == null): Theme.obtainStyledAttributes(attrs) maps
// to (defStyleAttr=0, defStyleRes=0); Theme.obtainStyledAttributes(resId, attrs)
// maps to (defStyleAttr=0, defStyleRes=resId).
void obtainStyledAttributes(const ResTable& table, const ResTable::Theme* theme,
                            const uint32_t* attrs, size_t attrCount,
                            uint32_t defStyleAttr, uint32_t defStyleRes,
                            StyledAttr* out);

} // namespace cdroid
#endif // __CDROID_ANDROIDFW_RESTABLE_H__
