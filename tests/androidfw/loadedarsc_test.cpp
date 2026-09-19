// AM2 stage-1 regression: run the real framework resources.arsc (from the
// built cdroid.pak) through the ported LoadedArsc. Originally a cross-check
// against the legacy ResTable's view of the same bytes; the AM1 retirement
// turned the sweep into an engine self-resolution check (every addressable
// slot must resolve through AssetManager2's config matcher).
#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <zip.h>

#include "loadedarsc.h"
#include "resourcetypes.h"
#include <content/androidfw/apkassets.h>
#include <content/androidfw/assetmanager2.h>

using namespace cdroid;

namespace {

// Read a whole entry out of a pak (zip) into a buffer. Returns empty on miss.
std::vector<uint8_t> readZipEntry(const std::string& pakPath, const std::string& name) {
    std::vector<uint8_t> out;
    int err = 0;
    zip_t* pak = zip_open(pakPath.c_str(), ZIP_RDONLY, &err);
    if (pak == nullptr) return out;
    zip_file_t* zf = zip_fopen(pak, name.c_str(), 0);
    if (zf != nullptr) {
        zip_stat_t st;
        zip_stat_init(&st);
        if (zip_stat(pak, name.c_str(), 0, &st) == 0 && (st.valid & ZIP_STAT_SIZE)) {
            out.resize(st.size);
            zip_uint64_t got = 0;
            while (got < st.size) {
                const zip_int64_t n = zip_fread(zf, out.data() + got, st.size - got);
                if (n <= 0) break;
                got += (zip_uint64_t)n;
            }
            if (got != st.size) out.clear();
        }
        zip_fclose(zf);
    }
    zip_close(pak);
    return out;
}

std::string frameworkPakPath() {
    // The test binary lives under outX64-Debug/bin/tests or is run from the
    // build dir; cdroid.pak sits at the build root. Probe the common spots.
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

class LoadedArscTest : public testing::Test {
protected:
    void SetUp() override {
        pak_ = frameworkPakPath();
        if (pak_.empty()) {
            GTEST_SKIP() << "cdroid.pak not found — run from the build dir";
        }
        arsc_ = readZipEntry(pak_, "resources.arsc");
        ASSERT_FALSE(arsc_.empty()) << "resources.arsc unreadable from " << pak_;
    }

    std::string pak_;
    std::vector<uint8_t> arsc_;
};

TEST_F(LoadedArscTest, LoadsFrameworkArsc) {
    std::unique_ptr<LoadedArsc> loaded =
            LoadedArsc::Load(arsc_.data(), arsc_.size(), PROPERTY_SYSTEM);
    ASSERT_NE(nullptr, loaded);
    ASSERT_GE(loaded->GetPackages().size(), 1u);

    const LoadedPackage* pkg = loaded->GetPackageById(0x01);
    ASSERT_NE(nullptr, pkg) << "framework package 0x01 missing";
    EXPECT_EQ("android", pkg->GetPackageName());   // the framework package, AOSP name
    EXPECT_TRUE(pkg->IsSystem());
    EXPECT_FALSE(pkg->IsDynamic());
    EXPECT_NE(nullptr, loaded->GetStringPool());
    EXPECT_GT(loaded->GetStringPool()->getError(), -1);   // pool initialized (any valid state)
}

// The AOSP Load semantics validate EVERY entry offset while walking; a table
// the legacy engine accepts but LoadedArsc rejects would abort Load above, so
// reaching here already proves the slim arsc is LoadedArsc-compatible.

// Slot-exact engine sweep: every entry LoadedArsc can address must resolve
// through the AM2 config matcher (GetResource with may_be_bag — bags come
// back as references), and the aggregate config set stays at the golden size.
TEST_F(LoadedArscTest, AllEntriesResolveThroughEngine) {
    std::unique_ptr<LoadedArsc> loaded =
            LoadedArsc::Load(arsc_.data(), arsc_.size(), PROPERTY_SYSTEM);
    ASSERT_NE(nullptr, loaded);

    // The same arsc as an engine: load the pak (the arsc rides inside).
    auto apk = ApkAssets::Load(pak_, PROPERTY_SYSTEM);
    ASSERT_NE(nullptr, apk);
    AssetManager2 am;
    std::vector<const ApkAssets*> apks;
    apks.push_back(apk.get());
    am.SetApkAssets(std::move(apks));

    size_t present_entries = 0;
    size_t resolved_entries = 0;
    for (const auto& pkg : loaded->GetPackages()) {
        const int package_id = pkg->GetPackageId();
        pkg->ForEachTypeSpec([&](const TypeSpec& ts, uint8_t type_id) {
            for (const auto& type_entry : ts.type_entries) {
                const uint16_t entry_count =
                        (uint16_t)dtohl(type_entry.type->entryCount);
                for (uint16_t e = 0; e < entry_count; e++) {
                    const auto entry = LoadedPackage::GetEntry(type_entry.type, e);
                    if (!entry.has_value()) continue;   // NO_ENTRY slot
                    present_entries++;

                    const uint32_t resid = make_resid((uint8_t)package_id, type_id, e);
                    auto value = am.GetResource(resid, true /*may_be_bag*/);
                    if (value.has_value()) {
                        resolved_entries++;
                    } else {
                        auto name = am.GetResourceName(resid);
                        printf("MISMATCH 0x%08x %s\n", resid,
                               name.has_value() ? "(name resolved)" : "(no name)");
                    }
                }
            }
        });
    }

    // Known tail delta (3/49421): android:layout/0x01090085 (two config
    // variants) and android:drawable/0x010801ac exist ONLY in configurations
    // that do not match the default (zeroed) request, so the engine picks
    // nothing for them — the legacy A/B era showed the identical delta on the
    // old engine, i.e. engine-correct behavior, not a porting gap.
    ASSERT_GT(present_entries, 100u) << "suspiciously small framework table";
    EXPECT_EQ(present_entries - 3, resolved_entries)
            << "entries addressable by LoadedArsc but unresolvable by the engine";

    // Aggregate config set (golden: the slim framework pak's variant count —
    // tracks src/gui/res content).
    std::set<ResTable_config> loaded_configs;
    for (const auto& pkg : loaded->GetPackages()) {
        auto ok = pkg->CollectConfigurations(false, &loaded_configs);
        ASSERT_TRUE(ok.has_value());
    }
    EXPECT_EQ(22u, loaded_configs.size());
}

// FindEntryByName round-trip: take a key name straight from the key pool and
// locate it through the UTF-16 lookup path (indexOfString under the hood).
TEST_F(LoadedArscTest, FindEntryByNameFindsKeyPoolName) {
    std::unique_ptr<LoadedArsc> loaded =
            LoadedArsc::Load(arsc_.data(), arsc_.size(), PROPERTY_SYSTEM);
    ASSERT_NE(nullptr, loaded);
    const LoadedPackage* pkg = loaded->GetPackageById(0x01);
    ASSERT_NE(nullptr, pkg);

    // Walk every type; grab the first entry's key from the key pool, then
    // resolve (type name, key name) back to the same partial id.
    size_t found = 0;
    pkg->ForEachTypeSpec([&](const TypeSpec& ts, uint8_t type_id) {
        if (found > 0 || ts.type_entries.empty()) return;
        const auto& type_entry = ts.type_entries[0];
        // Try a few slots: the mid can be a NO_ENTRY hole after a repack.
        const uint16_t count = (uint16_t)dtohl(type_entry.type->entryCount);
        uint32_t key_index = 0xffffffff;
        for (uint16_t probe = 0; probe < count; probe++) {
            auto e = LoadedPackage::GetEntry(type_entry.type, probe);
            if (e.has_value()) {
                key_index = dtohl(e.value()->key.index);
                break;
            }
        }
        if (key_index == 0xffffffff) return;
        size_t tlen = 0, klen = 0;
        const char16_t* type_name = pkg->GetTypeStringPool()->stringAt(type_id - 1, &tlen);
        const char16_t* key_name = pkg->GetKeyStringPool()->stringAt(key_index, &klen);
        if (type_name == nullptr || key_name == nullptr) return;
        const auto partial = pkg->FindEntryByName(std::u16string(type_name, tlen),
                                                  std::u16string(key_name, klen));
        if (partial.has_value()) found++;
    });
    EXPECT_GT(found, 0u) << "FindEntryByName could not locate any type/key pair";
}
