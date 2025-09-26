#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <animation/springanimation.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    w->setBackgroundColor(0xFF223344);
    Drawable*d=nullptr;
    StateListDrawable*sld;
    CompoundButton*chk;
    LOGD("test LOGF %d",__LINE__);
    LOG(DEBUG)<<"Test Stream(DEBUG)";
#if 10
    Button *btn=new Button("Button",120,60);
    //d=ctx->getDrawable("cdroid:drawable/btn_default.xml");
    d=ctx->getDrawable("cdroid:mipmap/textfield_default_mtrl_alpha");
    sld=dynamic_cast<StateListDrawable*>(d);
    btn->setBackgroundColor(0xFF332211);
    btn->setOnTouchListener([&argc](View&v,MotionEvent&e){
        const bool down=e.getAction()==MotionEvent::ACTION_DOWN;
        AnimatorSet*aset= new AnimatorSet();
        Animator* alpha = ObjectAnimator::ofFloat(&v, "alpha", {0.f});
        Animator* scale = ObjectAnimator::ofFloat(&v, "scaleX", {1.5f});
        alpha->setDuration(2000);
        scale->setDuration(2000);
        if(argc%2)aset->playTogether({alpha,scale});
        else aset->playSequentially({scale,alpha});
        aset->start();
        return false;
    });
    SpringAnimation spa(btn,(FloatProperty*)&SpringAnimation::SCALE_X,0.2);
    spa.getSpring()
        ->setStiffness(20.0f)
        .setDampingRatio(0.1f);
    DynamicAnimation::OnAnimationUpdateListener upls([&](DynamicAnimation& animation, float value, float velocity) {
        printf("[UPDATE] frame value=%.2f  velocity=%.2f\n", value, velocity);
    }); 
    DynamicAnimation::OnAnimationEndListener endls([&](DynamicAnimation& animation,bool canceled, float value, float velocity) {
        printf("[END] frame value=%.2f  velocity=%.2f\n", value, velocity);
    });
    spa.addUpdateListener(upls).addEndListener(endls).start();

    LOGD_IF(sld,"%p statecount=%d",sld,sld->getStateCount());
    btn->setBackgroundTintList(ctx->getColorStateList("cdroid:color/textview"));
    btn->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
    btn->setOnClickListener([](View&v){LOGD(" Button Clicked ");});
    btn->setOnLongClickListener([](View&v)->bool{LOGD(" Button LongClicked ");return true;});
    w->addView(btn);
    btn->setId(100);
    btn->layout(10,60,200,60);

    ShapeDrawable*sd=new ShapeDrawable();
    sd->setShape(new ArcShape(0,360));
    sd->getShape()->setGradientColors({0x20FFFFFF,0xFFFFFFFF,0x00FFFFFF});//setSolidColor(0x800000FF);
    RippleDrawable*rp=new RippleDrawable(ColorStateList::valueOf(0x80222222),new ColorDrawable(0x8000FF00),sd);
    btn=new Button("RippleButton",300,64);
    btn->setMinimumHeight(64);
    btn->setBackground(rp);
    btn->setClickable(true);
    w->addView(btn);
    btn->setId(101);
    btn->layout(200,60,300,64);

    btn=new ToggleButton(120,40);
    d=ctx->getDrawable("cdroid:drawable/btn_toggle_bg.xml");
    btn->setBackground(d);
    btn->setTextColor(ctx->getColorStateList("cdroid:color/textview"));
    ((ToggleButton*)btn)->setTextOn("ON");
    ((ToggleButton*)btn)->setTextOff("Off");
    w->addView(btn);
    btn->setId(101);
    btn->layout(200,150,120,40);
    btn->setClickable(true);

    chk=new CheckBox("CheckME",200,60);
    d = ctx->getDrawable("cdroid:drawable/btn_check.xml");
    chk->setButtonDrawable(d);
    chk->setChecked(true);
    w->addView(chk);
    chk->layout(350,150,200,60);
	
    /*AnalogClock*clk=new AnalogClock(300,300);
    d=ctx->getDrawable("cdroid:drawable/analog.xml");
    clk->setClockDrawable(d,AnalogClock::DIAL);
    d=ctx->getDrawable("cdroid:drawable/analog_second.xml");
    clk->setClockDrawable(d,AnalogClock::SECOND);
    w->addView(clk).layout(600,300,300,300);*/

#if 1 
    chk=new RadioButton("Radio",120,60);
    Drawable*dr=ctx->getDrawable("cdroid:drawable/btn_radio.xml");
    chk->setButtonDrawable(dr);
    chk->setChecked(true);
    w->addView(chk);
    chk->layout(600,150,120,60);
	
    EditText*edt=new EditText("Edit Me!",200,60);
    d=ctx->getDrawable("cdroid:drawable/edit_text.xml");//editbox_background.xml");
    edt->setBackground(d);
    edt->setTextColor(ctx->getColorStateList("cdroid:color/textview.xml"));
    w->addView(edt);
    edt->setId(102);
    edt->layout(800,60,200,60);
#endif
///////////////////////////////////////////////////////////
#if 1
    ProgressBar*pb=new ProgressBar(500,40);
    d=ctx->getDrawable("cdroid:drawable/progress_horizontal.xml");
    LOGD("progress_horizontal drawable=%p",d);
    pb->setProgressDrawable(d);
    pb->setProgress(34);
    pb->setSecondaryProgress(15);
    w->addView(pb);
    pb->layout(50,150,500,40);
#endif
#if 1
    //////////////////////////////////////////////////////////    
    ProgressBar*pb2=new ProgressBar(72,72);
    d=ctx->getDrawable("cdroid:drawable/progress_large.xml");
    pb2->setIndeterminateDrawable(d);
    LOGD("Indeterminate drawable=%p",d);
    w->addView(pb2);
    pb2->layout(50,450,72,72);
    pb2->setProgressDrawable(new ColorDrawable(0xFF112233));
    pb2->setIndeterminate(true);
#endif
#endif
#if 1
    SeekBar*sb=new SeekBar(800,30);
    SeekBar*sb2=new SeekBar(800,60);

    d=ctx->getDrawable("cdroid:drawable/progress_horizontal.xml");
    sb->setProgressDrawable(d);
    sb2->setProgressDrawable(d->getConstantState()->newDrawable());

    d=ctx->getDrawable("cdroid:drawable/seek_thumb.xml");
    sb->setThumb(d);
    sb2->setThumb(d->getConstantState()->newDrawable());
    d=ctx->getDrawable("cdroid:drawable/seekbar_tick_mark.xml");
    sb->setTickMark(d);
    sb2->setTickMark(d->getConstantState()->newDrawable());
    w->addView(sb);
    sb->layout(150,250,800,30);
    w->addView(sb2);
    sb2->layout(150,300,800,30);

#endif
    return app.exec();
}
