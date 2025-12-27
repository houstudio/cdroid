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
    StateListDrawable*sld=nullptr;
    CompoundButton*chk;
#if 10
    Button *btn=new Button("Hello World!",350,200);
    d = ctx->getDrawable("cdroid:drawable/btn_default");
    LOGD("d=%p",d);
    btn->setBackground(d);

    LOGD_IF(sld,"%p statecount=%d",sld,sld->getStateCount());
    btn->setBackgroundTintList(ctx->getColorStateList("cdroid:color/textview"));
    btn->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
    btn->setOnClickListener([](View&v){LOGD(" Button Clicked ");});
    btn->setOnLongClickListener([](View&v)->bool{LOGD(" Button LongClicked ");return true;});
    w->addView(btn);
    btn->setId(100);
    btn->setTextSize(50);
    btn->layout(10,40,350,200);

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
    btn->layout(400,60,300,64);

    btn=new ToggleButton(120,40);
    d=ctx->getDrawable("cdroid:drawable/btn_toggle_bg.xml");
    btn->setBackground(d);
    btn->setTextColor(ctx->getColorStateList("cdroid:color/textview"));
    ((ToggleButton*)btn)->setTextOn("ON");
    ((ToggleButton*)btn)->setTextOff("Off");
    w->addView(btn);
    btn->setId(101);
    btn->layout(400,150,120,40);
    btn->setClickable(true);

    chk=new CheckBox("CheckME",200,60);
    chk->setId(1000);
    chk->setChecked(true);
    d = ctx->getDrawable("cdroid:drawable/btn_check.xml");
    chk->setButtonDrawable(d);
    chk->setClickable(true);
    w->addView(chk);
    chk->setOnCheckedChangeListener([](CompoundButton&btn,bool checked){
            LOGD("btn %p checked=%d",&btn,checked);
            });
    chk->layout(450,360,200,60);
	
#if 1 
    chk=new RadioButton("Radio",120,60);
    Drawable*dr=ctx->getDrawable("cdroid:drawable/btn_radio.xml");
    chk->setButtonDrawable(dr);
    chk->setId(1001);
    dynamic_cast<Checkable*>(chk)->setChecked(true);
    //chk->setChecked(true);
    w->addView(chk);
    chk->layout(700,360,120,60);
	
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
    pb->layout(400,150,500,40);
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
