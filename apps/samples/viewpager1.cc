#include <cdroid.h>
#include <widget/simplemonthview.h>
#include <pagetransformers.h>
static int mPageCount=5;
class MyPageAdapter:public PagerAdapter{
public:
    int getCount(){return 8;}
    bool isViewFromObject(View* view, void*object) { return view==object;}
    void* instantiateItem(ViewGroup* container, int position) {
#if ENABLE(DAYTIME_WIDGETS)
        if(position<getCount()/2){
            SimpleMonthView*sm=new  SimpleMonthView(100,100);
            sm->setMonthParams(23,Calendar::MAY+position,2021,-1,1,31);
            container->addView(sm);
            return sm;
        }else
#endif
        {
            View*sm=new View(100,100);
            sm->setBackground(new ColorDrawable(0xFF000000|(0xFF<<((position%3)*4))));
            container->addView(sm);
            return sm;
        }
    }
    void destroyItem(ViewGroup* container, int position,void* object){
        container->removeView((View*)object);
        delete (View*)object;
    }
    float getPageWidth(int position){return 0.2f;}//if returned calue <1 OffscreenPageLimit must be larger to workfine 
};

class MyWindow:public Window,public ViewPager::OnPageChangeListener{
public:
     MyWindow(int x,int y,int w,int h):Window(x,y,w,h){
	 onPageSelected =std::bind(&MyWindow::onViewPageSelected,this,std::placeholders::_1);
	 onPageScrolled =std::bind(&MyWindow::onViewPageScrolled,this,std::placeholders::_1,
			 std::placeholders::_2,std::placeholders::_3);
     }
     void onViewPageSelected(int position){
         LOGD("selected %d",position);
     }
     void onViewPageScrolled(int position, float positionOffset, int positionOffsetPixels){
         LOGD("position=%d positionOffset=%f positionOffsetPixels=%d",position,positionOffset,positionOffsetPixels);
     }
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    MyWindow*w=new MyWindow(0,0,-1,-1);

    if(argc>1)mPageCount=std::max(5L,std::strtol(argv[1],nullptr,10));
    ViewPager*pager=new ViewPager(800,560);
    pager->setHorizontalFadingEdgeEnabled(true);
    pager->setFadingEdgeLength(200);
    MyPageAdapter*gpAdapter=new MyPageAdapter();
    pager->setOffscreenPageLimit(8);
    pager->setAdapter(gpAdapter);
    pager->addOnPageChangeListener(*w);
    pager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    w->addView(pager);
    pager->layout(0,40,800,560);
    gpAdapter->notifyDataSetChanged();
    pager->setCurrentItem(0);
    w->requestLayout();
    if(argc>1)
    pager->setPageTransformer(true,new RotateUpTransformer());

    app.exec();
}
