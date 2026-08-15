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
#include <core/typedarray.h>
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

