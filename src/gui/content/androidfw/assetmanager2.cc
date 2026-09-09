// Port of AOSP androidfw/AssetManager2.cpp (frameworks/base/libs/androidfw/
// AssetManager2.cpp), namespace cdroid. Trim boundary and C++14 seams are
// documented in assetmanager2.h; per-function adaptations inline.
//
// Copyright (C) 2016 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.

#include <content/androidfw/assetmanager2.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <map>
#include <set>
#include <sstream>

#include <porting/cdlog.h>

namespace cdroid {

namespace {

// AOSP Util.h resource-id helpers.
inline bool is_valid_resid(uint32_t resid) {
    return (resid & 0xFF0000U) != 0 && resid != 0x0;
}

inline uint32_t get_package_id(uint32_t resid) { return resid >> 24; }
inline uint32_t get_type_id(uint32_t resid) { return (resid >> 16) & 0xFF; }
inline uint32_t get_entry_id(uint32_t resid) { return resid & 0xFFFF; }

inline uint32_t fix_package_id(uint32_t resid, uint8_t new_package_id) {
    return (resid & 0x00FFFFFFU) | (static_cast<uint32_t>(new_package_id) << 24);
}

inline bool is_internal_resid(uint32_t resid) {
    return (resid & 0xFFFF0000U) == 0 && (resid & 0xFFU) != 0;
}

// AOSP reads DynamicRefTable::mAssignedPackageId directly (a public field
// there); our port keeps it private — read it back through the id-pair
// mapping that BuildDynamicRefTable records.
inline uint8_t assigned_id_of(const DynamicRefTable& table) {
    return table.getAssignedPackageId();
}

// std::variant<Res_value, verified_map_ptr<ResTable_map_entry>> — a tagged
// union is all the port needs (the C++14 seam).
struct EntryValue {
    bool is_map = false;
    Res_value value{};
    incfs::verified_map_ptr<ResTable_map_entry> map_entry;
};

base::expected<EntryValue, IOError> GetEntryValue(
    incfs::verified_map_ptr<ResTable_entry> table_entry) {
  const uint16_t entry_size = dtohs(table_entry->size);

  // Check if the entry represents a bag value.
  if (entry_size >= sizeof(ResTable_map_entry) &&
      (dtohs(table_entry->flags) & ResTable_entry::FLAG_COMPLEX)) {
    const auto map_entry = table_entry.convert<ResTable_map_entry>();
    if (!map_entry) {
      return base::unexpected<IOError>(IOError::PAGES_MISSING);
    }
    EntryValue out;
    out.is_map = true;
    out.map_entry = map_entry.verified();
    return out;
  }

  // The entry represents a non-bag value.
  const auto entry_value = table_entry.offset(entry_size).convert<Res_value>();
  if (!entry_value) {
    return base::unexpected<IOError>(IOError::PAGES_MISSING);
  }
  EntryValue out;
  out.value.copyFrom_dtoh(entry_value.value());
  return out;
}

} // namespace

struct FindEntryResult {
  // The cookie representing the ApkAssets in which the value resides.
  ApkAssetsCookie cookie;

  // The value of the resource table entry. Either an android::Res_value for non-bag types or an
  // incfs::verified_map_ptr<ResTable_map_entry> for bag types.
  EntryValue entry;

  // The configuration for which the resulting entry was defined. This is already swapped to host
  // endianness.
  ResTable_config config;

  // The bitmask of configuration axis with which the resource value varies.
  uint32_t type_flags;

  // The dynamic package ID map for the package from which this resource came from.
  const DynamicRefTable* dynamic_ref_table;

  // The package name of the resource.
  const std::string* package_name;

  // The string pool reference to the type's name. This uses a different string pool than
  // the global string pool, but this is hidden from the caller.
  StringPoolRef type_string_ref;

  // The string pool reference to the entry's name. This uses a different string pool than
  // the global string pool, but this is hidden from the caller.
  StringPoolRef entry_string_ref;
};

AssetManager2::AssetManager2() {
  memset(&configuration_, 0, sizeof(configuration_));
}

bool AssetManager2::SetApkAssets(std::vector<const ApkAssets*> apk_assets, bool invalidate_caches) {
  apk_assets_ = std::move(apk_assets);
  BuildDynamicRefTable();
  RebuildFilterList();
  if (invalidate_caches) {
    InvalidateCaches(static_cast<uint32_t>(-1));
  }
  return true;
}

void AssetManager2::BuildDynamicRefTable() {
  package_groups_.clear();
  package_ids_.fill(0xff);

  // The assets cookie must map to the position of the apk assets in the unsorted apk assets list.
  std::unordered_map<const ApkAssets*, ApkAssetsCookie> apk_assets_cookies;
  apk_assets_cookies.reserve(apk_assets_.size());
  for (size_t i = 0, n = apk_assets_.size(); i < n; i++) {
    apk_assets_cookies[apk_assets_[i]] = static_cast<ApkAssetsCookie>(i);
  }

  // 0x01 is reserved for the android package.
  int next_package_id = 0x02;
  for (const ApkAssets* apk_assets : apk_assets_) {
    const LoadedArsc* loaded_arsc = apk_assets->GetLoadedArsc();
    for (const std::unique_ptr<const LoadedPackage>& package : loaded_arsc->GetPackages()) {
      // Get the package ID or assign one if a shared library.
      int package_id;
      if (package->IsDynamic()) {
        package_id = next_package_id++;
      } else {
        package_id = package->GetPackageId();
      }

      uint8_t idx = package_ids_[package_id];
      if (idx == 0xff) {
        // Add the mapping for package ID to index if not present.
        package_ids_[package_id] = idx = static_cast<uint8_t>(package_groups_.size());
        package_groups_.emplace_back();
        PackageGroup& new_group = package_groups_.back();

        DynamicRefTable* ref_table = new_group.dynamic_ref_table.get();
        ref_table->setAssignedPackageId((uint8_t)package_id);   // AOSP assigns the field
        ref_table->addMapping((uint8_t)package_id, (uint8_t)package_id);
      }

      // Add the package and to the set of packages with the same ID.
      PackageGroup* package_group = &package_groups_[idx];
      package_group->packages_.push_back(ConfiguredPackage{package.get(), {}});
      package_group->cookies_.push_back(apk_assets_cookies[apk_assets]);

      // Add the package name -> build time ID mappings.
      for (const DynamicPackageEntry& entry : package->GetDynamicPackageMap()) {
        package_group->dynamic_ref_table->addMapping(entry.package_name,
                                                     static_cast<uint8_t>(entry.package_id));
      }
    }
  }
}

void AssetManager2::DumpToLog() const {
  LOGI("AssetManager2(this=%p)", this);

  for (const auto& package_group: package_groups_) {
    for (const auto& package : package_group.packages_) {
      const LoadedPackage* loaded_package = package.loaded_package_;
      LOGI("PG (%02x): %s(%02x%s)",
           package_group.dynamic_ref_table->entries().empty() ? 0 : 0,
           loaded_package->GetPackageName().c_str(),
           loaded_package->GetPackageId(),
           (loaded_package->IsDynamic() ? " dynamic" : ""));
    }
  }
}

const ResStringPool* AssetManager2::GetStringPoolForCookie(ApkAssetsCookie cookie) const {
  if (cookie < 0 || static_cast<size_t>(cookie) >= apk_assets_.size()) {
    return nullptr;
  }
  return apk_assets_[cookie]->GetLoadedArsc()->GetStringPool();
}

const DynamicRefTable* AssetManager2::GetDynamicRefTableForPackage(uint32_t package_id) const {
  if (package_id >= package_ids_.size()) {
    return nullptr;
  }

  const size_t idx = package_ids_[package_id];
  if (idx == 0xff) {
    return nullptr;
  }
  return package_groups_[idx].dynamic_ref_table.get();
}

std::shared_ptr<const DynamicRefTable> AssetManager2::GetDynamicRefTableForCookie(
    ApkAssetsCookie cookie) const {
  for (const PackageGroup& package_group : package_groups_) {
    for (const ApkAssetsCookie& package_cookie : package_group.cookies_) {
      if (package_cookie == cookie) {
        return package_group.dynamic_ref_table;
      }
    }
  }
  return nullptr;
}

const std::unordered_map<std::string, std::string>*
  AssetManager2::GetOverlayableMapForPackage(uint32_t package_id) const {

  if (package_id >= package_ids_.size()) {
    return nullptr;
  }

  const size_t idx = package_ids_[package_id];
  if (idx == 0xff) {
    return nullptr;
  }

  const PackageGroup& package_group = package_groups_[idx];
  if (package_group.packages_.empty()) {
    return nullptr;
  }

  const auto loaded_package = package_group.packages_[0].loaded_package_;
  return &loaded_package->GetOverlayableMap();
}

bool AssetManager2::ContainsAllocatedTable() const {
  return std::find_if(apk_assets_.begin(), apk_assets_.end(),
                      [](const ApkAssets* a) { return a->IsTableAllocated(); }) != apk_assets_.end();
}

void AssetManager2::SetConfiguration(const ResTable_config& configuration) {
  const int diff = configuration_.diff(configuration);
  configuration_ = configuration;

  if (diff) {
    RebuildFilterList();
    InvalidateCaches(static_cast<uint32_t>(diff));
  }
}

base::expected<std::set<ResTable_config>, IOError> AssetManager2::GetResourceConfigurations(
    bool exclude_system, bool exclude_mipmap) const {
  std::set<ResTable_config> configurations;
  for (const PackageGroup& package_group : package_groups_) {
    for (size_t i = 0; i < package_group.packages_.size(); i++) {
      const ConfiguredPackage& package = package_group.packages_[i];
      if (exclude_system && package.loaded_package_->IsSystem()) {
        continue;
      }

      auto result = package.loaded_package_->CollectConfigurations(exclude_mipmap, &configurations);
      if (!result.has_value()) {
        return base::unexpected<IOError>(result.error());
      }
    }
  }
  return configurations;
}

std::set<std::string> AssetManager2::GetResourceLocales(bool exclude_system,
                                                        bool merge_equivalent_languages) const {
  std::set<std::string> locales;

  for (const PackageGroup& package_group : package_groups_) {
    for (size_t i = 0; i < package_group.packages_.size(); i++) {
      const ConfiguredPackage& package = package_group.packages_[i];
      if (exclude_system && package.loaded_package_->IsSystem()) {
        continue;
      }

      package.loaded_package_->CollectLocales(merge_equivalent_languages, &locales);
    }
  }
  return locales;
}

std::unique_ptr<Asset> AssetManager2::Open(const std::string& filename,
                                           Asset::AccessMode mode) const {
  const std::string new_path = "assets/" + filename;
  return OpenNonAsset(new_path, mode);
}

std::unique_ptr<Asset> AssetManager2::Open(const std::string& filename, ApkAssetsCookie cookie,
                                           Asset::AccessMode mode) const {
  const std::string new_path = "assets/" + filename;
  return OpenNonAsset(new_path, cookie, mode);
}

// Search in reverse because that's how we used to do it and we need to preserve behaviour.
// This is unfortunate, because ClassLoaders delegate to the parent first, so the order
// is inconsistent for split APKs.
std::unique_ptr<Asset> AssetManager2::OpenNonAsset(const std::string& filename,
                                                   Asset::AccessMode mode,
                                                   ApkAssetsCookie* out_cookie) const {
  for (int32_t i = (int32_t)apk_assets_.size() - 1; i >= 0; i--) {
    std::unique_ptr<Asset> asset = apk_assets_[i]->GetAssetsProvider()->Open(filename, mode);
    if (asset) {
      if (out_cookie != nullptr) {
        *out_cookie = i;
      }
      return asset;
    }
  }

  if (out_cookie != nullptr) {
    *out_cookie = kInvalidCookie;
  }
  return {};
}

std::unique_ptr<Asset> AssetManager2::OpenNonAsset(const std::string& filename,
                                                   ApkAssetsCookie cookie,
                                                   Asset::AccessMode mode) const {
  if (cookie < 0 || static_cast<size_t>(cookie) >= apk_assets_.size()) {
    return {};
  }
  return apk_assets_[cookie]->GetAssetsProvider()->Open(filename, mode);
}

base::expected<FindEntryResult, NullOrIOError> AssetManager2::FindEntry(
    uint32_t resid, uint16_t density_override, bool stop_at_first_match,
    bool ignore_configuration) const {
  const bool logging_enabled = resource_resolution_logging_enabled_;
  if (logging_enabled) {
    // Clear the last logged resource resolution.
    ResetResourceResolution();
    last_resolution_.resid = resid;
  }

  // Might use this if density_override != 0.
  ResTable_config density_override_config;

  // Select our configuration or generate a density override configuration.
  const ResTable_config* desired_config = &configuration_;
  if (density_override != 0 && density_override != configuration_.density) {
    density_override_config = configuration_;
    density_override_config.density = density_override;
    desired_config = &density_override_config;
  }

  // Retrieve the package group from the package id of the resource id.
  if (!is_valid_resid(resid)) {
    LOGE("Invalid ID 0x%08x.", resid);
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  const uint32_t package_id = get_package_id(resid);
  const uint8_t type_idx = get_type_id(resid) - 1;
  const uint16_t entry_idx = get_entry_id(resid);
  uint8_t package_idx = package_ids_[package_id];
  if (package_idx == 0xff) {
    LOGE("No package ID %02x found for ID 0x%08x.", package_id, resid);
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  const PackageGroup& package_group = package_groups_[package_idx];
  auto result = FindEntryInternal(package_group, type_idx, entry_idx, *desired_config,
                                  stop_at_first_match, ignore_configuration);
  if (!result.has_value()) {
    return base::unexpected<NullOrIOError>(result.error());
  }

  if (logging_enabled) {
    last_resolution_.cookie = result.value().cookie;
    last_resolution_.type_string_ref = result.value().type_string_ref;
    last_resolution_.entry_string_ref = result.value().entry_string_ref;
  }

  return result;
}

base::expected<FindEntryResult, NullOrIOError> AssetManager2::FindEntryInternal(
    const PackageGroup& package_group, uint8_t type_idx, uint16_t entry_idx,
    const ResTable_config& desired_config, bool stop_at_first_match,
    bool ignore_configuration) const {
  const bool logging_enabled = resource_resolution_logging_enabled_;
  ApkAssetsCookie best_cookie = kInvalidCookie;
  const LoadedPackage* best_package = nullptr;
  incfs::verified_map_ptr<ResTable_type> best_type;
  const ResTable_config* best_config = nullptr;
  uint32_t best_offset = 0U;
  uint32_t type_flags = 0U;

  std::vector<Resolution::Step> resolution_steps;

  // If `desired_config` is not the same as the set configuration or the caller will accept a value
  // from any configuration, then we cannot use our filtered list of types since it only it contains
  // types matched to the set configuration.
  const bool use_filtered = !ignore_configuration && &desired_config == &configuration_;

  const size_t package_count = package_group.packages_.size();
  for (size_t pi = 0; pi < package_count; pi++) {
    const ConfiguredPackage& loaded_package_impl = package_group.packages_[pi];
    const LoadedPackage* loaded_package = loaded_package_impl.loaded_package_;
    const ApkAssetsCookie cookie = package_group.cookies_[pi];

    // If the type IDs are offset in this package, we need to take that into account when searching
    // for a type.
    const TypeSpec* type_spec = loaded_package->GetTypeSpecByTypeIndex(type_idx);
    if (type_spec == nullptr) {
      continue;
    }

    // Allow custom loader packages to overlay resource values with configurations equivalent to the
    // current best configuration.
    const bool package_is_loader = loaded_package->IsCustomLoader();

    type_flags |= type_spec->GetFlagsForEntryIndex(entry_idx);

    const FilteredConfigGroup& filtered_group = loaded_package_impl.filtered_configs_[type_idx];
    const size_t type_entry_count = (use_filtered) ? filtered_group.type_entries.size()
                                                   : type_spec->type_entries.size();
    for (size_t i = 0; i < type_entry_count; i++) {
      const TypeSpec::TypeEntry* type_entry = (use_filtered) ? filtered_group.type_entries[i]
                                                             : &type_spec->type_entries[i];

      // We can skip calling ResTable_config::match() if the caller does not care for the
      // configuration to match or if we're using the list of types that have already had their
      // configuration matched.
      const ResTable_config& this_config = type_entry->config;
      if (!(use_filtered || ignore_configuration || this_config.match(desired_config))) {
        continue;
      }

      Resolution::Step::Type resolution_type;
      if (best_config == nullptr) {
        resolution_type = Resolution::Step::Type::INITIAL;
      } else if (this_config.isBetterThan(*best_config, &desired_config)) {
        resolution_type = Resolution::Step::Type::BETTER_MATCH;
      } else if (package_is_loader && this_config.compare(*best_config) == 0) {
        resolution_type = Resolution::Step::Type::OVERLAID;
      } else {
        if (logging_enabled) {
          resolution_steps.push_back(Resolution::Step{Resolution::Step::Type::SKIPPED,
                                                      this_config.toString(),
                                                      cookie});
        }
        continue;
      }

      // The configuration matches and is better than the previous selection.
      // Find the entry value if it exists for this configuration.
      const auto& type = type_entry->type;
      const auto offset = LoadedPackage::GetEntryOffset(type, entry_idx);
      if (!offset.has_value()) {
        if (logging_enabled) {
          resolution_steps.push_back(Resolution::Step{Resolution::Step::Type::NO_ENTRY,
                                                      this_config.toString(),
                                                      cookie});
        }
        continue;
      }

      best_cookie = cookie;
      best_package = loaded_package;
      best_type = type;
      best_config = &this_config;
      best_offset = offset.value();

      if (logging_enabled) {
        last_resolution_.steps.push_back(Resolution::Step{resolution_type,
                                                          this_config.toString(),
                                                          cookie});
      }

      // Any configuration will suffice, so break.
      if (stop_at_first_match) {
        break;
      }
    }
  }

  if (logging_enabled) {
    // Append the deferred steps (the AOSP appends into last_resolution_ inline;
    // the local vector keeps the skipped/no-entry steps ordered before the
    // accepted ones of the same iteration).
    for (auto& step : resolution_steps) {
      last_resolution_.steps.push_back(std::move(step));
    }
  }

  if (best_cookie == kInvalidCookie) {
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  auto best_entry_result = LoadedPackage::GetEntryFromOffset(best_type, best_offset);
  if (!best_entry_result.has_value()) {
    return base::unexpected<NullOrIOError>(best_entry_result.error());
  }

  const incfs::map_ptr<ResTable_entry> best_entry = *best_entry_result;
  if (!best_entry) {
    return base::unexpected<NullOrIOError>(IOError::PAGES_MISSING);
  }

  const auto entry = GetEntryValue(best_entry.verified());
  if (!entry.has_value()) {
    return base::unexpected<NullOrIOError>(entry.error());
  }

  FindEntryResult out;
  out.cookie = best_cookie;
  out.entry = *entry;
  out.config = *best_config;
  out.type_flags = type_flags;
  out.package_name = &best_package->GetPackageName();
  out.type_string_ref = StringPoolRef(best_package->GetTypeStringPool(), best_type->id - 1);
  out.entry_string_ref = StringPoolRef(best_package->GetKeyStringPool(),
                                       best_entry->key.index);
  out.dynamic_ref_table = package_group.dynamic_ref_table.get();
  return out;
}

void AssetManager2::ResetResourceResolution() const {
  last_resolution_.cookie = kInvalidCookie;
  last_resolution_.resid = 0;
  last_resolution_.steps.clear();
  last_resolution_.type_string_ref = StringPoolRef();
  last_resolution_.entry_string_ref = StringPoolRef();
}

void AssetManager2::SetResourceResolutionLoggingEnabled(bool enabled) {
  resource_resolution_logging_enabled_ = enabled;
  if (!enabled) {
    ResetResourceResolution();
  }
}

std::string AssetManager2::GetLastResourceResolution() const {
  if (!resource_resolution_logging_enabled_) {
    LOGE("Must enable resource resolution logging before getting path.");
    return {};
  }

  const ApkAssetsCookie cookie = last_resolution_.cookie;
  if (cookie == kInvalidCookie) {
    LOGE("AssetManager hasn't resolved a resource to read resolution path.");
    return {};
  }

  const uint32_t resid = last_resolution_.resid;
  const auto package = apk_assets_[cookie]->GetLoadedArsc()->GetPackageById(get_package_id(resid));

  std::string resource_name_string = "<unknown>";
  if (package != nullptr) {
    size_t tlen = 0, elen = 0;
    const char* type8 = last_resolution_.type_string_ref.string8(&tlen);
    const char* entry8 = last_resolution_.entry_string_ref.string8(&elen);
    if (type8 != nullptr && entry8 != nullptr) {
      resource_name_string = package->GetPackageName() + ":" + std::string(type8, tlen) + "/" +
                             std::string(entry8, elen);
    }
  }

  std::stringstream log_stream;
  log_stream << "Resolution for 0x" << std::hex << resid << std::dec << " "
             << resource_name_string << "\n\tFor config - " << configuration_.toString();

  for (const Resolution::Step& step : last_resolution_.steps) {
    static const std::unordered_map<int, const char*> kStepStrings = {
        {(int)Resolution::Step::Type::INITIAL,         "Found initial"},
        {(int)Resolution::Step::Type::BETTER_MATCH,    "Found better"},
        {(int)Resolution::Step::Type::OVERLAID,        "Overlaid"},
        {(int)Resolution::Step::Type::OVERLAID_INLINE, "Overlaid inline"},
        {(int)Resolution::Step::Type::SKIPPED,         "Skipped"},
        {(int)Resolution::Step::Type::NO_ENTRY,        "No entry"}
    };

    const auto prefix = kStepStrings.find((int)step.type);
    if (prefix == kStepStrings.end()) {
      continue;
    }

    log_stream << "\n\t" << prefix->second << ": " << apk_assets_[step.cookie]->GetDebugName();
    if (!step.config_name.empty()) {
      log_stream << " - " << step.config_name;
    }
  }

  return log_stream.str();
}

namespace {

// ToResourceName (ResourceUtils.cpp): prefer the UTF-8 faces, fall back to
// UTF-16, into the ResourceName out-param.
AssetManager2::ResourceName ToResourceName(const StringPoolRef& type_ref,
                                           const StringPoolRef& key_ref,
                                           const std::string& package) {
  AssetManager2::ResourceName out;
  out.package = package.data();
  out.package_len = package.size();

  size_t len = 0;
  if (const char* type8 = type_ref.string8(&len)) {
    out.type = type8;
    out.type_len = len;
  } else if (const char16_t* type16 = type_ref.string16(&len)) {
    out.type16 = type16;
    out.type_len = len;
  }

  if (const char* entry8 = key_ref.string8(&len)) {
    out.entry = entry8;
    out.entry_len = len;
  } else if (const char16_t* entry16 = key_ref.string16(&len)) {
    out.entry16 = entry16;
    out.entry_len = len;
  }

  return out;
}

// ExtractResourceName (ResourceUtils.cpp): '[package:][type/]entry'.
bool ExtractResourceName(const std::string& name,
                         std::string* out_package, std::string* out_type, std::string* out_entry) {
  size_t entry_start = 0;
  const size_t colon = name.find(':');
  if (colon != std::string::npos) {
    *out_package = name.substr(0, colon);
    entry_start = colon + 1;
  }

  const size_t slash = name.find('/', entry_start);
  if (slash != std::string::npos) {
    *out_type = name.substr(entry_start, slash - entry_start);
    entry_start = slash + 1;
  }

  *out_entry = name.substr(entry_start);
  return !out_entry->empty();
}

bool Utf8ToUtf16(const std::string& str, std::u16string* out) {
  out->clear();
  out->reserve(str.size());
  for (size_t i = 0; i < str.size(); ) {
    const unsigned char c = (unsigned char)str[i];
    uint32_t cp = 0;
    size_t len = 1;
    if (c < 0x80) {
      cp = c;
    } else if ((c & 0xE0) == 0xC0 && i + 1 < str.size()) {
      cp = c & 0x1F; len = 2;
    } else if ((c & 0xF0) == 0xE0 && i + 2 < str.size()) {
      cp = c & 0x0F; len = 3;
    } else if ((c & 0xF8) == 0xF0 && i + 3 < str.size()) {
      cp = c & 0x07; len = 4;
    } else {
      return false;   // malformed
    }
    for (size_t k = 1; k < len; k++) {
      const unsigned char cc = (unsigned char)str[i + k];
      if ((cc & 0xC0) != 0x80) return false;
      cp = (cp << 6) | (cc & 0x3F);
    }
    i += len;

    if (cp <= 0xFFFF) {
      out->push_back((char16_t)cp);
    } else {
      cp -= 0x10000;
      out->push_back((char16_t)(0xD800 | (cp >> 10)));
      out->push_back((char16_t)(0xDC00 | (cp & 0x3FF)));
    }
  }
  return true;
}

}  // namespace

base::expected<AssetManager2::ResourceName, NullOrIOError> AssetManager2::GetResourceName(
    uint32_t resid) const {
  auto result = FindEntry(resid, 0u /* density_override */, true /* stop_at_first_match */,
                          true /* ignore_configuration */);
  if (!result.has_value()) {
    return base::unexpected<NullOrIOError>(result.error());
  }

  return ToResourceName(result->type_string_ref,
                        result->entry_string_ref,
                        *result->package_name);
}

base::expected<uint32_t, NullOrIOError> AssetManager2::GetResourceTypeSpecFlags(
    uint32_t resid) const {
  auto result = FindEntry(resid, 0u /* density_override */, false /* stop_at_first_match */,
                          true /* ignore_configuration */);
  if (!result.has_value()) {
    return base::unexpected<NullOrIOError>(result.error());
  }
  return result->type_flags;
}

base::expected<AssetManager2::SelectedValue, NullOrIOError> AssetManager2::GetResource(
    uint32_t resid, bool may_be_bag, uint16_t density_override) const {
  auto result = FindEntry(resid, density_override, false /* stop_at_first_match */,
                          false /* ignore_configuration */);
  if (!result.has_value()) {
    return base::unexpected<NullOrIOError>(result.error());
  }

  if (result->entry.is_map) {
    if (!may_be_bag) {
      LOGE("Resource %08x is a complex map type.", resid);
      return base::unexpected<NullOrIOError>(base::nullopt);
    }

    // Create a reference since we can't represent this complex type as a Res_value.
    return SelectedValue(Res_value::TYPE_REFERENCE, resid, result->cookie, result->type_flags,
                         resid, result->config);
  }

  // Convert the package ID to the runtime assigned package ID.
  Res_value value = result->entry.value;
  result->dynamic_ref_table->lookupResourceValue(&value);

  return SelectedValue(value.dataType, value.data, result->cookie, result->type_flags,
                       resid, result->config);
}

base::expected<base::monostate, NullOrIOError> AssetManager2::ResolveReference(
    AssetManager2::SelectedValue& value, bool cache_value) const {
  if (value.type != Res_value::TYPE_REFERENCE || value.data == 0U) {
    // Not a reference. Nothing to do.
    return base::expected<base::monostate, NullOrIOError>();
  }

  const uint32_t original_flags = value.flags;
  const uint32_t original_resid = value.data;
  if (cache_value) {
    auto cached_value = cached_resolved_values_.find(value.data);
    if (cached_value != cached_resolved_values_.end()) {
      value = cached_value->second;
      value.flags |= original_flags;
      return base::expected<base::monostate, NullOrIOError>();
    }
  }

  uint32_t combined_flags = 0U;
  uint32_t resolve_resid = original_resid;
  constexpr const uint32_t kMaxIterations = 20;
  for (uint32_t i = 0U;; i++) {
    auto result = GetResource(resolve_resid, true /*may_be_bag*/);
    if (!result.has_value()) {
      value.resid = resolve_resid;
      return base::unexpected<NullOrIOError>(result.error());
    }

    // If resource resolution fails, the value should be set to the last reference that was able to
    // be resolved successfully.
    value = *result;
    value.flags |= combined_flags;

    if (result->type != Res_value::TYPE_REFERENCE ||
        result->data == Res_value::DATA_NULL_UNDEFINED ||
        result->data == resolve_resid || i == kMaxIterations) {
      // This reference can't be resolved, so exit now and let the caller deal with it.
      if (cache_value) {
        cached_resolved_values_[original_resid] = value;
      }

      // Above value is cached without original_flags to ensure they don't get included in future
      // queries that hit the cache
      value.flags |= original_flags;
      return base::expected<base::monostate, NullOrIOError>();
    }

    combined_flags = result->flags;
    resolve_resid = result->data;
  }
}

const std::vector<uint32_t> AssetManager2::GetBagResIdStack(uint32_t resid) const {
  auto cached_iter = cached_bag_resid_stacks_.find(resid);
  if (cached_iter != cached_bag_resid_stacks_.end()) {
    return cached_iter->second;
  }

  std::vector<uint32_t> found_resids;
  GetBag(resid, found_resids);
  cached_bag_resid_stacks_.emplace(resid, found_resids);
  return found_resids;
}

base::expected<const ResolvedBag*, NullOrIOError> AssetManager2::ResolveBag(
    AssetManager2::SelectedValue& value) const {
  if (value.type != Res_value::TYPE_REFERENCE) {
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  auto bag = GetBag(value.data);
  if (bag.has_value()) {
    value.flags |= (*bag)->type_spec_flags;
  }
  return bag;
}

base::expected<const ResolvedBag*, NullOrIOError> AssetManager2::GetBag(uint32_t resid) const {
  std::vector<uint32_t> found_resids;
  const auto bag = GetBag(resid, found_resids);
  cached_bag_resid_stacks_.emplace(resid, found_resids);
  return bag;
}

base::expected<const ResolvedBag*, NullOrIOError> AssetManager2::GetBag(
    uint32_t resid, std::vector<uint32_t>& child_resids) const {
  auto cached_iter = cached_bags_.find(resid);
  if (cached_iter != cached_bags_.end()) {
    return cached_iter->second.get();
  }

  auto entry = FindEntry(resid, 0u /* density_override */, false /* stop_at_first_match */,
                         false /* ignore_configuration */);
  if (!entry.has_value()) {
    return base::unexpected<NullOrIOError>(entry.error());
  }

  if (!entry->entry.is_map) {
    // Not a bag, nothing to do.
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  auto map = entry->entry.map_entry;
  auto map_entry = map.offset(dtohs(map->size)).convert<ResTable_map>();
  const auto map_entry_end = map_entry + dtohl(map->count);

  // Keep track of ids that have already been seen to prevent infinite loops caused by circular
  // dependencies between bags.
  child_resids.push_back(resid);

  uint32_t parent_resid = dtohl(map->parent.ident);
  if (parent_resid == 0U ||
      std::find(child_resids.begin(), child_resids.end(), parent_resid) != child_resids.end()) {
    // There is no parent or a circular parental dependency exist, meaning there is nothing to
    // inherit and we can do a simple copy of the entries in the map.
    const size_t entry_count = map_entry_end - map_entry;
    ResolvedBagPtr new_bag{reinterpret_cast<ResolvedBag*>(
        malloc(sizeof(ResolvedBag) + (entry_count * sizeof(ResolvedBag::Entry))))};

    bool sort_entries = false;
    for (auto new_entry = new_bag->entries; map_entry != map_entry_end; ++map_entry) {
      if (!map_entry) {
        return base::unexpected<NullOrIOError>(IOError::PAGES_MISSING);
      }

      uint32_t new_key = dtohl(map_entry->name.ident);
      if (!is_internal_resid(new_key)) {
        // Attributes, arrays, etc don't have a resource id as the name. They specify
        // other data, which would be wrong to change via a lookup.
        if (entry->dynamic_ref_table->lookupResourceId(&new_key) != NO_ERROR) {
          LOGE("Failed to resolve key 0x%08x in bag 0x%08x.", new_key, resid);
          return base::unexpected<NullOrIOError>(base::nullopt);
        }
      }

      new_entry->cookie = entry->cookie;
      new_entry->key = new_key;
      new_entry->key_pool = nullptr;
      new_entry->type_pool = nullptr;
      new_entry->style = resid;
      new_entry->value.copyFrom_dtoh(map_entry->value);
      const status_t err = entry->dynamic_ref_table->lookupResourceValue(&new_entry->value);
      if (err != NO_ERROR) {
        LOGE("Failed to resolve value t=0x%02x d=0x%08x for key 0x%08x.",
             new_entry->value.dataType, new_entry->value.data, new_key);
        return base::unexpected<NullOrIOError>(base::nullopt);
      }

      sort_entries = sort_entries ||
          (new_entry != new_bag->entries && (new_entry->key < (new_entry - 1U)->key));
      ++new_entry;
    }

    if (sort_entries) {
      std::sort(new_bag->entries, new_bag->entries + entry_count,
                [](const ResolvedBag::Entry& lhs, const ResolvedBag::Entry& rhs) {
                    return lhs.key < rhs.key; });
    }

    new_bag->type_spec_flags = entry->type_flags;
    new_bag->entry_count = static_cast<uint32_t>(entry_count);
    ResolvedBag* result = new_bag.get();
    cached_bags_[resid] = std::move(new_bag);
    return result;
  }

  // In case the parent is a dynamic reference, resolve it.
  entry->dynamic_ref_table->lookupResourceId(&parent_resid);

  // Get the parent and do a merge of the keys.
  const auto parent_bag = GetBag(parent_resid, child_resids);
  if (!parent_bag.has_value()) {
    // Failed to get the parent that should exist.
    LOGE("Failed to find parent 0x%08x of bag 0x%08x.", parent_resid, resid);
    return base::unexpected<NullOrIOError>(parent_bag.error());
  }

  // Create the max possible entries we can make. Once we construct the bag,
  // we will realloc to fit to size.
  const size_t max_count = (*parent_bag)->entry_count + dtohl(map->count);
  ResolvedBagPtr new_bag{reinterpret_cast<ResolvedBag*>(
      malloc(sizeof(ResolvedBag) + (max_count * sizeof(ResolvedBag::Entry))))};
  ResolvedBag::Entry* new_entry = new_bag->entries;

  const ResolvedBag::Entry* parent_entry = (*parent_bag)->entries;
  const ResolvedBag::Entry* const parent_entry_end = parent_entry + (*parent_bag)->entry_count;

  // The keys are expected to be in sorted order. Merge the two bags.
  bool sort_entries = false;
  while (map_entry != map_entry_end && parent_entry != parent_entry_end) {
    if (!map_entry) {
      return base::unexpected<NullOrIOError>(IOError::PAGES_MISSING);
    }

    uint32_t child_key = dtohl(map_entry->name.ident);
    if (!is_internal_resid(child_key)) {
      if (entry->dynamic_ref_table->lookupResourceId(&child_key) != NO_ERROR) {
        LOGE("Failed to resolve key 0x%08x in bag 0x%08x.", child_key, resid);
        return base::unexpected<NullOrIOError>(base::nullopt);
      }
    }

    if (child_key <= parent_entry->key) {
      // Use the child key if it comes before the parent
      // or is equal to the parent (overrides).
      new_entry->cookie = entry->cookie;
      new_entry->key = child_key;
      new_entry->key_pool = nullptr;
      new_entry->type_pool = nullptr;
      new_entry->value.copyFrom_dtoh(map_entry->value);
      new_entry->style = resid;
      const status_t err = entry->dynamic_ref_table->lookupResourceValue(&new_entry->value);
      if (err != NO_ERROR) {
        LOGE("Failed to resolve value t=0x%02x d=0x%08x for key 0x%08x.",
             new_entry->value.dataType, new_entry->value.data, child_key);
        return base::unexpected<NullOrIOError>(base::nullopt);
      }
      ++map_entry;
    } else {
      // Take the parent entry as-is.
      memcpy(new_entry, parent_entry, sizeof(*new_entry));
    }

    sort_entries = sort_entries ||
        (new_entry != new_bag->entries && (new_entry->key < (new_entry - 1U)->key));
    if (child_key >= parent_entry->key) {
      // Move to the next parent entry if we used it or it was overridden.
      ++parent_entry;
    }
    // Increment to the next entry to fill.
    ++new_entry;
  }

  // Finish the child entries if they exist.
  while (map_entry != map_entry_end) {
    if (!map_entry) {
      return base::unexpected<NullOrIOError>(IOError::PAGES_MISSING);
    }

    uint32_t new_key = dtohl(map_entry->name.ident);
    if (!is_internal_resid(new_key)) {
      if (entry->dynamic_ref_table->lookupResourceId(&new_key) != NO_ERROR) {
        LOGE("Failed to resolve key 0x%08x in bag 0x%08x.", new_key, resid);
        return base::unexpected<NullOrIOError>(base::nullopt);
      }
    }
    new_entry->cookie = entry->cookie;
    new_entry->key = new_key;
    new_entry->key_pool = nullptr;
    new_entry->type_pool = nullptr;
    new_entry->value.copyFrom_dtoh(map_entry->value);
    new_entry->style = resid;
    const status_t err = entry->dynamic_ref_table->lookupResourceValue(&new_entry->value);
    if (err != NO_ERROR) {
      LOGE("Failed to resolve value t=0x%02x d=0x%08x for key 0x%08x.",
           new_entry->value.dataType, new_entry->value.data, new_key);
      return base::unexpected<NullOrIOError>(base::nullopt);
    }
    sort_entries = sort_entries ||
        (new_entry != new_bag->entries && (new_entry->key < (new_entry - 1U)->key));
    ++map_entry;
    ++new_entry;
  }

  // Finish the parent entries if they exist.
  if (parent_entry != parent_entry_end) {
    // Take the rest of the parent entries as-is.
    const size_t num_entries_to_copy = parent_entry_end - parent_entry;
    memcpy(new_entry, parent_entry, num_entries_to_copy * sizeof(*new_entry));
    new_entry += num_entries_to_copy;
  }

  // Resize the resulting array to fit.
  const size_t actual_count = new_entry - new_bag->entries;
  if (actual_count != max_count) {
    new_bag.reset(reinterpret_cast<ResolvedBag*>(realloc(
        new_bag.release(), sizeof(ResolvedBag) + (actual_count * sizeof(ResolvedBag::Entry)))));
  }

  if (sort_entries) {
    std::sort(new_bag->entries, new_bag->entries + actual_count,
              [](const ResolvedBag::Entry& lhs, const ResolvedBag::Entry& rhs) {
                  return lhs.key < rhs.key; });
  }

  // Combine flags from the parent and our own bag.
  new_bag->type_spec_flags = entry->type_flags | (*parent_bag)->type_spec_flags;
  new_bag->entry_count = static_cast<uint32_t>(actual_count);
  ResolvedBag* result = new_bag.get();
  cached_bags_[resid] = std::move(new_bag);
  return result;
}

base::expected<uint32_t, NullOrIOError> AssetManager2::GetResourceId(
    const std::string& resource_name, const std::string& fallback_type,
    const std::string& fallback_package) const {
  std::string package_name, type, entry;
  if (!ExtractResourceName(resource_name, &package_name, &type, &entry)) {
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  if (entry.empty()) {
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  if (package_name.empty()) {
    package_name = fallback_package;
  }

  if (type.empty()) {
    type = fallback_type;
  }

  std::u16string type16;
  if (!Utf8ToUtf16(type, &type16)) {
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  std::u16string entry16;
  if (!Utf8ToUtf16(entry, &entry16)) {
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  const std::u16string kAttr16 = u"attr";
  const std::u16string kAttrPrivate16 = u"^attr-private";

  for (const PackageGroup& package_group : package_groups_) {
    for (const ConfiguredPackage& package_impl : package_group.packages_) {
      const LoadedPackage* package = package_impl.loaded_package_;
      if (package_name != package->GetPackageName()) {
        // All packages in the same group are expected to have the same package name.
        break;
      }

      base::expected<uint32_t, NullOrIOError> resid = package->FindEntryByName(type16, entry16);

      if (!resid.has_value() && kAttr16 == type16) {
        // Private attributes in libraries (such as the framework) are sometimes encoded
        // under the type '^attr-private' in order to leave the ID space of public 'attr'
        // free for future additions. Check '^attr-private' for the same name.
        resid = package->FindEntryByName(kAttrPrivate16, entry16);
      }

      if (resid.has_value()) {
        return fix_package_id(*resid, assigned_id_of(*package_group.dynamic_ref_table));
      }
    }
  }
  return base::unexpected<NullOrIOError>(base::nullopt);
}

void AssetManager2::RebuildFilterList() {
  for (PackageGroup& group : package_groups_) {
    for (ConfiguredPackage& impl : group.packages_) {
      // Destroy it.
      impl.filtered_configs_.~ByteBucketArray<FilteredConfigGroup>();

      // Re-create it.
      new (&impl.filtered_configs_) ByteBucketArray<FilteredConfigGroup>();

      // Create the filters here.
      impl.loaded_package_->ForEachTypeSpec([&](const TypeSpec& type_spec, uint8_t type_id) {
        FilteredConfigGroup& cfg_group = impl.filtered_configs_.editItemAt(type_id - 1);
        for (const auto& type_entry : type_spec.type_entries) {
          if (type_entry.config.match(configuration_)) {
            cfg_group.type_entries.push_back(&type_entry);
          }
        }
      });
    }
  }
}

void AssetManager2::InvalidateCaches(uint32_t diff) {
  cached_bag_resid_stacks_.clear();

  if (diff == 0xffffffffu) {
    // Everything must go.
    cached_bags_.clear();
    return;
  }

  // Be more conservative with what gets purged. Only if the bag has other possible
  // variations with respect to what changed (diff) should we remove it.
  for (auto iter = cached_bags_.cbegin(); iter != cached_bags_.cend();) {
    if (diff & iter->second->type_spec_flags) {
      iter = cached_bags_.erase(iter);
    } else {
      ++iter;
    }
  }

  cached_resolved_values_.clear();
}

uint8_t AssetManager2::GetAssignedPackageId(const LoadedPackage* package) const {
  for (auto& package_group : package_groups_) {
    for (auto& package2 : package_group.packages_) {
      if (package2.loaded_package_ == package) {
        return assigned_id_of(*package_group.dynamic_ref_table);
      }
    }
  }
  return 0;
}

std::unique_ptr<Theme> AssetManager2::NewTheme() {
  constexpr size_t kInitialReserveSize = 32;
  auto theme = std::unique_ptr<Theme>(new Theme(this));
  theme->entries_.reserve(kInitialReserveSize);
  return theme;
}

// CDROID seam: the themed-cache identity source (process-monotonic).
static uint32_t NextThemeGeneration() {
  static uint32_t sGeneration = 0;
  return ++sGeneration;
}

Theme::Theme(AssetManager2* asset_manager) : asset_manager_(asset_manager) {
  cache_generation_ = NextThemeGeneration();
}

Theme::~Theme() = default;

struct Theme::Entry {
  uint32_t attr_res_id;
  ApkAssetsCookie cookie;
  uint32_t type_spec_flags;
  Res_value value;
};

namespace {
struct ThemeEntryKeyComparer {
  bool operator() (const Theme::Entry& entry, uint32_t attr_res_id) const noexcept {
    return entry.attr_res_id < attr_res_id;
  }
};
} // namespace

base::expected<base::monostate, NullOrIOError> Theme::ApplyStyle(uint32_t resid, bool force) {
  cache_generation_ = NextThemeGeneration();
  auto bag = asset_manager_->GetBag(resid);
  if (!bag.has_value()) {
    return base::unexpected<NullOrIOError>(bag.error());
  }

  // Merge the flags from this style.
  type_spec_flags_ |= (*bag)->type_spec_flags;

  for (auto it = begin(*bag); it != end(*bag); ++it) {
    const uint32_t attr_res_id = it->key;

    // If the resource ID passed in is not a style, the key can be some other identifier that is not
    // a resource ID. We should fail fast instead of operating with strange resource IDs.
    if (!is_valid_resid(attr_res_id)) {
      return base::unexpected<NullOrIOError>(base::nullopt);
    }

    // DATA_NULL_EMPTY (@empty) is a valid resource value and DATA_NULL_UNDEFINED represents
    // an absence of a valid value.
    bool is_undefined = it->value.dataType == Res_value::TYPE_NULL &&
        it->value.data != Res_value::DATA_NULL_EMPTY;
    if (!force && is_undefined) {
      continue;
    }

    Theme::Entry new_entry{attr_res_id, it->cookie, (*bag)->type_spec_flags, it->value};
    auto entry_it = std::lower_bound(entries_.begin(), entries_.end(), attr_res_id,
                                     ThemeEntryKeyComparer{});
    if (entry_it != entries_.end() && entry_it->attr_res_id == attr_res_id) {
      if (is_undefined) {
        // DATA_NULL_UNDEFINED clears the value of the attribute in the theme only when `force` is
        /// true.
        entries_.erase(entry_it);
      } else if (force) {
        *entry_it = new_entry;
      }
    } else {
      entries_.insert(entry_it, new_entry);
    }
  }
  return base::expected<base::monostate, NullOrIOError>();
}

void Theme::Rebase(AssetManager2* am, const uint32_t* style_ids, const uint8_t* force,
                   size_t style_count) {
  // Reset the entries without changing the vector capacity to prevent reallocations during
  // ApplyStyle.
  entries_.clear();
  asset_manager_ = am;
  cache_generation_ = NextThemeGeneration();
  for (size_t i = 0; i < style_count; i++) {
    ApplyStyle(style_ids[i], force[i]);
  }
}

base::optional<AssetManager2::SelectedValue> Theme::GetAttribute(uint32_t resid) const {
  constexpr const uint32_t kMaxIterations = 20;
  uint32_t type_spec_flags = 0u;
  for (uint32_t i = 0; i <= kMaxIterations; i++) {
    auto entry_it = std::lower_bound(entries_.begin(), entries_.end(), resid,
                                     ThemeEntryKeyComparer{});
    if (entry_it == entries_.end() || entry_it->attr_res_id != resid) {
      return base::nullopt;
    }

    type_spec_flags |= entry_it->type_spec_flags;
    if (entry_it->value.dataType == Res_value::TYPE_ATTRIBUTE) {
      resid = entry_it->value.data;
      continue;
    }

    return AssetManager2::SelectedValue(entry_it->value.dataType, entry_it->value.data,
                                        entry_it->cookie, type_spec_flags, 0U /* resid */,
                                        ResTable_config{} /* config */);
  }
  return base::nullopt;
}

base::expected<base::monostate, NullOrIOError> Theme::ResolveAttributeReference(
      AssetManager2::SelectedValue& value) const {
  if (value.type != Res_value::TYPE_ATTRIBUTE) {
    return asset_manager_->ResolveReference(value);
  }

  base::optional<AssetManager2::SelectedValue> result = GetAttribute(value.data);
  if (!result.has_value()) {
    return base::unexpected<NullOrIOError>(base::nullopt);
  }

  auto resolve_result = asset_manager_->ResolveReference(*result, true /* cache_value */);
  if (resolve_result.has_value()) {
    result->flags |= value.flags;
    value = *result;
  }
  return resolve_result;
}

void Theme::Clear() {
  entries_.clear();
  cache_generation_ = NextThemeGeneration();
}

void Theme::GetAllAttributes(std::vector<uint32_t>& out) const {
  out.clear();
  out.reserve(entries_.size());
  for (const Entry& entry : entries_) {
    out.push_back(entry.attr_res_id);
  }
}

base::expected<base::monostate, IOError> Theme::SetTo(const Theme& source) {
  if (this == &source) {
    return base::expected<base::monostate, IOError>();
  }

  type_spec_flags_ = source.type_spec_flags_;

  if (asset_manager_ == source.asset_manager_) {
    entries_ = source.entries_;
    cache_generation_ = NextThemeGeneration();
    return base::expected<base::monostate, IOError>();
  }

  // CDROID trim: the cross-AssetManager entry rewrite (cookie/package-id
  // remapping for shared ApkAssets across managers) rides on runtime
  // re-assignment which stays single-manager here; fail like AOSP's
  // non-overlapping case would for reference entries.
  LOGW("Theme::SetTo across different AssetManagers is not supported on CDROID");
  return base::unexpected<IOError>(IOError::PAGES_MISSING);
}

void Theme::Dump() const {
  LOGI("Theme(this=%p, AssetManager2=%p)", this, asset_manager_);
  for (auto& entry : entries_) {
    LOGI("  entry(0x%08x)=(0x%08x) type=(0x%02x), cookie(%d)",
         entry.attr_res_id, entry.value.data, entry.value.dataType, entry.cookie);
  }
}

}  // namespace cdroid
