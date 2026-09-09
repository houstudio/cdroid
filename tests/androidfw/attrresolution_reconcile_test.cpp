// The ③-3 style-resolution semantics, now guarded directly against the LIVE
// AM2 engine (AssetManager2::Theme + AttributeResolution). Originally the
// tests pinned these behaviors on the legacy restable 4-way merge and the
// engine swap reconciled against it; the AM1 retirement removed the legacy
// oracle, so these are the permanent regression tests for the semantics the
// whole widget layer rides on (forced-override-sticky / first-set-wins /
// theme final fallback / bag key order).
#include <gtest/gtest.h>

#include <cstdio>
#include <string>
#include <vector>

#include <content/androidfw/apkassets.h>
#include <content/androidfw/assetmanager2.h>
#include "resourcetypes.h"

using namespace cdroid;

namespace {

std::string frameworkPakPath() {
    static const char* candidates[] = { "cdroid.pak", "../../../cdroid.pak", nullptr };
    for (const char** c = candidates; *c; c++) {
        if (FILE* f = fopen(*c, "rb")) { fclose(f); return *c; }
    }
    return "";
}

}  // namespace

class AttrResolutionReconcileTest : public testing::Test {
protected:
    void SetUp() override {
        const std::string pak = frameworkPakPath();
        if (pak.empty()) GTEST_SKIP() << "cdroid.pak not found — run from the build dir";
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

// Priority 1: a FORCED ApplyStyle must replace values already set by an
// earlier sticky application (AOSP Theme::ApplyStyle force semantics — the
// widget layer's defStyleAttr → xml-style forced chain depends on it).
TEST_F(AttrResolutionReconcileTest, ForcedXmlStyleOverridesStickyDefaults) {
    const uint32_t text_appearance = idOf("TextAppearance", "style");
    ASSERT_NE(0u, text_appearance);
    const uint32_t text_appearance_small = idOf("TextAppearance.Small", "style");
    ASSERT_NE(0u, text_appearance_small);

    // Chain: the base style first (sticky), then the more specific style
    // FORCED — the forced application must replace the sticky values.
    auto chain = am_.NewTheme();
    ASSERT_TRUE(chain->ApplyStyle(text_appearance).has_value());
    ASSERT_TRUE(chain->ApplyStyle(text_appearance_small, true /*force*/).has_value());

    // textSize is defined by BOTH styles — the forced (more specific) one wins.
    const uint32_t text_size = idOf("textSize", "attr");
    ASSERT_NE(0u, text_size);
    auto v = chain->GetAttribute(text_size);
    ASSERT_TRUE(v.has_value());

    // TextAppearance.Small's textSize must differ from TextAppearance's for
    // this to prove the override; apply the base alone for comparison.
    auto plain = am_.NewTheme();
    ASSERT_TRUE(plain->ApplyStyle(text_appearance).has_value());
    auto base = plain->GetAttribute(text_size);
    ASSERT_TRUE(base.has_value());
    EXPECT_FALSE(v->data == base->data && v->type == base->type)
            << "forced style failed to override the sticky default (values identical)";
}

// Priority 2: sticky application order — a base style attribute keeps the
// FIRST value when the same attr exists in a later-applied style too
// ("first-set wins", the defStyleAttr-before-defStyleRes chain contract).
TEST_F(AttrResolutionReconcileTest, StickyFirstSetWinsAcrossChain) {
    const uint32_t base_style = idOf("TextAppearance", "style");
    ASSERT_NE(0u, base_style);
    const uint32_t other_style = idOf("TextAppearance.Material", "style");
    if (other_style == 0u) GTEST_SKIP() << "Material variant not in slim res";

    auto chain = am_.NewTheme();
    ASSERT_TRUE(chain->ApplyStyle(base_style).has_value());
    ASSERT_TRUE(chain->ApplyStyle(other_style).has_value());

    const uint32_t text_size = idOf("textSize", "attr");
    ASSERT_NE(0u, text_size);
    auto chained = chain->GetAttribute(text_size);
    ASSERT_TRUE(chained.has_value());

    auto plain = am_.NewTheme();
    ASSERT_TRUE(plain->ApplyStyle(base_style).has_value());
    auto plain_v = plain->GetAttribute(text_size);
    ASSERT_TRUE(plain_v.has_value());

    // Sticky: the base (first-applied) style's value survives the second apply.
    EXPECT_TRUE(chained->data == plain_v->data && chained->type == plain_v->type)
            << "second sticky ApplyStyle overrode the first-set value";
}

// Priority 3: theme-level attributes are the LAST resort — an attr absent
// from every style source must still resolve from the theme.
TEST_F(AttrResolutionReconcileTest, ThemeIsFinalFallback) {
    const uint32_t theme_style = idOf("Theme", "style");
    ASSERT_NE(0u, theme_style);

    auto theme = am_.NewTheme();
    ASSERT_TRUE(theme->ApplyStyle(theme_style).has_value());

    // textColorPrimary is defined by Theme in the slim res.
    const uint32_t text_color = idOf("textColorPrimary", "attr");
    if (text_color == 0u) GTEST_SKIP() << "textColorPrimary not in slim attr set";
    auto v = theme->GetAttribute(text_color);
    ASSERT_TRUE(v.has_value()) << "theme must resolve attrs absent from style sources";
    EXPECT_NE(Res_value::TYPE_NULL, v->type);
}

// Bag shape: a real style must read back as a ResolvedBag whose entries are
// sorted by resource id (the BackTrackingAttributeFinder contract that
// BagAttributeFinder relies on).
TEST_F(AttrResolutionReconcileTest, StyleBagEntriesAreSortedById) {
    const uint32_t style = idOf("TextAppearance", "style");
    ASSERT_NE(0u, style);
    auto bag = am_.GetBag(style);
    ASSERT_TRUE(bag.has_value());
    const ResolvedBag* resolved = *bag;
    ASSERT_GT(resolved->entry_count, 4u);
    for (size_t i = 1; i < resolved->entry_count; i++) {
        EXPECT_LT(resolved->entries[i - 1].key, resolved->entries[i].key)
                << "style bag not sorted by attr id at " << i
                << " — BackTrackingAttributeFinder contract broken";
    }
}
