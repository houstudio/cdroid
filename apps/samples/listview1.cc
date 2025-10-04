#include <cdroid.h>
#include <cdlog.h>
#include <animation/animations.h>
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
            tv->setSingleLine(true);
        }
        if(itemType==1)tv->setLayoutDirection(position<10?View::LAYOUT_DIRECTION_RTL:View::LAYOUT_DIRECTION_LTR);
        tv->setId(position);
        tv->setText("position :"+std::to_string(position));
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0x80002222);
        tv->setTextSize(24);
        return tv;
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    MyAdapter*adapter=new MyAdapter(0);

    Animation *anim= new ScaleAnimation(0.5,1,0.1,1,Animation::RELATIVE_TO_PARENT,.5,Animation::RELATIVE_TO_SELF,.5);
    anim->setDuration(500);
    w->setId(10);
    LayoutAnimationController*lac = new LayoutAnimationController(anim,0.02);
    ListView*lv = new ListView(460,500);
    w->addView(lv);
    lv->setId(100);
    lv->layout(10,10,460,500);
    adapter->setNotifyOnChange(true);
    lv->setAdapter(adapter);
    for(int i=0;i<56;i++) adapter->add("");

    lv->setLayoutAnimation(lac);
    adapter->notifyDataSetChanged();
    lv->startLayoutAnimation();
    lv->setVerticalScrollBarEnabled(true);    
    lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    lv->setSmoothScrollbarEnabled(true);
    lv->setSelector(new ColorDrawable(0x8800FF00));
    //lv->setSelection(2);
    lv->setDivider(new ColorDrawable(0x80224422));
    lv->setDividerHeight(1);
    lv->setOnItemClickListener([](AdapterView&lv,View&v,int pos,long id){
        LOGD("clicked %d",pos);
    });

    ListView::OnItemSelectedListener listener={nullptr,nullptr};
    listener.onItemSelected=[](AdapterView&lv,View&v,int pos,long id){
        LOGD("selected position %d",pos);
    }; 
    lv->setOnItemSelectedListener(listener);
////////////////////////////////////////////////////////////////////////////////////////

    MyAdapter*adapter2=new MyAdapter(1);
    ListView*lv2 = new ListView(500,500);
    w->addView(lv2);
    lv2->setId(200);
    ToggleButton *toggle=new ToggleButton(300,40);
    w->addView(toggle);
    toggle->layout(500,520,300,40);
    lv2->layout(500,10,500,500);
    lv2->setAdapter(adapter2);
    for(int i=0;i<56;i++)  adapter2->add("");
   
    lv2->setDivider(new ColorDrawable(0x80224422));
    lv2->setDividerHeight(1);
    lv2->setVerticalScrollBarEnabled(true);
    lv2->setSmoothScrollbarEnabled(true);
    lv2->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    lv2->setAdapter(adapter2);
    lv2->setSelector(new ColorDrawable(0x88FF0000));
    lv2->setChoiceMode(ListView::CHOICE_MODE_SINGLE);//MULTIPLE);
    adapter2->notifyDataSetChanged();
    lv2->setOnItemClickListener([&](AdapterView&lv,View&v,int pos,long id){
        LOGD("clicked %d",pos);
        lv2->setItemChecked(pos,((CheckBox&)v).isChecked());//lv2->isItemChecked(pos));
    });
    lv2->setMultiChoiceModeListener([&](int position, long id, bool checked){
        LOGD("multichoice %d checked=%d",position,checked);
        //lv2->setItemChecked(position,checked);
    });
    toggle->setTextOn("SingleChoice");
    toggle->setTextOff("MultiChoice");
    toggle->setBackgroundResource("cdroid:drawable/btn_toggle_bg.xml");
    toggle->setOnCheckedChangeListener([&](CompoundButton&view,bool check){
        lv2->setChoiceMode(check?ListView::CHOICE_MODE_SINGLE:ListView::CHOICE_MODE_MULTIPLE);
    });

    Runnable rd;
    rd=[&rd,lv,w,adapter](){
        for(int i=0;i<10;i++)adapter->add("");
        lv->startLayoutAnimation();
        //w->postDelayed(rd,1000);
    };
    w->postDelayed(rd,10000);
    app.exec();
    return 0;
};
