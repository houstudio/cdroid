#include <gtest/gtest.h>
#include <cdroid.h>
#include <core/assets.h>
#include <drawables/drawables.h>
#include <fstream>
#include <sstream>
#include <core/systemclock.h>
#include <drawables/drawableinflater.h>
using namespace Cairo;
class MUTATE:public testing::Test{
public:
    static void SetUpTestCase(){
    }
    static void TearDownCase(){
    }
    virtual void SetUp(){
    }
    virtual void TearDown(){
    }
};

TEST_F(MUTATE,color){
    ColorDrawable*d1=new ColorDrawable(0xFFF0000);
    ColorDrawable*d2=(ColorDrawable*)d1->getConstantState()->newDrawable();
    ASSERT_EQ(d1->getConstantState(),d2->getConstantState());
    d2->setColor(0xFFFFFFFF);
    ASSERT_EQ(d1->getColor(),d2->getColor());
    ASSERT_EQ(d1->getColor(),0xFFFFFFFF);
    d2->mutate();
    d2->setColor(0x00FF0000);
    ASSERT_EQ((uint32_t)d1->getColor(),0xFFFFFFFF);
    ASSERT_EQ((uint32_t)d2->getColor(),0x00FF0000);
    ASSERT_NE(d1->getColor(),d2->getColor());
}

TEST_F(MUTATE,bitmap){
    RefPtr<ImageSurface>img=ImageSurface::create(Cairo::Surface::Format::ARGB32,100,100);
    BitmapDrawable*d1=new BitmapDrawable(img);
    BitmapDrawable*d2=(BitmapDrawable*)d1->getConstantState()->newDrawable();
    ASSERT_EQ(d1->getBitmap().get(),d2->getBitmap().get());
}

TEST_F(MUTATE,bitmap2){
    RefPtr<ImageSurface>img;//=cdroid::Context::loadImage("/home/houzh/Miniwin/src/gui/res/mipmap/seek_thumb_selected.png");
    BitmapDrawable*d1=new BitmapDrawable(img);
    BitmapDrawable*d2=(BitmapDrawable*)d1->getConstantState()->newDrawable();
    ASSERT_NE(dynamic_cast<BitmapDrawable*>(d2),(void*)nullptr);
    ASSERT_EQ(d1->getBitmap().get(),d2->getBitmap().get());
    ASSERT_EQ(d1->getIntrinsicWidth(),d2->getIntrinsicWidth());
    ASSERT_EQ(d1->getIntrinsicHeight(),d2->getIntrinsicHeight());
    ASSERT_EQ(d1->getMinimumWidth(),d2->getMinimumWidth());
    ASSERT_EQ(d1->getMinimumHeight(),d2->getMinimumHeight());

}

TEST_F(MUTATE,ninepatch){
    RefPtr<ImageSurface>img;//=cdroid::Context::loadImage("/home/houzh/Miniwin/src/gui/res/mipmap/btn_default_pressed.9.png");
    NinePatchDrawable*d1=new NinePatchDrawable(img);
    NinePatchDrawable*d2=(NinePatchDrawable*)d1->getConstantState()->newDrawable();
    printf("intrinsicsize d1=%dx%d d2=%dx%d\r\n",d1->getIntrinsicWidth(),d1->getIntrinsicHeight(),
           d2->getIntrinsicWidth(),d2->getIntrinsicHeight());
    ASSERT_NE(dynamic_cast<NinePatchDrawable*>(d2),(void*)nullptr);
    ASSERT_EQ(d1->getIntrinsicWidth(),d2->getIntrinsicWidth());
    ASSERT_EQ(d1->getIntrinsicHeight(),d2->getIntrinsicHeight());
    ASSERT_EQ(d1->getMinimumWidth(),d2->getMinimumWidth());
    ASSERT_EQ(d1->getMinimumHeight(),d2->getMinimumHeight());
}

TEST_F(MUTATE,clip){
    ColorDrawable*c=new ColorDrawable(0xFFFF0000);
    ClipDrawable*d1=new ClipDrawable(c,0,0);
    ClipDrawable*d2=(ClipDrawable*)d1->getConstantState()->newDrawable();
    ASSERT_NE(d2->getDrawable(),(void*)nullptr);
    ASSERT_NE(dynamic_cast<ClipDrawable*>(d2),(void*)nullptr);
    delete d1;
    delete d2;
}

TEST_F(MUTATE,clipshape){
    ShapeDrawable*c=new ShapeDrawable();
    ClipDrawable*d1=new ClipDrawable(c,0,0);
    ClipDrawable*d2=(ClipDrawable*)d1->getConstantState()->newDrawable();
    ASSERT_NE(d2->getDrawable(),(void*)nullptr);
    ASSERT_NE(dynamic_cast<ShapeDrawable*>(d2->getDrawable()),(void*)nullptr);
    delete d1;
    delete d2;
}

TEST_F(MUTATE,statelist){
    StateListDrawable*d1=new StateListDrawable();
    std::vector<int>s1={1};
    d1->addState(s1,new ColorDrawable(0xFF000000));
    std::vector<int>s2={1,2};
    d1->addState(s2,new ColorDrawable(0xFF00FF00));
    StateListDrawable*d2=(StateListDrawable*)d1->getConstantState()->newDrawable();
    for(int i=0;i<d1->getStateCount();i++){
        const std::vector<int>&s1=d1->getStateSet(i);
        const std::vector<int>&s2=d2->getStateSet(i);
        ASSERT_EQ(s1.size(),s2.size());
        ASSERT_EQ(0,memcmp(s1.data(),s2.data(),sizeof(int)*s1.size()));
    }
    delete d1;
    delete d2;
}

TEST_F(MUTATE,statelist2){
    StateListDrawable*d1=(StateListDrawable*)DrawableInflater::loadDrawable(nullptr,"/home/houzh/Miniwin/src/gui/res/drawable/seek_thumb.xml");
    StateListDrawable*d2=dynamic_cast<StateListDrawable*>(d1->getConstantState()->newDrawable());
    ASSERT_TRUE(d2->getChildCount()>0);

    for(int i=0;i<d1->getStateCount();i++){
        const std::vector<int>&s1=d1->getStateSet(i);
        const std::vector<int>&s2=d2->getStateSet(i);
        ASSERT_EQ(s1.size(),s2.size());
        ASSERT_EQ(0,memcmp(s1.data(),s2.data(),sizeof(int)*s1.size()));
    }
    for(int i=0;i<d2->getChildCount();i++){
        d2->selectDrawable(i);
        Drawable*nd1=d1->getChild(i);
        Drawable*nd2=d2->getChild(i);
        ASSERT_NE(nd1,(void*)nullptr);
        ASSERT_NE(nd2,(void*)nullptr);
        ASSERT_EQ(nd1->getIntrinsicWidth(),nd2->getIntrinsicWidth());
        ASSERT_EQ(nd1->getIntrinsicHeight(),nd2->getIntrinsicHeight());
        ASSERT_EQ(nd1->getMinimumWidth(),nd2->getMinimumWidth());
        ASSERT_EQ(nd1->getMinimumHeight(),nd2->getMinimumHeight());
    }
    delete d1;
    delete d2;
}

TEST_F(MUTATE,layer){
    LayerDrawable*d1=(LayerDrawable*)DrawableInflater::loadDrawable(nullptr,"/home/houzh/Miniwin/src/gui/res/drawable/progress_horizontal.xml");
    LayerDrawable*d2=dynamic_cast<LayerDrawable*>(d1->getConstantState()->newDrawable());

    ASSERT_GT(d2->getNumberOfLayers(),0);
    ASSERT_EQ(d1->getIntrinsicWidth(),d2->getIntrinsicWidth());
    ASSERT_EQ(d1->getIntrinsicHeight(),d2->getIntrinsicHeight());
    ASSERT_EQ(d1->getMinimumWidth(),d2->getMinimumWidth());
    ASSERT_EQ(d1->getMinimumHeight(),d2->getMinimumHeight());
    for(int i=0;i<d2->getNumberOfLayers();i++){
        Drawable*nd1=d1->getDrawable(i);
        Drawable*nd2=d2->getDrawable(i);
        ASSERT_NE(nd2,(void*)nullptr);
        ASSERT_NE(nd2,(void*)nullptr);
        printf("Intrinsicsize nd1=%dx%d,nd2=%dx%d   level=%d,%d \r\n",nd1->getIntrinsicWidth(),nd1->getIntrinsicHeight(),
               nd2->getIntrinsicWidth(),nd2->getIntrinsicHeight(),nd1->getLevel(),nd2->getLevel());
        ASSERT_EQ(nd1->getIntrinsicWidth(),nd2->getIntrinsicWidth());
        ASSERT_EQ(nd1->getIntrinsicHeight(),nd2->getIntrinsicHeight());
        ASSERT_EQ(nd1->getMinimumWidth(),nd2->getMinimumWidth());
        ASSERT_EQ(nd1->getMinimumHeight(),nd2->getMinimumHeight());
        ASSERT_EQ(nd1->getLevel(),nd2->getLevel());
        if(i==0){
            ASSERT_NE(dynamic_cast<ShapeDrawable*>(nd1),(void*)nullptr);
            ASSERT_NE(dynamic_cast<ShapeDrawable*>(nd2),(void*)nullptr);
        }else{
            ASSERT_NE(dynamic_cast<ClipDrawable*>(nd1),(void*)nullptr);
            ASSERT_NE(dynamic_cast<ClipDrawable*>(nd2),(void*)nullptr);
        }
    }
    delete d1;
    delete d2;
}

TEST_F(MUTATE,parsexml){
    Drawable*d1,*d2;
    d1=DrawableInflater::loadDrawable(nullptr,"/home/houzh/cdroid/src/gui/res/drawable/progress_horizontal.xml");
    ASSERT_EQ(d1->getConstantState().use_count(),1);
    d2=d1->getConstantState()->newDrawable();
    ASSERT_EQ(d1->getConstantState().use_count(),2);
    ASSERT_EQ(d1->getConstantState(),d2->getConstantState());
    //delete d1;
    //delete d2;
}

