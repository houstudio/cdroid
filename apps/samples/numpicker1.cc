#include <cdroid.h>
#include <widget/R.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Point size;
    WindowManager::getInstance().getDefaultDisplay().getSize(size);
    Window*w=new Window(0,0,size.x,size.y);

    LinearLayout*layout=new LinearLayout(size.x,size.y);
    for(int i=0;i<1;i++){
        NumberPicker*np1=new NumberPicker(600,60);
        EditText*edt =(EditText*)np1->findViewById(cdroid::R::id::numberpicker_input);
	if(edt){
	   edt->setBackgroundColor(0xFFFF1100+(i*33)+11);
	   edt->setTextColor(0xFFFFFFFF);
	   edt->setTextSize(40);
        }
	np1->setTextColor(0xFFFFFFFF);
        np1->setMinValue(1);
        np1->setMaxValue(12);
        np1->setSelector(7);
        np1->setMinHeight(220);
        np1->setBackgroundColor(0xFF111100+(i*33));
        if(i==2){
           np1->setFormatter([](int v)->std::string{
               return std::to_string(v)+"月";
           });
	}
	np1->setOnValueChangedListener([](NumberPicker&np,int prev,int value){
            LOGI("numberpicker:%p value %d->%d",&np,prev,value);
        });
        layout->addView(np1,new LinearLayout::LayoutParams(-1,-1,0.3f)).setId(100+i);
    }
    w->addView(layout,new LinearLayout::LayoutParams(-1,-1));
    layout->requestLayout();
    w->requestLayout();
    app.exec();
}
