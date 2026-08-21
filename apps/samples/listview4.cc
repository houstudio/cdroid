#include <cdroid.h>
#include <cdlog.h>

class MyAdapter:public ArrayAdapter<std::string>{
public:
    MyAdapter():ArrayAdapter(){
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            tv=new TextView(&App::getInstance());
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

View*createHeader(int id){
    LinearLayout*ll=new LinearLayout(&App::getInstance());
    ll->setOrientation(LinearLayout::HORIZONTAL);
    for(int i=0;i<1;i++){
        std::string txt;
        TextView*tv;
        if(i%2){
            txt="Button"+std::to_string(i/2);
            tv=new Button(&App::getInstance()); tv->setText(txt);
        }else{
            txt="CheckItem"+std::to_string(i/2);
            tv=new CheckBox(&App::getInstance()); tv->setText(txt);
            tv->setClickable(true);
        }
        ll->addView(tv);
    }
    ll->setId(id);
    return ll;
}
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    MyAdapter*adapter=new MyAdapter();
    ListView*lv = new ListView(&App::getInstance());
    w->addView(lv);
    lv->layout(10,10,460,500);
    lv->setDivider(new ColorDrawable(0x66008800));
    lv->setVerticalScrollBarEnabled(true);
    lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    lv->setDividerHeight(1);
    for(int i=0;i<64;i++){
        adapter->add("");
    }
    lv->setAdapter(adapter);
    adapter->notifyDataSetChanged();
    lv->setSelector(new ColorDrawable(0x8800FF00));
    lv->setSelection(2);
    lv->setOnItemClickListener([adapter](AdapterView&lv,View&v,int pos,long id){
        LOGD("clicked %d",pos);
        if(pos==60)
            ((AbsListView&)lv).smoothScrollToPosition(0);
    });
    Button*add=new Button(&App::getInstance()); add->setText("Add Header" );
    add->setOnClickListener([lv,adapter](View&){
        lv->addHeaderView(createHeader(888888),nullptr,false);
	adapter->notifyDataSetChanged();
    });
    w->addView(add);
    add->layout(480,10,200,60);
    Button*del=new Button(&App::getInstance()); del->setText("Remove Header" );
    del->setOnClickListener([lv,adapter](View&){
	View*header=lv->findViewById(888888);
        lv->removeHeaderView(header);
        adapter->notifyDataSetChanged();
    });
    w->addView(del);
    del->layout(480,80,200,60);
    app.exec();
    return 0;
};
