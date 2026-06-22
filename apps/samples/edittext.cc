#include<cdroid.h>
#include<cdlog.h>
struct TestString{
    const char*text;
    bool singleline;
    TextUtils::TruncateAt ellipsis;
    int txtalignment;
    int gravity;
};

TestString testStrings[]={
   { 
      "Single line textView (setted by setSingleLine(true) ",
      true, 
      Layout::ELLIPSIS_NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
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
        LinearLayout::LayoutParams*layoutParams=new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
        layoutParams->setMargins(0,1,0,1); 
        EditText*edt=new EditText(ts->text,0,0);
        edt->setTextColor(0xFFFFFFFF);
        edt->setFocusable(true);
        edt->setSingleLine(ts->singleline);
        edt->setEllipsize(ts->ellipsis);
        edt->setTextAlignment(ts->txtalignment);
        edt->setGravity(ts->gravity);
        if(ts->ellipsis==Layout::ELLIPSIS_MARQUEE)
            edt->setSelected(true);//onle focused or selected ot checked state can be marqueed
        edt->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
        int cc=i*10+8;
        edt->setBackgroundColor(0xFF000000|(cc<<16)|(cc<<8)|cc);
        edt->setTextSize(22+i);
        layout->addView(edt,layoutParams);
    }

    auto tv=new EditText("textview with leftdrawable",0,0);
    tv->setBackgroundResource("cdroid:drawable/progress_horizontal.xml");
    tv->setCompoundDrawablesWithIntrinsicBounds("cdroid:drawable/progress_large.xml","","cdroid:drawable/progress_small.xml","");
    layout->addView(tv);
    tv->setFocusable(true);
    w->requestLayout();//addView by code must call requestLayout ,auto call only used by Window::inflate.
    return app.exec();
}
