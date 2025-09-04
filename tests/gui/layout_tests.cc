#include <gtest/gtest.h>
#include <cdroid.h>
#include <widget/tablelayout.h>
#include <widget/framelayout.h>
#include <widget/absolutelayout.h>
#include <widget/gridlayout.h>
#include <widget/radiogroup.h>
#include <drawables/drawableinflater.h>
#include <guienvironment.h>
class LAYOUT:public testing::Test{
public:
    int argc;
    const char**argv;
    virtual void SetUp(){
        argc = GUIEnvironment::getInstance()->getArgc();
        argv = GUIEnvironment::getInstance()->getArgv();
    }
    virtual void TearDown(){
    }
};

TEST_F(LAYOUT,linear){
    App app(argc,argv);
    Window*w=new Window(0,0,800,600);
    LinearLayout*ll=new LinearLayout(800,80);
    ShapeDrawable*sd=new ShapeDrawable();
    Shape*shape=new RectShape();
    shape->setSolidColor(0xFFFF0000);
    sd->setShape(shape);
    ll->setDividerPadding(2);
    ll->setShowDividers(7);
    sd->setIntrinsicWidth(3);//setBounds(0,0,1,100);
    sd->setIntrinsicHeight(100);
    ll->setDividerDrawable(sd);
#if 1
    LinearLayout::LayoutParams*lp =new LinearLayout::LayoutParams(0,LayoutParams::WRAP_CONTENT,1.0f);
    Button*btn=new Button("OK",0,0);
    //lp->setMarginsRelative(10,0,10,0);
    lp->gravity=Gravity::TOP;
    ll->setBackground(new ColorDrawable(0xFFFFFFFF));
    ll->addView(btn,lp);
    btn->setBackgroundColor(0xFFFF0000);
    btn->setTextSize(30);
    btn->setTextColor(0xFFFFFF00);

    lp =new LinearLayout::LayoutParams(0,LayoutParams::WRAP_CONTENT,4.0f);
    lp->gravity=(Gravity::CENTER_VERTICAL);
    btn=new Button("Cancel",0,0);
    btn->setTextColor(0xFF0088FF);
    btn->setTextSize(30);
    ll->addView(btn,lp);
    btn->setBackgroundColor(0xFF00FF00);
#endif
#if 10
    ProgressBar*pb2=new ProgressBar(72,72);
    Drawable*d=DrawableInflater::loadDrawable(&app,"@cdroid:drawable/progress_large");
    lp =new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);//,1.0f);
    lp->gravity=Gravity::BOTTOM;//CENTER_VERTICAL);
    pb2->setIndeterminateDrawable(d);
    pb2->setProgressDrawable(new ColorDrawable(0xFF112233));
    pb2->setIndeterminate(true);
    ll->addView(pb2,lp);
#endif
    lp =new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
    lp->gravity=(Gravity::CENTER_VERTICAL);
    lp->setMarginsRelative(20,0,0,0);
    w->addView(ll,lp);
    ll->measure(800,80);
    ll->layout(0,0,800,80);
    app.exec();
}
TEST_F(LAYOUT,radiogroup){
    App app(argc,argv);
    const char*captions[]={"News","Sport","Reading","Walking","I saw A brown fox jump over a lazy dog!"};
    Window*w=new Window(0,0,800,600);
    AttributeSet attrs;
    RadioGroup *rg=new RadioGroup(500,300);//&app,attrs);
    for(int i=0;i<sizeof(captions)/sizeof(captions[0]);i++){
        RadioButton*rb=new RadioButton(captions[i],200,60);
        rb->setId(100+i);
        rb->setPadding(0,8,0,8);
        Drawable*d=DrawableInflater::loadDrawable(&app,"@cdroid:drawable/btn_radio");
        rb->setButtonDrawable(d);
        rb->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);
        rb->setTextColor(0xFFFF0000);
        LinearLayout::LayoutParams*lp=new LinearLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
        rg->addView(rb,i,lp);
    }
    rg->measure(500,300);
    rg->layout(0,0,500,300);
    w->addView(rg);
    app.exec();
}
TEST_F(LAYOUT,frame){
    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    FrameLayout*frame=new FrameLayout(800,600);
    FrameLayout::LayoutParams*lp=new FrameLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT,Gravity::CENTER);

    TextView*tv=new TextView("Hello world!",0,0);
    tv->setBackground(new ColorDrawable(0xFF884444));
    tv->setTextColor(0xFF00FF88);
    tv->setTextSize(100);
    frame->addView(tv,lp);

    lp=new FrameLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT,Gravity::CENTER);
    Button*btn=new Button("OK",0,0);
    btn->setBackground(new ColorDrawable(0xFF4444ff));
    btn->setTextColor(0xFF00FF00);
    frame->addView(btn,lp);

    frame->measure(800,600);
    frame->layout(0,0,800,600);
    frame->setBackground(new ColorDrawable(0xFF222222));
    w->addView(frame);
    app.exec();
}

TEST_F(LAYOUT,absolute){
    App app(argc,argv);
    Window*w=new Window(100,100,800,600);
    AbsoluteLayout*ll=new AbsoluteLayout(800,600);
    AbsoluteLayout::LayoutParams*lp=new AbsoluteLayout::LayoutParams(300,50,100,100);
    TextView*tv=new TextView("A crown fox jump over the lazy dog!",0,0);
    tv->setBackground(new ColorDrawable(0xFF4444ff));
    tv->setTextColor(0xFF00FF00);
    ll->addView(tv,lp);

    lp=new AbsoluteLayout::LayoutParams(150,60,200,200);
    Button *btn=new Button("Browser",0,0);
    btn->setBackground(new ColorDrawable(0xFF4444ff));
    btn->setTextColor(0xFF00FF00);
    ll->addView(btn,lp);
    ll->measure(800,600);
    ll->layout(0,0,800,600);
    w->addView(ll);
    app.exec();
}

TEST_F(LAYOUT,tablerow){
    App app(argc,argv);
    const char*captions[]={"OK","Cancel","Ignore"};
    Window*w=new Window(0,0,800,600);
    TableRow*row=new TableRow(800,80);
    row->setBackground(new ColorDrawable(0xFF444444));
    for(int i=0;i<sizeof(captions)/sizeof(captions[0]);i++){
        TableRow::LayoutParams*rp=new TableRow::LayoutParams(200+i*20,80);
        rp->setMarginsRelative(10,5,10,5);
        Button*btn=new Button(captions[i],0,0);
        btn->setBackground(new ColorDrawable(0xFFFF0000));
        btn->setTextColor(0xFF00FF88);
        row->addView(btn,rp);
    }
    row->measure(800,80);
    row->layout(0,0,800,80);
    w->addView(row);
    app.exec();
}

TEST_F(LAYOUT,table){
    App app(argc,argv);
    const char*captions[]={"OK","Cancel","Ignore"};
    Window*w=new Window(0,0,800,600);
    TableLayout*tbl=new TableLayout(800,320);
    TableRow* row[4];
    for(int j=0;j<4;j++){
        row[j]=new TableRow(800,80);
        for(int i=0;i<sizeof(captions)/sizeof(captions[0]);i++){
            TableRow::LayoutParams*rp=new TableRow::LayoutParams(200+i*40,80);
            rp->setMarginsRelative(10,0,10,0);
            Button*btn=new Button(captions[i],0,0);
            btn->setBackground(new ColorDrawable(0xFFFF0000+i*20));
            btn->setTextColor(0xFF00FF00+i*20);
            row[j]->addView(btn,rp);
        }
        TableLayout::LayoutParams*tp=new TableLayout::LayoutParams(800,80);
        tp->setMarginsRelative(0,5,0,5);
        tbl->addView(row[j],tp);
        //row[j]->measure(800,80);
        //row[j]->layout(0,0,800,80);
    }
    tbl->measure(800,320);
    tbl->layout(0,0,800,320);
    w->addView(tbl);
    app.exec();
}

TEST_F(LAYOUT,grid){
    App app(argc,argv);
    const char*captions[]={"OK","Cancel","Ignore","Hello world!","Sina News"};
    Window*w=new Window(0,0,800,600);
    GridLayout*grd=new GridLayout(800,400);
    const int N=sizeof(captions)/sizeof(captions[0]);
    for(int i=0;i<9;i++){
        GridLayout::LayoutParams*gp=new GridLayout::LayoutParams();
        gp->setMarginsRelative(5,5,5,5);
        Button*btn=new Button(captions[i%N],0,0);
        btn->setBackground(new ColorDrawable(0xFFFF0000+i*20));
        btn->setTextColor(0xFF00FF00+i*20);
        grd->addView(btn,gp);
    }
    grd->measure(800,400);
    grd->layout(0,0,800,400);
    w->addView(grd);
    app.exec();
 
}
