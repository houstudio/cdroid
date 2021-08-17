#include <windows.h>
#include <cdlog.h>

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
        tv->setLayoutDirection(position<10?View::LAYOUT_DIRECTION_RTL:View::LAYOUT_DIRECTION_LTR);
        tv->setId(position);
        tv->setText("position :"+std::to_string(position));
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0x80002222);
        tv->setTextSize(40);
        return tv;
    }
};

int main(int argc,const char*argv[]){
    App app;
    Window*w=new Window(100,50,1200,620);
    MyAdapter*adapter=new MyAdapter();
    ListView*lv=(ListView*)&w->addView(new ListView(460,500));
    lv->setPos(10,10);
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

    MyAdapter*adapter2=new MyAdapter(1);
    ListView*lstchk=(ListView*)&w->addView(new ListView(500,500));
    lstchk->setPos(500,10);
    lstchk->setAdapter(adapter2);
    for(int i=0;i<56;i++){
        adapter2->add("");
    }
    lstchk->setAdapter(adapter2);
    lstchk->setSelector(new ColorDrawable(0x88FF0000));
    lstchk->setChoiceMode(ListView::CHOICE_MODE_MULTIPLE_MODAL);
    adapter2->notifyDataSetChanged();
    lstchk->setOnItemClickListener([](AdapterView&lv,View&v,int pos,long id){
        LOGD("clicked %d",pos);
        ((CheckBox&)v).toggle();
    });

    app.exec();
    return 0;
};
