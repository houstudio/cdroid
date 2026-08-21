#include <cdroid.h>
#include <cdlog.h>
#include <fstream>
#include <animation/springanimation.h>
#include <drawable/badgeutils.h>
#include <widget/R.h>   // public cdroid::R (android.R role)
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
    ScrollView*scroller=new ScrollView(&app);
    scroller->setSmoothScrollingEnabled(true);
    scroller->setVerticalScrollBarEnabled(true);
    w->addView(scroller);
    LinearLayout*content=new LinearLayout(&app); // MATCH_PARENT width, WRAP_CONTENT height
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

    // AOSP code construction: Button(Context) -> buttonStyle default style;
    // the theme supplies background/minHeight/padding (no manual drawables).
    Button *btn=new Button(&app); btn->setText("Hello World!");

    LOGD_IF(sld,"%p statecount=%d",sld,sld->getStateCount());
    // (the old string call asked for cdroid:color/textview, which never
    // existed in the framework res set — it silently tinted with null)
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
    btn=new Button(&app); btn->setText("RippleButton" );
    btn->setMinimumHeight(64);
    btn->setBackground(rp);
    btn->setClickable(true);
    btn->setId(101);
    row(btn,300,64);

    btn=new ToggleButton(&app);   // buttonStyleToggle default style
    ((ToggleButton*)btn)->setTextOn("ON");
    ((ToggleButton*)btn)->setTextOff("Off");
    btn->setId(101);
    btn->setClickable(true);
    row(btn,120,40);

    chk=new CheckBox(&app);      // checkboxStyle default style
    chk->setText("CheckME");
    chk->setId(1000);
    chk->setChecked(true);
    chk->setClickable(true);
    chk->setOnCheckedChangeListener([](CompoundButton&btn,bool checked){
            LOGD("btn %p checked=%d",&btn,checked);
            });
    row(chk,200,60);

#if 1
    chk=new RadioButton(&app);   // radioButtonStyle default style
    chk->setText("Radio");
    chk->setId(1001);
    dynamic_cast<Checkable*>(chk)->setChecked(true);
    //chk->setChecked(true);
    row(chk,120,60);

    EditText*edt=new EditText(&app);   // editTextStyle default style
    edt->setText("Edit Me!");
    edt->setId(102);
    row(edt,200,60);
#endif
///////////////////////////////////////////////////////////
#if 1
    // AOSP: new ProgressBar(context, null, android.R.attr.progressBarStyleHorizontal)
    ProgressBar*pb=new ProgressBar(&app,nullptr,cdroid::R::attr::progressBarStyleHorizontal);
    pb->setProgress(34);
    pb->setSecondaryProgress(15);
    row(pb,500,40);
#endif
#if 1
    //////////////////////////////////////////////////////////
    ProgressBar*pb2=new ProgressBar(&app);   // progressBarStyle (circular indeterminate)
    pb2->setProgressDrawable(new ColorDrawable(0xFF112233));
    pb2->setIndeterminate(true);
    row(pb2,72,72);
#endif
#endif
#if 1
    SeekBar*sb=new SeekBar(&app);   // seekBarStyle supplies progress/thumb/tick
    SeekBar*sb2=new SeekBar(&app);
    row(sb,800,30);
    row(sb2,800,60);
#endif
    content->requestLayout();
    return app.exec();
}
