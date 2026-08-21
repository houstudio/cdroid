#include <cdroid.h>
#include <cdlog.h>
#include <widget/candidateview.h>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    w->setId(10000);
    w->setBackgroundColor(0xFF111111);
    LinearLayout*ll=new LinearLayout(&app);
    ll->setOrientation(LinearLayout::VERTICAL);
    w->addView(ll);
    Button *btnAdd=new Button(&app); btnAdd->setText("Add" );
    ll->addView(btnAdd);
    btnAdd->setBackgroundColor(0xFF112233);
    btnAdd->setPadding(0,10,0,10);
    static int cnt=1;
    btnAdd->setOnClickListener([&](View&){
        Button*btn=new Button(&App::getInstance()); btn->setText(std::string("Button")+std::to_string(cnt++));
        btn->setId(10000+cnt);
        btn->setHasTransientState(true);
        ll->addView(btn);
        btn->setBackgroundColor(0xFF182800|(cnt*8)|(cnt*7<<8));
        btn->setPadding(0,10,0,10);
    });
    Button* btnDel=new Button(&app); btnDel->setText("Del" );
    ll->addView(btnDel);
    btnDel->setBackgroundColor(0xFF332211);
    btnDel->setPadding(0,10,0,10);
    btnDel->setOnClickListener([&](View&){
	    ll->removeViewAt(2);
    });
    ll->setLayoutTransition(new LayoutTransition());
    return app.exec();
}
