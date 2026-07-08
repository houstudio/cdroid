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
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      InputType::TYPE_CLASS_TEXT
   },
   {
      "Password",
      true,
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      InputType::TYPE_CLASS_TEXT|InputType::TYPE_TEXT_VARIATION_PASSWORD
   },
   {
      "666",
      true,
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      InputType::TYPE_CLASS_NUMBER
   },
   {
      "666",
      true,
      TextUtils::TruncateAt::NONE/*0*/,
      View::TEXT_ALIGNMENT_INHERIT/*0*/,
      Gravity::NO_GRAVITY/*0*/,
      InputType::TYPE_CLASS_PHONE
   },
   {
       "zhhou@sanboen.com",
       true,
       TextUtils::TruncateAt::NONE/*0*/,
       View::TEXT_ALIGNMENT_INHERIT/*0*/,
       Gravity::NO_GRAVITY/*0*/,
       InputType::TYPE_TEXT_VARIATION_EMAIL_ADDRESS
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
        edt->setClickable(true);
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

    // ---- RTL/LTR interleaved bidi cursor-movement tests ----
    // Place the caret at the LTR<->RTL boundary and arrow left/right: Android
    // moves the caret one LOGICAL char per step (which may be visually right in
    // an RTL run), so the caret should walk the text in logical order, not jump.
    // Numbers are weak-LTR even inside RTL. These cases stress that.
    static struct { const char* label; const char* text; } bidiCases[] = {
        { "pure RTL (Arabic)",       "مرحبا بالعالم" },
        { "LTR / RTL / LTR",         "Hello مرحبا World" },
        { "RTL / LTR / RTL",         "مرحبا Hello مرحبا" },
        { "LTR digits inside RTL",   "abc123مرحبة" },
        { "RTL + digits + LTR",      "مرحبا 1234 hello" },
        { "Hebrew RTL",              "שלום עולם" },
    };
    for (int i = 0; i < (int)(sizeof(bidiCases) / sizeof(bidiCases[0])); i++) {
        auto* lbl = new TextView(bidiCases[i].label, 0, 0);
        lbl->setTextSize(14);
        lbl->setTextColor(0xFFAABBCC);
        layout->addView(lbl, new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT));

        EditText* edt = new EditText(bidiCases[i].text, 0, 0);
        edt->setTextSize(24);
        edt->setFocusable(true);
        edt->setClickable(true);
        edt->setSingleLine(true);
        edt->setGravity(Gravity::LEFT | Gravity::CENTER_VERTICAL);
        edt->setBackgroundColor(0xFF223344);
        edt->setId(200000+i);
        edt->setInputType(InputType::TYPE_CLASS_TEXT);
        layout->addView(edt, new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT));
    }

    // ---- same bidi cases under RTL LAYOUT_DIRECTION (paragraph base direction RTL) ----
    static struct { const char* label; const char* text; } bidiRtlCases[] = {
        { "RTL layout: pure LTR text", "Hello World" },
        { "RTL layout: LTR/RTL/LTR",   "Hello مرحبا World" },
        { "RTL layout: RTL/LTR/RTL",   "مرحبا Hello مرحبا" },
    };
    for (int i = 0; i < (int)(sizeof(bidiRtlCases) / sizeof(bidiRtlCases[0])); i++) {
        auto* lbl = new TextView(bidiRtlCases[i].label, 0, 0);
        lbl->setTextSize(14);
        lbl->setTextColor(0xFFCCBBAA);
        lbl->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
        layout->addView(lbl, new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT));

        EditText* edt = new EditText(bidiRtlCases[i].text, 0, 0);
        edt->setTextSize(24);
        edt->setFocusable(true);
        edt->setClickable(true);
        edt->setSingleLine(true);
        edt->setLayoutDirection(View::LAYOUT_DIRECTION_RTL);
        edt->setBackgroundColor(0xFF332211);
        edt->setId(210000+i);
        edt->setInputType(InputType::TYPE_CLASS_TEXT);
        layout->addView(edt, new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT));
    }

    w->requestLayout();//addView by code must call requestLayout ,auto call only used by Window::inflate.
    return app.exec();
}
