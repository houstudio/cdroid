#include<cdroid.h>
#include<widget/textview.h>
#include<widget/gridlayout.h>
#include<cdlog.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,720);
    GridLayout*gl=new GridLayout(800,640);
    w->addView(gl).setId(100);
    gl->setRowCount(3);
    gl->setColumnCount(3);
    for(int i=0;i<8;i++){
        std::string txt=std::string("Text")+std::to_string(i);
        TextView*tv =new TextView(txt,100,100);
        tv->setId(1000+i);
        tv->setTextSize(64);
        auto rowspec=GridLayout::spec(GridLayout::UNDEFINED,1,1);
        auto colspec=GridLayout::spec(GridLayout::UNDEFINED,1+(i==0),1);
        GridLayout::LayoutParams*lp=new GridLayout::LayoutParams(rowspec,colspec);
        lp->setMargins(5,5,5,5);
        gl->addView(tv,lp); 
        if(i==0)tv->setBackgroundColor(0xFF00FF00);
        else tv->setBackgroundColor(0xFF000000+i*31);
        
    }
    gl->requestLayout();
    app.exec();
}
