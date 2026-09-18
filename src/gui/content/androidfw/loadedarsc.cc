// Port of AOSP androidfw/LoadedArsc.cpp (frameworks/base/libs/androidfw/
// LoadedArsc.cpp), namespace cdroid.
//
// Adaptations (see loadedarsc.h for the trim boundary): no idmap/RRO parameter,
// LOG streams -> cdlog printf faces, StringPrintf -> snprintf, structured
// bindings unpacked for C++14, this tree's raw-pointer ResStringPool faces.
//
// Copyright (C) 2016 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.

#include <content/androidfw/loadedarsc.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <limits>

#include <porting/cdlog.h>

namespace cdroid {

constexpr const static int kFrameworkPackageId = 0x01;
constexpr const static int kAppPackageId = 0x7f;

namespace {

// util::ReadUtf16StringFromDevice (androidfw/Util.h): a NUL-terminated
// UTF-16LE char array on "device" (file) into a std::string (UTF-8). CDROID
// hosts are little-endian; code points beyond BMP encode as proper UTF-8.
void ReadUtf16StringFromDevice(const uint16_t* src, size_t len, std::string* dst) {
    dst->clear();
    for (size_t i = 0; i < len && src[i] != 0; i++) {
        uint32_t cp = src[i];
        if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < len) {
            const uint32_t lo = src[i + 1];
            if (lo >= 0xDC00 && lo <= 0xDFFF) {
                cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                i++;
            }
        }
        if (cp < 0x80) {
            dst->push_back((char)cp);
        } else if (cp < 0x800) {
            dst->push_back((char)(0xC0 | (cp >> 6)));
            dst->push_back((char)(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
            dst->push_back((char)(0xE0 | (cp >> 12)));
            dst->push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
            dst->push_back((char)(0x80 | (cp & 0x3F)));
        } else {
            dst->push_back((char)(0xF0 | (cp >> 18)));
            dst->push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
            dst->push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
            dst->push_back((char)(0x80 | (cp & 0x3F)));
        }
    }
}

// Builder that helps accumulate Type structs and then create a single
// contiguous block of memory to store both the TypeSpec struct and
// the Type structs.
struct TypeSpecBuilder {
    explicit TypeSpecBuilder(incfs::verified_map_ptr<ResTable_typeSpec> header) : header_(header) {}

    void AddType(incfs::verified_map_ptr<ResTable_type> type) {
        type_entries.emplace_back();
        TypeSpec::TypeEntry& entry = type_entries.back();
        entry.config.copyFromDtoH(type->config);
        entry.type = type;
    }

    TypeSpec Build() {
        return {header_, std::move(type_entries)};
    }

private:
    TypeSpecBuilder(const TypeSpecBuilder&) = delete;
    TypeSpecBuilder& operator=(const TypeSpecBuilder&) = delete;

    incfs::verified_map_ptr<ResTable_typeSpec> header_;
    std::vector<TypeSpec::TypeEntry> type_entries;
};

}  // namespace

// Precondition: The header passed in has already been verified, so reading any fields and trusting
// the ResChunk_header is safe.
static bool VerifyResTableType(incfs::map_ptr<ResTable_type> header) {
    if (header->id == 0) {
        LOGE("RES_TABLE_TYPE_TYPE has invalid ID 0.");
        return false;
    }

    const size_t entry_count = dtohl(header->entryCount);
    if (entry_count > std::numeric_limits<uint16_t>::max()) {
        LOGE("RES_TABLE_TYPE_TYPE has too many entries (%zu).", entry_count);
        return false;
    }

    // Make sure that there is enough room for the entry offsets.
    const size_t offsets_offset = dtohs(header->header.headerSize);
    const size_t entries_offset = dtohl(header->entriesStart);
    const size_t offsets_length = sizeof(uint32_t) * entry_count;

    if (offsets_offset > entries_offset || entries_offset - offsets_offset < offsets_length) {
        LOGE("RES_TABLE_TYPE_TYPE entry offsets overlap actual entry data.");
        return false;
    }

    if (entries_offset > dtohl(header->header.size)) {
        LOGE("RES_TABLE_TYPE_TYPE entry offsets extend beyond chunk.");
        return false;
    }

    if (entries_offset & 0x03U) {
        LOGE("RES_TABLE_TYPE_TYPE entries start at unaligned address.");
        return false;
    }
    return true;
}

static base::expected<base::monostate, androidfw::NullOrIOError> VerifyResTableEntry(
        incfs::verified_map_ptr<ResTable_type> type, uint32_t entry_offset) {
    // Check that the offset is aligned.
    if (entry_offset & 0x03U) {
        LOGE("Entry at offset %u is not 4-byte aligned.", entry_offset);
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    // Check that the offset doesn't overflow.
    if (entry_offset > std::numeric_limits<uint32_t>::max() - dtohl(type->entriesStart)) {
        // Overflow in offset.
        LOGE("Entry at offset %u is too large.", entry_offset);
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    const size_t chunk_size = dtohl(type->header.size);

    entry_offset += dtohl(type->entriesStart);
    if (entry_offset > chunk_size - sizeof(ResTable_entry)) {
        LOGE("Entry at offset %u is too large. No room for ResTable_entry.", entry_offset);
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    auto entry = type.offset(entry_offset).convert<ResTable_entry>();
    if (!entry) {
        return base::unexpected<androidfw::NullOrIOError>(androidfw::IOError::PAGES_MISSING);
    }

    const size_t entry_size = dtohs(entry->size);
    if (entry_size < sizeof(ResTable_entry)) {
        LOGE("ResTable_entry size %zu at offset %u is too small.", entry_size, entry_offset);
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    if (entry_size > chunk_size || entry_offset > chunk_size - entry_size) {
        LOGE("ResTable_entry size %zu at offset %u is too large.", entry_size, entry_offset);
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    if (entry_size < sizeof(ResTable_map_entry)) {
        // There needs to be room for one Res_value struct.
        if (entry_offset + entry_size > chunk_size - sizeof(Res_value)) {
            LOGE("No room for Res_value after ResTable_entry at offset %u for type %d.",
                 entry_offset, (int)type->id);
            return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
        }

        auto value = entry.offset(entry_size).convert<Res_value>();
        if (!value) {
            return base::unexpected<androidfw::NullOrIOError>(androidfw::IOError::PAGES_MISSING);
        }

        const size_t value_size = dtohs(value->size);
        if (value_size < sizeof(Res_value)) {
            LOGE("Res_value at offset %u is too small.", entry_offset);
            return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
        }

        if (value_size > chunk_size || entry_offset + entry_size > chunk_size - value_size) {
            LOGE("Res_value size %zu at offset %u is too large.", value_size, entry_offset);
            return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
        }
    } else {
        auto map = entry.convert<ResTable_map_entry>();
        if (!map) {
            return base::unexpected<androidfw::NullOrIOError>(androidfw::IOError::PAGES_MISSING);
        }

        const size_t map_entry_count = dtohl(map->count);
        size_t map_entries_start = entry_offset + entry_size;
        if (map_entries_start & 0x03U) {
            LOGE("Map entries at offset %u start at unaligned offset.", entry_offset);
            return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
        }

        // Each entry is sizeof(ResTable_map) big.
        if (map_entry_count > ((chunk_size - map_entries_start) / sizeof(ResTable_map))) {
            LOGE("Too many map entries in ResTable_map_entry at offset %u.", entry_offset);
            return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
        }
    }
    return base::expected<base::monostate, androidfw::NullOrIOError>();
}

LoadedPackage::iterator::iterator(const LoadedPackage* lp, size_t ti, size_t ei)
    : loadedPackage_(lp),
      typeIndex_(ti),
      entryIndex_(ei),
      typeIndexEnd_(lp->resource_ids_.size() + 1) {
    while (typeIndex_ < typeIndexEnd_ && loadedPackage_->resource_ids_[typeIndex_] == 0) {
        typeIndex_++;
    }
}

LoadedPackage::iterator& LoadedPackage::iterator::operator++() {
    while (typeIndex_ < typeIndexEnd_) {
        if (entryIndex_ + 1 < loadedPackage_->resource_ids_[typeIndex_]) {
            entryIndex_++;
            break;
        }
        entryIndex_ = 0;
        typeIndex_++;
        if (typeIndex_ < typeIndexEnd_ && loadedPackage_->resource_ids_[typeIndex_] != 0) {
            break;
        }
    }
    return *this;
}

uint32_t LoadedPackage::iterator::operator*() const {
    if (typeIndex_ >= typeIndexEnd_) {
        return 0;
    }
    return make_resid(loadedPackage_->package_id_,
            static_cast<uint8_t>(typeIndex_ + loadedPackage_->type_id_offset_), entryIndex_);
}

base::expected<incfs::map_ptr<ResTable_entry>, androidfw::NullOrIOError> LoadedPackage::GetEntry(
        incfs::verified_map_ptr<ResTable_type> type_chunk, uint16_t entry_index) {
    base::expected<uint32_t, androidfw::NullOrIOError> entry_offset =
            GetEntryOffset(type_chunk, entry_index);
    if (!entry_offset.has_value()) {
        return base::unexpected<androidfw::NullOrIOError>(entry_offset.error());
    }
    return GetEntryFromOffset(type_chunk, entry_offset.value());
}

base::expected<uint32_t, androidfw::NullOrIOError> LoadedPackage::GetEntryOffset(
        incfs::verified_map_ptr<ResTable_type> type_chunk, uint16_t entry_index) {
    // The configuration matches and is better than the previous selection.
    // Find the entry value if it exists for this configuration.
    const size_t entry_count = dtohl(type_chunk->entryCount);
    const size_t offsets_offset = dtohs(type_chunk->header.headerSize);

    // Check if there is the desired entry in this type.
    if (type_chunk->flags & ResTable_type::FLAG_SPARSE) {
        // This is encoded as a sparse map, so perform a binary search.
        bool error = false;
        auto sparse_indices = type_chunk.offset(offsets_offset)
                                      .convert<ResTable_sparseTypeEntry>().iterator();
        auto sparse_indices_end = sparse_indices + entry_count;
        auto result = std::lower_bound(sparse_indices, sparse_indices_end, entry_index,
                                   [&error](const incfs::map_ptr<ResTable_sparseTypeEntry>& entry,
                                            uint16_t entry_idx) {
            if (!entry) {
                return error = true;
            }
            return dtohs(entry->idx) < entry_idx;
        });

        if (result == sparse_indices_end) {
            // No entry found.
            return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
        }

        const incfs::verified_map_ptr<ResTable_sparseTypeEntry> entry = (*result).verified();
        if (dtohs(entry->idx) != entry_index) {
            if (error) {
                return base::unexpected<androidfw::NullOrIOError>(androidfw::IOError::PAGES_MISSING);
            }
            return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
        }

        // Extract the offset from the entry. Each offset must be a multiple of 4 so we store it as
        // the real offset divided by 4.
        return uint32_t{dtohs(entry->offset)} * 4u;
    }

    // This type is encoded as a dense array.
    if (entry_index >= entry_count) {
        // This entry cannot be here.
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    const auto entry_offset_ptr = type_chunk.offset(offsets_offset).convert<uint32_t>() + entry_index;
    if (!entry_offset_ptr) {
        return base::unexpected<androidfw::NullOrIOError>(androidfw::IOError::PAGES_MISSING);
    }

    const uint32_t value = dtohl(entry_offset_ptr.value());
    if (value == ResTable_type::NO_ENTRY) {
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    return value;
}

base::expected<incfs::map_ptr<ResTable_entry>, androidfw::NullOrIOError> LoadedPackage::GetEntryFromOffset(
        incfs::verified_map_ptr<ResTable_type> type_chunk, uint32_t offset) {
    auto valid = VerifyResTableEntry(type_chunk, offset);
    if (!valid.has_value()) {
        return base::unexpected<androidfw::NullOrIOError>(valid.error());
    }
    return type_chunk.offset(offset + dtohl(type_chunk->entriesStart)).convert<ResTable_entry>();
}

base::expected<base::monostate, androidfw::IOError> LoadedPackage::CollectConfigurations(
        bool exclude_mipmap, std::set<ResTable_config>* out_configs) const {
    for (const auto& type_spec : type_specs_) {
        if (exclude_mipmap) {
            const int type_idx = type_spec.first - 1;
            size_t name_len = 0;
            // This tree's ResStringPool face: raw pointer + out-len; nullptr
            // on absent (AOSP's IsIOError can't happen without incfs).
            if (const char16_t* type_name16 = type_string_pool_.stringAt(type_idx, &name_len)) {
                const std::u16string name(type_name16, name_len);
                if (name.compare(0, name.size(), u"mipmap", 6) == 0) {
                    // This is a mipmap type, skip collection.
                    continue;
                }
            }
            if (const char* type_name = type_string_pool_.string8At(type_idx, &name_len)) {
                if (name_len >= 6 && strncmp(type_name, "mipmap", 6) == 0) {
                    // This is a mipmap type, skip collection.
                    continue;
                }
            }
        }

        for (const auto& type_entry : type_spec.second.type_entries) {
            out_configs->insert(type_entry.config);
        }
    }
    return base::expected<base::monostate, androidfw::IOError>();
}

void LoadedPackage::CollectLocales(bool canonicalize, std::set<std::string>* out_locales) const {
    char temp_locale[40];   // RESTABLE_MAX_LOCALE_LEN
    for (const auto& type_spec : type_specs_) {
        for (const auto& type_entry : type_spec.second.type_entries) {
            if (type_entry.config.locale != 0) {
                type_entry.config.getBcp47Locale(temp_locale, canonicalize);
                std::string locale(temp_locale);
                out_locales->insert(std::move(locale));
            }
        }
    }
}

base::expected<uint32_t, androidfw::NullOrIOError> LoadedPackage::FindEntryByName(
        const std::u16string& type_name, const std::u16string& entry_name) const {
    const ssize_t type_idx = type_string_pool_.indexOfString(type_name.data(), type_name.size());
    if (type_idx < 0) {
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    const ssize_t key_idx = key_string_pool_.indexOfString(entry_name.data(), entry_name.size());
    if (key_idx < 0) {
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    const TypeSpec* type_spec = GetTypeSpecByTypeIndex(type_idx);
    if (type_spec == nullptr) {
        return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
    }

    for (const auto& type_entry : type_spec->type_entries) {
        const incfs::verified_map_ptr<ResTable_type>& type = type_entry.type;

        size_t entry_count = dtohl(type->entryCount);
        for (size_t entry_idx = 0; entry_idx < entry_count; entry_idx++) {
            auto entry_offset_ptr = type.offset(dtohs(type->header.headerSize)).convert<uint32_t>() +
                entry_idx;
            if (!entry_offset_ptr) {
                return base::unexpected<androidfw::NullOrIOError>(androidfw::IOError::PAGES_MISSING);
            }

            uint32_t offset;
            uint16_t res_idx;
            if (type->flags & ResTable_type::FLAG_SPARSE) {
                auto sparse_entry = entry_offset_ptr.convert<ResTable_sparseTypeEntry>();
                offset = dtohs(sparse_entry->offset) * 4u;
                res_idx = dtohs(sparse_entry->idx);
            } else {
                offset = dtohl(entry_offset_ptr.value());
                res_idx = entry_idx;
            }
            if (offset != ResTable_type::NO_ENTRY) {
                auto entry = type.offset(dtohl(type->entriesStart) + offset).convert<ResTable_entry>();
                if (!entry) {
                    return base::unexpected<androidfw::NullOrIOError>(androidfw::IOError::PAGES_MISSING);
                }

                if (dtohl(entry->key.index) == static_cast<uint32_t>(key_idx)) {
                    // The package ID will be overridden by the caller (due to runtime assignment of package
                    // IDs for shared libraries).
                    return make_resid(0x00, static_cast<uint8_t>(type_idx + type_id_offset_ + 1),
                                      res_idx);
                }
            }
        }
    }
    return base::unexpected<androidfw::NullOrIOError>(base::nullopt);
}

const LoadedPackage* LoadedArsc::GetPackageById(uint8_t package_id) const {
    for (const auto& loaded_package : packages_) {
        if (loaded_package->GetPackageId() == package_id) {
            return loaded_package.get();
        }
    }
    return nullptr;
}

std::unique_ptr<const LoadedPackage> LoadedPackage::Load(const Chunk& chunk,
                                                        package_property_t property_flags) {
    std::unique_ptr<LoadedPackage> loaded_package(new LoadedPackage());

    // typeIdOffset was added at some point, but we still must recognize apps built before this
    // was added.
    constexpr size_t kMinPackageSize =
        sizeof(ResTable_package) - sizeof(ResTable_package::typeIdOffset);
    const incfs::map_ptr<ResTable_package> header = chunk.header<ResTable_package, kMinPackageSize>();
    if (!header) {
        LOGE("RES_TABLE_PACKAGE_TYPE too small.");
        return {};
    }

    if ((property_flags & PROPERTY_SYSTEM) != 0) {
        loaded_package->property_flags_ |= PROPERTY_SYSTEM;
    }

    if ((property_flags & PROPERTY_LOADER) != 0) {
        loaded_package->property_flags_ |= PROPERTY_LOADER;
    }

    if ((property_flags & PROPERTY_OVERLAY) != 0) {
        // Overlay resources must have an exclusive resource id space for referencing internal
        // resources.
        loaded_package->property_flags_ |= PROPERTY_OVERLAY | PROPERTY_DYNAMIC;
    }

    loaded_package->package_id_ = dtohl(header->id);
    if (loaded_package->package_id_ == 0 ||
        (loaded_package->package_id_ == kAppPackageId && (property_flags & PROPERTY_DYNAMIC) != 0)) {
        loaded_package->property_flags_ |= PROPERTY_DYNAMIC;
    }

    if (header->header.headerSize >= sizeof(ResTable_package)) {
        uint32_t type_id_offset = dtohl(header->typeIdOffset);
        if (type_id_offset > std::numeric_limits<uint8_t>::max()) {
            LOGE("RES_TABLE_PACKAGE_TYPE type ID offset too large.");
            return {};
        }
        loaded_package->type_id_offset_ = static_cast<int>(type_id_offset);
    }

    ReadUtf16StringFromDevice(header->name, sizeof(header->name) / sizeof(header->name[0]),
                              &loaded_package->package_name_);

    // A map of TypeSpec builders, each associated with an type index.
    // We use these to accumulate the set of Types available for a TypeSpec, and later build a single,
    // contiguous block of memory that holds all the Types together with the TypeSpec.
    std::unordered_map<int, std::unique_ptr<TypeSpecBuilder>> type_builder_map;

    ChunkIterator iter(chunk.data_ptr(), chunk.data_size());
    while (iter.HasNext()) {
        const Chunk child_chunk = iter.Next();
        switch (child_chunk.type()) {
        case RES_STRING_POOL_TYPE: {
            const auto pool_address = child_chunk.header<ResChunk_header>();
            if (!pool_address) {
                LOGE("RES_STRING_POOL_TYPE is incomplete due to incremental installation.");
                return {};
            }

            if (pool_address == header.offset(dtohl(header->typeStrings)).convert<ResChunk_header>()) {
                // This string pool is the type string pool.
                status_t err = loaded_package->type_string_pool_.setTo(
                        child_chunk.header<ResStringPool_header>().unsafe_ptr(), child_chunk.size());
                if (err != NO_ERROR) {
                    LOGE("RES_STRING_POOL_TYPE for types corrupt.");
                    return {};
                }
            } else if (pool_address ==
                       header.offset(dtohl(header->keyStrings)).convert<ResChunk_header>()) {
                // This string pool is the key string pool.
                status_t err = loaded_package->key_string_pool_.setTo(
                        child_chunk.header<ResStringPool_header>().unsafe_ptr(), child_chunk.size());
                if (err != NO_ERROR) {
                    LOGE("RES_STRING_POOL_TYPE for keys corrupt.");
                    return {};
                }
            } else {
                LOGW("Too many RES_STRING_POOL_TYPEs found in RES_TABLE_PACKAGE_TYPE.");
            }
        } break;

        case RES_TABLE_TYPE_SPEC_TYPE: {
            const auto type_spec = child_chunk.header<ResTable_typeSpec>();
            if (!type_spec) {
                LOGE("RES_TABLE_TYPE_SPEC_TYPE too small.");
                return {};
            }

            if (type_spec->id == 0) {
                LOGE("RES_TABLE_TYPE_SPEC_TYPE has invalid ID 0.");
                return {};
            }

            if (loaded_package->type_id_offset_ + static_cast<int>(type_spec->id) >
                std::numeric_limits<uint8_t>::max()) {
                LOGE("RES_TABLE_TYPE_SPEC_TYPE has out of range ID.");
                return {};
            }

            // The data portion of this chunk contains entry_count 32bit entries,
            // each one representing a set of flags.
            // Here we only validate that the chunk is well formed.
            const size_t entry_count = dtohl(type_spec->entryCount);

            // There can only be 2^16 entries in a type, because that is the ID
            // space for entries (EEEE) in the resource ID 0xPPTTEEEE.
            if (entry_count > std::numeric_limits<uint16_t>::max()) {
                LOGE("RES_TABLE_TYPE_SPEC_TYPE has too many entries (%zu).", entry_count);
                return {};
            }

            if (entry_count * sizeof(uint32_t) > chunk.data_size()) {
                LOGE("RES_TABLE_TYPE_SPEC_TYPE too small to hold entries.");
                return {};
            }

            std::unique_ptr<TypeSpecBuilder>& builder_ptr = type_builder_map[type_spec->id];
            if (builder_ptr == nullptr) {
                builder_ptr.reset(new TypeSpecBuilder(type_spec.verified()));
                loaded_package->resource_ids_.set(type_spec->id, entry_count);
            } else {
                LOGW("RES_TABLE_TYPE_SPEC_TYPE already defined for ID %02x", type_spec->id);
            }
        } break;

        case RES_TABLE_TYPE_TYPE: {
            const auto type = child_chunk.header<ResTable_type, kResTableTypeMinSize>();
            if (!type) {
                LOGE("RES_TABLE_TYPE_TYPE too small.");
                return {};
            }

            if (!VerifyResTableType(type)) {
                return {};
            }

            // Type chunks must be preceded by their TypeSpec chunks.
            std::unique_ptr<TypeSpecBuilder>& builder_ptr = type_builder_map[type->id];
            if (builder_ptr != nullptr) {
                builder_ptr->AddType(type.verified());
            } else {
                LOGE("RES_TABLE_TYPE_TYPE with ID %02x found without preceding RES_TABLE_TYPE_SPEC_TYPE.",
                     type->id);
                return {};
            }
        } break;

        case RES_TABLE_LIBRARY_TYPE: {
            const auto lib = child_chunk.header<ResTable_lib_header>();
            if (!lib) {
                LOGE("RES_TABLE_LIBRARY_TYPE too small.");
                return {};
            }

            if (child_chunk.data_size() / sizeof(ResTable_lib_entry) < dtohl(lib->count)) {
                LOGE("RES_TABLE_LIBRARY_TYPE too small to hold entries.");
                return {};
            }

            loaded_package->dynamic_package_map_.reserve(dtohl(lib->count));

            const auto entry_begin = child_chunk.data_ptr().convert<ResTable_lib_entry>();
            const auto entry_end = entry_begin + dtohl(lib->count);
            for (auto entry_iter = entry_begin; entry_iter != entry_end; ++entry_iter) {
                if (!entry_iter) {
                    return {};
                }

                std::string package_name;
                ReadUtf16StringFromDevice(entry_iter->packageName,
                                          sizeof(entry_iter->packageName) / sizeof(uint16_t),
                                          &package_name);

                if (dtohl(entry_iter->packageId) >= std::numeric_limits<uint8_t>::max()) {
                    LOGE("Package ID %02x in RES_TABLE_LIBRARY_TYPE too large for package '%s'.",
                         dtohl(entry_iter->packageId), package_name.c_str());
                    return {};
                }

                loaded_package->dynamic_package_map_.emplace_back(std::move(package_name),
                                                                  dtohl(entry_iter->packageId));
            }
        } break;

        case RES_TABLE_OVERLAYABLE_TYPE: {
            const auto overlayable = child_chunk.header<ResTable_overlayable_header>();
            if (!overlayable) {
                LOGE("RES_TABLE_OVERLAYABLE_TYPE too small.");
                return {};
            }

            std::string name;
            ReadUtf16StringFromDevice(overlayable->name,
                                      sizeof(overlayable->name) / sizeof(uint16_t), &name);
            std::string actor;
            ReadUtf16StringFromDevice(overlayable->actor,
                                      sizeof(overlayable->actor) / sizeof(uint16_t), &actor);

            if (loaded_package->overlayable_map_.find(name) !=
                loaded_package->overlayable_map_.end()) {
                LOGE("Multiple <overlayable> blocks with the same name '%s'.", name.c_str());
                return {};
            }
            loaded_package->overlayable_map_.emplace(name, actor);

            // Iterate over the overlayable policy chunks contained within the overlayable chunk data
            ChunkIterator overlayable_iter(child_chunk.data_ptr(), child_chunk.data_size());
            while (overlayable_iter.HasNext()) {
                const Chunk overlayable_child_chunk = overlayable_iter.Next();

                switch (overlayable_child_chunk.type()) {
                    case RES_TABLE_OVERLAYABLE_POLICY_TYPE: {
                        const auto policy_header =
                            overlayable_child_chunk.header<ResTable_overlayable_policy_header>();
                        if (!policy_header) {
                            LOGE("RES_TABLE_OVERLAYABLE_POLICY_TYPE too small.");
                            return {};
                        }

                        if ((overlayable_child_chunk.data_size() / sizeof(ResTable_ref))
                            < dtohl(policy_header->entry_count)) {
                            LOGE("RES_TABLE_OVERLAYABLE_POLICY_TYPE too small to hold entries.");
                            return {};
                        }

                        // Retrieve all the resource ids belonging to this policy chunk
                        std::unordered_set<uint32_t> ids;
                        const auto ids_begin = overlayable_child_chunk.data_ptr().convert<ResTable_ref>();
                        const auto ids_end = ids_begin + dtohl(policy_header->entry_count);
                        for (auto id_iter = ids_begin; id_iter != ids_end; ++id_iter) {
                            if (!id_iter) {
                                return {};
                            }
                            ids.insert(dtohl(id_iter->ident));
                        }

                        // Add the pairing of overlayable properties and resource ids to the package
                        OverlayableInfo overlayable_info{};
                        overlayable_info.name = name;
                        overlayable_info.actor = actor;
                        overlayable_info.policy_flags = policy_header->policy_flags;
                        loaded_package->overlayable_infos_.emplace_back(overlayable_info, ids);
                        loaded_package->defines_overlayable_ = true;
                        break;
                    }

                    default:
                        LOGW("Unknown chunk type '%02x'.", chunk.type());
                        break;
                }
            }

            if (overlayable_iter.HadError()) {
                LOGE("Error parsing RES_TABLE_OVERLAYABLE_TYPE: %s",
                     overlayable_iter.GetLastError().c_str());
                if (overlayable_iter.HadFatalError()) {
                    return {};
                }
            }
        } break;

        case RES_TABLE_STAGED_ALIAS_TYPE: {
            if (loaded_package->package_id_ != kFrameworkPackageId) {
                LOGW("Alias chunk ignored for non-framework package '%s'",
                     loaded_package->package_name_.c_str());
                break;
            }

            std::unordered_set<uint32_t> finalized_ids;
            const auto lib_alias = child_chunk.header<ResTable_staged_alias_header>();
            if (!lib_alias) {
                LOGE("RES_TABLE_STAGED_ALIAS_TYPE is too small.");
                return {};
            }
            if ((child_chunk.data_size() / sizeof(ResTable_staged_alias_entry))
                < dtohl(lib_alias->count)) {
                LOGE("RES_TABLE_STAGED_ALIAS_TYPE is too small to hold entries.");
                return {};
            }
            const auto entry_begin = child_chunk.data_ptr().convert<ResTable_staged_alias_entry>();
            const auto entry_end = entry_begin + dtohl(lib_alias->count);
            for (auto entry_iter = entry_begin; entry_iter != entry_end; ++entry_iter) {
                if (!entry_iter) {
                    return {};
                }
                auto finalized_id = dtohl(entry_iter->finalizedResId);
                if (!finalized_ids.insert(finalized_id).second) {
                    LOGE("Repeated finalized resource id '%08x' in staged aliases.", finalized_id);
                    return {};
                }

                auto staged_id = dtohl(entry_iter->stagedResId);
                // C++14: no structured bindings.
                auto insert_result = loaded_package->alias_id_map_.insert(
                        std::make_pair(staged_id, finalized_id));
                if (!insert_result.second) {
                    LOGE("Repeated staged resource id '%08x' in staged aliases.", staged_id);
                    return {};
                }
            }
        } break;

        default:
            LOGW("Unknown chunk type '%02x'.", chunk.type());
            break;
        }
    }

    if (iter.HadError()) {
        LOGE("%s", iter.GetLastError().c_str());
        if (iter.HadFatalError()) {
            return {};
        }
    }

    // Flatten and construct the TypeSpecs.
    for (auto& entry : type_builder_map) {
        TypeSpec type_spec = entry.second->Build();
        uint8_t type_id = static_cast<uint8_t>(entry.first);
        loaded_package->type_specs_[type_id] = std::move(type_spec);
    }

    return loaded_package;
}

bool LoadedArsc::LoadTable(const Chunk& chunk, package_property_t property_flags) {
    incfs::map_ptr<ResTable_header> header = chunk.header<ResTable_header>();
    if (!header) {
        LOGE("RES_TABLE_TYPE too small.");
        return false;
    }

    const size_t package_count = dtohl(header->packageCount);
    size_t packages_seen = 0;

    packages_.reserve(package_count);

    ChunkIterator iter(chunk.data_ptr(), chunk.data_size());
    while (iter.HasNext()) {
        const Chunk child_chunk = iter.Next();
        switch (child_chunk.type()) {
        case RES_STRING_POOL_TYPE:
            // Only use the first string pool. Ignore others.
            if (global_string_pool_->getError() == NO_INIT) {
                status_t err = global_string_pool_->setTo(
                        child_chunk.header<ResStringPool_header>().unsafe_ptr(),
                        child_chunk.size());
                if (err != NO_ERROR) {
                    LOGE("RES_STRING_POOL_TYPE corrupt.");
                    return false;
                }
            } else {
                LOGW("Multiple RES_STRING_POOL_TYPEs found in RES_TABLE_TYPE.");
            }
            break;

        case RES_TABLE_PACKAGE_TYPE: {
            if (packages_seen + 1 > package_count) {
                LOGE("More package chunks were found than the %zu declared in the header.",
                     package_count);
                return false;
            }
            packages_seen++;

            std::unique_ptr<const LoadedPackage> loaded_package =
                LoadedPackage::Load(child_chunk, property_flags);
            if (!loaded_package) {
                return false;
            }
            packages_.push_back(std::move(loaded_package));
        } break;

        default:
            LOGW("Unknown chunk type '%02x'.", chunk.type());
            break;
        }
    }

    if (iter.HadError()) {
        LOGE("%s", iter.GetLastError().c_str());
        if (iter.HadFatalError()) {
            return false;
        }
    }
    return true;
}

std::unique_ptr<LoadedArsc> LoadedArsc::Load(incfs::map_ptr<void> data,
                                            const size_t length,
                                            const package_property_t property_flags) {
    // Not using make_unique because the constructor is private.
    std::unique_ptr<LoadedArsc> loaded_arsc(new LoadedArsc());

    ChunkIterator iter(data, length);
    while (iter.HasNext()) {
        const Chunk chunk = iter.Next();
        switch (chunk.type()) {
        case RES_TABLE_TYPE:
            if (!loaded_arsc->LoadTable(chunk, property_flags)) {
                return {};
            }
            break;

        default:
            LOGW("Unknown chunk type '%02x'.", chunk.type());
            break;
        }
    }

    if (iter.HadError()) {
        LOGE("%s", iter.GetLastError().c_str());
        if (iter.HadFatalError()) {
            return {};
        }
    }

    return loaded_arsc;
}

std::unique_ptr<LoadedArsc> LoadedArsc::CreateEmpty() {
    return std::unique_ptr<LoadedArsc>(new LoadedArsc());
}

}  // namespace cdroid
