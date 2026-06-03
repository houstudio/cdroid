#include<cdroid.h>
#include<cdlog.h>
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
      Layout::ELLIPSIS_NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },
   {
      "single line alignment start",
      true,
      Layout::ELLIPSIS_NONE,
      View::TEXT_ALIGNMENT_GRAVITY,
      Gravity::START,
      LayoutParams::WRAP_CONTENT,
   },
   {
      "single line alignment center",
      true,
      Layout::ELLIPSIS_NONE,
      View::TEXT_ALIGNMENT_CENTER,
      Gravity::CENTER,
      LayoutParams::WRAP_CONTENT
   },
   {
      "single line alignment end",
      true,
      Layout::ELLIPSIS_NONE,
      View::TEXT_ALIGNMENT_TEXT_END,
      Gravity::END,
      LayoutParams::WRAP_CONTENT,
   },
   {
      "single line alignment center_horizontal|top",
      true,
      Layout::ELLIPSIS_NONE,
      0,
      Gravity::CENTER_HORIZONTAL|Gravity::TOP,
      50
   },

   {
      "single line alignment center_horizontal|center_vertical",
      true,
      Layout::ELLIPSIS_NONE,
      0,
      Gravity::CENTER_HORIZONTAL|Gravity::CENTER_VERTICAL,
      50
   },
   {
      "single line alignment center_horizontal|bottom",
      true,
      Layout::ELLIPSIS_NONE,
      0,
      Gravity::CENTER_HORIZONTAL|Gravity::BOTTOM,
      50
   },

   { 
     "Multiple lines (setted by setSingleLine(false),word break is supported by default",
      false, 
      Layout::ELLIPSIS_NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },

   { 
      "Ellipsis test ,Text with ellipsis at line start/middle/end,line must be very long,otherwise ellipsis cant be showed",
      true,
      Layout::ELLIPSIS_START,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },

   {
      "Ellipsis test ,Text with ellipsis at line start/middle/end,line must be very long,otherwise ellipsis cant be showed",
      true,
      Layout::ELLIPSIS_MIDDLE,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },

   {
      "Ellipsis test ,Text with ellipsis at line start/middle/end,line must be very long,otherwise ellipsis cant be showed",
      true,
      Layout::ELLIPSIS_END,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },
   {
      "Ellipsis test ,Text with ellipsis at line start/middle/end,line must be very long,otherwise ellipsis cant be showed",
      true,
      Layout::ELLIPSIS_MARQUEE,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   },
   {
       "Hello World! السلام عليكم (Peace be upon you) مرحبا ",
      true,
      Layout::ELLIPSIS_NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      LayoutParams::WRAP_CONTENT
   }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);

    LinearLayout*layout=new LinearLayout(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    layout->setOrientation(LinearLayout::VERTICAL);
    layout->setBackgroundColor(0xFF111111);
    w->addView(layout);

    for(int i=0;i<sizeof(testStrings)/sizeof(testStrings[0]);i++){
        TestString*ts=testStrings+i;
        LinearLayout::LayoutParams*layoutParams=new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,ts->height);
        layoutParams->setMargins(0,1,0,1); 
        TextView*tv=new TextView(ts->text,0,0);
        tv->setId(i);
        tv->setTextColor(0xFFFFFFFF);
        tv->setSingleLine(ts->singleline);
        tv->setEllipsize(ts->ellipsis);
        tv->setTextAlignment(ts->txtalignment);
        tv->setGravity(ts->gravity);
        if(ts->ellipsis==Layout::ELLIPSIS_MARQUEE)
            tv->setSelected(true);//onle focused or selected ot checked state can be marqueed
        int cc=i*10+8;
        tv->setBackgroundColor(0xFF000000|(cc<<16)|(cc<<8)|cc);
        tv->setTextSize(22+i);
        layout->addView(tv,layoutParams);
    }
#if 0
    SpannableStringBuilder spanText;
    spanText.append("Span sample: ");
    spanText.append("RED ", std::vector<std::shared_ptr<CharacterStyle>>{
        std::make_shared<ForegroundColorSpan>(0xFFFF6666),
        std::make_shared<AbsoluteSizeSpan>(24)
    }, 0);
    spanText.append("GREEN ", std::vector<std::shared_ptr<CharacterStyle>>{
        std::make_shared<ForegroundColorSpan>(0xFF66FF66),
        std::make_shared<AbsoluteSizeSpan>(18)
    }, 0);
    spanText.append("Bold ", std::make_shared<StyleSpan>(Typeface::BOLD), 0);
    spanText.append("Italic ", std::make_shared<StyleSpan>(Typeface::ITALIC), 0);
    spanText.append("Underline ", std::make_shared<UnderlineSpan>(), 0);
    spanText.append("Strikethrough ",std::make_shared<StrikethroughSpan>(),0);
    spanText.append("RelativeSize X",std::make_shared<RelativeSizeSpan>(1.2),0);
    spanText.append("2",std::make_shared<SuperscriptSpan>(),0);
    spanText.append("H",std::make_shared<StyleSpan>(Typeface::ITALIC), 0);
    spanText.append("2",std::make_shared<SubscriptSpan>(),0);
    spanText.append(" cdroid",std::make_shared<URLSpan>("http://www.gitee.com/houstudio/Cdroid"),0);
    TextView* spanTv = new TextView("", 0, 0);
    spanTv->setText(spanText);
    spanTv->setTextSize(24);
    spanTv->setLineHeight(60);
    spanTv->setTextColor(0xFFFFFFFF);
    spanTv->setBackgroundColor(0xFF333333);
    spanTv->setSingleLine(false);
    layout->addView(spanTv,new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT));
#endif
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

    w->requestLayout();//addView by code must call requestLayout ,auto call only used by Window::inflate.
    return app.exec();
}
