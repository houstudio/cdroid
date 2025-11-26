#include<cdroid.h>
#include<widget/listview.h>
#include<widget/drawerlayout.h>
#include<cdlog.h>

class MyAdapter:public ArrayAdapter<std::string>{
public:
    MyAdapter():ArrayAdapter(){
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            tv=new TextView("",600,20);
            tv->setPadding(20,0,0,0);
            tv->setFocusable(false);
        }
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
    Window*w = new Window(0,0,-1,-1);

    DrawerLayout*dl = new DrawerLayout(100,100);

    /*CONTENT main area*/
    LinearLayout*content = new LinearLayout(100,100);
    content->setOrientation(LinearLayout::VERTICAL);
    TextView*tv = new TextView("TextView",40,40);
    content->setBackgroundColor(0xFFFF0000);
    tv->setTextSize(60);
    content->setId(0);
    content->addView(tv);

    ShapeDrawable*sd = new ShapeDrawable();
    sd->setShape(new ArcShape(0,360));
    sd->getShape()->setGradientColors({0x20FFFFFF,0xFFFFFFFF,0x00FFFFFF});//setSolidColor(0x800000FF);
    RippleDrawable*rp = new RippleDrawable(ColorStateList::valueOf(0x80222222),new ColorDrawable(0x8000FF00),sd);
    Button*btn = new Button("Open",100,64);
    btn->setMinimumHeight(64);
    btn->setBackground(rp);
    content->addView(btn);
    btn->setOnClickListener([dl](View&){
        LOGD("openDrawer");
        dl->openDrawer(Gravity::START);
    });

    btn=new Button("Close",100,64);
    content->addView(btn);
    btn->setOnClickListener([dl](View&){
        LOGD("closeDrawer");
        dl->closeDrawer(Gravity::START);
    });
   
    dl->setBackgroundColor(0xFF778899);
    DrawerLayout::LayoutParams*lp = new  DrawerLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    lp->gravity = Gravity::NO_GRAVITY;
    dl->addView(content,0,lp);


    /**create LEFT slider*/
    lp = new DrawerLayout::LayoutParams(240,LayoutParams::MATCH_PARENT);
    lp->gravity = Gravity::START;
    LinearLayout*left=new LinearLayout(0,0);
    left->setOrientation(LinearLayout::VERTICAL);
    left->setBackgroundColor(0xFF00FF00);
    left->setZ(100);
    left->setId(1);

    MyAdapter*adapter = new MyAdapter();
    ListView*lv = new ListView(460,500);
    left->addView(lv,new LinearLayout::LayoutParams(-1,-1));
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
    lv->setDivider(new ColorDrawable(0x80224422));
    lv->setDividerHeight(1);

    dl->addView(left,1,lp);

    /*RIGHT slider*/
    LinearLayout*right = new LinearLayout(300,720);
    right->setOrientation(LinearLayout::VERTICAL);
    tv = new TextView("Right Panel",40,40);
    tv->setTextSize(40);
    right->addView(tv);
    right->setId(3);
    right->setBackgroundColor(0x80222222);
    lp = new DrawerLayout::LayoutParams(320,LayoutParams::MATCH_PARENT);
    lp->gravity = Gravity::END;
    dl->addView(right,2,lp);


    /*TOP slider*/
    LinearLayout*top =new LinearLayout(1280,128);
    top->setOrientation(LinearLayout::VERTICAL);
    tv = new TextView("Top Panel",40,128);
    tv->setTextSize(40);
    top->addView(tv);
    top->setId(2);
    top->setBackgroundColor(0x88FF0000);
    lp = new DrawerLayout::LayoutParams(LayoutParams::MATCH_PARENT,128);
    lp->gravity =Gravity::TOP;
    dl->addView(top,3,lp);

    /*BOTTOM slider*/
    LinearLayout*bottom =new LinearLayout(1280,128);
    bottom->setOrientation(LinearLayout::VERTICAL);
    tv = new TextView("Bottom Panel",40,128);
    tv->setTextSize(40);
    bottom->addView(tv);
    bottom->setId(4);
    bottom->setBackgroundColor(0x8800FF00);
    lp = new DrawerLayout::LayoutParams(LayoutParams::MATCH_PARENT,128);
    lp->gravity =Gravity::BOTTOM;
    dl->addView(bottom,4,lp);

    w->addView(dl);
    dl->requestLayout();
    DrawerLayout::DrawerListener dls;
    dls.onDrawerSlide=[](View&view,float offset){
        view.setAlpha(offset);
    };
    dl->addDrawerListener(dls);
    app.exec();
}
