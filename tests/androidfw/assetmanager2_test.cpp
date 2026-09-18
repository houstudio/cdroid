// AM2 ③-4 regression suite on the real framework pak. Originally an A/B
// reconciliation against the legacy ResTable; the AM1 retirement converted
// every oracle answer into a baked golden (captured from the green A/B run —
// the constants track the framework pak CONTENT and move only when
// src/gui/res changes).
#include <gtest/gtest.h>

#include <cstdio>
#include <string>
#include <vector>

#include <content/androidfw/apkassets.h>
#include <content/androidfw/assetmanager2.h>
#include <content/androidfw/loadedarsc.h>
#include "resourcetypes.h"

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

class AssetManager2Test : public testing::Test {
protected:
    void SetUp() override {
        const std::string pak = frameworkPakPath();
        if (pak.empty()) {
            GTEST_SKIP() << "cdroid.pak not found — run from the build dir";
        }
        apk_ = ApkAssets::Load(pak, PROPERTY_SYSTEM);
        ASSERT_NE(nullptr, apk_);
        am_.SetApkAssets({apk_.get()});
    }

    int idOf(const char* name, const char* type) const {
        auto r = am_.GetResourceId(std::string("android:") + type + "/" + name);
        return r.has_value() ? (int)*r : 0;
    }

    std::unique_ptr<ApkAssets> apk_;
    AssetManager2 am_;
};

TEST_F(AssetManager2Test, PackageRegistration) {
    // ForEachPackage walks the registered package groups.
    size_t count = 0;
    am_.ForEachPackage([&](const std::string& name, uint8_t id) {
        EXPECT_EQ("android", name);
        EXPECT_EQ(0x01, id);
        count++;
        return true;
    });
    EXPECT_EQ(1u, count);
}

TEST_F(AssetManager2Test, GetResourceResolvesGoldenString) {
    // A simple string: @android:string/ok (golden: the slim framework ids).
    const uint32_t ok = idOf("ok", "string");
    ASSERT_EQ(0x0104000au, ok);

    auto am2_value = am_.GetResource(ok);
    ASSERT_TRUE(am2_value.has_value());
    EXPECT_EQ(0x03, am2_value->type);          // Res_value::TYPE_STRING
    EXPECT_EQ(0x00001f99u, am2_value->data)    // global string-pool index
            << "TYPE_STRING data is the global-pool index — must stay stable";
}

TEST_F(AssetManager2Test, GetBagShapeMatchesGolden) {
    // A real style bag: TextAppearance (golden shape from the A/B era).
    const uint32_t style = idOf("TextAppearance", "style");
    ASSERT_EQ(0x0103003eu, style);

    auto bag = am_.GetBag(style);
    ASSERT_TRUE(bag.has_value());
    const ResolvedBag* resolved = *bag;
    ASSERT_EQ(6u, resolved->entry_count);

    // Keys ascending, with the golden (key, type, data) triples.
    static const struct { uint32_t key; uint8_t type; uint32_t data; } kGolden[] = {
        { 0x01010095u, 0x05, 0x00001002u },   // textSize = 16sp dimension
        { 0x01010097u, 0x11, 0x00000000u },   // typeface = 0 (TYPE_INT_HEX)
        { 0x01010098u, 0x02, 0x01010036u },   // textColor = ?attr/textColorPrimary
        { 0x01010099u, 0x02, 0x01010099u },
        { 0x0101009au, 0x02, 0x0101009au },
        { 0x0101009bu, 0x02, 0x0101009bu },
    };
    for (size_t i = 0; i < resolved->entry_count; i++) {
        EXPECT_EQ(kGolden[i].key, resolved->entries[i].key) << "bag key at " << i;
        EXPECT_EQ(kGolden[i].type, resolved->entries[i].value.dataType) << "value type at " << i;
        EXPECT_EQ(kGolden[i].data, resolved->entries[i].value.data) << "value data at " << i;
    }
}

TEST_F(AssetManager2Test, ThemeApplyStyleYieldsGoldenTextSize) {
    // ApplyStyle(TextAppearance) must land textSize = 16sp (golden).
    const uint32_t text_appearance = idOf("TextAppearance", "style");
    ASSERT_EQ(0x0103003eu, text_appearance);
    const uint32_t text_size = idOf("textSize", "attr");
    ASSERT_EQ(0x01010095u, text_size);

    auto theme = am_.NewTheme();
    ASSERT_TRUE(theme->ApplyStyle(text_appearance).has_value());

    auto am2_attr = theme->GetAttribute(text_size);
    ASSERT_TRUE(am2_attr.has_value());
    EXPECT_EQ(0x05, am2_attr->type);           // TYPE_DIMENSION
    EXPECT_EQ(0x00001002u, am2_attr->data);    // 16sp (unit=SP, mantissa=16)
}

TEST_F(AssetManager2Test, ForcedStyleOverridesSticky) {
    const uint32_t base = idOf("TextAppearance", "style");
    const uint32_t small = idOf("TextAppearance.Small", "style");
    ASSERT_EQ(0x0103003eu, base);
    ASSERT_EQ(0x01030046u, small);
    const uint32_t text_size = idOf("textSize", "attr");
    ASSERT_EQ(0x01010095u, text_size);

    auto theme = am_.NewTheme();
    ASSERT_TRUE(theme->ApplyStyle(base).has_value());
    auto base_attr = theme->GetAttribute(text_size);
    ASSERT_TRUE(base_attr.has_value());
    EXPECT_EQ(0x00001002u, base_attr->data) << "TextAppearance textSize golden";

    ASSERT_TRUE(theme->ApplyStyle(small, true /*force*/).has_value());
    auto forced_attr = theme->GetAttribute(text_size);
    ASSERT_TRUE(forced_attr.has_value());

    EXPECT_FALSE(forced_attr->data == base_attr->data)
            << "forced ApplyStyle failed to override the sticky value";
}

TEST_F(AssetManager2Test, ResolveReferenceFollowsChains) {
    // A themed attribute reference chain: Theme's textColorPrimary reads as
    // a reference (golden: @color/... 0x01060001) and must flatten through
    // ResolveAttributeReference to a concrete value.
    const uint32_t theme_id = idOf("Theme", "style");
    ASSERT_EQ(0x01030005u, theme_id);
    const uint32_t text_color = idOf("textColorPrimary", "attr");
    if (text_color == 0u) GTEST_SKIP() << "textColorPrimary not in slim attr set";
    ASSERT_EQ(0x01010036u, text_color);

    auto theme = am_.NewTheme();
    ASSERT_TRUE(theme->ApplyStyle(theme_id).has_value());

    auto value = theme->GetAttribute(text_color);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(0x01, value->type) << "raw theme entry golden: a reference";
    EXPECT_EQ(0x01060001u, value->data);
    auto resolved = theme->ResolveAttributeReference(*value);
    EXPECT_TRUE(resolved.has_value()) << "theme attr reference chain must resolve";
    EXPECT_NE(Res_value::TYPE_REFERENCE, value->type)
            << "final value should not stay a raw reference";
}

TEST_F(AssetManager2Test, GetResourceIdRoundTrips) {
    // Name -> id -> name round trip through AM2.
    const uint32_t ok = idOf("ok", "string");
    ASSERT_EQ(0x0104000au, ok);

    auto found = am_.GetResourceId("android:string/ok");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(ok, found.value());

    auto name = am_.GetResourceName(ok);
    ASSERT_TRUE(name.has_value());
    ASSERT_NE(nullptr, name.value().package);
    EXPECT_STREQ("android", name.value().package);
    // The pak's type/key pools are UTF-16, so by the AOSP contract the Utf16
    // faces are the populated ones ("Utf8 strings are preferred, and only if
    // they are unavailable are the Utf16 variants populated").
    if (name.value().type != nullptr) {
        EXPECT_STREQ("string", name.value().type);
    } else {
        ASSERT_NE(nullptr, name.value().type16);
        EXPECT_EQ(u"string", std::u16string(name.value().type16, name.value().type_len));
    }
    if (name.value().entry != nullptr) {
        EXPECT_STREQ("ok", name.value().entry);
    } else {
        ASSERT_NE(nullptr, name.value().entry16);
        EXPECT_EQ(u"ok", std::u16string(name.value().entry16, name.value().entry_len));
    }
}

TEST_F(AssetManager2Test, ConfigurationsCountMatchesGolden) {
    auto am2_configs = am_.GetResourceConfigurations();
    ASSERT_TRUE(am2_configs.has_value());
    // Golden: the slim framework pak carries 22 distinct config variants
    // (tracks src/gui/res content).
    EXPECT_EQ(22u, am2_configs.value().size());
}
