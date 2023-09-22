#include <cdroid.h>
#include <widget/expandablelayout.h>
using namespace cdroid;

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w =new Window(0,0,-1,-1);
    LinearLayout*ll=new LinearLayout(-1,-1);
    ll->setOrientation(1);
    Button *btn=new Button("Expand",200,40);
    ll->addView(btn);
    ExpandableLayout* el= new ExpandableLayout(-1,-1);
    el->setBackgroundColor(0xFF112233);
    TextView*tv=new TextView("line1\nline2\nline3\nline4\nline5\nline6\nline7\nline8\nline9",-1,400);
    tv->setBackgroundColor(0xFF332211);
    tv->setSingleLine(false);
    el->addView(tv);
    ll->addView(el);
    ll->addView(new Button("Bottom",200,40));
    w->addView(ll);
    btn->setOnClickListener([&el](View&){
       if(el->isExpanded())el->collapse(true);
       else el->expand(true);
    });
    return app.exec();
}
