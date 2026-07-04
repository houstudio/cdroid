#include<cdroid.h>
#include<cdlog.h>
#include <text/inputtype.h>
struct TestString{
    const char*text;
    bool singleline;
    TextUtils::TruncateAt ellipsis;
    int txtalignment;
    int gravity;
    int intputType;
};

TestString testStrings[]={
   {
      "Text",
      true,
      Layout::ELLIPSIS_NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      InputType::TYPE_CLASS_TEXT
   },
   {
      "Password",
      true,
      Layout::ELLIPSIS_NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      InputType::TYPE_CLASS_TEXT|InputType::TYPE_TEXT_VARIATION_PASSWORD
   },
   {
      "666",
      true,
      Layout::ELLIPSIS_NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      InputType::TYPE_CLASS_NUMBER
   }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);

    LinearLayout*layout=new LinearLayout(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    layout->setOrientation(LinearLayout::VERTICAL);
    layout->setBackgroundColor(0xFF334455);
    w->addView(layout);

    for(int i=0;i<sizeof(testStrings)/sizeof(testStrings[0]);i++){
        TestString*ts=testStrings+i;
        LinearLayout::LayoutParams*layoutParams=new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
        layoutParams->setMargins(0,8,0,8);
        EditText*edt=new EditText(ts->text,0,0);
        edt->setTextColor(i?0xffff0000:0xFF44FFaa);
        edt->setFocusable(true);
        edt->setSingleLine(ts->singleline);
        edt->setTextAlignment(ts->txtalignment);
        edt->setGravity(ts->gravity);
        edt->setBackgroundColor(i?0xffaabbcc:0xff112233);
        edt->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
        int cc=i*10+8;
        edt->setTextSize(22+i);
        edt->setInputType(ts->intputType);
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
