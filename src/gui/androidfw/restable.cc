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
// ResTable engine implementation (resources.arsc reader), split out of
// resourcetypes.cc. See restable.h for the layering rationale. Depends only on
// the foundational binary-format types in resourcetypes.h (pulled in via restable.h).
//
#include "restable.h"
#include <porting/cdlog.h>
#include <cstring>      // memset/memcpy/memcmp
#include <algorithm>    // std::lower_bound (sparse type entries)

namespace cdroid {

ResTable::ResTable()
    : mError(NO_INIT), mCookie(-1), mNextPackageId(2) {
    memset(&mParams, 0, sizeof(mParams));
    mParams.size = sizeof(ResTable_config);
    mPackageMap.assign(256, 0);
}

ResTable::~ResTable() {
    uninit();
}

void ResTable::uninit() {
    mError = NO_INIT;
    for (Header& h : mHeaders) h.values.uninit();
    mHeaders.clear();
    mPackages.clear();
    mPackageMap.assign(256, 0);
    mNextPackageId = 2;
}

status_t ResTable::add(const void* data, size_t size, int32_t cookie, bool copyData) {
    return addInternal(data, size, /*appAsLib*/ false, cookie, copyData);
}

status_t ResTable::add(const void* data, size_t size, bool appAsLib,
                       int32_t cookie, bool copyData) {
    return addInternal(data, size, appAsLib, cookie, copyData);
}

status_t ResTable::getError() const {
    return mError;
}

status_t ResTable::addInternal(const void* data, size_t size, bool appAsLib,
                               int32_t cookie, bool copyData) {
    mCookie = cookie;
    if (!data || !size) return (mError = BAD_TYPE);

    // Each add() is one loaded arsc = one Header (owns its global value pool).
    // Reserve to prevent reallocation: Package's ResStringPool stores raw
    // pointers into Header's owned data; if mHeaders reallocates, those
    // pointers dangle. 8 is plenty for any realistic multi-pak scenario.
    if (mHeaders.capacity() == mHeaders.size()) mHeaders.reserve(mHeaders.size() + 8);
    size_t hdrIdx = mHeaders.size();
    mHeaders.push_back(Header());
    Header* hdr = &mHeaders[hdrIdx];
    hdr->index = (int32_t)hdrIdx;
    hdr->cookie = cookie;
    hdr->size = size;
    if (copyData) {
        hdr->ownedData = malloc(size);
        if (!hdr->ownedData) return (mError = NO_MEMORY);
        memcpy(hdr->ownedData, data, size);
        hdr->data = (const uint8_t*)hdr->ownedData;
    } else {
        hdr->data = (const uint8_t*)data;
    }
    const uint8_t* base = hdr->data;

    const ResTable_header* rth = (const ResTable_header*)base;
    if (ResChunk_header::validate_chunk(&rth->header, sizeof(ResTable_header), base + size, "ResTable_header") != NO_ERROR) {
        mHeaders.pop_back();
        return (mError = BAD_TYPE);
    }
    hdr->dataEnd = base + dtohl(rth->header.size);
    const uint8_t* dataEnd = hdr->dataEnd;

    // Walk the top-level chunks: a global value string pool + package(s).
    const ResChunk_header* chunk =
        (const ResChunk_header*)(base + dtohs(rth->header.headerSize));
    while (((const uint8_t*)chunk) <= (dataEnd - sizeof(ResChunk_header)) &&
           ((const uint8_t*)chunk) <= (dataEnd - dtohl(chunk->size))) {
        if (ResChunk_header::validate_chunk(chunk, sizeof(ResChunk_header), dataEnd, "ResTable") != NO_ERROR) {
            return (mError = BAD_TYPE);
        }
        const uint16_t type = dtohs(chunk->type);
        const uint32_t csize = dtohl(chunk->size);
        if (type == RES_STRING_POOL_TYPE) {
            if (hdr->values.setTo(chunk, csize) != NO_ERROR) return (mError = BAD_TYPE);
        } else if (type == RES_TABLE_PACKAGE_TYPE) {
            uint8_t runtimeId = 0;
            status_t err = parsePackage((const ResTable_package*)chunk, hdr, appAsLib, &runtimeId);
            if (err != NO_ERROR) return (mError = err);
        }
        chunk = (const ResChunk_header*)(((const uint8_t*)chunk) + csize);
    }

    mError = mHeaders.empty() ? NO_INIT : mHeaders[0].values.getError();
    return mError;
}

status_t ResTable::parsePackage(const ResTable_package* pkg, Header* header,
                                bool appAsLib, uint8_t* outRuntimeId) {
    const uint8_t* base = (const uint8_t*)pkg;
    const uint8_t* dataEnd = header->dataEnd;
    if (ResChunk_header::validate_chunk(&pkg->header, sizeof(ResTable_package) - sizeof(pkg->typeIdOffset),
                       dataEnd, "ResTable_package") != NO_ERROR) {
        return BAD_TYPE;
    }
    const uint32_t pkgSize = dtohl(pkg->header.size);
    const uint32_t typeStringsOff = dtohl(pkg->typeStrings);
    const uint32_t keyStringsOff = dtohl(pkg->keyStrings);
    if (typeStringsOff >= pkgSize || (typeStringsOff & 0x3) ||
        keyStringsOff >= pkgSize || (keyStringsOff & 0x3)) {
        return BAD_TYPE;
    }
    const uint32_t rawId = dtohl(pkg->id);
    if (rawId == 0 || rawId >= 256) return BAD_TYPE;  // dynamic/staged ids unsupported
    const uint8_t buildId = (uint8_t)rawId;

    // Runtime id assignment (AOSP addInternal): an app-id (0x7f) package loaded as
    // a shared library is reassigned a fresh runtime id; its self-references are
    // translated via its DynamicRefTable. Other packages keep their build-time id.
    const bool isDynamic = (buildId == APP_PACKAGE_ID && appAsLib);
    const uint8_t runtimeId = isDynamic ? mNextPackageId++ : buildId;
    if (mNextPackageId >= APP_PACKAGE_ID) mNextPackageId = 2;  // never hand out the app id
    *outRuntimeId = runtimeId;

    Package package;
    package.headerIndex = header->index;  // store index, not pointer (mHeaders reallocs)
    package.id = runtimeId;
    package.buildId = buildId;
    package.isDynamic = isDynamic;
    package.dynamicRefTable = DynamicRefTable(runtimeId, appAsLib);
    // Package name (UTF-16 -> UTF-8; ASCII/BMP names only in practice).
    for (int i = 0; i < 128 && pkg->name[i] != 0; i++) {
        uint32_t c = pkg->name[i];
        if (c < 0x80) package.name += (char)c;
        else if (c < 0x800) { package.name += (char)(0xC0|(c>>6)); package.name += (char)(0x80|(c&0x3F)); }
        else { package.name += (char)(0xE0|(c>>12)); package.name += (char)(0x80|((c>>6)&0x3F)); package.name += (char)(0x80|(c&0x3F)); }
    }
    if (package.typeStrings.setTo(base + typeStringsOff, dataEnd - (base + typeStringsOff)) != NO_ERROR) {
        return BAD_TYPE;
    }
    if (package.keyStrings.setTo(base + keyStringsOff, dataEnd - (base + keyStringsOff)) != NO_ERROR) {
        return BAD_TYPE;
    }

    // Iterate the package's chunks: TypeSpec (creates a TypeGroup) + Type
    // (appends a config variant to the most recent TypeGroup for that id).
    const ResChunk_header* chunk =
        (const ResChunk_header*)(base + dtohs(pkg->header.headerSize));
    const uint8_t* endPos = base + pkgSize;
    while (((const uint8_t*)chunk) <= (endPos - sizeof(ResChunk_header)) &&
           ((const uint8_t*)chunk) <= (endPos - dtohl(chunk->size))) {
        if (ResChunk_header::validate_chunk(chunk, sizeof(ResChunk_header), endPos, "ResTable_package chunk") != NO_ERROR) {
            return BAD_TYPE;
        }
        const uint16_t ctype = dtohs(chunk->type);
        const uint32_t csize = dtohl(chunk->size);

        if (ctype == RES_TABLE_TYPE_SPEC_TYPE) {
            const ResTable_typeSpec* ts = (const ResTable_typeSpec*)chunk;
            if (ResChunk_header::validate_chunk(&ts->header, sizeof(ResTable_typeSpec), endPos, "ResTable_typeSpec") != NO_ERROR) {
                return BAD_TYPE;
            }
            const uint32_t entryCount = dtohl(ts->entryCount);
            if (dtohs(ts->header.headerSize) + sizeof(uint32_t) * entryCount > csize) {
                return BAD_TYPE;
            }
            if (entryCount > 0) {
                uint8_t typeIndex = ts->id - 1;
                if (typeIndex >= package.types.size()) package.types.resize(typeIndex + 1);
                package.types[typeIndex].entryCount = entryCount;
                package.types[typeIndex].specFlags = (const uint32_t*)(
                    ((const uint8_t*)ts) + dtohs(ts->header.headerSize));
            }
        } else if (ctype == RES_TABLE_TYPE_TYPE) {
            const ResTable_type* tp = (const ResTable_type*)chunk;
            if (ResChunk_header::validate_chunk(&tp->header, sizeof(ResTable_type) - sizeof(ResTable_config) + 4,
                               endPos, "ResTable_type") != NO_ERROR) {
                return BAD_TYPE;
            }
            const uint32_t entryCount = dtohl(tp->entryCount);
            if (dtohs(tp->header.headerSize) + sizeof(uint32_t) * entryCount > csize) {
                return BAD_TYPE;
            }
            if (entryCount > 0) {
                uint8_t typeIndex = tp->id - 1;
                if (typeIndex >= package.types.size()) {
                    return BAD_TYPE;  // No TypeSpec seen first
                }
                package.types[typeIndex].configs.push_back(tp);
            }
        }
        chunk = (const ResChunk_header*)(((const uint8_t*)chunk) + csize);
    }

    // Register the package under its RUNTIME id; merge if an entry already exists.
    if (mPackageMap[runtimeId] == 0) {
        mPackageMap[runtimeId] = (int)mPackages.size() + 1;
        mPackages.push_back(std::move(package));
    } else {
        Package& dst = mPackages[mPackageMap[runtimeId] - 1];
        for (size_t t = 0; t < package.types.size(); t++) {
            if (t >= dst.types.size()) dst.types.resize(t + 1);
            if (package.types[t].entryCount) dst.types[t].entryCount = package.types[t].entryCount;
            if (package.types[t].specFlags) dst.types[t].specFlags = package.types[t].specFlags;
            for (const ResTable_type* c : package.types[t].configs) dst.types[t].configs.push_back(c);
        }
    }
    return NO_ERROR;
}

const ResStringPool& ResTable::getStringPool() const {
    static const ResStringPool kEmpty;
    return mHeaders.empty() ? kEmpty : mHeaders[0].values;
}

status_t ResTable::setParameters(const ResTable_config* params) {
    if (params) {
        mParams = *params;
        mParams.size = sizeof(ResTable_config);
    } else {
        memset(&mParams, 0, sizeof(mParams));
        mParams.size = sizeof(ResTable_config);
    }
    return NO_ERROR;
}

void ResTable::getParameters(ResTable_config* params) const {
    if (params) *params = mParams;
}

const ResTable::Package* ResTable::packageForId(uint32_t pkgId) const {
    int idx = (pkgId < mPackageMap.size()) ? mPackageMap[pkgId] : 0;
    return idx ? &mPackages[idx - 1] : nullptr;
}
ResTable::Package* ResTable::packageForId(uint32_t pkgId) {
    int idx = (pkgId < mPackageMap.size()) ? mPackageMap[pkgId] : 0;
    return idx ? &mPackages[idx - 1] : nullptr;
}

// Core selection (mirrors AOSP ResTable::getEntry). Iterate the config variants
// of typeId, drop those that don't match() `desired` or whose entry is absent
// (NO_ENTRY), keep the best (isBetterThan vs `desired`). Aggregate the TypeSpec
// flags for the entry into *outSpecFlags. Returns the entry pointer or nullptr.
const ResTable_entry* ResTable::getBestEntry(const Package& pkg, uint8_t typeId,
                                             uint32_t entryId,
                                             const ResTable_config& desired,
                                             ResTable_config* outConfig,
                                             uint32_t* outSpecFlags) const {
    if (typeId == 0 || (size_t)(typeId - 1) >= pkg.types.size()) return nullptr;
    const TypeGroup& group = pkg.types[typeId - 1];
    if (group.configs.empty()) return nullptr;
    if (entryId >= group.entryCount) return nullptr;

    if (outSpecFlags && group.specFlags) *outSpecFlags |= dtohl(group.specFlags[entryId]);

    const ResTable_type* bestType = nullptr;
    uint32_t bestOffset = ResTable_type::NO_ENTRY;
    ResTable_config bestConfig;
    memset(&bestConfig, 0, sizeof(bestConfig));

    for (const ResTable_type* tp : group.configs) {
        ResTable_config thisConfig;
        thisConfig.copyFromDtoH(tp->config);
        if (!thisConfig.match(desired)) continue;

        const uint32_t* eindex = (const uint32_t*)(((const uint8_t*)tp) + dtohs(tp->header.headerSize));
        uint32_t thisOffset;
        if (tp->flags & ResTable_type::FLAG_SPARSE) {
            const ResTable_sparseTypeEntry* sparse =
                (const ResTable_sparseTypeEntry*)eindex;
            const ResTable_sparseTypeEntry* end = sparse + dtohl(tp->entryCount);
            const ResTable_sparseTypeEntry* lo = std::lower_bound(
                sparse, end, (uint16_t)entryId,
                [](const ResTable_sparseTypeEntry& a, uint16_t b) { return dtohs(a.idx) < b; });
            if (lo == end || dtohs(lo->idx) != entryId) continue;
            thisOffset = dtohs(lo->offset) * 4u;
        } else {
            if (entryId >= dtohl(tp->entryCount)) continue;
            thisOffset = dtohl(eindex[entryId]);
        }
        if (thisOffset == ResTable_type::NO_ENTRY) continue;

        if (bestType != nullptr && !thisConfig.isBetterThan(bestConfig, &desired)) continue;

        bestType = tp;
        bestOffset = thisOffset;
        bestConfig = thisConfig;
    }

    if (bestType == nullptr) return nullptr;

    bestOffset += dtohl(bestType->entriesStart);
    if (bestOffset > dtohl(bestType->header.size) - sizeof(ResTable_entry)) return nullptr;
    if ((bestOffset & 0x3) != 0) return nullptr;

    const ResTable_entry* entry = (const ResTable_entry*)(((const uint8_t*)bestType) + bestOffset);
    if (dtohs(entry->size) < sizeof(ResTable_entry)) return nullptr;
    if (outConfig) *outConfig = bestConfig;
    return entry;
}

// Faithful to AOSP ResTable::getResource. `density`>0 overrides mParams.density;
// a complex (bag) entry returns BAD_VALUE (mayBeBag only silences the log);
// outSpecFlags gets the aggregated TypeSpec flags; outConfig the matched variant.
// Returns the owning asset blockIndex (>= 0, always 0 in the single-asset model)
// or a negative error.
ssize_t ResTable::getResource(uint32_t resID, Res_value* outValue, bool mayBeBag,
                              uint16_t density, uint32_t* outSpecFlags,
                              ResTable_config* outConfig) const {
    if (mError != NO_ERROR) return mError;
    uint8_t pkgId  = (uint8_t)((resID >> 24) & 0xff);
    uint8_t typeId = (uint8_t)((resID >> 16) & 0xff);
    uint32_t entryId = resID & 0xffff;
    const Package* pkg = packageForId(pkgId);
    if (!pkg) return BAD_INDEX;
    if (typeId == 0) return BAD_INDEX;

    ResTable_config desired = mParams;
    if (density > 0) desired.density = density;

    uint32_t specFlags = 0;
    const ResTable_entry* entry = getBestEntry(*pkg, typeId, entryId, desired, outConfig,
                                               outSpecFlags ? &specFlags : nullptr);
    if (!entry) return NAME_NOT_FOUND;

    if (dtohs(entry->flags) & ResTable_entry::FLAG_COMPLEX) {
        if (!mayBeBag) {
            LOGW("Requesting resource 0x%08x failed because it is complex", resID);
        }
        return BAD_VALUE;
    }

    const Res_value* inValue = (const Res_value*)(((const uint8_t*)entry) + dtohs(entry->size));
    if (outValue) {
        outValue->copyFrom_dtoh(*inValue);
        // Translate build-time package ids in references to runtime ids (shared
        // library / add-on package case). For fixed-id packages this is identity.
        if (pkg->dynamicRefTable.lookupResourceValue(outValue) != NO_ERROR) {
            LOGW("Failed to resolve referenced package: 0x%08x", outValue->data);
            return BAD_VALUE;
        }
    }
    if (outSpecFlags) *outSpecFlags = specFlags;
    return (ssize_t)pkg->headerIndex;  // owning-asset blockIndex
}

// Faithful to AOSP ResTable::resolveReference. Follows TYPE_REFERENCE chains
// (count < 20), passing mayBeBag=true so a reference into a style/bag leaves the
// last good value. Returns the final blockIndex (>= 0) or BAD_INDEX.
ssize_t ResTable::resolveReference(Res_value* value, ssize_t blockIndex,
                                   uint32_t* outLastRef, uint32_t* inoutTypeSpecFlags,
                                   ResTable_config* outConfig) const {
    int count = 0;
    while (blockIndex >= 0 && value->dataType == Res_value::TYPE_REFERENCE
            && value->data != 0 && count < 20) {
        if (outLastRef) *outLastRef = value->data;
        uint32_t newFlags = 0;
        const ssize_t newIndex = getResource(value->data, value, true, 0, &newFlags, outConfig);
        if (newIndex == BAD_INDEX) {
            return BAD_INDEX;
        }
        if (inoutTypeSpecFlags) *inoutTypeSpecFlags |= newFlags;
        if (newIndex < 0) {
            // Referenced resource couldn't be read as a value (e.g. a style):
            // leave *value as the reference and return the last good index.
            return blockIndex;
        }
        blockIndex = newIndex;
        count++;
    }
    return blockIndex;
}

const char16_t* ResTable::getResourceString(uint32_t resId, size_t* outLen) const {
    Res_value v;
    ssize_t block = getResource(resId, &v);
    if (block < 0) return nullptr;
    block = resolveReference(&v, block);
    if (block < 0 || v.dataType != Res_value::TYPE_STRING) return nullptr;
    // String values index into their owning arsc's global pool.
    const ResStringPool& pool = (block < (ssize_t)mHeaders.size()) ? mHeaders[block].values : getStringPool();
    size_t len = 0;
    const char16_t* s = pool.stringAt(v.data, &len);
    if (outLen) *outLen = len;
    return s;
}

const char16_t* ResTable::stringAtBlock(ssize_t block, uint32_t index, size_t* outLen) const {
    // TYPE_STRING bag values index into their owning arsc's global pool
    // (mHeaders[block].values), mirroring the pool lookup in getResourceString.
    if (block < 0 || (size_t)block >= mHeaders.size()) return nullptr;
    size_t len = 0;
    const char16_t* s = mHeaders[block].values.stringAt(index, &len);
    if (outLen) *outLen = len;
    return s;
}

const ResTable_map* ResTable::getBag(uint32_t resId, size_t* outCount,
                                     ResTable_config* outConfig,
                                     ssize_t* outBlock,
                                     uint32_t* outSpecFlags) const {
    if (outCount) *outCount = 0;
    if (outBlock) *outBlock = -1;
    if (outSpecFlags) *outSpecFlags = 0;
    if (mError != NO_ERROR) return nullptr;
    uint8_t pkgId  = (uint8_t)((resId >> 24) & 0xff);
    uint8_t typeId = (uint8_t)((resId >> 16) & 0xff);
    uint32_t entryId = resId & 0xffff;
    const Package* pkg = packageForId(pkgId);
    if (!pkg) return nullptr;
    const ResTable_entry* entry = getBestEntry(*pkg, typeId, entryId, mParams, outConfig, outSpecFlags);
    if (!entry || !(entry->flags & ResTable_entry::FLAG_COMPLEX)) return nullptr;
    const ResTable_map_entry* me = (const ResTable_map_entry*)entry;
    if (outCount) *outCount = dtohl(me->count);
    if (outBlock) *outBlock = (ssize_t)pkg->headerIndex;  // owning header (string pool)
    return (const ResTable_map*)(((const uint8_t*)entry) + dtohs(entry->size));
}

bool ResTable::getResourceName(uint32_t resId, std::string* outPackage,
                               std::string* outType, std::string* outKey) const {
    uint8_t pkgId  = (uint8_t)((resId >> 24) & 0xff);
    uint8_t typeId = (uint8_t)((resId >> 16) & 0xff);
    uint32_t entryId = resId & 0xffff;
    const Package* pkg = packageForId(pkgId);
    if (!pkg) return false;
    if (outPackage) *outPackage = pkg->name;
    if (outType && typeId >= 1 && (size_t)(typeId - 1) < pkg->typeStrings.size()) {
        *outType = poolString(pkg->typeStrings, typeId - 1);
    }
    if (outKey) {
        const ResTable_entry* entry = getBestEntry(*pkg, typeId, entryId, mParams, nullptr, nullptr);
        if (entry) *outKey = poolString(pkg->keyStrings, dtohl(entry->key.index));
    }
    return true;
}

// Find a string index in a pool by content (linear; pools are unsorted).
/*static*/ int32_t ResTable::poolIndexOf(const ResStringPool& pool, const std::string& needle) {
    size_t n = pool.size();
    for (uint32_t i = 0; i < n; i++) {
        if (poolString(pool, i) == needle) return (int32_t)i;
    }
    return -1;
}

uint32_t ResTable::getIdentifier(const std::string& name, const std::string& type,
                                 const std::string& package) const {
    if (name.empty() || type.empty()) return 0;
    for (const Package& pkg : mPackages) {
        if (!package.empty() && pkg.name != package) continue;
        // Find the type id by type name.
        int32_t keyType = -1;
        for (uint32_t t = 0; t < pkg.typeStrings.size(); t++) {
            if (poolString(pkg.typeStrings, t) == type) { keyType = (int32_t)t; break; }
        }
        if (keyType < 0 || (size_t)keyType >= pkg.types.size()) continue;
        // Find the entry whose key matches the name.
        int32_t keyIdx = poolIndexOf(pkg.keyStrings, name);
        if (keyIdx < 0) continue;
        const TypeGroup& tg = pkg.types[keyType];
        for (uint32_t e = 0; e < tg.entryCount; e++) {
            const ResTable_entry* entry = anyEntry(pkg, (uint8_t)(keyType + 1), e);
            if (entry && (int32_t)dtohl(entry->key.index) == keyIdx) {
                return ((uint32_t)pkg.id << 24) | ((uint32_t)(keyType + 1) << 16) | e;
            }
        }
    }
    return 0;
}

void ResTable::getConfigurations(std::vector<ResTable_config>* out) const {
    if (!out) return;
    out->clear();
    for (const Package& pkg : mPackages) {
        for (const TypeGroup& tg : pkg.types) {
            for (const ResTable_type* tp : tg.configs) {
                ResTable_config c;
                c.copyFromDtoH(tp->config);
                // Dedupe.
                bool seen = false;
                for (const auto& e : *out) {
                    if (memcmp(&e, &c, sizeof(ResTable_config)) == 0) { seen = true; break; }
                }
                if (!seen) out->push_back(c);
            }
        }
    }
}

std::vector<std::string> ResTable::listPackageNames() const {
    std::vector<std::string> names;
    for (const Package& pkg : mPackages) names.push_back(pkg.name);
    return names;
}

// Decode string-pool entry idx to UTF-8 regardless of the pool's encoding
// (UTF-8 pools return raw bytes; UTF-16 pools transcode ASCII/BMP).
/*static*/ std::string ResTable::poolString(const ResStringPool& pool, uint32_t idx) {
    std::string out;
    if (pool.isUTF8()) {
        size_t len = 0;
        const char* s = pool.string8At(idx, &len);
        if (s) out.assign(s, len);
        return out;
    }
    size_t len = 0;
    const char16_t* s = pool.stringAt(idx, &len);
    for (size_t i = 0; s && i < len; ++i) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00 && s[i + 1] <= 0xDFFF)
            c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        if (c < 0x80) out += (char)c;
        else if (c < 0x800) { out += (char)(0xC0 | (c >> 6)); out += (char)(0x80 | (c & 0x3F)); }
        else if (c < 0x10000) { out += (char)(0xE0 | (c >> 12)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
        else { out += (char)(0xF0 | (c >> 18)); out += (char)(0x80 | ((c >> 12) & 0x3F)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
    }
    return out;
}

// First present entry for (typeId, entryId) across any config (no config
// preference) — used to enumerate resources without resolving a value.
const ResTable_entry* ResTable::anyEntry(const Package& pkg, uint8_t typeId,
                                         uint32_t entryId) const {
    if (typeId == 0 || (size_t)(typeId - 1) >= pkg.types.size()) return nullptr;
    const TypeGroup& group = pkg.types[typeId - 1];
    if (entryId >= group.entryCount) return nullptr;
    for (const ResTable_type* tp : group.configs) {
        const uint32_t* eindex = (const uint32_t*)(((const uint8_t*)tp) + dtohs(tp->header.headerSize));
        uint32_t off;
        if (tp->flags & ResTable_type::FLAG_SPARSE) {
            const ResTable_sparseTypeEntry* sparse = (const ResTable_sparseTypeEntry*)eindex;
            const ResTable_sparseTypeEntry* end = sparse + dtohl(tp->entryCount);
            const ResTable_sparseTypeEntry* lo = std::lower_bound(
                sparse, end, (uint16_t)entryId,
                [](const ResTable_sparseTypeEntry& a, uint16_t b) { return dtohs(a.idx) < b; });
            if (lo == end || dtohs(lo->idx) != entryId) continue;
            off = dtohs(lo->offset) * 4u;
        } else {
            if (entryId >= dtohl(tp->entryCount)) continue;
            off = dtohl(eindex[entryId]);
        }
        if (off == ResTable_type::NO_ENTRY) continue;
        off += dtohl(tp->entriesStart);
        if (off > dtohl(tp->header.size) - sizeof(ResTable_entry)) continue;
        const ResTable_entry* e = (const ResTable_entry*)(((const uint8_t*)tp) + off);
        if (dtohs(e->size) >= sizeof(ResTable_entry)) return e;
    }
    return nullptr;
}

size_t ResTable::listAllResources(std::vector<ResourceRef>* out) const {
    if (out) out->clear();
    size_t count = 0;
    for (const Package& pkg : mPackages) {
        for (size_t t = 0; t < pkg.types.size(); t++) {
            uint8_t typeId = (uint8_t)(t + 1);
            const TypeGroup& group = pkg.types[t];
            if (group.entryCount == 0) continue;
            std::string typeName = poolString(pkg.typeStrings, typeId - 1);
            for (uint32_t entryId = 0; entryId < group.entryCount; entryId++) {
                const ResTable_entry* e = anyEntry(pkg, typeId, entryId);
                if (!e) continue;  // entry not present in any config
                ResourceRef r;
                r.resId = ((uint32_t)pkg.id << 24) | ((uint32_t)typeId << 16) | entryId;
                r.packageId = pkg.id;
                r.type = typeName;
                r.key = poolString(pkg.keyStrings, dtohl(e->key.index));
                if (out) out->push_back(std::move(r));
                count++;
            }
        }
    }
    return count;
}

uint32_t ResTable::getBagParent(uint32_t resId, ResTable_config* outConfig) const {
    if (mError != NO_ERROR) return 0;
    uint8_t pkgId  = (uint8_t)((resId >> 24) & 0xff);
    uint8_t typeId = (uint8_t)((resId >> 16) & 0xff);
    uint32_t entryId = resId & 0xffff;
    const Package* pkg = packageForId(pkgId);
    if (!pkg) return 0;
    const ResTable_entry* e = getBestEntry(*pkg, typeId, entryId, mParams, outConfig, nullptr);
    if (!e || !(dtohs(e->flags) & ResTable_entry::FLAG_COMPLEX)) return 0;
    return dtohl(((const ResTable_map_entry*)e)->parent.ident);
}

// ===========================================================================
// ResTable::Theme — applied-style set with parent inheritance + force/NULL
// override semantics (port of AOSP ResTable::Theme, minus the per-type bag_set
// locking cache; a plain map is semantically equivalent).
// ===========================================================================

ResTable::Theme::Theme(const ResTable& table) : mTable(table) {}
ResTable::Theme::~Theme() {}

status_t ResTable::Theme::clear() {
    mEntries.clear();
    mTypeSpecFlags = 0;
    return NO_ERROR;
}

status_t ResTable::Theme::setTo(const Theme& other) {
    if (this == &other) return NO_ERROR;
    mEntries = other.mEntries;
    mTypeSpecFlags = other.mTypeSpecFlags;
    return NO_ERROR;
}

uint32_t ResTable::Theme::getChangingConfigurations() const {
    return mTypeSpecFlags;
}

status_t ResTable::Theme::applyStyle(uint32_t resID, bool force) {
    return applyStyleChain(resID, force, 0);
}

// Parent inheritance: apply THIS (most-derived) style's items first, then walk
// up the parent chain. Combined with the "overwrite only if slot is null" rule
// below, a child's already-set value is sticky and the parent only fills gaps.
status_t ResTable::Theme::applyStyleChain(uint32_t resID, bool force, int depth) {
    if (depth > 16) return BAD_VALUE;

    size_t count = 0;
    ResTable_config cfg;
    ssize_t block = -1;
    uint32_t specFlags = 0;
    const ResTable_map* map = mTable.getBag(resID, &count, &cfg, &block, &specFlags);
    if (!map) return NAME_NOT_FOUND;  // not a (complex) style
    mTypeSpecFlags |= specFlags;  // aggregate config axes (for getChangingConfigurations)

    for (size_t i = 0; i < count; i++) {
        const uint32_t attrRes = dtohl(map[i].name.ident);
        Res_value v;
        v.copyFrom_dtoh(map[i].value);
        ThemedItem& it = mEntries[attrRes];
        // AOSP override rule: force wins; otherwise an unset slot, or a slot
        // holding TYPE_NULL undefined, is overwritten (sticky otherwise).
        const bool overwrite = force || !it.set ||
            (it.value.dataType == Res_value::TYPE_NULL &&
             it.value.data == Res_value::DATA_NULL_UNDEFINED);
        if (overwrite) {
            it.value = v;
            it.stringBlock = block;
            it.typeSpecFlags = mTypeSpecFlags;
            it.set = true;
        }
    }

    // Now the parent fills any attributes this style didn't set.
    uint32_t parent = mTable.getBagParent(resID);
    if (parent != 0 && parent != resID) {
        status_t err = applyStyleChain(parent, force, depth + 1);
        if (err != NO_ERROR && err != NAME_NOT_FOUND) return err;
    }
    return NO_ERROR;
}

ssize_t ResTable::Theme::getAttribute(uint32_t resID, Res_value* outValue,
                                      uint32_t* outTypeSpecFlags) const {
    auto it = mEntries.find(resID);
    if (it == mEntries.end() || !it->second.set) return NAME_NOT_FOUND;
    if (outValue) *outValue = it->second.value;
    if (outTypeSpecFlags) *outTypeSpecFlags = it->second.typeSpecFlags | mTypeSpecFlags;
    return it->second.stringBlock;  // owning header index (for string values)
}

// Like ResTable::resolveReference, but TYPE_ATTRIBUTE resolves against this
// theme (getAttribute) rather than the table. Returns the final blockIndex or a
// negative error; bounded to < 20 iterations.
ssize_t ResTable::Theme::resolveAttributeReference(Res_value* inOutValue, ssize_t blockIndex,
                                                   uint32_t* outLastRef,
                                                   uint32_t* inoutTypeSpecFlags,
                                                   ResTable_config* inoutConfig) const {
    int count = 0;
    while (blockIndex >= 0 && count < 20) {
        if (inOutValue->dataType == Res_value::TYPE_ATTRIBUTE) {
            uint32_t newFlags = 0;
            ssize_t newBlock = getAttribute(inOutValue->data, inOutValue, &newFlags);
            if (inoutTypeSpecFlags) *inoutTypeSpecFlags |= newFlags;
            if (newBlock < 0) return newBlock;
            blockIndex = newBlock;
        } else if (inOutValue->dataType == Res_value::TYPE_REFERENCE) {
            if (outLastRef) *outLastRef = inOutValue->data;
            if (inOutValue->data == 0) break;
            uint32_t newFlags = 0;
            ssize_t newBlock = mTable.getResource(inOutValue->data, inOutValue, true, 0,
                                                  &newFlags, inoutConfig);
            if (newBlock == BAD_INDEX) return BAD_INDEX;
            if (inoutTypeSpecFlags) *inoutTypeSpecFlags |= newFlags;
            if (newBlock < 0) return blockIndex;  // e.g. target is a style/bag
            blockIndex = newBlock;
        } else {
            break;
        }
        count++;
    }
    return blockIndex;
}

// ===========================================================================
// obtainStyledAttributes (applyStyle merge) + TypedArray — full attribute
// resolution: element > style= > defStyleAttr(theme) > defStyleRes > theme.
// ===========================================================================

void obtainStyledAttributes(const ResXMLTree& xml, const ResTable& table,
                            const ResTable::Theme* theme,
                            const uint32_t* attrs, size_t attrCount,
                            uint32_t defStyleAttr, uint32_t defStyleRes,
                            StyledAttr* out) {
    for (size_t i = 0; i < attrCount; i++) { out[i].set = false; out[i].stringBlock = -1; }
    if (xml.getEventType() != ResXMLParser::START_TAG) return;

    // The element's style= attribute (no namespace, name "style") -> style resId.
    uint32_t styleRes = 0;
    ssize_t styleIdx = xml.indexOfAttribute(nullptr, "style");
    if (styleIdx >= 0) {
        Res_value sv;
        if (xml.getAttributeValue((size_t)styleIdx, &sv) == sizeof(Res_value) &&
            (sv.dataType == Res_value::TYPE_REFERENCE || sv.dataType == Res_value::TYPE_ATTRIBUTE)) {
            styleRes = sv.data;
        }
    }

    // Style/theme fallback chain. Lowest priority is applied first so the
    // sticky "first-set wins" rule yields the right precedence.
    ResTable::Theme chain(table);
    if (defStyleRes) chain.applyStyle(defStyleRes);
    if (defStyleAttr && theme) {
        Res_value dv;
        if (theme->getAttribute(defStyleAttr, &dv) >= 0 &&
            (dv.dataType == Res_value::TYPE_REFERENCE || dv.dataType == Res_value::TYPE_ATTRIBUTE)) {
            chain.applyStyle(dv.data);
        }
    }
    if (styleRes) chain.applyStyle(styleRes);

    const size_t elemCount = xml.getAttributeCount();
    for (size_t i = 0; i < attrCount; i++) {
        const uint32_t a = attrs[i];
        // 1. Element's own attribute (matched by resource id).
        bool found = false;
        for (size_t j = 0; j < elemCount; j++) {
            if (xml.getAttributeNameResID(j) == a) {
                Res_value v;
                if (xml.getAttributeValue(j, &v) == sizeof(Res_value)) {
                    out[i].value = v; out[i].stringBlock = -2; out[i].set = true; found = true;
                }
                break;
            }
        }
        if (found) continue;
        // 2. style / defStyleAttr / defStyleRes chain.
        Res_value v;
        ssize_t blk = chain.getAttribute(a, &v);
        if (blk >= 0) {
            // Flatten ?attr (TYPE_ATTRIBUTE) / @ref (TYPE_REFERENCE) chains to a
            // concrete value before storing, so callers see the resolved value.
            blk = chain.resolveAttributeReference(&v, blk);
            out[i].value = v; out[i].stringBlock = blk; out[i].set = true; continue;
        }
        // 3. Theme direct value.
        if (theme && theme->getAttribute(a, &v) >= 0) {
            ssize_t tblk = 0;
            tblk = theme->resolveAttributeReference(&v, tblk);
            out[i].value = v; out[i].stringBlock = tblk; out[i].set = true; continue;
        }
    }
}


} // namespace cdroid
