#include <gtest/gtest.h>
#include <cdroid.h>
#include <cdlog.h>
#include <animation/objectanimator.h>
#include <guienvironment.h>
class ANIMATOR:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(ANIMATOR,callback){
    App app(GUIEnvironment::getInstance()->getArgc(),GUIEnvironment::getInstance()->getArgv());
    ValueAnimator *anim=new ValueAnimator();
    anim->getAnimationHandler().addAnimationFrameCallback(anim,100);
    anim->getAnimationHandler().removeCallback(anim);
    app.exec();
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
    App app(GUIEnvironment::getInstance()->getArgc(),GUIEnvironment::getInstance()->getArgv());
    FloatPropertyValuesHolder fprop;
    fprop.setValues(std::vector<float>({0,100}));
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        LOGD("value=%f",GET_VARIANT(anim.getAnimatedValue(),float));
    }));
    anim->setDuration(200);
    anim->start();
    LOGD("====");
    app.exec();
}
TEST_F(ANIMATOR,startDelay){
    App app(GUIEnvironment::getInstance()->getArgc(),GUIEnvironment::getInstance()->getArgv());
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
    app.exec();
}

class MyProperty: public Property{
public:
   MyProperty(const std::string&name):Property(name){
   }
   void set(void* object,const AnimateValue& value)override{
       LOGD("value=%f",GET_VARIANT(value,float));
   }
};
TEST_F(ANIMATOR,ofProperty){
    MyProperty*myprop=new MyProperty("test");
    ObjectAnimator*anim=ObjectAnimator::ofInt(nullptr,myprop,{0,100});
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,loopdrivered){
    App app(GUIEnvironment::getInstance()->getArgc(),GUIEnvironment::getInstance()->getArgv());
    IntPropertyValuesHolder iprop;
    iprop.setValues(std::vector<int>({0,100}));

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&iprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        LOGD("value=%d",GET_VARIANT(anim.getAnimatedValue(),int));
    })); 
    anim->setDuration(2000);
    anim->start();
    app.exec();
}

TEST_F(ANIMATOR,translate){
    App app(GUIEnvironment::getInstance()->getArgc(),GUIEnvironment::getInstance()->getArgv());
    Window*w=new Window(0,0,800,600);
    TextView*tv=new TextView("Hello World!",120,30);
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

    anim->setDuration(5000);
    anim->start();
    app.exec();
}
TEST_F(ANIMATOR,scale){
    App app(GUIEnvironment::getInstance()->getArgc(),GUIEnvironment::getInstance()->getArgv());
    Window*w=new Window(0,0,800,600);
    TextView*tv=new TextView("Hello World!",120,30);
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
    anim->setDuration(5000);
    anim->start();
    app.exec();
}

