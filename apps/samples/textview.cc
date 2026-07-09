#include<widget/cdwindow.h>
#include<widget/textview.h>
#include<widget/scrollview.h>
#include<widget/linearlayout.h>
#include<porting/cdlog.h>
#include<text/html.h>
#include<text/spannablestringbuilder.h>
#include<core/app.h>
struct TestString{
    const char*text;
    bool singleline;
    int ellipsis;
    int txtalignment;
    int gravity;
    int height;
};

TestString testStrings[]={
   { 
      "Single line textView (setted by setSingleLine(true) ",
      true, 
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },
   {
      "single line alignment start",
      true,
      TextUtils::TruncateAt::NONE,
      View::TEXT_ALIGNMENT_GRAVITY,
      Gravity::START,
      LayoutParams::WRAP_CONTENT,
   },
   {
      "single line alignment center",
      true,
      TextUtils::TruncateAt::NONE,
      View::TEXT_ALIGNMENT_CENTER,
      Gravity::CENTER,
      LayoutParams::WRAP_CONTENT
   },
   {
      "single line alignment end",
      true,
      TextUtils::TruncateAt::NONE,
      View::TEXT_ALIGNMENT_TEXT_END,
      Gravity::END,
      LayoutParams::WRAP_CONTENT,
   },
   {
      "single line alignment center_horizontal|top",
      true,
      TextUtils::TruncateAt::NONE,
      0,
      Gravity::CENTER_HORIZONTAL|Gravity::TOP,
      50
   },

   {//5
      "single line alignment center_horizontal|center_vertical",
      true,
      TextUtils::TruncateAt::NONE,
      0,
      Gravity::CENTER_HORIZONTAL|Gravity::CENTER_VERTICAL,
      50
   },
   {
      "single line alignment center_horizontal|bottom",
      true,
      TextUtils::TruncateAt::NONE,
      0,
      Gravity::CENTER_HORIZONTAL|Gravity::BOTTOM,
      50
   },

   { 
     "Multiple lines (setted by setSingleLine(false),word break is supported by default",
      false, 
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },
   {
     "Multiple lines (setted by setSingleLine(false),\nword soft break is supported by default(TOP)",
      false, 
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::TOP/*0*/,
      88
   },
   { 
     "Multiple lines (setted by setSingleLine(false),\nword soft break is supported by default(VCENTER)",
      false, 
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::CENTER_VERTICAL/*0*/,
      88
   },
   {//10 
     "Multiple lines (setted by setSingleLine(false),\nword soft break is supported by default(BOTTOM)",
      false, 
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::BOTTOM/*0*/,
      88
   },

   { 
      "Ellipsis test ,Text with ellipsis at line start/middle/end,line must be very long,otherwise ellipsis cant be showed",
      true,
      TextUtils::TruncateAt::START,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },

   {
      "Ellipsis test ,Text with ellipsis at line start/middle/end,line must be very long,otherwise ellipsis cant be showed",
      true,
      TextUtils::TruncateAt::MIDDLE,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },

   {
      "Ellipsis test ,Text with ellipsis at line start/middle/end,line must be very long,otherwise ellipsis cant be showed",
      true,
      TextUtils::TruncateAt::END,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },
   {
      "Ellipsis test ,Text with ellipsis at line start/middle/end,line must be very long,otherwise ellipsis cant be showed",
      true,
      TextUtils::TruncateAt::MARQUEE,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },
   {//15
      "Ellipsis test ,Text with ellipsis at line start/middle/end,line must be very long,otherwise ellipsis cant be showed",
      true,
      TextUtils::TruncateAt::NONE,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },
   {
       "Hello World! السلام عليكم (Peace be upon you) مرحبا ",
      true,
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },
   {
       "The quick brown fox jumps over the lazy dog. Justification "
       "should stretch word spacing on every wrapped line except the last.",
      false,
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    ScrollView*sv=new ScrollView(0,0);
    LinearLayout*layout=new LinearLayout(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    sv->addView(layout);
    layout->setOrientation(LinearLayout::VERTICAL);
    layout->setBackgroundColor(0xFF111111);

    for(int i=0;i<sizeof(testStrings)/sizeof(testStrings[0]);i++){
        TestString*ts=testStrings+i;
        LinearLayout::LayoutParams*layoutParams=new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,ts->height);
        layoutParams->setMargins(0,1,0,1); 
        TextView*tv=new TextView(ts->text,0,0);
        tv->setId(i);
        tv->setTextColor(0xFFFFFFFF);
        tv->setSingleLine(ts->singleline);
        tv->setEllipsize(static_cast<TextUtils::TruncateAt>(ts->ellipsis));
        tv->setTextAlignment(ts->txtalignment);
        tv->setGravity(ts->gravity);
        if(ts->ellipsis== TextUtils::TruncateAt::MARQUEE)
            tv->setSelected(true);//onle focused or selected ot checked state can be marqueed
        int cc=i*10+8;
        tv->setBackgroundColor(0xFF000000|(cc<<16)|(cc<<8)|cc);
        tv->setTextSize(22+i);
        layout->addView(tv,layoutParams);
        if(i>=8&&i<=10)tv->setLineSpacing(4,1.00);
        if(i==sizeof(testStrings)/sizeof(testStrings[0])-1){
            tv->setJustificationMode(Layout::JUSTIFICATION_MODE_INTER_WORD);
        }
    }
    SpannableStringBuilder* spanText=new SpannableStringBuilder();
    spanText->append(u"Span sample:",0,12);
    spanText->append(u"RED ", {
        new ForegroundColorSpan(0xFFFF6666),
        new AbsoluteSizeSpan(24) }, 0);
    spanText->append(u"GREEN ",{
        new ForegroundColorSpan(0xFF66FF66),
        new AbsoluteSizeSpan(18)
    }, 0);
    spanText->append(u"2",new SuperscriptSpan(),0);
    spanText->append(u"Hello,你好",new ForegroundColorSpan(0xFF66FF66),0);
    TextView* spanTv = new TextView("", 0, 0);
    spanTv->setSingleLine(false);
    spanTv->setText(spanText);
    spanTv->setTextSize(24);
    spanTv->setLineHeight(60);
    spanTv->setTextColor(0xFFFFFFFF);
    spanTv->setBackgroundColor(0xFF333333);
    layout->addView(spanTv,new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT));

    TextView*tv=new TextView("textview with background drawable",0,0);
    tv->setBackgroundResource("cdroid:drawable/btn_default.xml");
    layout->addView(tv);

    tv=new TextView("textview with leftdrawable",0,0);
    tv->setBackgroundResource("cdroid:drawable/progress_horizontal.xml");
    tv->setCompoundDrawablesWithIntrinsicBounds("cdroid:drawable/progress_large.xml","","cdroid:drawable/progress_small.xml","");
    layout->addView(tv);
    int level=0;
    Runnable run;
    run=[tv,&level,&w,&run](){
       Drawable*dr=tv->getBackground();
       dr->setLevel(level);
       level=(level+1000)%10000;
       w->postDelayed(run,200);
    };
    w->postDelayed(run,500);

    w->postDelayed([&layout](){
            TextView*tv=(TextView*)layout->getChildAt(12);
            tv->setEllipsize(TextUtils::TruncateAt::MARQUEE);
            tv->setSelected(true);
            },1000);
    w->addView(sv);
    w->requestLayout();//addView by code must call requestLayout ,auto call only used by Window::inflate.
    auto spannedstr=Html::fromHtml("",0,nullptr,nullptr);
    return app.exec();
}
