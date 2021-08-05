#include<windows.h>
#include<widget/listview.h>
#include<widget/gridview.h>
#include<widget/scrollview.h>
#include<widget/spinner.h>
#include<widget/horizontalscrollview.h>
#include<widget/simplemonthview.h>
#include<widget/viewpager.h>
#include<animations.h>
#include<drawables.h>
#include<cdlog.h>
class MyAdapter:public ArrayAdapter<std::string>{
public:
   MyAdapter():ArrayAdapter(){
   }
   View*getView(int position, View* convertView, ViewGroup* parent)override{

       TextView*tv=(TextView*)convertView;
       if(convertView==nullptr)
           tv=new TextView("",600,20);
	   tv->setPadding(20,0,0,0);
       tv->setId(position);
       tv->setText("position :"+std::to_string(position));
       tv->setTextColor(0xFFFFFFFF);
       tv->setBackgroundColor(0x80002222);
       tv->setTextSize(40);
       return tv;
   }
};
class MyPageAdapter:public PagerAdapter{
public:
    int getCount(){return 5;}
    bool isViewFromObject(View* view, void*object) { return view==object;}
    void* instantiateItem(ViewGroup* container, int position) {
        if(position!=2){
            SimpleMonthView*sm=new  SimpleMonthView(100,100);
            sm->setMonthParams(23,Calendar::MAY+position,2021,-1,1,31);
            container->addView(sm);
            return sm;
        }else{
            ListView*lv=new  ListView(100,100);
            MyAdapter*ma=new MyAdapter();
            for(int i=0;i<50;i++)ma->add("");
            container->addView(lv);
            lv->setAdapter(ma);
            lv->setSelector(new ColorDrawable(0x8800FF00));
            ma->notifyDataSetChanged();
            return lv;
        }
    }
    void destroyItem(ViewGroup* container, int position,void* object){
        container->removeView((View*)object);
    }
};
class TestWindow:public Window{
private:
    RefPtr<ImageSurface>imgSurface;
    float mDegrees;
    int mLevel;
public:
    float sdot(float a,float b,float c,float d) {
       return a * b + c * d;
    }
    TestWindow(int x,int y,int w,int h):Window(x,y,w,h){
        imgSurface=ImageSurface::create(Surface::Format::ARGB32,160,160);
        RefPtr<Cairo::Context>canvas=Cairo::Context::create(imgSurface);
        FontExtents fe;
        canvas->set_source_rgb(1,1,1);
        canvas->rectangle(0,0,160,160);
        canvas->fill_preserve();
        canvas->set_source_rgb(0,1,0);
        canvas->set_line_width(5);
        canvas->stroke();
        canvas->arc(5,5,10,0,2.f*M_PI);
        canvas->fill();
        mDegrees=0;
    }
    /*bool onMessage(DWORD msg,DWORD wp,ULONG lp)override{
        if(msg==1000){
             invalidate(nullptr);
        }
        return Window::onMessage(msg,wp,lp);
    }*/
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new TestWindow(0,0,1280,640);
    MyAdapter* adapter=new MyAdapter();
    MyPageAdapter*gpAdapter=new MyPageAdapter();
    w->setId(0);
    for(int i=0;i<50;i++)adapter->add(""); 
    int optionid=0;
    RECT rect;
    if(argc>1)optionid=std::atoi(argv[1]);
    w->setBackgroundColor(0xFFFF0000);
    AbsListView::OnScrollListener ons={nullptr,nullptr};
    ons.onScroll=[](AbsListView&lv,int firstVisibleItem,int visibleItemCount, int totalItemCount){
        LOGV("firstVisibleItem=%d visibleItemCount=%d totalItemCount=%d",firstVisibleItem,visibleItemCount,totalItemCount);
    };
    ons.onScrollStateChanged=[](AbsListView& view, int scrollState){
        LOGV("scrollState=%d",scrollState);
    };
    switch(optionid){
    case 0:{
        ListView*lv=new ListView(550,400);
        lv->setTranscriptMode(2);
	lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
        w->addView(lv).setBackgroundColor(0xFF881111);
        lv->setId(1010);
        lv->getDrawingRect(rect);
        lv->setOnScrollListener(ons);
        lv->setChoiceMode(ListView::CHOICE_MODE_MULTIPLE_MODAL);//CHOICE_MODE_SINGLE);//CHOICE_MODE_MULTIPLE
        lv->setMultiChoiceModeListener([](int position, long id, bool checked){
             LOGD("item %d checked:%d",position,checked );
        });
        //[](AbsListView&,int fist,int vc,int total){});
        //lv->setItemsCanFocus(true);
        lv->setVerticalScrollBarEnabled(true);
        ListView::OnItemSelectedListener listener={
            .onItemSelected=[](AdapterView&parent,View&v,int pos,long id){
                LOGD("adapterview.itemselected=%p view=%p pos=%d,id=%ld",&parent,&v,pos,id);
            },
            .onNothingSelected=[](AdapterView&parent){
                LOGD("adapterview.onNothingSelected");
            }
        };
        lv->setOnItemSelectedListener(listener);
        lv->setOnItemClickListener([](AdapterView&parent,View&v,int pos,long id){
            LOGD("adapterview.itemlick=%p view=%p pos=%d,id=%ld",&parent,&v,pos,id);
        });
        lv->setOnItemLongClickListener([](AdapterView&parent,View&v,int pos,long id)->bool{
            LOGD("adapterview.itemlonglick=%p view=%p pos=%d,id=%ld",&parent,&v,pos,id);
            return true;
        });
        lv->setDivider(new ColorDrawable(0xFF0000FF));
        lv->setDividerHeight(1);
        lv->setAdapter(adapter);
        //lv->addHeaderView(new TextView("====================",400,20),nullptr,false);
        lv->setSmoothScrollbarEnabled(true);
        adapter->notifyDataSetChanged(); 
    
        lv->setBackgroundColor(0xFFFF0000);
        lv->setSelector(new ColorDrawable(0x8800FF00));
        lv->setSelection(0);
        //lv->smoothScrollBy(200,5000);
    }break;
    case 1:{
        GridView*gv=new GridView(0,0);
        w->addView(gv).setId(100);
        gv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
        gv->setVerticalScrollBarEnabled(true);
        
        gv->setAdapter(adapter);
        adapter->notifyDataSetChanged();

        gv->setNumColumns(2);
        gv->setColumnWidth(300);
        gv->setHorizontalSpacing(1);
        gv->setVerticalSpacing(2);
        gv->setSelector(new ColorDrawable(0x8800FF00));
        gv->setSelection(0);
        w->requestLayout();
    }break;
    case 2:{//w(0)-->hs(1)-->ll(10)-->buttons(100+i)
        HorizontalScrollView* hs=new HorizontalScrollView(800,400);
        LinearLayout*ll=new LinearLayout(400,100);
        ColorStateList*cl=ColorStateList::inflate(nullptr,"/home/houzh/Miniwin/src/gui/res/color/textview.xml");
        ll->setId(10);
        hs->addView(ll);
        hs->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
        w->addView(hs).setId(1);

        for(int i=0;i<16;i++){
            Button*btn=new Button("Hello Button"+std::to_string(i),150,30);
            btn->setPadding(5,5,5,5);
            btn->setTextColor(cl);
            btn->setTextSize(30);
            btn->setBackgroundColor(0xFF000000|i*20<<8|i*8);
            btn->setId(100+i);
            ll->addView(btn);
        }
        w->requestLayout();
	hs->scrollTo(600,0);
    }break;
    case 3:{
       LinearLayout*ll=new LinearLayout(400,100);
       LinearLayoutParams*lp=new LinearLayoutParams(-1,500);
       FrameLayoutParams*flp=new FrameLayoutParams(-1,-1);
       ll->setOrientation(1);
       ScrollView*sv=new ScrollView(600,400);
       sv->setId(10);
       sv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
       ll->setId(100);

       ListView*lv=new ListView(400,600);
       lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
       lv->setAdapter(adapter);
       lv->setBackgroundColor(0xFFFF0000);
       lv->setSelector(new ColorDrawable(0x8800FF00));
       lv->setVerticalScrollBarEnabled(true);
       lv->setId(101);
       for(int i=0;i<10;i++){
           TextView*tv=new TextView("Hello world"+std::to_string(i),200,40);
           ll->addView(tv).setPos(10,42*i).setId(1000+i);
           tv->setBackgroundColor(0xFF000000|(i*8<<8));
       }
       ll->addView(lv,lp).setPos(0,42*5+10);
       adapter->notifyDataSetChanged();
       sv->addView(ll,flp);
       sv->setVerticalScrollBarEnabled(true);
       w->addView(sv);
       w->requestLayout();
       break;
    }
    case 4:
      for(int i=0;i<5;i++){
          ColorStateList*cl=ColorStateList::inflate(nullptr,"/home/houzh/Miniwin/src/gui/res/color/textview.xml");
          Button*btn=new Button("Test Button"+std::to_string(i),180,50);
          btn->setTextColor(cl);
          btn->setBackgroundColor(0xFF222222);
          btn->setFocusableInTouchMode(true);
          w->addView(btn).setPos(20+200*i,50).setId(100+i);
          btn->setNextFocusRightId(100+(i+1)%5);
          btn->setNextFocusDownId(100+(i+1)%5);
      }
      break;
    case 5:{
        TabWidget*tab=new TabWidget(600,80);
        for(int i=0;i<6;i++){
            const std::string tbnm="Tab"+std::to_string(i);
            tab->addView(new TextView(tbnm,100,40));
        }
        tab->setLeftStripDrawable(new ColorDrawable(0xFFFF0000));
        tab->setRightStripDrawable(new ColorDrawable(0xFF00FF00));
        tab->setCurrentTab(0);
        w->addView(tab);
        w->requestLayout();
    }break;
    case 6:{
        LinearLayout*ll=new LinearLayout(640,480);
        SimpleMonthView*mv=new SimpleMonthView(480,320);
        mv->setBackgroundDrawable(new ColorDrawable(0xFF111111));
        mv->setMonthParams(23,Calendar::MAY,2021,-1,1,31);
        ll->addView(mv,new LinearLayout::LayoutParams(-1,-1));
        w->addView(ll);
        w->requestLayout();
    }break;
    case 7:{
        NumberPicker*np=new NumberPicker(400,100);
        np->setMinValue(2000);
        np->setMaxValue(2100);
        np->setBackgroundDrawable(new ColorDrawable(0xFF222222));
        w->addView(np).setPos(100,200);
        w->requestLayout();
    }break;
    case 8:{
        ViewPager*vp=new ViewPager(800,400);
        vp->setOffscreenPageLimit(4);
        vp->setAdapter(gpAdapter);
        vp->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
        gpAdapter->notifyDataSetChanged();
        w->addView(vp);
        w->requestLayout();
    }break;
    case 9:
       {
            EditText*tv=new EditText("mChoreographer->postFrameCallbackDelayed(mRestartCallback, MARQUEE_DELAY);",300,40);
            w->addView(tv);
            tv->requestFocus();
            tv->setEllipsize(Layout::ELLIPSIS_MARQUEE);
       }break;
    case 10:{
           ViewFlipper*vfp=new ViewFlipper(400,600);
           w->addView(vfp);
           vfp->setFlipInterval(1500);
           ScaleAnimation*sa=new ScaleAnimation(0,1.f,0.f,1.f,200,300);
           sa->setDuration(1000);
           vfp->setInAnimation(sa);
           sa= new ScaleAnimation(0,1.f,0.f,1.f,200,300);
           sa->setDuration(1000);
           vfp->setOutAnimation(sa); 
           for(int i=0;i<10;i++){
               TextView*tv=new TextView(std::to_string(i),400,600);
               tv->setTextSize(120);
               tv->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);//setGravity(Gravity::CENTER);
               tv->setTextColor(0xFFFFFFFF);
               tv->setBackgroundColor(0xFF000000|(i*10)<<((i%2+1)*8));
               vfp->addView(tv,i,new ViewFlipper::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT));
           }
           vfp->startFlipping(); 
           w->requestLayout();
           RotateAnimation*ra= new RotateAnimation(0,360,200,300);
           ra->setDuration(30000);
           vfp->startAnimation(ra);
        }break;
    default:    break;
    }
    app.exec();
}
