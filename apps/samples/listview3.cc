#include <widget/linearlayout.h>
#include <widget/checkable.h>
#include <widget/textview.h>
#include <widget/checkbox.h>
#include <widget/listview.h>
#include <core/app.h>
#include <porting/cdlog.h>

class DataView:public LinearLayout,public Checkable{
private:
    TextView* id;
    TextView* name;
    CheckBox* chk;
public:
    DataView():LinearLayout(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT){
        id=new TextView("",0,0);
        id->setBackgroundColor(0xFF222222);
        setGravity(Gravity::CENTER);
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(50,LayoutParams::WRAP_CONTENT);
        addView(id,lp);

        name=new TextView("",0,0);
        lp=new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT,5);
        addView(name,lp);

        chk=new CheckBox("",0,0);
        chk->setClickable(true);
        lp= new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT,1);
        addView(chk,lp);
#if defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE
        setChecked=[this](bool checked){ chk->setChecked(checked); };
        isChecked=[this]()->bool{ return chk->isChecked(); };
        toggle = [this](){ chk->toggle(); };
#endif
    }
#if !(defined(FUNCTION_AS_CHECKABLE)&&FUNCTION_AS_CHECKABLE)
    void setChecked(bool checked)override{
	chk->setChecked(checked);
	LOGD_IF(checked,"setChecked(%d)",checked);
    }
    bool isChecked()const override{
	return chk->isChecked();
    }
    void toggle()override{
	chk->toggle();
	LOGD("toggle");
    }
#endif
    void setName(const std::string&txt){
        name->setText(txt);
    }
    void setId(int idpos){
        id->setText(std::to_string(idpos));
    }
    void startMarqueeIfNeed(bool enableMarquee){
        name->setSingleLine(enableMarquee);
        name->setEllipsize(enableMarquee?Layout::ELLIPSIS_MARQUEE:Layout::ELLIPSIS_NONE);
        name->setSelected(enableMarquee);
        requestLayout();
    }
};
struct MyData{
    int id;
    std::string name;
};
class MyAdapter:public ArrayAdapter<MyData>{
public:
    MyAdapter():ArrayAdapter(){
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        DataView *dv=(DataView*)convertView;
        MyData&dt=getItemAt(position);
        if(convertView==nullptr){
            dv=new DataView();
        }
        dv->setId(position);
        dv->startMarqueeIfNeed(position==10);
        dv->setName(dt.name);
        dv->setChecked(position%3==0);
        return dv;
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    MyAdapter*adapter=new MyAdapter();
    ListView*lv=new ListView(460,500);
    w->addView(lv);
    lv->layout(10,10,460,500);
    lv->setDivider(new ColorDrawable(0x66008800));
    lv->setVerticalScrollBarEnabled(true);
    lv->setDividerHeight(1);
    for(int i=0;i<56;i++){
        MyData dt;
        dt.id=i;
        dt.name=std::string("Name ")+std::to_string(i);
        if(i==10||i==11)
           dt.name="test line with marquee support ,the lie must be long enough";
        adapter->add(dt);
    }
    lv->setAdapter(adapter);
    adapter->notifyDataSetChanged();
    lv->setSelector(new ColorDrawable(0x8800FF00));
    lv->setChoiceMode(ListView::CHOICE_MODE_MULTIPLE);//SINGLE);
    lv->setSelection(2);
    lv->setOnItemClickListener([](AdapterView&lv,View&v,int pos,long id){
        LOGD("clicked %d",pos);
    });
    app.exec();
    return 0;
};
