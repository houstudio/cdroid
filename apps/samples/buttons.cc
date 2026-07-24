#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <animation/springanimation.h>
#include <drawable/badgeutils.h>
int main(int argc,const char*argv[]){
    App app(argc,argv);
    cdroid::Context*ctx=&app;
    Window*w=new Window(0,0,-1,-1);
    w->setId(1);
    w->setBackgroundColor(0xFF223344);

    // Window::doLayout now always measures+lays out its (FrameLayout) children, so
    // views added directly to the Window pile up at (0,0). Wrap the widget zoo in a
    // vertical LinearLayout inside a ScrollView, so any number of widgets stack and
    // scroll instead of piling up.
    ScrollView*scroller=new ScrollView(-1,-1);
    scroller->setSmoothScrollingEnabled(true);
    scroller->setVerticalScrollBarEnabled(true);
    w->addView(scroller);
    LinearLayout*content=new LinearLayout(-1,-2); // MATCH_PARENT width, WRAP_CONTENT height
    content->setOrientation(LinearLayout::VERTICAL);
    scroller->addView(content);
    auto row=[&](View*v,int width,int height){
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(width,height);
        lp->topMargin=lp->leftMargin=8;
        content->addView(v,lp);
    };

    Drawable*d=nullptr;
    StateListDrawable*sld=nullptr;
    CompoundButton*chk;
#if 10
    BadgeDrawable*bd = BadgeDrawable::create(&app);
    bd->setNumber(96);
    w->setId(10000);

    Button *btn=new Button("Hello World!",350,200);
    d = ctx->getDrawable("cdroid:drawable/btn_default");
    LOGD("d=%p",d);
    btn->setBackground(d);

    LOGD_IF(sld,"%p statecount=%d",sld,sld->getStateCount());
    btn->setBackgroundTintList(ctx->getColorStateList("cdroid:color/textview"));
    btn->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
    btn->setOnClickListener([](View&v){LOGD(" Button Clicked ");});
    btn->setOnLongClickListener([](View&v)->bool{LOGD(" Button LongClicked ");return true;});
    btn->setId(100);
    btn->setTextSize(50);
    row(btn,350,200);
    bd->setBounds(0,0,350,200);
    BadgeUtils::attachBadgeDrawable(bd,btn);

    ShapeDrawable*sd=new ShapeDrawable();
    sd->setShape(new ArcShape(0,360));
    sd->getShape()->setGradientColors({0x20FFFFFF,0xFFFFFFFF,0x00FFFFFF});//setSolidColor(0x800000FF);
    RippleDrawable*rp=new RippleDrawable(ColorStateList::valueOf(0x80222222),new ColorDrawable(0x8000FF00),sd);
    btn=new Button("RippleButton",300,64);
    btn->setMinimumHeight(64);
    btn->setBackground(rp);
    btn->setClickable(true);
    btn->setId(101);
    row(btn,300,64);

    btn=new ToggleButton(120,40);
    d=ctx->getDrawable("cdroid:drawable/btn_toggle_bg.xml");
    btn->setBackground(d);
    //btn->setTextColor(ctx->getColorStateList("cdroid:color/textview"));
    ((ToggleButton*)btn)->setTextOn("ON");
    ((ToggleButton*)btn)->setTextOff("Off");
    btn->setId(101);
    btn->setClickable(true);
    row(btn,120,40);

    chk=new CheckBox("CheckME",200,60);
    chk->setId(1000);
    chk->setChecked(true);
    d = ctx->getDrawable("cdroid:drawable/btn_check.xml");
    chk->setButtonDrawable(d);
    chk->setClickable(true);
    chk->setOnCheckedChangeListener([](CompoundButton&btn,bool checked){
            LOGD("btn %p checked=%d",&btn,checked);
            });
    row(chk,200,60);

#if 1
    chk=new RadioButton("Radio",120,60);
    Drawable*dr=ctx->getDrawable("cdroid:drawable/btn_radio.xml");
    chk->setButtonDrawable(dr);
    chk->setId(1001);
    dynamic_cast<Checkable*>(chk)->setChecked(true);
    //chk->setChecked(true);
    row(chk,120,60);

    EditText*edt=new EditText("Edit Me!",200,60);
    d=ctx->getDrawable("cdroid:drawable/edit_text.xml");//editbox_background.xml");
    edt->setBackground(d);
    //edt->setTextColor(ctx->getColorStateList("cdroid:color/textview.xml"));
    edt->setId(102);
    row(edt,200,60);
#endif
///////////////////////////////////////////////////////////
#if 1
    ProgressBar*pb=new ProgressBar(500,40);
    d=ctx->getDrawable("cdroid:drawable/progress_horizontal.xml");
    LOGD("progress_horizontal drawable=%p",d);
    pb->setProgressDrawable(d);
    pb->setProgress(34);
    pb->setSecondaryProgress(15);
    row(pb,500,40);
#endif
#if 1
    //////////////////////////////////////////////////////////
    ProgressBar*pb2=new ProgressBar(72,72);
    d=ctx->getDrawable("cdroid:drawable/progress_large.xml");
    pb2->setIndeterminateDrawable(d);
    LOGD("Indeterminate drawable=%p",d);
    pb2->setProgressDrawable(new ColorDrawable(0xFF112233));
    pb2->setIndeterminate(true);
    row(pb2,72,72);
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
    row(sb,800,30);
    row(sb2,800,60);
#endif
    content->requestLayout();
    return app.exec();
}
