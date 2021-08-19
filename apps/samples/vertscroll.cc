#include<windows.h>
#include<cdlog.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    ScrollView*sv=new ScrollView(800,600);
    w->addView(sv);
    LinearLayout*ll=new LinearLayout(800,600);
    ll->setOrientation(LinearLayout::VERTICAL);
    sv->addView(ll,new LinearLayoutParams(LayoutParams::MATCH_PARENT,(LayoutParams::MATCH_PARENT)));
    for(int i=0;i<10;i++){
       LinearLayoutParams*lp=new LinearLayoutParams(LayoutParams::MATCH_PARENT,(LayoutParams::WRAP_CONTENT));
       lp->setMargins(5,2,5,2);
       EditText*edit=new EditText(TEXT("Hello world! This value is positive for typical fonts that include"),680,200);
       edit->setTextColor(0xFFFFFFFF);//app.getColorStateList("cdroid:color/textview.xml"));
       //edit->setTextColor(app.getColorStateList("cdroid:color/textview.xml"));
       edit->setSingleLine(false);
       edit->setInputType(EditText::TYPE_ANY);
       edit->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
       edit->setBackgroundColor(0xFF000000|((i*8)<<16)|((i*8)<<8)|(i*8));
       edit->setTextSize(40);
       ll->addView(edit,lp);
    }
    sv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    sv->setVerticalScrollBarEnabled(true);
    w->requestLayout();
    return app.exec();
}
