#include <gtest/gtest.h>
#include <cdroid.h>
#include <widget/gridview.h>
#include <widget/listview.h>
#include <ngl_os.h>
#include<ngl_timer.h>

class LISTVIEW:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

TEST_F(LISTVIEW,Selector){
}

class MyAdapter:public ArrayAdapter<std::string>{
 public:
    MyAdapter():ArrayAdapter(){
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{

        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            tv=new TextView("",600,20);
        tv->setPadding(20,0,0,0);
        }
        tv->setId(position);
        tv->setText("position :"+std::to_string(position));
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0x80002222);
        tv->setTextSize(40);
        return tv;
    }
};

TEST_F(LISTVIEW,ListView){
    App app;
    Window*w=new Window(100,50,800,620);
    MyAdapter*adapter=new MyAdapter();
    ListView*lv=(ListView*)&w->addView(new ListView(300,500));
    lv->setPos(10,10);
    for(int i=0;i<56;i++){
        adapter->add("");
    }
    lv->setAdapter(adapter);
    adapter->notifyDataSetChanged();
    w->addView(lv);
    lv->setSelector(new ColorDrawable(0x8800FF00));
    lv->setSelection(2);
    app.exec();
}

TEST_F(LISTVIEW,GridView){
    App app;
    Window*w=new Window(100,50,800,640);
    GridView*g=new GridView(600,600);
    w->addView(g).setPos(10,10);
    MyAdapter*adapter=new MyAdapter();
    for(int i=0;i<56;i++){
       adapter->add("");
    }
    g->setNumColumns(2);
    g->setAdapter(adapter);
    adapter->notifyDataSetChanged();
    g->setSelector(new ColorDrawable(0x8800FF00));
    g->setSelection(2);
    app.exec();
}

