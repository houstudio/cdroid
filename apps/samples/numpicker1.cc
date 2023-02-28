#include <cdroid.h>
#include <widget/R.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,600);

    LinearLayout*layout=new LinearLayout(1280,600);
    for(int i=0;i<3;i++){
        NumberPicker*np1=new NumberPicker(200,600);
        EditText*edt =(EditText*)np1->findViewById(cdroid::R::id::numberpicker_input);
	if(edt){
	   edt->setBackgroundColor(0xFFFF1100+(i*33)+11);
	   edt->setTextColor(0xFFFFFFFF);
	   edt->setTextSize(40);
        }
        np1->setMinValue(1);
        np1->setMaxValue(12);
        np1->setSelector(7,-1);
        np1->setMinHeight(220);
        np1->setBackgroundColor(0xFF111100+(i*33));
        if(i==2)
           np1->setFormatter([](int v)->std::string{
               return std::to_string(v)+"月";
           });	
        layout->addView(np1,new LinearLayout::LayoutParams(-1,-1,0.3f)).setId(100+i);
    }
    w->addView(layout,new LinearLayout::LayoutParams(-1,-1));
    w->requestLayout();
    app.exec();
}
