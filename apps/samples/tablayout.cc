#include <cdroid.h>
class MyPageAdapter:public PagerAdapter{
public:
    int getCount()override{return 8;}
    bool isViewFromObject(View* view, void*object)override{ return view==object;}
    void* instantiateItem(ViewGroup* container, int position)override{
        SimpleMonthView*sm=new  SimpleMonthView(1280,560);
        sm->setMonthParams(23,Calendar::MAY+position,2021,-1,1,31);
        container->addView(sm).setId(100+position);
        return sm;
    }
    void destroyItem(ViewGroup* container, int position,void* object)override{
        container->removeView((View*)object);
        delete (View*)object;
    }
    std::string getPageTitle(int position)override{
        return std::string("Tab")+std::to_string(position);
    }
    float getPageWidth(int position)override{return 1.f;}//if returned calue <1 OffscreenPageLimit must be larger to workfine
};


int main(int argc,const char*argv[]){
    App app(argc,argv);
    AnimationHandler::getInstance().setFrameDelay(200);
    Window*w=new Window(0,0,1280,600);
    MyPageAdapter*gpAdapter=new MyPageAdapter();

    LinearLayout*layout=new LinearLayout(1280,600);
    layout->setOrientation(LinearLayout::VERTICAL);

    TabLayout* tab=new TabLayout(1280,36);
    ViewPager* pager=new ViewPager(1280,560);
    pager->setOffscreenPageLimit(gpAdapter->getCount());
    pager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    pager->setAdapter(gpAdapter);
    pager->setBackgroundColor(0xFFF3333);
    tab->setSelectedTabIndicatorColor(0x8000FF00);
    tab->setSelectedTabIndicatorHeight(4); 
    tab->setTabIndicatorGravity(Gravity::BOTTOM);//TOP/BOTTOM/CENTER_VERTICAL/FILL_VERTICAL
    tab->setupWithViewPager(pager);
    layout->addView(tab).setId(1);
    layout->addView(pager).setId(10);
    tab->setTabTextColors(0xFFFF0000,0xFF00FF00);
    w->addView(layout);
    w->requestLayout();
    int result = app.exec();
    delete gpAdapter;
    return result;
}
