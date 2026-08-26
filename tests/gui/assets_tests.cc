#include <gtest/gtest.h>
#include <core/app.h>
#include <drawable/colordrawable.h>
#include <drawable/animationdrawable.h>
#include <drawable/animatedstatelistdrawable.h>
#include <drawable/animatedvectordrawable.h>
#include <drawable/statelistdrawable.h>
#include <drawable/transitiondrawable.h>
#include "R.h"
#include <drawable/vectordrawable.h>
#include <drawable/ninepatchdrawable.h>
#include <drawable/bitmapdrawable.h>
#include <guienvironment.h>
#include <widget/internal_R.h>
#include <content/typedarray.h>
#include <content/Locale.h>
#include <content/assetmanager.h>
using namespace cdroid;

class ASSETS:public testing::Test{
public:
    int argc;
    const char**argv;
    virtual void SetUp(){
        argc = GUIEnvironment::getInstance()->getArgc();
        argv = GUIEnvironment::getInstance()->getArgv();
    }
    virtual void TearDown(){
    }
};

TEST_F(ASSETS,string){
   App&app=App::getInstance();
   std::string str=app.getResources().getString(cdroid::internal::R::string::number_picker_decrement_button);
   printf("str=%s\n",str.c_str());
}
// getArray stays on the string face: it renders reference-typed elements
// ("@color/...") as strings, which the arsc bridge handles; the id-face
// Resources.getStringArray via the int face. The two old string-key forms
// ("cdroid:array/..." / "@cdroid:array/...") were the only thing array2 tested
// differently — both retired with the string-key getters, so the cases merged.
TEST_F(ASSETS,array){
   App&app=App::getInstance();
   std::vector<std::string>array = app.getResources().getStringArray(
           gui_test::R::array::string_array_test);
   for(auto a:array)printf("%s\r\n",a.c_str());
   printf("size=%lu\r\n",array.size());
   ASSERT_EQ(array.size(),(size_t)3);
}
TEST_F(ASSETS,color){
    App&app=App::getInstance();
    /* theme-attribute reference: the int face resolves through
       obtainStyledAttributes (AOSP Theme.obtainStyledAttributes(int[])),
       the counterpart of getColorStateList("?attr/...") on the string face. */
    const uint32_t bgAttrs[] = {cdroid::internal::R::attr::colorBackground, 0};
    auto ta = app.obtainStyledAttributes(bgAttrs);
    ASSERT_TRUE(ta != nullptr);
    auto cl = ta->getColorStateList(0);
    ASSERT_TRUE(cl!=NULL);
    const uint32_t cpAttrs[] = {cdroid::internal::R::attr::colorPrimary, 0};
    ta = app.obtainStyledAttributes(cpAttrs);
    cl = ta->getColorStateList(0);
    ASSERT_TRUE(cl!=NULL);
    /* ColorStateList::dump() retired */
}
TEST_F(ASSETS,drawable){
    App&app=App::getInstance();
    ColorDrawable* cl = (ColorDrawable*)app.getDrawable(cdroid::internal::R::color::black);
    ASSERT_TRUE(cl!=NULL);
    LOGD("COLOR=%x",(uint32_t)cl->getColor());
    ASSERT_EQ((uint32_t)cl->getColor(),(uint32_t)0xFF000000);
    cl=(ColorDrawable*)app.getDrawable(cdroid::internal::R::color::transparent);
    ASSERT_TRUE(cl!=NULL);
    LOGD("COLOR=%x",(uint32_t)cl->getColor());
    ASSERT_EQ((uint32_t)cl->getColor(),0);
    pumpFor(100);
}

TEST_F(ASSETS,animation_list){
    App&app=App::getInstance();
    AnimationDrawable*ad=(AnimationDrawable*)app.getDrawable(cdroid::internal::R::drawable::progress_indeterminate_horizontal);
    ASSERT_EQ(ad->getChildCount(),3);
    for(int i=0;i<ad->getChildCount();i++) ASSERT_NE(dynamic_cast<BitmapDrawable*>(ad->getChild(i)),nullptr);
}

TEST_F(ASSETS,state_layerlist){
    App&app=App::getInstance();
    StateListDrawable* st = (StateListDrawable*)app.getDrawable(cdroid::internal::R::drawable::list_selector_background);
    ASSERT_NE(st,nullptr);
    ASSERT_EQ(st->getChildCount(),6);
    ASSERT_NE(dynamic_cast<ColorDrawable*>(st->getStateDrawable(0)),nullptr);
    ASSERT_NE(dynamic_cast<NinePatchDrawable*>(st->getStateDrawable(1)),nullptr);
    ASSERT_NE(dynamic_cast<NinePatchDrawable*>(st->getStateDrawable(2)),nullptr);
    ASSERT_NE(dynamic_cast<TransitionDrawable*>(st->getStateDrawable(3)),nullptr);
    TransitionDrawable*td1=dynamic_cast<TransitionDrawable*>(st->getStateDrawable(3));
       ASSERT_NE(td1,nullptr);
       ASSERT_EQ(td1->getNumberOfLayers(),2);
       ASSERT_NE(dynamic_cast<NinePatchDrawable*>(td1->getDrawable(0)),nullptr);
       ASSERT_NE(dynamic_cast<NinePatchDrawable*>(td1->getDrawable(1)),nullptr);
    TransitionDrawable*td2=dynamic_cast<TransitionDrawable*>(st->getStateDrawable(4));
       ASSERT_NE(td2,nullptr);
       ASSERT_EQ(td2->getNumberOfLayers(),2);
       ASSERT_NE(dynamic_cast<NinePatchDrawable*>(td2->getDrawable(0)),nullptr);
       ASSERT_NE(dynamic_cast<NinePatchDrawable*>(td2->getDrawable(1)),nullptr);
    ASSERT_NE(dynamic_cast<TransitionDrawable*>(st->getStateDrawable(4)),nullptr);
    ASSERT_NE(dynamic_cast<NinePatchDrawable*>(st->getStateDrawable(5)),nullptr);
    pumpFor(100);
}

TEST_F(ASSETS,animated_selector){
    App&app=App::getInstance();
    AnimatedStateListDrawable* asd = (AnimatedStateListDrawable*)app.getDrawable(cdroid::internal::R::drawable::btn_check_material_anim);
    ASSERT_NE(asd,nullptr);
    ASSERT_EQ(asd->getChildCount(),4);
    ASSERT_NE(dynamic_cast<VectorDrawable*>(asd->getStateDrawable(0)),nullptr);
    ASSERT_NE(dynamic_cast<VectorDrawable*>(asd->getStateDrawable(1)),nullptr);
    ASSERT_NE(dynamic_cast<AnimatedVectorDrawable*>(asd->getStateDrawable(2)),nullptr);
    ASSERT_NE(dynamic_cast<AnimatedVectorDrawable*>(asd->getStateDrawable(3)),nullptr);
    AnimatedVectorDrawable*td1 = dynamic_cast<AnimatedVectorDrawable*>(asd->getStateDrawable(2));
       ASSERT_NE(td1,nullptr);
       ASSERT_NE(dynamic_cast<Drawable*>(td1),nullptr);
    AnimatedVectorDrawable* td2 = dynamic_cast<AnimatedVectorDrawable*>(asd->getStateDrawable(3));
       ASSERT_NE(td2,nullptr);
       ASSERT_NE(dynamic_cast<Drawable*>(td2),nullptr);
    pumpFor(100);
}
TEST_F(ASSETS,animatedselector){
    App&app=App::getInstance();
    AnimatedStateListDrawable* asd = (AnimatedStateListDrawable*)app.getDrawable(cdroid::internal::R::drawable::btn_radio_material_anim);
    ASSERT_NE(asd,nullptr);
    ASSERT_EQ(asd->getChildCount(),4);
    ASSERT_NE(dynamic_cast<VectorDrawable*>(asd->getStateDrawable(0)),nullptr);
    ASSERT_NE(dynamic_cast<VectorDrawable*>(asd->getStateDrawable(1)),nullptr);
    ASSERT_NE(dynamic_cast<AnimatedVectorDrawable*>(asd->getStateDrawable(2)),nullptr);
    ASSERT_NE(dynamic_cast<AnimatedVectorDrawable*>(asd->getStateDrawable(3)),nullptr);

    LOGD("AnimatedStateListDrawable %p",asd);
    LOGD("    %p VectorDrawable",asd->getStateDrawable(0));
    LOGD("    %p VectorDrawable",asd->getStateDrawable(1));
    LOGD("    %p AnimatedVectorDrawable",asd->getStateDrawable(2));
    LOGD("    %p AnimatedVectorDrawable",asd->getStateDrawable(3));

    AnimatedVectorDrawable*td1 = dynamic_cast<AnimatedVectorDrawable*>(asd->getStateDrawable(2));
       ASSERT_NE(td1,nullptr);
       ASSERT_NE(dynamic_cast<Drawable*>(td1),nullptr);
    AnimatedVectorDrawable*td2 = dynamic_cast<AnimatedVectorDrawable*>(asd->getStateDrawable(3));
       ASSERT_NE(td2,nullptr);
       ASSERT_NE(dynamic_cast<Drawable*>(td2),nullptr);
    pumpFor(100);
}


// AOSP Resources.Theme face for framework-internal consumers: the typed reads
// (resolveAttribute / Theme.obtainStyledAttributes) plus the @hide
// introspection (getAllAttributes/getChangingConfigurations/rebase), exercised
// against the default Theme.Material — no Context/AttributeSet involved.
TEST_F(ASSETS, theme_face){
    App&app=App::getInstance();
    Resources::Theme theme = app.getTheme();

    // @hide getAllAttributes: the default theme carries real entries.
    const auto attrs = theme.getAllAttributes();
    ASSERT_GT(attrs.size(),(size_t)0) << "default Theme.Material resolved empty";

    // resolveAttribute (AOSP resolveRefs form) on a themed attr.
    TypedValue v;
    ASSERT_TRUE(theme.resolveAttribute((int)attrs.front(),&v,true));

    // Theme.obtainStyledAttributes(int[]) — the entry point internal
    // facilities use (no AttributeSet, no Context).
    const uint32_t set[] = {attrs.front(), attrs.back(), 0};
    auto ta = theme.obtainStyledAttributes(set);
    ASSERT_NE(ta,nullptr);
    ASSERT_TRUE(ta->hasValue(0) || ta->hasValue(1));

    // newTheme() + setTo() + applyStyle() + rebase() (@hide): an OWNED theme
    // the internal facility can restyle without touching the app's theme.
    const int colorPrimary = (int)cdroid::internal::R::attr::colorPrimary;
    auto own = app.getResources().newTheme();
    own.setTo(theme);   // this snapshot becomes the rebase base
    TypedValue v2;
    ASSERT_TRUE(own.resolveAttribute(colorPrimary,&v2,true));
    own.applyStyle(gui_test::R::style::theme_face_probe, /*force=*/true); // non-force would keep Theme.Material's value
    ASSERT_TRUE(own.resolveAttribute(colorPrimary,&v2,false));
    ASSERT_EQ(v2.data,(uint32_t)0xFF123456) << "applyStyle did not overlay the probe";
    own.rebase();      // erases the probe overlay, restores the setTo snapshot
    ASSERT_TRUE(own.resolveAttribute(colorPrimary,&v2,true));
    ASSERT_NE(v2.data,(uint32_t)0xFF123456) << "rebase did not restore the setTo state";

    theme.dump("theme_face");
}

// AOSP AssetManager.getLocales via the Assets face: the distinct "xx-YY"
// configs the loaded arsc carries (framework values-* survive the SDK filter
// — locales:[] keeps all). Each tag feeds Locale::forLanguageTag(), which is
// the "language list → matching country" pairing apps need for a language
// settings menu.
TEST_F(ASSETS, locales)
{
    App& app = App::getInstance();
    const std::vector<std::string> locales = app.getAssets().getLocales();
    ASSERT_GT(locales.size(), (size_t)0);

    auto has = [&locales](const char* t) {
        return std::find(locales.begin(), locales.end(), t) != locales.end();
    };
    // Framework res carries values-zh-rCN / values-zh-rTW / values-ar.
    EXPECT_TRUE(has("zh-CN"));
    EXPECT_TRUE(has("zh-TW"));
    EXPECT_TRUE(has("ar"));

    // Two AOSP paths: system locales (framework-res / android package alone)
    // vs non-system (the app's own languages).
    const std::vector<std::string> system = app.getAssets().getSystemLocales();
    ASSERT_GT(system.size(), (size_t)0);
    auto sysHas = [&system](const char* t) {
        return std::find(system.begin(), system.end(), t) != system.end();
    };
    EXPECT_TRUE(sysHas("zh-CN"));   // framework values-zh-rCN
    EXPECT_TRUE(sysHas("ar"));
    // Every non-system locale is a subset of the full list.
    const std::vector<std::string> nonSystem = app.getAssets().getNonSystemLocales();
    for (const std::string& l : nonSystem) {
        EXPECT_TRUE(has(l.c_str())) << "non-system locale missing from union: " << l;
    }

    // Language → country pairing straight off the tag.
    const Locale zhCN = Locale::forLanguageTag("zh-CN");
    EXPECT_EQ(zhCN.getLanguage(), "zh");
    EXPECT_EQ(zhCN.getCountry(), "CN");
    const Locale zhTW = Locale::forLanguageTag("zh-TW");
    EXPECT_EQ(zhTW.getCountry(), "TW");
}
