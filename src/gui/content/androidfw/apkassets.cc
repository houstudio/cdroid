// Port of AOSP androidfw/ApkAssets.cpp (frameworks/base/libs/androidfw/
// ApkAssets.cpp), namespace cdroid. The arsc rides the zero-copy path: the
// STORED entry hands a per-asset mmap window whose buffer feeds LoadedArsc
// directly (no heap copy, the AOSP createFromUncompressedMap -> getIncFsBuffer
// -> LoadedArsc::Load chain).
//
// Copyright (C) 2016 The Android Open Source Project
// Licensed under the Apache License, Version 2.0.

#include <content/androidfw/apkassets.h>

#include <porting/cdlog.h>

namespace cdroid {

constexpr const char* kResourcesArsc = "resources.arsc";

ApkAssets::ApkAssets(std::unique_ptr<Asset> resources_asset,
                     std::unique_ptr<LoadedArsc> loaded_arsc,
                     std::unique_ptr<AssetsProvider> assets,
                     package_property_t property_flags)
    : resources_asset_(std::move(resources_asset)),
      loaded_arsc_(std::move(loaded_arsc)),
      assets_provider_(std::move(assets)),
      property_flags_(property_flags) {}

std::unique_ptr<ApkAssets> ApkAssets::Load(const std::string& path, package_property_t flags) {
    return Load(ZipAssetsProvider::Create(path, flags), flags);
}

std::unique_ptr<ApkAssets> ApkAssets::Load(std::unique_ptr<AssetsProvider> assets,
                                            package_property_t flags) {
    return LoadImpl(std::move(assets), flags);
}

std::unique_ptr<ApkAssets> ApkAssets::LoadTable(std::unique_ptr<Asset> resources_asset,
                                                std::unique_ptr<AssetsProvider> assets,
                                                package_property_t flags) {
    if (resources_asset == nullptr) {
        return {};
    }
    return LoadImpl(std::move(resources_asset), std::move(assets), flags);
}

std::unique_ptr<ApkAssets> ApkAssets::LoadImpl(std::unique_ptr<AssetsProvider> assets,
                                               package_property_t property_flags) {
    if (assets == nullptr) {
        return {};
    }

    // Open the resource table via mmap unless it is compressed. This logic is taken care of by Open.
    bool resources_asset_exists = false;
    auto resources_asset = assets->Open(kResourcesArsc, Asset::AccessMode::ACCESS_BUFFER,
                                         &resources_asset_exists);
    if (resources_asset == nullptr && resources_asset_exists) {
        LOGE("Failed to open '%s' in APK '%s'.", kResourcesArsc,
             assets->GetDebugName().c_str());
        return {};
    }

    return LoadImpl(std::move(resources_asset), std::move(assets), property_flags);
}

std::unique_ptr<ApkAssets> ApkAssets::LoadImpl(std::unique_ptr<Asset> resources_asset,
                                               std::unique_ptr<AssetsProvider> assets,
                                               package_property_t property_flags) {
    if (assets == nullptr) {
        return {};
    }

    std::unique_ptr<LoadedArsc> loaded_arsc;
    if (resources_asset != nullptr) {
        const void* data = resources_asset->getBuffer(true /* aligned */);
        const size_t length = (size_t)resources_asset->getLength();
        if (data == nullptr || length == 0) {
            LOGE("Failed to read resources table in APK '%s'.",
                 assets->GetDebugName().c_str());
            return {};
        }
        loaded_arsc = LoadedArsc::Load(const_cast<uint8_t*>((const uint8_t*)data), length,
                                       property_flags);
    } else {
        loaded_arsc = LoadedArsc::CreateEmpty();
    }

    if (loaded_arsc == nullptr) {
        LOGE("Failed to load resources table in APK '%s'.", assets->GetDebugName().c_str());
        return {};
    }

    return std::unique_ptr<ApkAssets>(new ApkAssets(std::move(resources_asset),
                                                    std::move(loaded_arsc), std::move(assets),
                                                    property_flags));
}

const std::string* ApkAssets::GetPath() const {
    return assets_provider_->GetPath();
}

const std::string& ApkAssets::GetDebugName() const {
    return assets_provider_->GetDebugName();
}

bool ApkAssets::IsUpToDate() const {
    // Loaders are invalidated by the app, not the system, so assume they are up to date.
    return IsLoader() || assets_provider_->IsUpToDate();
}

}  // namespace cdroid
