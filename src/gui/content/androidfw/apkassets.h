// Port of AOSP androidfw/ApkAssets.h (frameworks/base/libs/androidfw/include/
// androidfw/ApkAssets.h), namespace cdroid. Holds an APK: its AssetsProvider
// (files) + LoadedArsc (table). Trim boundary: no idmap/LoadOverlay (RRO).
//
// Copyright (C) 2016 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.
#ifndef __CDROID_ANDROIDFW_APKASSETS_H__
#define __CDROID_ANDROIDFW_APKASSETS_H__

#include <memory>
#include <string>

#include <content/androidfw/assetsprovider.h>
#include <content/androidfw/loadedarsc.h>

namespace cdroid {

// Holds an APK.
class ApkAssets {
public:
    // Creates an ApkAssets from a path on device.
    static std::unique_ptr<ApkAssets> Load(const std::string& path,
                                            package_property_t flags = 0U);

    // Creates an ApkAssets from an AssetsProvider.
    // The ApkAssets will take care of destroying the AssetsProvider when it is destroyed.
    static std::unique_ptr<ApkAssets> Load(std::unique_ptr<AssetsProvider> assets,
                                            package_property_t flags = 0U);

    // Creates an ApkAssets from the given asset file representing a resources.arsc.
    static std::unique_ptr<ApkAssets> LoadTable(std::unique_ptr<Asset> resources_asset,
                                                std::unique_ptr<AssetsProvider> assets,
                                                package_property_t flags = 0U);

    // Path to the contents of the ApkAssets on disk. The path could represent an APk, a directory,
    // or some other file type.
    const std::string* GetPath() const;

    const std::string& GetDebugName() const;

    const AssetsProvider* GetAssetsProvider() const {
        return assets_provider_.get();
    }

    // This is never nullptr.
    const LoadedArsc* GetLoadedArsc() const {
        return loaded_arsc_.get();
    }

    bool IsLoader() const {
        return (property_flags_ & PROPERTY_LOADER) != 0;
    }

    bool IsOverlay() const {
        return (property_flags_ & PROPERTY_OVERLAY) != 0;
    }

    // Returns whether the resources.arsc is allocated in RAM (not mmapped).
    bool IsTableAllocated() const {
        return resources_asset_ != nullptr && resources_asset_->isAllocated();
    }

    bool IsUpToDate() const;

private:
    static std::unique_ptr<ApkAssets> LoadImpl(std::unique_ptr<AssetsProvider> assets,
                                                package_property_t property_flags);

    static std::unique_ptr<ApkAssets> LoadImpl(std::unique_ptr<Asset> resources_asset,
                                                std::unique_ptr<AssetsProvider> assets,
                                                package_property_t property_flags);

    ApkAssets(std::unique_ptr<Asset> resources_asset,
              std::unique_ptr<LoadedArsc> loaded_arsc,
              std::unique_ptr<AssetsProvider> assets,
              package_property_t property_flags);

    std::unique_ptr<Asset> resources_asset_;
    std::unique_ptr<LoadedArsc> loaded_arsc_;

    std::unique_ptr<AssetsProvider> assets_provider_;
    package_property_t property_flags_ = 0U;
};

}  // namespace cdroid

#endif  // __CDROID_ANDROIDFW_APKASSETS_H__
