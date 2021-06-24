#include <windows.h>
#include <widget/simplemonthview.h>

class MyPageAdapter:public PagerAdapter{
public:
    int getCount(){return 5;}
    bool isViewFromObject(View* view, void*object) { return view==object;}
    void* instantiateItem(ViewGroup* container, int position) {
        if(true){//position!=2){
            SimpleMonthView*sm=new  SimpleMonthView(100,100);
            sm->setMonthParams(23,Calendar::MAY+position,2021,-1,1,31);
            container->addView(sm);
            return sm;
        }/*else{
            ListView*lv=new  ListView(100,100);
            MyAdapter*ma=new MyAdapter();
            for(int i=0;i<50;i++)ma->add("");
            container->addView(lv);
            lv->setAdapter(ma);
            lv->setSelector(new ColorDrawable(0x8800FF00));
            ma->notifyDataSetChanged();
            return lv;
        }*/
    }
    void destroyItem(ViewGroup* container, int position,void* object){
        container->removeView((View*)object);
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,800,600);
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
        btn->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
        ll->addView(btn,new LinearLayout::LayoutParams(800,LayoutParams::WRAP_CONTENT));
    }
    ViewPager*vp=new ViewPager(800,560);
    MyPageAdapter*gpAdapter=new MyPageAdapter();
    vp->setOffscreenPageLimit(4);
    vp->setAdapter(gpAdapter);
    ViewPager::OnPageChangeListener listener={nullptr,nullptr,nullptr};
    listener.onPageSelected=[&](int position){
        //hs->
    };
    listener.onPageScrolled=[&](int position, float positionOffset, int positionOffsetPixels){
        hs->scrollTo(position*vp->getWidth()+positionOffsetPixels,0);
    };
    vp->addOnPageChangeListener(listener);
    vp->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    w->addView(vp).setPos(0,40);
    gpAdapter->notifyDataSetChanged();
    vp->setCurrentItem(0);
    w->requestLayout();
    app.exec();
}
