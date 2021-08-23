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
    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    ScrollView*scroller=new ScrollView(800,600);
    w->addView(scroller);
    LinearLayout*layout=new LinearLayout(800,600);
    layout->setOrientation(LinearLayout::VERTICAL);
    scroller->addView(layout,new LinearLayoutParams(LayoutParams::MATCH_PARENT,(LayoutParams::MATCH_PARENT)));
    for(int i=0;i<10;i++){
        LinearLayoutParams*lp=new LinearLayoutParams(LayoutParams::MATCH_PARENT,(LayoutParams::WRAP_CONTENT));
        lp->setMargins(5,2,5,2);
        EditText*edit=new EditText(TEXT("Hello world! This value is positive for typical fonts that include"),680,200);
        edit->setTextColor(0xFFFFFFFF);//app.getColorStateList("cdroid:color/textview.xml"));
        //edit->setTextColor(app.getColorStateList("cdroid:color/textview.xml"));
        edit->setSingleLine(false);
        edit->setInputType(EditText::TYPE_ANY);
        edit->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
        edit->setBackgroundColor(0xFF000000|((i*8)<<16)|((i*8)<<8)|(i*8));
        edit->setTextSize(40);
        edit->setFocusable(i==0);
        layout->addView(edit,lp);
    }

    ListView*lv=new ListView(0,300);
    MyAdapter*adapter=new MyAdapter();
    lv->setAdapter(adapter);
    lv->setSelector(new ColorDrawable(0xFF00FF00));
    lv->setDivider(new ColorDrawable(0x40FFFFFF));
    lv->setDividerHeight(1);
    layout->addView(lv,new LinearLayoutParams(LayoutParams::MATCH_PARENT,300));

    for(int i=0;i<10;i++)
       adapter->add("");
    adapter->notifyDataSetChanged();
    scroller->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    scroller->setVerticalScrollBarEnabled(true);
    w->requestLayout();
    return app.exec();
}
