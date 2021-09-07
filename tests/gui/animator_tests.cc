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
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&iprop});
    anim->addUpdateListener([this](ValueAnimator&anim){
        IntPropertyValuesHolder*ip=(IntPropertyValuesHolder*)anim.getValues(0);
        LOGD("value=%d",ip->getAnimatedValue());
    }); 
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,ofFloat){
    FloatPropertyValuesHolder fprop;
    fprop.setValues({0,100});
    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    anim->addUpdateListener([this](ValueAnimator&anim){
        FloatPropertyValuesHolder*fp=(FloatPropertyValuesHolder*)anim.getValues(0);
        LOGD("value=%f",fp->getAnimatedValue());
    }); 
    for(int i=0;i<=10;i++){
        anim->setCurrentFraction((float)i/10.f);
    }
}

TEST_F(ANIMATOR,loopdrivered){
    App app;
    IntPropertyValuesHolder iprop;
    iprop.setValues({0,100});

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&iprop});
    anim->addUpdateListener([this](ValueAnimator&anim){
        IntPropertyValuesHolder*fp=(IntPropertyValuesHolder*)anim.getValues(0);
        LOGD("value=%d",fp->getAnimatedValue());
    }); 
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

    IntPropertyValuesHolder yprop;
    yprop.setPropertyName("y");
    yprop.setValues({0,200,200});

    ColorPropertyValuesHolder cprop;
    cprop.setValues({0xFF000000,0xFFFF8844});

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&xprop,&yprop,&cprop});
    anim->addUpdateListener([tv](ValueAnimator&anim){
        IntPropertyValuesHolder*xp=(IntPropertyValuesHolder*)anim.getValues(0);
        IntPropertyValuesHolder*yp=(IntPropertyValuesHolder*)anim.getValues(1);
        ColorPropertyValuesHolder*cp=(ColorPropertyValuesHolder*)anim.getValues(2);
        tv->setPos(xp->getAnimatedValue(),tv->getTop());
        tv->setPos(tv->getLeft(),yp->getAnimatedValue());
        tv->setBackgroundColor(cp->getAnimatedValue());
    });

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

    ValueAnimator*anim=ValueAnimator::ofPropertyValuesHolder({&fprop});
    anim->addUpdateListener([tv](ValueAnimator&anim){
        FloatPropertyValuesHolder*fp=(FloatPropertyValuesHolder*)anim.getValues(0);
        float scale=fp->getAnimatedValue();
        tv->setScaleX(scale);
        tv->setScaleY(scale);
    });
    anim->setDuration(5000);
    anim->start();
    app.exec();
}

