#include <cdroid.h>
#include <widget/simplemonthview.h>
#include <pagetransformers.h>
static int mPageCount=5;
class MyPageAdapter:public PagerAdapter{
public:
    int getCount(){return 8;}
    bool isViewFromObject(View* view, void*object) { return view==object;}
    void* instantiateItem(ViewGroup* container, int position) {
        if(position<getCount()/2){
            SimpleMonthView*sm=new  SimpleMonthView(100,100);
            sm->setMonthParams(23,Calendar::MAY+position,2021,-1,1,31);
            container->addView(sm);
            return sm;
        }else{
            View*sm=new View(100,100);
            sm->setBackground(new ColorDrawable(0xFF000000|(0xFF<<((position%3)*8))));
            container->addView(sm);
            return sm;
        }
    }
    void destroyItem(ViewGroup* container, int position,void* object){
        container->removeView((View*)object);
        delete (View*)object;
    }
    float getPageWidth(int position){return 1.f;}//if returned calue <1 OffscreenPageLimit must be larger to workfine 
};

class VerticalViewPager:public ViewPager {
public:
    VerticalViewPager(int w,int h):ViewPager(w,h){
    }
  
    bool onTouchEvent(MotionEvent& ev)override {
       swapTouchEvent(ev);
       return ViewPager::onTouchEvent(ev);
    }
  
    bool onInterceptTouchEvent(MotionEvent ev) {
       swapTouchEvent(ev);
       return ViewPager::onInterceptTouchEvent(ev);
    }
private:  
   MotionEvent swapTouchEvent(MotionEvent&event) {
      float width = getWidth();
      float height = getHeight();
      event.setLocation((event.getY() / height) * width, ((event.getX() / width) * height));
      return event;
   }
};

class VerticalPageTransformer:public ViewPager::PageTransformer {
public:
   void transformPage(View& view, float position) {
       /** * 0 当前界面  * -1 前一页  * 1 后一页  */
       if (position >= -1 && position <= 1) {
           view.setTranslationX(view.getWidth() * -position);
           float yPosition = position * view.getHeight();
           view.setTranslationY(yPosition);
       }
   }
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
    ViewPager*pager=new VerticalViewPager(800,560);
    MyPageAdapter*gpAdapter=new MyPageAdapter();
    pager->setOffscreenPageLimit(8);
    pager->setAdapter(gpAdapter);
    pager->addOnPageChangeListener(*w);
    w->addView(pager).setPos(0,40);
    gpAdapter->notifyDataSetChanged();
    pager->setCurrentItem(0);
    w->requestLayout();
    pager->setPageTransformer(true,new VerticalPageTransformer);

    app.exec();
}
