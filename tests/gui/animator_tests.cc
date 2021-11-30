#include <gtest/gtest.h>
#include <windows.h>
#include <ngl_os.h>
#include <ngl_timer.h>
#include <cdlog.h>
#include <animation/objectanimator.h>

class ANIMATOR:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(ANIMATOR,callback){
    App app;
    ValueAnimator *anim=new ValueAnimator();
    anim->getAnimationHandler().addAnimationFrameCallback(anim,100);
    anim->getAnimationHandler().removeCallback(anim);
    app.exec();
}

TEST_F(ANIMATOR,ofInt1){
    ValueAnimator*anim=ValueAnimator::ofInt({0,100});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        PropertyValuesHolder*ip=anim.getValues(0);
        LOGD("value=%d",ip->getAnimatedValue().get<int>());
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
        PropertyValuesHolder*ip=anim.getValues(0);
        LOGD("value=%d",ip->getAnimatedValue().get<int>());
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
        PropertyValuesHolder*fp=anim.getValues(0);
        LOGD("value=%f",fp->getAnimatedValue().get<float>());
    })); 
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

class MyProperty: public Property{
public:
   MyProperty(const std::string&name):Property(name){
   }
   void set(void* object, float value)override{
       LOGD("value=%f",value);
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
    App app;
    IntPropertyValuesHolder iprop;
    iprop.setValues(std::vector<int>({0,100}));

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&iprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        PropertyValuesHolder*fp=anim.getValues(0);
        LOGD("value=%d",fp->getAnimatedValue().get<int>());
    })); 
    anim->setDuration(2000);
    anim->start();
    app.exec();
}

TEST_F(ANIMATOR,translate){
    App app;
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
    cprop.setValues(std::vector<uint32_t>({0xFF000000,0xFFFF8844}));

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&xprop,&yprop,&cprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([tv](ValueAnimator&anim){
        PropertyValuesHolder*xp=anim.getValues(0);
        PropertyValuesHolder*yp=anim.getValues(1);
        PropertyValuesHolder*cp=anim.getValues(2);
        tv->setPos(xp->getAnimatedValue().get<int>(),yp->getAnimatedValue().get<int>());
        tv->setBackgroundColor(cp->getAnimatedValue().get<uint32_t>());
    }));

    anim->setDuration(5000);
    anim->start();
    app.exec();
}
TEST_F(ANIMATOR,scale){
    App app;
    Window*w=new Window(0,0,800,600);
    TextView*tv=new TextView("Hello World!",120,30);
    tv->setBackgroundColor(0xFF111111);
    w->addView(tv);

    FloatPropertyValuesHolder fprop;
    fprop.setPropertyName("scale");
    fprop.setValues(std::vector<float>({0,2.0}));

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    anim->addUpdateListener(ValueAnimator::AnimatorUpdateListener([tv](ValueAnimator&anim){
        FloatPropertyValuesHolder*fp=(FloatPropertyValuesHolder*)anim.getValues(0);
        const float scale=fp->getAnimatedValue().get<float>();
        tv->setScaleX(scale);
        tv->setScaleY(scale);
    }));
    anim->setDuration(5000);
    anim->start();
    app.exec();
}

