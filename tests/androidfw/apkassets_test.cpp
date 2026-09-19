// AM2 stage-2 reconciliation: ApkAssets loads the real framework pak end to
// end — the STORED arsc becomes a per-asset mmap window whose buffer feeds
// LoadedArsc with ZERO heap copies, and a DEFLATED entry inflates to bytes
// matching what libzip (the old path) produces for the same entry.
#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <zip.h>

#include <content/androidfw/apkassets.h>
#include <content/androidfw/assetsprovider.h>
#include <content/androidfw/loadedarsc.h>

using namespace cdroid;

namespace {

std::string frameworkPakPath() {
    static const char* candidates[] = {
        "cdroid.pak",
        "../../../cdroid.pak",
        "./bin/tests/../../../cdroid.pak",
    };
    for (const char* c : candidates) {
        if (FILE* f = fopen(c, "rb")) {
            fclose(f);
            return c;
        }
    }
    return "";
}

}  // namespace

class ApkAssetsTest : public testing::Test {
protected:
    void SetUp() override {
        const std::string pak = frameworkPakPath();
        if (pak.empty()) {
            GTEST_SKIP() << "cdroid.pak not found — run from the build dir";
        }
        apk_ = ApkAssets::Load(pak, PROPERTY_SYSTEM);
        ASSERT_NE(nullptr, apk_);
    }

    std::unique_ptr<ApkAssets> apk_;
};

TEST_F(ApkAssetsTest, LoadsTableFromStoredWindow) {
    const LoadedArsc* table = apk_->GetLoadedArsc();
    ASSERT_NE(nullptr, table);
    ASSERT_GE(table->GetPackages().size(), 1u);
    EXPECT_EQ("android", table->GetPackageById(0x01)->GetPackageName());
    // The arsc asset is a private mmap window — NOT heap-allocated.
    EXPECT_FALSE(apk_->IsTableAllocated());
}

TEST_F(ApkAssetsTest, AssetReadsMatchLibzipGroundTruth) {
    const AssetsProvider* provider = apk_->GetAssetsProvider();

    // One STORED entry (the arsc) and one DEFLATED entry, both cross-checked
    // against libzip's read of the same pak.
    int fd = -1;
    const char* pakPath = apk_->GetPath() ? apk_->GetPath()->c_str() : nullptr;
    ASSERT_NE(nullptr, pakPath);
    zip_t* z = zip_open(pakPath, ZIP_RDONLY, &fd);
    ASSERT_NE(nullptr, z);

    // DEFLATED: an anim XML.
    {
        auto asset = provider->Open("res/anim/accelerate_interpolator.xml",
                                    Asset::AccessMode::ACCESS_BUFFER);
        ASSERT_NE(nullptr, asset);
        zip_file_t* zf = zip_fopen(z, "res/anim/accelerate_interpolator.xml", 0);
        ASSERT_NE(nullptr, zf);
        zip_stat_t st;
        zip_stat(z, "res/anim/accelerate_interpolator.xml", 0, &st);
        std::vector<char> truth(st.size);
        ASSERT_EQ(st.size, (size_t)zip_fread(zf, truth.data(), st.size));
        zip_fclose(zf);
        ASSERT_EQ((size_t)asset->getLength(), truth.size());
        EXPECT_EQ(0, memcmp(asset->getBuffer(true), truth.data(), truth.size()));
    }

    // STORED: the arsc itself.
    {
        auto asset = provider->Open("resources.arsc", Asset::AccessMode::ACCESS_BUFFER);
        ASSERT_NE(nullptr, asset);
        zip_file_t* zf = zip_fopen(z, "resources.arsc", 0);
        ASSERT_NE(nullptr, zf);
        zip_stat_t st;
        zip_stat(z, "resources.arsc", 0, &st);
        std::vector<char> truth(st.size);
        ASSERT_EQ(st.size, (size_t)zip_fread(zf, truth.data(), st.size));
        zip_fclose(zf);
        ASSERT_EQ((size_t)asset->getLength(), truth.size());
        EXPECT_EQ(0, memcmp(asset->getBuffer(true), truth.data(), truth.size()));
    }

    zip_close(z);
}

TEST_F(ApkAssetsTest, ForEachFileListsLayoutDirectory) {
    const AssetsProvider* provider = apk_->GetAssetsProvider();
    size_t files = 0, dirs = 0;
    ASSERT_TRUE(provider->ForEachFile("res/layout",
            [&](const std::string& name, FileType type) {
                if (type == kFileTypeRegular) files++;
                else if (type == kFileTypeDirectory) dirs++;
            }));
    EXPECT_GT(files, 50u) << "framework layout dir should list dozens of files";
    EXPECT_EQ(0u, dirs) << "flat dir — no subdirectories expected";
}
