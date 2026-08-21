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
            tv=new TextView(&App::getInstance());
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

    DrawerLayout*dl = new DrawerLayout(&app);

    /*CONTENT main area*/
    LinearLayout*content = new LinearLayout(&app);
    content->setOrientation(LinearLayout::VERTICAL);
    TextView*tv =new TextView(&app); tv->setText("TextView" );
    content->setBackgroundColor(0xFFFF0000);
    tv->setTextSize(60);
    content->setId(0);
    content->addView(tv);

    ShapeDrawable*sd = new ShapeDrawable();
    sd->setShape(new ArcShape(0,360));
    sd->getShape()->setGradientColors({0x20FFFFFF,0xFFFFFFFF,0x00FFFFFF});//setSolidColor(0x800000FF);
    RippleDrawable*rp = new RippleDrawable(ColorStateList::valueOf(0x80222222),new ColorDrawable(0x8000FF00),sd);
    Button*btn =new Button(&app); btn->setText("Open" );
    btn->setMinimumHeight(64);
    btn->setBackground(rp);
    content->addView(btn);
    btn->setOnClickListener([dl](View&){
        LOGD("openDrawer");
        dl->openDrawer(Gravity::START);
    });

    btn=new Button(&app); btn->setText("Close" );
    content->addView(btn);
    btn->setOnClickListener([dl](View&){
        LOGD("closeDrawer");
        dl->closeDrawer(Gravity::START);
    });
    SeekBar*pb=new SeekBar(&app);
    content->addView(pb);
   
    dl->setBackgroundColor(0xFF778899);
    DrawerLayout::LayoutParams*lp = new  DrawerLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    lp->gravity = Gravity::NO_GRAVITY;
    dl->addView(content,0,lp);


    /**create LEFT slider*/
    lp = new DrawerLayout::LayoutParams(240,LayoutParams::MATCH_PARENT);
    lp->gravity = Gravity::START;
    LinearLayout*left=new LinearLayout(&app);
    left->setOrientation(LinearLayout::VERTICAL);
    left->setBackgroundColor(0xFF00FF00);
    left->setZ(100);
    left->setId(1);

    MyAdapter*adapter = new MyAdapter();
    ListView*lv = new ListView(&app);
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
    LinearLayout*right = new LinearLayout(&app);
    right->setOrientation(LinearLayout::VERTICAL);
    tv =new TextView(&app); tv->setText("Right Panel" );
    tv->setTextSize(40);
    right->addView(tv);
    right->setId(3);
    right->setBackgroundColor(0x80222222);
    lp = new DrawerLayout::LayoutParams(320,LayoutParams::MATCH_PARENT);
    lp->gravity = Gravity::END;
    dl->addView(right,2,lp);


    /*TOP slider*/
    LinearLayout*top =new LinearLayout(&app);
    top->setOrientation(LinearLayout::VERTICAL);
    tv =new TextView(&app); tv->setText("Top Panel" );
    tv->setTextSize(40);
    top->addView(tv);
    top->setId(2);
    top->setBackgroundColor(0x88FF0000);
    lp = new DrawerLayout::LayoutParams(LayoutParams::MATCH_PARENT,128);
    lp->gravity =Gravity::TOP;
    dl->addView(top,3,lp);

    /*BOTTOM slider*/
    LinearLayout*bottom =new LinearLayout(&app);
    bottom->setOrientation(LinearLayout::VERTICAL);
    tv =new TextView(&app); tv->setText("Bottom Panel" );
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
