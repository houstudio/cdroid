// Port of AOSP CTS Resources_ThemeTest
// (cts/tests/tests/content/src/android/content/res/cts/Resources_ThemeTest.java,
//  android-12). Case-for-case where the gui_test harness supports it; the
// locale/RTL testRebase case is deferred (needs Resources.updateConfiguration
// plumbing) — rebase semantics are covered structurally by ASSETS.theme_face.
#include <gtest/gtest.h>
#include <core/app.h>
#include <core/resources.h>
#include <core/typedarray.h>
#include <core/typedvalue.h>
#include <widget/internal_R.h>
#include <drawable/colordrawable.h>
#include "R.h"
#include <guienvironment.h>
using namespace cdroid;

class RESOURCES_THEME : public testing::Test {
protected:
    Resources::Theme mResTheme;
    RESOURCES_THEME() : mResTheme(App::getInstance().getResources().newTheme()) {}
};

// CTS testSetMethods: the native call-through trio must not blow up.
TEST_F(RESOURCES_THEME, setMethods) {
    mResTheme.applyStyle(gui_test::R::style::theme_face_probe, false);
    mResTheme.dump("hello", "world");
    Resources::Theme other = App::getInstance().getTheme();
    mResTheme.setTo(other);
}

// CTS testObtainStyledAttributes: all three AttributeSet-less overloads.
TEST_F(RESOURCES_THEME, obtainStyledAttributes) {
    const uint32_t attrs[] = {(uint32_t)gui_test::R::attr::testString, 0};
    auto ta = mResTheme.obtainStyledAttributes(attrs);
    ASSERT_NE(ta, nullptr);
    ASSERT_GT(ta->length(), (size_t)0);

    ta = mResTheme.obtainStyledAttributes(gui_test::R::style::Theme_InlineString, attrs);
    ASSERT_NE(ta, nullptr);
    ASSERT_GT(ta->length(), (size_t)0);

    // AttributeSet form over a real XML document (binary AXML path); CTS uses
    // R.xml.colors, this pak has the drawable XMLs instead.
    XmlPullParser parser(&App::getInstance(), gui_test::R::drawable::cts_level_list_correct);
    const AttributeSet& set = parser;
    int type;
    while (((type = parser.next()) != XmlPullParser::START_TAG)
            && (type != XmlPullParser::END_DOCUMENT)) {}
    const uint32_t xmlAttrs[] = {(uint32_t)gui_test::R::attr::testString, 0};
    ta = mResTheme.obtainStyledAttributes(&set, xmlAttrs, 0, 0);
    ASSERT_NE(ta, nullptr);
    ASSERT_GT(ta->length(), (size_t)0);
}

// CTS testObtainStyledAttributesWithInlineStringInTheme: a themed inline
// string reads back as TYPE_STRING with its value.
TEST_F(RESOURCES_THEME, obtainStyledAttributesWithInlineStringInTheme) {
    mResTheme.applyStyle(gui_test::R::style::Theme_InlineString, false);
    const uint32_t attrs[] = {(uint32_t)gui_test::R::attr::testString, 0};
    auto ta = mResTheme.obtainStyledAttributes(attrs);
    ASSERT_NE(ta, nullptr);
    ASSERT_EQ(ta->length(), (size_t)1);
    ASSERT_EQ(ta->getType(0), (int)TypedValue::TYPE_STRING);
    ASSERT_EQ(ta->getString(0), "This is a string");
}

// CTS testResolveAttribute: an attribute the theme never set resolves false.
TEST_F(RESOURCES_THEME, resolveAttribute) {
    TypedValue value;
    // No style was applied to mResTheme in this case, so testString is unset.
    ASSERT_FALSE(mResTheme.resolveAttribute(gui_test::R::attr::testString, &value, false));
}

// CTS testGetResources: the theme reports its owning Resources.
TEST_F(RESOURCES_THEME, getResources) {
    Resources& res = App::getInstance().getResources();
    Resources::Theme theme = res.newTheme();
    ASSERT_EQ(&theme.getResources(), &res);
}

// CTS testEmptyDoesNotGetOverriden: @empty is a sticky real value — a later
// non-forced applyStyle does not override it, a forced one does.
TEST_F(RESOURCES_THEME, emptyDoesNotGetOverriden) {
    Resources& res = App::getInstance().getResources();
    Resources::Theme theme = res.newTheme();

    theme.applyStyle(gui_test::R::style::Theme_Empty, false /*force*/);

    TypedValue tv;
    ASSERT_TRUE(theme.resolveAttribute(gui_test::R::attr::type1, &tv, false));
    ASSERT_EQ(tv.type, (int)TypedValue::TYPE_NULL);
    ASSERT_EQ(tv.data, (int)TypedValue::DATA_NULL_EMPTY);

    // @empty is treated just like a regular value. No override unless forced.
    theme.applyStyle(gui_test::R::style::Whatever, false /*force*/);
    ASSERT_TRUE(theme.resolveAttribute(gui_test::R::attr::type1, &tv, false));
    ASSERT_EQ(tv.type, (int)TypedValue::TYPE_NULL);
    ASSERT_EQ(tv.data, (int)TypedValue::DATA_NULL_EMPTY);

    // Force the override now.
    theme.applyStyle(gui_test::R::style::Whatever, true /*force*/);
    ASSERT_TRUE(theme.resolveAttribute(gui_test::R::attr::type1, &tv, false));
    ASSERT_NE(tv.type, (int)TypedValue::TYPE_NULL);
}

// CTS testGetChangingConfigurations (adapted): applyStyle over a style that
// exists in a config-qualified variant (values-land) marks CONFIG_ORIENTATION.
TEST_F(RESOURCES_THEME, getChangingConfigurations) {
    Resources& res = App::getInstance().getResources();
    Resources::Theme theme = res.newTheme();
    ASSERT_EQ(theme.getChangingConfigurations(), 0)
        << "Initial changing configuration mask is empty";

    theme.applyStyle(gui_test::R::style::Theme_OrientationDependent, true);
    // ActivityInfo.CONFIG_ORIENTATION
    constexpr int CONFIG_ORIENTATION = 0x80;
    ASSERT_EQ(theme.getChangingConfigurations(), CONFIG_ORIENTATION)
        << "applyStyle() sets changing configuration";

    Resources::Theme other = res.newTheme();
    other.setTo(theme);
    ASSERT_EQ(other.getChangingConfigurations(), CONFIG_ORIENTATION)
        << "setTo() copies changing configuration";
}

// CTS testGetDrawable (adapted): theme.getDrawable loads through the theme's
// Resources and returns the themed ColorDrawable value.
TEST_F(RESOURCES_THEME, getDrawable) {
    Resources& res = App::getInstance().getResources();
    Resources::Theme theme = res.newTheme();
    theme.applyStyle(gui_test::R::style::theme_face_probe, true);
    // A framework color as drawable: inline color → ColorDrawable.
    Drawable* dr = theme.getDrawable((int)cdroid::internal::R::color::black);
    ASSERT_NE(dr, nullptr);
    auto* cdr = dynamic_cast<ColorDrawable*>(dr);
    ASSERT_NE(cdr, nullptr);
    ASSERT_EQ(cdr->getColor(), (int)0xFF000000 /* Color.BLACK */);
    delete dr;
}

// CTS testRebase: deferred — locale/config switching (updateConfiguration)
// is not wired in this harness; rebase's snapshot/restore behavior is covered
// by ASSETS.theme_face (setTo → applyStyle(force) → rebase).

// AOSP Drawable.applyTheme(): a drawable inflated WITHOUT a theme carries the
// ?attr as a pending theme attr (mThemeAttrs); applyTheme() re-resolves it
// through the given theme and refreshes the state.
TEST_F(RESOURCES_THEME, applyThemeReResolvesPendingAttrs) {
    Resources& res = App::getInstance().getResources();
    // Unthemed load: pass a null Theme explicitly (no default-theme chain).
    Drawable* dr = res.getDrawable(gui_test::R::drawable::cts_apply_theme_color,
                                   (const Resources::Theme*)nullptr);
    ASSERT_NE(dr, nullptr);
    auto* cdr = dynamic_cast<ColorDrawable*>(dr);
    ASSERT_NE(cdr, nullptr);
    ASSERT_TRUE(cdr->canApplyTheme());

    Resources::Theme themeA = res.newTheme();
    themeA.applyStyle(gui_test::R::style::theme_face_probe, true);
    cdr->applyTheme(themeA);
    EXPECT_EQ(cdr->getColor(), (int)0xFF123456);

    // AOSP applyTheme consumes the pending attrs: a second apply is a no-op.
    EXPECT_FALSE(cdr->canApplyTheme());
    delete cdr;
}
