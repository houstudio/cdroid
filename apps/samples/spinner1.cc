#include <cdroid.h>
class MyView:public View{
public:
   MyView(int w,int h):View(w,h){
       setMinimumWidth(w);
       setMinimumHeight(h);
   }
   void onDraw(Canvas&c){
       unsigned int colors[]={
            0xFF000000,0xFFFF0000,0xFF00FF00,0xFF0000FF,0xFFFFFF00,
            0xFF00FFFF,0xFFFF00FF,0xFFFFFFFF,0xFFFF8844,0xFFFF4488,0xFF0088AA};
       c.set_color(colors[mID%(sizeof(colors)/sizeof(int))]);
       c.rectangle(0,0,getWidth(),getHeight());
       c.fill();
   }
   
};
class MyAdapter:public ArrayAdapter<std::string>{
private:
    int mType;
public:
    MyAdapter(int tp=0):ArrayAdapter(){
        mType=tp;
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        View*tv=(View*)convertView;
        if(convertView==nullptr){
            if(mType==0)tv=new TextView("",300,36);
            else tv= new MyView(300,36);
            tv->setFocusable(false);
        }
        tv->setId(1000+position);
        if(mType==0){
           ((TextView*)tv)->setText("position:"+std::to_string(position));
           ((TextView*)tv)->setTextColor(0xFFFFFFFF);
           ((TextView*)tv)->setTextSize(24);
        }
        tv->setBackgroundColor(0x80002222);
        return tv;
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,800,600);
    MyAdapter*adapter=new MyAdapter(argc>1?atoi(argv[1]):0);
    for(int i=0;i<10;i++)adapter->add("");
    LinearLayout*linear = new LinearLayout(0,0);
    linear->setOrientation(LinearLayout::VERTICAL);
    w->addView(linear);
    Spinner*spinner=new Spinner(300,40);
    spinner->setAdapter(adapter);
    spinner->setId(100);
    linear->addView(new TextView("Hello world!",200,40));
    LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
    linear->addView(spinner,lp).setPos(100,100).setBackgroundColor(0xFF222222);
    spinner->requestFocus();
    spinner->requestLayout();
    spinner->setSelection(0);
    app.exec();
}
