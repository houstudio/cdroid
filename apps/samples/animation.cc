#include<windows.h>
#include<cdlog.h>

class MyAdapter:public ArrayAdapter<std::string>{
private:
    int itemType;
public:
    MyAdapter(int type=0):ArrayAdapter(){
        itemType=type;
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{

        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            if(itemType==0) tv=new TextView("",600,20);
            else tv=new CheckBox("",600,20);
            tv->setPadding(20,0,0,0);
            tv->setFocusable(false);
        }
        if(itemType==1)tv->setLayoutDirection(position<10?View::LAYOUT_DIRECTION_RTL:View::LAYOUT_DIRECTION_LTR);
        tv->setId(position);
        tv->setText("position :"+std::to_string(position));
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0x80002222);
        tv->setTextSize(40);
        return tv;
    }
};

int main(int argc,const char*argv[]){

    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
  
    MyAdapter*adapter=new MyAdapter(0);

    ListView*lv=(ListView*)&w->addView(new ListView(320,480));
    lv->setPos(10,10);
    lv->setId(1000);
    for(int i=0;i<56;i++){
        adapter->add("");
    }
    lv->setAdapter(adapter);
    adapter->notifyDataSetChanged();
    lv->setVerticalScrollBarEnabled(true);
    lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    lv->setSmoothScrollbarEnabled(true);
    lv->setSelector(new ColorDrawable(0x8800FF00));
    //lv->setSelection(2);
    lv->setDivider(new ColorDrawable(0x80224422));
    lv->setDividerHeight(1);

    TextView*tv=new TextView("HelloWorld",200,40);
    w->addView(tv).setPos(400,500);
    tv->setBackgroundColor(0xFF00FF00);
    float rotation=.0f;
    Runnable r([&](){
        lv->setRotation(rotation);
        lv->invalidate();
        rotation+=5;
        w->postDelayed(r,80);
    });
    w->postDelayed(r,1000);
    return app.exec();
}


