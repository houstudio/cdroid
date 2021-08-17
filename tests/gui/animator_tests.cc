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

TEST_F(ANIMATOR,ofInt){
    IntPropertyValuesHolder iprop;
    iprop.setValues({0,100});
    iprop.setPropertySetter([](void*target,float fraction,int v){
        LOGD("fraction=%f value=%d",fraction,v);
    });
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&iprop});
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,ofFloat){
    FloatPropertyValuesHolder fprop;
    fprop.setValues({0,100});
    fprop.setPropertySetter([](void*target,float fraction,float v){
        LOGD("fraction=%f value=%f",fraction,v);
    });
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,loopdrivered){
    App app;
    IntPropertyValuesHolder iprop;
    iprop.setValues({0,100});
    iprop.setPropertySetter([](void*target,float fraction,int v){
        LOGD("fraction=%f value=%d",fraction,v);
    });

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&iprop});
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
    xprop.setValues({0,100,300});
    xprop.setPropertySetter([&](void*target,float fraction,int v){
        tv->setPos(v,tv->getTop());
    });

    IntPropertyValuesHolder yprop;
    yprop.setPropertyName("y");
    yprop.setValues({0,200,200});
    yprop.setPropertySetter([&](void*target,float fraction,int v){
        tv->setPos(tv->getLeft(),v);
    });

    ColorPropertyValuesHolder cprop;
    cprop.setValues({0xFF000000,0xFFFF8844});
    cprop.setPropertySetter([&](void*target,float fraction,uint32_t v){
       tv->setBackgroundColor(v); 
    });
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&xprop,&yprop,&cprop});
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
    fprop.setValues({0,2.0});
    fprop.setPropertySetter([&](void*target,float fraction,float scale){
        tv->setScaleX(scale);
        tv->setScaleY(scale);
    });
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    anim->setDuration(5000);
    anim->start();
    app.exec();
}
