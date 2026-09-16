#include <gtest/gtest.h>
#include <core/app.h>
#include <widget/cdwindow.h>
#include <widget/textview.h>
#include <animation/animator.h>
#include <animation/valueanimator.h>
#include <animation/property.h>
#include <animation/propertyvaluesholder.h>
#include <porting/cdlog.h>
#include <animation/objectanimator.h>
#include <guienvironment.h>
using namespace cdroid;
class ANIMATOR:public testing::Test{
public:
    int argc;
    const char**argv;
public :
   virtual void SetUp(){
       argc = GUIEnvironment::getInstance()->getArgc();
       argv = GUIEnvironment::getInstance()->getArgv();
   }
   virtual void TearDown(){
   }
};

/* Pump until onAnimationEnd — the definitive completion signal (isStarted()/
   isRunning() are unreliable during startDelay). maxMs is a safety cap. */
static void pumpUntilEnd(Animator&anim,int maxMs){
    bool ended=false;
    Animator::AnimatorListener lst;
    lst.onAnimationEnd=[&ended](Animator&,bool){ ended=true; };
    anim.addListener(lst);
    pumpUntil([&ended]{ return ended; }, maxMs);
}

TEST_F(ANIMATOR,callback){
    App&app=App::getInstance();
    ValueAnimator *anim=new ValueAnimator();
    anim->getAnimationHandler().addAnimationFrameCallback(anim,100);
    anim->getAnimationHandler().removeCallback(anim);
    pumpUntilIdle(300);
}

TEST_F(ANIMATOR,ofInt1){
    ValueAnimator*anim=ValueAnimator::ofInt({0,100});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        LOGD("value=%d",GET_VARIANT(anim.getAnimatedValue(),int));
    }));
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,ofInt2){
    IntPropertyValuesHolder iprop;
    iprop.setValues(std::vector<int>({0,100}));
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&iprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        LOGD("value=%d",GET_VARIANT(anim.getAnimatedValue(),int));
    })); 
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,ofFloat){
    FloatPropertyValuesHolder fprop;
    fprop.setValues(std::vector<float>({0,100}));
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        LOGD("value=%f",GET_VARIANT(anim.getAnimatedValue(),float));
    })); 
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,start){
    App&app=App::getInstance();
    FloatPropertyValuesHolder fprop;
    fprop.setValues(std::vector<float>({0,100}));
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        LOGD("value=%f",GET_VARIANT(anim.getAnimatedValue(),float));
    }));
    anim->setDuration(200);
    anim->start();
    LOGD("====");
    pumpUntilEnd(*anim,1600);
}
TEST_F(ANIMATOR,startDelay){
    App&app=App::getInstance();
    FloatPropertyValuesHolder fprop;
    fprop.setValues(std::vector<float>({0,100}));
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        LOGD("value=%f",GET_VARIANT(anim.getAnimatedValue(),float));
    }));
    anim->setDuration(200);
    anim->setStartDelay(1000);
    anim->start();
    LOGD("====");
    pumpUntilEnd(*anim, 1500);
}

class MyProperty: public Property{
public:
   MyProperty(const std::string&name):Property(name,FLOAT_TYPE){
   }
   void set(void* object,const AnimateValue& value)const override{
       LOGD("value=%f",GET_VARIANT(value,float));
   }
};
TEST_F(ANIMATOR,ofProperty){
    MyProperty*myprop=new MyProperty("test");
    ObjectAnimator*anim=ObjectAnimator::ofFloat(nullptr,myprop,{0.f,100.f});
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,loopdrivered){
    App&app=App::getInstance();
    IntPropertyValuesHolder iprop;
    iprop.setValues(std::vector<int>({0,100}));

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&iprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        LOGD("value=%d",GET_VARIANT(anim.getAnimatedValue(),int));
    })); 
    anim->setDuration(2000);
    anim->start();
    pumpUntilEnd(*anim, 2500);
}

TEST_F(ANIMATOR,translate){
    App&app=App::getInstance();
    ViewGroup*w=GUIEnvironment::content();
    TextView*tv=new TextView(&app); tv->setText("Hello World!");
    tv->setBackgroundColor(0xFF111111);
    w->addView(tv);

    IntPropertyValuesHolder xprop;
    xprop.setPropertyName("x");
    xprop.setValues(std::vector<int>({0,100,300}));

    IntPropertyValuesHolder yprop;
    yprop.setPropertyName("y");
    yprop.setValues(std::vector<int>({0,200,200}));

    PropertyValuesHolder cprop;
    cprop.setValues(std::vector<int32_t>({int(0xFF000000),int(0xFFFF8844)}));

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&xprop,&yprop,&cprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([tv](ValueAnimator&anim){
        PropertyValuesHolder*xp=anim.getValues(0);
        PropertyValuesHolder*yp=anim.getValues(1);
        PropertyValuesHolder*cp=anim.getValues(2);
        tv->setLeft(GET_VARIANT(xp->getAnimatedValue(),int));
        tv->setTop(GET_VARIANT(yp->getAnimatedValue(),int));
        tv->setBackgroundColor(GET_VARIANT(cp->getAnimatedValue(),int32_t));
    }));

    anim->setDuration(4000);
    anim->start();
    pumpUntilEnd(*anim, 5500);
}
TEST_F(ANIMATOR,scale){
    App&app=App::getInstance();
    ViewGroup*w=GUIEnvironment::content();
    TextView*tv=new TextView(&app); tv->setText("Hello World!");
    tv->setBackgroundColor(0xFF111111);
    w->addView(tv);

    FloatPropertyValuesHolder fprop;
    fprop.setPropertyName("scale");
    fprop.setValues(std::vector<float>({0,2.0}));

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([tv](ValueAnimator&anim){
        const float scale=GET_VARIANT(anim.getAnimatedValue(),float);
        tv->setScaleX(scale);
        tv->setScaleY(scale);
    }));
    anim->setDuration(4000);
    anim->start();
    pumpUntilEnd(*anim, 5500);
}


// AOSP ofFloat(target, x, y, path) / PropertyValuesHolder.ofPointF(property,
// path): the programmatic PathKeyframes route (Path::approximate's
// error-bounded sampling, X/Y projections) — the same machinery the XML
// pathxy route uses, replacing the retired uniform N=32 sampling.
TEST_F(ANIMATOR,pathKeyframesProgrammatic){
    App&app=App::getInstance();
    auto path=Cairo::RefPtr<cdroid::Path>(new cdroid::Path());
    path->moveTo(0,0);
    path->lineTo(100,100);

    MyProperty propX("pathX"), propY("pathY");
    ObjectAnimator*anim=ObjectAnimator::ofFloat(nullptr,&propX,&propY,path);
    anim->setCurrentFraction(0.5f);
    const std::vector<PropertyValuesHolder*>&holders=anim->getValues();
    ASSERT_EQ(holders.size(),(size_t)2);
    // Midpoint of the diagonal: (50, 50).
    EXPECT_NEAR(GET_VARIANT(holders[0]->getAnimatedValue(),float),50.f,0.5f);
    EXPECT_NEAR(GET_VARIANT(holders[1]->getAnimatedValue(),float),50.f,0.5f);

    MyProperty pointProp("pathPoint");
    PropertyValuesHolder*point=PropertyValuesHolder::ofPointF(&pointProp,path);
    ValueAnimator*pointAnim=ValueAnimator::ofPropertyValuesHolder({point});
    pointAnim->setCurrentFraction(0.5f);
    const PointF&p=GET_VARIANT(point->getAnimatedValue(),PointF);
    EXPECT_NEAR(p.x,50.f,0.5f);
    EXPECT_NEAR(p.y,50.f,0.5f);
}
