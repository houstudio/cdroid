// Port of AOSP androidfw/AssetManager2.h (frameworks/base/libs/androidfw/
// include/androidfw/AssetManager2.h), namespace cdroid — AM2 + Theme.
//
// Trim boundary (the campaign's): no RRO/idmap (overlays are build-time on
// CDROID) — ConfiguredOverlay / overlay branches / GetNonSystemOverlays /
// GetOverlayablesToString are dropped; SetTo keeps only the same-manager
// fast path (cross-manager rewrites need the cookie/package map only for
// runtime re-assignment, which stays single-manager on CDROID today).
//
// C++14 seams (see expected.h / mapptr.h): base::expected stays;
// std::optional -> base::optional; std::variant -> the EntryValue shim
// (assetmanager2.cc); StringPoolRef/StringPiece -> std::string faces;
// util::unique_cptr<ResolvedBag> -> unique_ptr with a free() deleter.
//
// Copyright (C) 2016 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.
#ifndef __CDROID_ANDROIDFW_ASSETMANAGER2_H_
#define __CDROID_ANDROIDFW_ASSETMANAGER2_H_

#include <array>
#include <limits>
#include <memory>
#include <set>
#include <unordered_map>

#include <content/androidfw/apkassets.h>
#include <content/androidfw/expected.h>
#include <content/androidfw/loadedarsc.h>
#include <content/androidfw/resourcetypes.h>
#include <content/assetdir.h>

namespace cdroid {

// The error seam lives one namespace deeper (expected.h); pull it in like the
// other AM2 files do.
using androidfw::IOError;
using androidfw::NullOrIOError;

class Theme;

using ApkAssetsCookie = int32_t;

enum : ApkAssetsCookie {
  kInvalidCookie = -1,
};

// A reference to a string in a specific ResStringPool (AOSP androidfw/
// ResourceUtils.h StringPoolRef).
class StringPoolRef {
 public:
  StringPoolRef() = default;
  StringPoolRef(const ResStringPool* pool, uint32_t index) : pool_(pool), index_(index) {}

  const ResStringPool* pool() const { return pool_; }
  uint32_t index() const { return index_; }

  const char* string8(size_t* out_len) const {
    return pool_ != nullptr ? pool_->string8At(index_, out_len) : nullptr;
  }

  const char16_t* string16(size_t* out_len) const {
    return pool_ != nullptr ? pool_->stringAt(index_, out_len) : nullptr;
  }

 private:
  const ResStringPool* pool_ = nullptr;
  uint32_t index_ = 0;
};

// Holds a bag that has been merged with its parent, if one exists.
struct ResolvedBag {
  // A single key-value entry in a bag.
  struct Entry {
    // The key, as described in ResTable_map::name.
    uint32_t key;

    Res_value value;

    // The resource ID of the origin style associated with the given entry.
    uint32_t style;

    // Which ApkAssets this entry came from.
    ApkAssetsCookie cookie;

    ResStringPool* key_pool;
    ResStringPool* type_pool;
  };

  // Denotes the configuration axis that this bag varies with.
  // If a configuration changes with respect to one of these axis,
  // the bag should be reloaded.
  uint32_t type_spec_flags;

  // The number of entries in this bag. Access them by indexing into `entries`.
  uint32_t entry_count;

  // The array of entries for this bag. An empty array is a neat trick to force alignment
  // of the Entry structs that follow this structure and avoids a bunch of casts.
  Entry entries[0];
};

// free()-allocated ResolvedBag (the malloc'd entries[] tail rides along).
struct ResolvedBagDeleter {
  void operator()(ResolvedBag* bag) const { ::free(bag); }
};
using ResolvedBagPtr = std::unique_ptr<ResolvedBag, ResolvedBagDeleter>;

struct FindEntryResult;

// AssetManager2 is the main entry point for accessing assets and resources.
// AssetManager2 provides caching of resources retrieved via the underlying ApkAssets.
class AssetManager2 {
  friend Theme;

 public:
  struct ResourceName {
    const char* package = nullptr;
    size_t package_len = 0u;

    const char* type = nullptr;
    const char16_t* type16 = nullptr;
    size_t type_len = 0u;

    const char* entry = nullptr;
    const char16_t* entry16 = nullptr;
    size_t entry_len = 0u;
  };

  AssetManager2();

  // Sets/resets the underlying ApkAssets for this AssetManager. The ApkAssets
  // are not owned by the AssetManager, and must have a longer lifetime.
  //
  // Only pass invalidate_caches=false when it is known that the structure
  // change in ApkAssets is due to a safe addition of resources with completely
  // new resource IDs.
  bool SetApkAssets(std::vector<const ApkAssets*> apk_assets, bool invalidate_caches = true);

  inline const std::vector<const ApkAssets*>& GetApkAssets() const {
    return apk_assets_;
  }

  // Returns the string pool for the given asset cookie.
  // Use the string pool returned here with a valid Res_value object of type Res_value::TYPE_STRING.
  const ResStringPool* GetStringPoolForCookie(ApkAssetsCookie cookie) const;

  // Returns the DynamicRefTable for the given package ID.
  // This may be nullptr if the APK represented by `cookie` has no resource table.
  const DynamicRefTable* GetDynamicRefTableForPackage(uint32_t package_id) const;

  // Returns the DynamicRefTable for the ApkAssets represented by the cookie.
  // This may be nullptr if the APK represented by `cookie` has no resource table.
  std::shared_ptr<const DynamicRefTable> GetDynamicRefTableForCookie(ApkAssetsCookie cookie) const;

  // Retrieve the assigned package id of the package if loaded into this AssetManager
  uint8_t GetAssignedPackageId(const LoadedPackage* package) const;

  const std::unordered_map<std::string, std::string>* GetOverlayableMapForPackage(
      uint32_t package_id) const;

  // Returns whether the resources.arsc of any loaded apk assets is allocated in RAM (not mmapped).
  bool ContainsAllocatedTable() const;

  // Sets/resets the configuration for this AssetManager. This will cause all
  // caches that are related to the configuration change to be invalidated.
  void SetConfiguration(const ResTable_config& configuration);

  inline const ResTable_config& GetConfiguration() const {
    return configuration_;
  }

  // Returns all configurations for which there are resources defined, or an I/O error if reading
  // resource data failed.
  //
  // This includes resource configurations in all the ApkAssets set for this AssetManager.
  // If `exclude_system` is set to true, resource configurations from system APKs
  // ('android' package, other libraries) will be excluded from the list.
  // If `exclude_mipmap` is set to true, resource configurations defined for resource type 'mipmap'
  // will be excluded from the list.
  base::expected<std::set<ResTable_config>, IOError> GetResourceConfigurations(
      bool exclude_system = false, bool exclude_mipmap = false) const;

  // Returns all the locales for which there are resources defined. This includes resource
  // locales in all the ApkAssets set for this AssetManager.
  // If `exclude_system` is set to true, resource locales from system APKs
  // ('android' package, other libraries) will be excluded from the list.
  // If `merge_equivalent_languages` is set to true, resource locales will be canonicalized
  // and de-duped in the resulting list.
  std::set<std::string> GetResourceLocales(bool exclude_system = false,
                                           bool merge_equivalent_languages = false) const;

  // Searches the set of APKs loaded by this AssetManager and opens the first one found located
  // in the assets/ directory.
  // `mode` controls how the file is opened.
  //
  // NOTE: The loaded APKs are searched in reverse order.
  std::unique_ptr<Asset> Open(const std::string& filename, Asset::AccessMode mode) const;

  // Opens a file within the assets/ directory of the APK specified by `cookie`.
  // `mode` controls how the file is opened.
  std::unique_ptr<Asset> Open(const std::string& filename, ApkAssetsCookie cookie,
                              Asset::AccessMode mode) const;

  // Searches the set of APKs loaded by this AssetManager and opens the first one found.
  // `mode` controls how the file is opened.
  // `out_cookie` is populated with the cookie of the APK this file was found in.
  //
  // NOTE: The loaded APKs are searched in reverse order.
  std::unique_ptr<Asset> OpenNonAsset(const std::string& filename, Asset::AccessMode mode,
                                      ApkAssetsCookie* out_cookie = nullptr) const;

  // Opens a file in the APK specified by `cookie`. `mode` controls how the file is opened.
  // This is typically used to open a specific AndroidManifest.xml, or a binary XML file
  // referenced by a resource lookup with GetResource().
  std::unique_ptr<Asset> OpenNonAsset(const std::string& filename, ApkAssetsCookie cookie,
                                      Asset::AccessMode mode) const;

  // Returns the resource name of the specified resource ID.
  //
  // Utf8 strings are preferred, and only if they are unavailable are the Utf16 variants populated.
  //
  // Returns a null error if the name is missing/corrupt, or an I/O error if reading resource data
  // failed.
  base::expected<ResourceName, NullOrIOError> GetResourceName(uint32_t resid) const;

  // Finds the resource ID assigned to `resource_name`.
  //
  // `resource_name` must be of the form '[package:][type/]entry'.
  // If no package is specified in `resource_name`, then `fallback_package` is used as the package.
  // If no type is specified in `resource_name`, then `fallback_type` is used as the type.
  //
  // Returns a null error if no resource by that name was found, or an I/O error if reading resource
  // data failed.
  base::expected<uint32_t, NullOrIOError> GetResourceId(
      const std::string& resource_name, const std::string& fallback_type = {},
      const std::string& fallback_package = {}) const;

  struct SelectedValue {
    friend AssetManager2;
    friend Theme;
    SelectedValue() = default;
    SelectedValue(const ResolvedBag* bag, const ResolvedBag::Entry& entry) :
        cookie(entry.cookie), data(entry.value.data), type(entry.value.dataType),
        flags(bag->type_spec_flags), resid(0U), config({}) {};

    // The cookie representing the ApkAssets in which the value resides.
    ApkAssetsCookie cookie = kInvalidCookie;

    // The data for this value, as interpreted according to `type`.
    uint32_t data;

    // Type of the data value.
    uint8_t type;

    // The bitmask of configuration axis that this resource varies with.
    // See ResTable_config::CONFIG_*.
    uint32_t flags;

    // The resource ID from which this value was resolved.
    uint32_t resid;

    // The configuration for which the resolved value was defined.
    ResTable_config config;

   private:
    SelectedValue(uint8_t value_type, uint32_t value_data, ApkAssetsCookie cookie,
                  uint32_t type_flags, uint32_t resid, const ResTable_config& config) :
                  cookie(cookie), data(value_data), type(value_type), flags(type_flags),
                  resid(resid), config(config) {};
  };

  // Retrieves the best matching resource value with ID `resid`.
  //
  // If `may_be_bag` is false, this function logs if the resource was a map/bag type and returns a
  // null result. If `density_override` is non-zero, the configuration to match against is
  // overridden with that density.
  //
  // Returns a null error if a best match could not be found, or an I/O error if reading resource
  // data failed.
  base::expected<SelectedValue, NullOrIOError> GetResource(uint32_t resid, bool may_be_bag = false,
                                                           uint16_t density_override = 0U) const;

  // Resolves the resource referenced in `value` if the type is Res_value::TYPE_REFERENCE.
  //
  // If the data type is not Res_value::TYPE_REFERENCE, no work is done. Configuration flags of the
  // values pointed to by the reference are OR'd into `value.flags`. If `cache_value` is true, then
  // the resolved value will be cached and used when attempting to resolve the resource id specified
  // in `value`.
  //
  // Returns a null error if the resource could not be resolved, or an I/O error if reading resource
  // data failed.
  base::expected<base::monostate, NullOrIOError> ResolveReference(SelectedValue& value,
                                                                 bool cache_value = false) const;

  // Retrieves the best matching bag/map resource with ID `resid`.
  //
  // This method will resolve all parent references for this bag and merge keys with the child.
  // To iterate over the keys, use the begin/end free functions below.
  //
  // Returns a null error if a best match could not be found, or an I/O error if reading resource
  // data failed.
  base::expected<const ResolvedBag*, NullOrIOError> GetBag(uint32_t resid) const;

  // Retrieves the best matching bag/map resource of the resource referenced in `value`.
  //
  // If `value.type` is not Res_value::TYPE_REFERENCE, a null result is returned.
  // Configuration flags of the bag pointed to by the reference are OR'd into `value.flags`.
  //
  // Returns a null error if a best match could not be found, or an I/O error if reading resource
  // data failed.
  base::expected<const ResolvedBag*, NullOrIOError> ResolveBag(SelectedValue& value) const;

  // Returns the android::ResTable_typeSpec flags of the resource ID.
  //
  // Returns a null error if the resource could not be resolved, or an I/O error if reading resource
  // data failed.
  base::expected<uint32_t, NullOrIOError> GetResourceTypeSpecFlags(uint32_t resid) const;

  const std::vector<uint32_t> GetBagResIdStack(uint32_t resid) const;

  // Resets the resource resolution structures in preparation for the next resource retrieval.
  void ResetResourceResolution() const;

  // Enables or disables resource resolution logging. Clears stored steps when disabled.
  void SetResourceResolutionLoggingEnabled(bool enabled);

  // Returns formatted log of last resource resolution path, or empty if no resource has been
  // resolved yet.
  std::string GetLastResourceResolution() const;

  // Creates a new Theme from this AssetManager.
  std::unique_ptr<Theme> NewTheme();

  void ForEachPackage(const std::function<bool(const std::string&, uint8_t)> func,
                      package_property_t excluded_property_flags = 0U) const {
    for (const PackageGroup& package_group : package_groups_) {
      const auto loaded_package = package_group.packages_[0].loaded_package_;
      if ((loaded_package->GetPropertyFlags() & excluded_property_flags) == 0U
          && !func(loaded_package->GetPackageName(),
                   package_group.dynamic_ref_table->getAssignedPackageId())) {
        return;
      }
    }
  }

  void DumpToLog() const;

 private:
  AssetManager2(const AssetManager2&) = delete;
  AssetManager2& operator=(const AssetManager2&) = delete;

  // A collection of configurations and their associated ResTable_type that match the current
  // AssetManager configuration.
  struct FilteredConfigGroup {
      std::vector<const TypeSpec::TypeEntry*> type_entries;
  };

  // Represents an single package.
  struct ConfiguredPackage {
    // A pointer to the immutable, loaded package info.
    const LoadedPackage* loaded_package_;

    // A mutable AssetManager-specific list of configurations that match the AssetManager's
    // current configuration. This is used as an optimization to avoid checking every single
    // candidate configuration when looking up resources.
    ByteBucketArray<FilteredConfigGroup> filtered_configs_;
  };

  // Represents a logical package, which can be made up of many individual packages. Each package
  // in a PackageGroup shares the same package name and package ID.
  struct PackageGroup {
    // The set of packages that make-up this group.
    std::vector<ConfiguredPackage> packages_;

    // The cookies associated with each package in the group. They share the same order as
    // packages_.
    std::vector<ApkAssetsCookie> cookies_;

    // A library reference table that contains build-package ID to runtime-package ID mappings.
    std::shared_ptr<DynamicRefTable> dynamic_ref_table = std::make_shared<DynamicRefTable>();
  };

  // Finds the best entry for `resid` from the set of ApkAssets. The entry can be a simple
  // Res_value, or a complex map/bag type. Returns a null result if a best entry cannot be found.
  //
  // `density_override` overrides the density of the current configuration when doing a search.
  //
  // When `stop_at_first_match` is true, the first match found is selected and the search
  // terminates. This is useful for methods that just look up the name of a resource and don't
  // care about the value. In this case, the value of `FindEntryResult::type_flags` is incomplete
  // and should not be used.
  //
  // When `ignore_configuration` is true, FindEntry will return always select the first entry in
  // for the type seen regardless of its configuration.
  //
  // NOTE: FindEntry takes care of ensuring that structs within FindEntryResult have been properly
  // bounds-checked. Callers of FindEntry are free to trust the data if this method succeeds.
  base::expected<FindEntryResult, NullOrIOError> FindEntry(uint32_t resid,
                                                           uint16_t density_override,
                                                           bool stop_at_first_match,
                                                           bool ignore_configuration) const;

  base::expected<FindEntryResult, NullOrIOError> FindEntryInternal(
      const PackageGroup& package_group, uint8_t type_idx, uint16_t entry_idx,
      const ResTable_config& desired_config, bool stop_at_first_match,
      bool ignore_configuration) const;

  // Assigns package IDs to all shared library ApkAssets.
  // Should be called whenever the ApkAssets are changed.
  void BuildDynamicRefTable();

  // Purge all resources that are cached and vary by the configuration axis denoted by the
  // bitmask `diff`.
  void InvalidateCaches(uint32_t diff);

  // Triggers the re-construction of lists of types that match the set configuration.
  // This should always be called when mutating the AssetManager's configuration or ApkAssets set.
  void RebuildFilterList();

  // AssetManager2::GetBag(resid) wraps this function to track which resource ids have already
  // been seen while traversing bag parents.
  base::expected<const ResolvedBag*, NullOrIOError> GetBag(
      uint32_t resid, std::vector<uint32_t>& child_resids) const;

  // The ordered list of ApkAssets to search. These are not owned by the AssetManager, and must
  // have a longer lifetime.
  std::vector<const ApkAssets*> apk_assets_;

  // DynamicRefTables for shared library package resolution.
  // These are ordered according to apk_assets_. The mappings may change depending on what is
  // in apk_assets_, therefore they must be stored in the AssetManager and not in the
  // immutable ApkAssets class.
  std::vector<PackageGroup> package_groups_;

  // An array mapping package ID to index into package_groups. This keeps the lookup fast
  // without taking too much memory.
  std::array<uint8_t, std::numeric_limits<uint8_t>::max() + 1> package_ids_;

  // The current configuration set for this AssetManager. When this changes, cached resources
  // may need to be purged.
  ResTable_config configuration_;

  // Cached set of bags. These are cached because they can inherit keys from parent bags,
  // which involves some calculation.
  mutable std::unordered_map<uint32_t, ResolvedBagPtr> cached_bags_;

  // Cached set of bag resid stacks for each bag. These are cached because they might be requested
  // a number of times for each view during View inspection.
  mutable std::unordered_map<uint32_t, std::vector<uint32_t>> cached_bag_resid_stacks_;

  // Cached set of resolved resource values.
  mutable std::unordered_map<uint32_t, SelectedValue> cached_resolved_values_;

  // Whether or not to save resource resolution steps
  bool resource_resolution_logging_enabled_ = false;

  struct Resolution {
    struct Step {
      enum class Type {
        INITIAL,
        BETTER_MATCH,
        OVERLAID,
        OVERLAID_INLINE,
        SKIPPED,
        NO_ENTRY,
      };

      // Marks what kind of override this step was.
      Type type;

      // Built name of configuration for this step.
      std::string config_name;

      ApkAssetsCookie cookie = kInvalidCookie;
    };

    // Last resolved resource ID.
    uint32_t resid;

    // Last resolved resource result cookie.
    ApkAssetsCookie cookie = kInvalidCookie;

    // Last resolved resource type.
    StringPoolRef type_string_ref;

    // Last resolved resource entry.
    StringPoolRef entry_string_ref;

    // Steps taken to resolve last resource.
    std::vector<Step> steps;
  };

  // Record of the last resolved resource's resolution path.
  mutable Resolution last_resolution_;
};

class Theme {
  friend class AssetManager2;

 public:
  ~Theme();

  // Applies the style identified by `resid` to this theme.
  //
  // This can be called multiple times with different styles. By default, any theme attributes that
  // are already defined before this call are not overridden. If `force` is set to true, this
  // behavior is changed and all theme attributes from the style at `resid` are applied.
  //
  // Returns a null error if the style could not be applied, or an I/O error if reading resource
  // data failed.
  base::expected<base::monostate, NullOrIOError> ApplyStyle(uint32_t resid, bool force = false);

  // Clears the existing theme, sets the new asset manager to use for this theme, and applies the
  // styles in `style_ids` through repeated invocations of `ApplyStyle`.
  void Rebase(AssetManager2* am, const uint32_t* style_ids, const uint8_t* force,
              size_t style_count);

  // Sets this Theme to be a copy of `source` if `source` has the same AssetManager as this Theme.
  //
  // If `source` does not have the same AssetManager as this theme, only attributes from ApkAssets
  // loaded into both AssetManagers will be copied to this theme.
  //
  // Returns an I/O error if reading resource data failed.
  base::expected<base::monostate, IOError> SetTo(const Theme& source);

  void Clear();

  // Retrieves the value of attribute ID `resid` in the theme.
  //
  // NOTE: This function does not do reference traversal. If you want to follow references to other
  // resources to get the "real" value to use, you need to call ResolveReference() after this
  // function.
  base::optional<AssetManager2::SelectedValue> GetAttribute(uint32_t resid) const;

  // This is like AssetManager2::ResolveReference(), but also takes care of resolving attribute
  // references to the theme.
  base::expected<base::monostate, NullOrIOError> ResolveAttributeReference(
      AssetManager2::SelectedValue& value) const;

  AssetManager2* GetAssetManager() {
    return asset_manager_;
  }

  const AssetManager2* GetAssetManager() const {
    return asset_manager_;
  }

  // Returns a bit mask of configuration changes that will impact this
  // theme (and thus require completely reloading it).
  uint32_t GetChangingConfigurations() const {
    return type_spec_flags_;
  }

  // CDROID seam: process-monotonic generation (the legacy ResTable::Theme
  // cacheGeneration face). Identity key for the themed resource caches — the
  // object pointer is NOT a safe key (a rebuilt theme typically lands on the
  // same address). Bumped on every mutation; distinct Theme states never
  // share a value.
  uint32_t cacheGeneration() const { return cache_generation_; }

  // CDROID seam: the AOSP @hide ThemeImpl.getAllAttributes() face (attr ids
  // this theme currently has values for), vector out-param form.
  void GetAllAttributes(std::vector<uint32_t>& out) const;

  void Dump() const;

  struct Entry;
 private:
  Theme(const Theme&) = delete;
  Theme& operator=(const Theme&) = delete;

  explicit Theme(AssetManager2* asset_manager);

  AssetManager2* asset_manager_ = nullptr;
  uint32_t type_spec_flags_ = 0u;
  uint32_t cache_generation_ = 0u;   // CDROID seam (themed cache key)

  std::vector<Entry> entries_;
};

inline const ResolvedBag::Entry* begin(const ResolvedBag* bag) {
  return bag->entries;
}

inline const ResolvedBag::Entry* end(const ResolvedBag* bag) {
  return bag->entries + bag->entry_count;
}

}  // namespace cdroid

#endif  // __CDROID_ANDROIDFW_ASSETMANAGER2_H_
