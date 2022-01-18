#include <cdroid.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,600);

    LinearLayout*layout=new LinearLayout(1280,600);
    layout->setOrientation(LinearLayout::VERTICAL);

    NumberPicker*np1=new NumberPicker(200,90);
    np1->setMinValue(1);
    np1->setMaxValue(12);
    np1->setSelector(7,-1);
    np1->setMinHeight(220);
    np1->setBackgroundColor(0xFF111111);
    
    layout->addView(np1);
    w->addView(layout);
    w->requestLayout();
    app.exec();
}
