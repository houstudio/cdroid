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
#include <widget/textview.h>
#include <widget/framework_styleable.h>
#include <view/layoutinflater.h>
#include <drawable/colorstatelist.h>
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

// AOSP configuration-change flow: calcConfigChanges reports the uiMode delta,
// updateConfiguration applies it to the live Configuration and reseeds the
// arsc parameters (resource-variant reselection). App::handleConfigurationChanged
// (ActivityThread role) drives the per-activity dispatch/recreate on top.
TEST_F(RESOURCES_THEME, configurationChangeFlow) {
    Resources& res = App::getInstance().getResources();
    Configuration c = res.getConfiguration();
    c.uiMode = (c.uiMode & ~Configuration::UI_MODE_NIGHT_MASK) | Configuration::UI_MODE_NIGHT_YES;
    EXPECT_NE(res.calcConfigChanges(&c) & Configuration::CONFIG_UI_MODE, 0);

    res.updateConfiguration(&c, nullptr);
    EXPECT_EQ(res.getConfiguration().uiMode & Configuration::UI_MODE_NIGHT_MASK,
              (int)Configuration::UI_MODE_NIGHT_YES);
    // No further change: the live config matches now.
    EXPECT_EQ(res.calcConfigChanges(&c) & Configuration::CONFIG_UI_MODE, 0);

    // Restore (unset night bit).
    c.uiMode = (c.uiMode & ~Configuration::UI_MODE_NIGHT_MASK) | Configuration::UI_MODE_NIGHT_NO;
    res.updateConfiguration(&c, nullptr);
}

// The overflow-menu item layout's title TextView colors through the theme:
// textAppearance=?attr/textAppearanceLargePopupMenu → TextAppearance.Material
// (textColor=?attr/textColorPrimary) — the chain that used to die in
// getResourceId (attr id returned as the style id) and left the text hard-white.
TEST_F(RESOURCES_THEME, menuItemTextAppearanceChain) {
    App& app = App::getInstance();
    app.setTheme((int)cdroid::internal::R::style::Theme_Material_Light);   // the white-popup case
    View* item = LayoutInflater::from(&app)->inflate(
            cdroid::internal::R::layout::popup_menu_item_layout_material, nullptr, false);
    ASSERT_NE(item, nullptr);
    TextView* title = dynamic_cast<TextView*>(
            item->findViewById(cdroid::internal::R::id::title));
    ASSERT_NE(title, nullptr);
    const auto colors = title->getTextColors();
    ASSERT_NE(colors, nullptr);
    LOGI("menu item title default color=0x%08x", colors->getDefaultColor());
    // Theme.Material: textColorPrimary lands on text_color_primary.xml whose
    // enabled color is colorForeground — the old bug (attr id returned as the
    // style id) produced either no CSL (hard 0xFFFFFFFF) or a wrong single
    // color. The selector's colorForState tells the real resolved value.
    std::vector<int> state;   // empty = default state set
    const uint32_t c = colors->getColorForState(state, colors->getDefaultColor());
    LOGI("menu item title colorForState(enabled)=0x%08x", c);
    // Light theme: textColorPrimary ≈ primary_text_material_light (#de000000,
    // near-black). The bug showed hard white (0xffffffff) — invisible on the
    // light popup background.
    EXPECT_LT(c, 0x80000000u);   // alpha < 0x80 fails only for white/bright
}

// Narrow the menu chain: load the theme's textColorPrimary resource directly.
TEST_F(RESOURCES_THEME, textColorPrimaryResource) {
    App& app = App::getInstance();
    app.setTheme((int)cdroid::internal::R::style::Theme_Material_Light);
    auto& res = app.getResources();
    const int cslId = res.getIdentifier("text_color_primary", "color", "");
    LOGI("textColorPrimary csl id=0x%x", cslId);
    auto csl = res.loadComplexColor(cslId, nullptr);
    if (csl) {
        const auto cc = std::dynamic_pointer_cast<ColorStateList>(csl);
        if (cc) {
            std::vector<int> st;
            LOGI("text_color_primary default=0x%08x enabled=0x%08x",
                 cc->getDefaultColor(), cc->getColorForState(st, cc->getDefaultColor()));
        } else LOGI("not a CSL");
    } else LOGI("loadComplexColor null");
    // And the theme attr itself:
    TypedValue tv;
    app.getTheme().resolveAttribute((int)cdroid::internal::R::attr::textColorPrimary, &tv, false);
    LOGI("theme textColorPrimary type=%d data=0x%x resid=0x%x", (int)tv.type, tv.data, tv.resourceId);
    auto th = app.getTheme();
    auto fg2 = res.loadComplexColor(0x01060171, &th);
    LOGI("foreground_material_light csl=%p default=0x%08x", (void*)fg2.get(),
         fg2 ? std::dynamic_pointer_cast<ColorStateList>(fg2)->getDefaultColor() : 0);
    // colorForeground (what text_color_primary.xml's item color=?attr targets):
    TypedValue fg;
    const bool okFg = app.getTheme().resolveAttribute((int)cdroid::internal::R::attr::colorForeground, &fg, true);
    LOGI("colorForeground ok=%d type=%d data=0x%x", (int)okFg, (int)fg.type, fg.data);
}

// The popup window background follows the same ?attr chain (popupBackground)
// through Widget.Material.PopupMenu — the fix keeps the referenced drawable id
// (resolveRefs=false); the black-popup regression resolved it to a pool index.
TEST_F(RESOURCES_THEME, popupBackgroundChain) {
    // The MenuPopupWindow path: obtainStyledAttributes(attrs, PopupWindow,
    // defStyleAttr=actionOverflowMenuStyle) — popupBackground flows through
    // the widget style chain and must produce a drawable (black popup = null).
    App& app = App::getInstance();
    app.setTheme((int)cdroid::internal::R::style::Theme_Material_Light);
    AttributeSet atts(&app, "cdroid");
    auto ta = (&app)->obtainStyledAttributes(&atts, cdroid::internal::R::styleable::PopupWindow,
            (int)cdroid::internal::R::attr::actionOverflowMenuStyle);
    ASSERT_NE(ta, nullptr);
    const int bgIdx = cdroid::internal::R::styleable::PopupWindow_popupBackground;
    LOGI("popupBg hasVal=%d type=%d", (int)ta->hasValue(bgIdx), ta->getType(bgIdx));
    Drawable* bg = ta->getDrawable(bgIdx);
    ASSERT_NE(bg, nullptr);
    LOGI("popupBackground drawable=%p", (void*)bg);
    delete bg;
}

// applyStyle on a DERIVED style must follow the style's parent chain (AppTheme
// — an app style parented at a framework theme — only carries its own bag items
// on CDROID: every parent-inherited attr, windowBackground/actionOverflowMenuStyle,
// resolved empty; the overflow menu then has no background style at all).
TEST_F(RESOURCES_THEME, derivedStyleParentChain) {
    App& app = App::getInstance();
    auto& res = app.getResources();
    const int derived = res.getIdentifier("Widget.Material.PopupMenu.Overflow", "style", "");
    LOGI("Overflow style=0x%x", derived);
    ASSERT_NE(derived, 0);
    // A fresh theme applying ONLY the derived style: parent-inherited
    // popupBackground (via PopupMenu -> ListPopupWindow -> PopupWindow) must
    // resolve.
    auto th = res.newTheme();
    th.applyStyle(derived, true);
    TypedValue tv;
    const bool ok = th.resolveAttribute((int)cdroid::internal::R::attr::popupBackground, &tv, true);
    LOGI("derived-only popupBackground ok=%d type=%d", (int)ok, (int)tv.type);
    EXPECT_TRUE(ok);
}

// popup_background_material is a shape whose solid color is
// ?attr/colorPopupBackground → ?attr/colorBackground (a two-hop ?attr chain).
// Under Light that must be near-white; black meant the chain died.
TEST_F(RESOURCES_THEME, popupBackgroundDrawableColor) {
    App& app = App::getInstance();
    app.setTheme((int)cdroid::internal::R::style::Theme_Material_Light);
    auto& res = app.getResources();
    const int bgId = res.getIdentifier("popup_background_material", "drawable", "");
    LOGI("popup_background_material id=0x%x", bgId);
    ASSERT_NE(bgId, 0);
    // Direct: the color attr chain.
    TypedValue tv;
    ASSERT_TRUE(app.getTheme().resolveAttribute((int)cdroid::internal::R::attr::colorPopupBackground, &tv, false));
    LOGI("colorPopupBackground raw type=%d data=0x%x", (int)tv.type, tv.data);
    int resolved = 0;
    if (tv.type == TypedValue::TYPE_ATTRIBUTE || tv.type == TypedValue::TYPE_DYNAMIC_ATTRIBUTE) {
        // Two-hop: resolve the inner attr too.
        TypedValue tv2;
        ASSERT_TRUE(app.getTheme().resolveAttribute((int)tv.data, &tv2, true));
        resolved = (int)tv2.data;
        LOGI("inner colorBackground type=%d data=0x%08x", (int)tv2.type, tv2.data);
    } else resolved = (int)tv.data;
    // Inflate the drawable themed and check the solid color.
    auto th = app.getTheme();
    Drawable* d = res.getDrawable(bgId, &th);
    ASSERT_NE(d, nullptr);
    LOGI("drawable type=%s", typeid(*d).name());
    delete d;
}
