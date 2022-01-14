#include <windows.h>
#include <cdlog.h>

class MyAdapter:public ArrayAdapter<std::string>{
public:
    MyAdapter():ArrayAdapter(){
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            tv=new TextView("",600,40);
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

View*createHeader(){
    LinearLayout*ll=new LinearLayout(-1,200);
    ll->setOrientation(LinearLayout::VERTICAL);
    for(int i=0;i<4;i++){
        std::string txt;
        TextView*tv;
        if(i%2){
            txt="Button"+std::to_string(i/2);
            tv=new Button(txt,200,40);
        }else{
            txt="CheckItem"+std::to_string(i/2);
            tv=new CheckBox(txt,200,40);
            tv->setClickable(true);
        }
        ll->addView(tv);
    }
    return ll;
}
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
    for(int i=0;i<32;i++){
        adapter->add("");
    }
    lv->addHeaderView(createHeader(),nullptr,false);
    lv->addFooterView(createHeader(),nullptr,true);
    lv->setAdapter(adapter);
    adapter->notifyDataSetChanged();
    lv->setSelector(new ColorDrawable(0x8800FF00));
    lv->setSelection(2);
    lv->setOnItemClickListener([](AdapterView&lv,View&v,int pos,long id){
        LOGD("clicked %d",pos);
    });
    lv->requestLayout();
    app.exec();
    return 0;
};
