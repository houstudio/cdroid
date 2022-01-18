#include <cdroid.h>
#include <cdlog.h>

class MyAdapter:public ArrayAdapter<std::string>{
public:
    MyAdapter():ArrayAdapter(){
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{

        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            tv=new TextView("",600,40);
            tv->setBackgroundResource("cdroid:drawable/progress_horizontal.xml");
            tv->setPadding(20,0,0,0);
            tv->setFocusable(false);
        }
        if(tv->getBackground())
            tv->getBackground()->setLevel(0);
        tv->setId(position);
        tv->setText("position :"+std::to_string(position));
        tv->setTextColor(0xFFFFFFFF);
        tv->setTextSize(30);
        return tv;
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(50,50,1200,640);
    MyAdapter*adapter=new MyAdapter();
    ListView*lv=(ListView*)&w->addView(new ListView(460,500));
    lv->setPos(10,10);
    lv->setDivider(new ColorDrawable(0x66008800));
    lv->setVerticalScrollBarEnabled(true);
    lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    lv->setDividerHeight(1);
    for(int i=0;i<56;i++){
        adapter->add("");
    }
    lv->setAdapter(adapter);
    adapter->notifyDataSetChanged();
    lv->setSelector(new ColorDrawable(0x8800FF00));
    lv->setSelection(2);
    lv->setOnItemClickListener([](AdapterView&lv,View&v,int pos,long id){
        LOGD("clicked %d",pos);
    });
    int index=0;
    Runnable run([&](){
        View* v=lv->getChildAt(index);
        Drawable*d=v->getBackground();
        if(d==nullptr)return;
        if(d->getLevel()<10000)
           d->setLevel(d->getLevel()+1000);
        else{d->setLevel(0); index=(index+1)%10;}
        w->postDelayed(run,100);
    });
    w->postDelayed(run,100);
    app.exec();
    return 0;
};
