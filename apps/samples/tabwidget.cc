#include<windows.h>
#include<cdlog.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    TabWidget *tab=new TabWidget(800,600);
    w->addView(tab);
    for(int i=0;i<6;i++){
        TextView*edit=new TextView("Hello "+std::to_string(i),0,0);
        edit->setTextColor(0xFFFFFFFF);//app.getColorStateList("cdroid:color/textview.xml"));
        //edit->setTextColor(app.getColorStateList("cdroid:color/textview.xml")) ;
        edit->setId(100+i);
        edit->setSingleLine(false);
        edit->setClickable(true);
        edit->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
        edit->setBackgroundColor(0xFF000000|((i*8)<<16)|((i*8)<<8)|(i*8));
        edit->setTextSize(24);
        tab->addView(edit);
    }

    w->requestLayout();
    return app.exec();
}
