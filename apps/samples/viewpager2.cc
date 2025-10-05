#include<cdroid.h>
#include<widget/listview.h>
#include<widget/gridview.h>
#include<widget/scrollview.h>
#include<widget/spinner.h>
#include<widget/horizontalscrollview.h>
#include<widget/simplemonthview.h>
#include<widget/viewpager.h>
#include<animation/animations.h>
#include<drawables/drawables.h>
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
#if ENABLE(DAYTIME_WIDGETS)
        if(position!=2){
            SimpleMonthView*sm=new  SimpleMonthView(100,100);
            sm->setMonthParams(23,Calendar::MAY+position,2021,-1,1,31);
            container->addView(sm);
            sm->setId(position);
            return sm;
        }else
#endif
        {
            ListView*lv=new  ListView(100,100);
            MyAdapter*ma=new MyAdapter();
            for(int i=0;i<50;i++)ma->add("");
            container->addView(lv);
            lv->setAdapter(ma);
            lv->setSelector(new ColorDrawable(0x8800FF00));
            ma->notifyDataSetChanged();
            lv->setId(position);
            return lv;
        }
    }
    void destroyItem(ViewGroup* container, int position,void* object){
        container->removeView((View*)object);
        delete (View*)object;
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    MyAdapter* adapter=new MyAdapter();
    MyPageAdapter*gpAdapter=new MyPageAdapter();
    w->setId(0);
    for(int i=0;i<50;i++)adapter->add(""); 
    int optionid=0;
    Rect rect;
    if(argc>1)optionid=std::atoi(argv[1]);
    w->setBackgroundColor(0xFFFF0000);
    AbsListView::OnScrollListener ons={nullptr,nullptr};
    ons.onScroll=[](AbsListView&lv,int firstVisibleItem,int visibleItemCount, int totalItemCount){
        LOGV("firstVisibleItem=%d visibleItemCount=%d totalItemCount=%d",firstVisibleItem,visibleItemCount,totalItemCount);
    };
    ons.onScrollStateChanged=[](AbsListView& view, int scrollState){
        LOGV("scrollState=%d",scrollState);
    };

    ViewPager*pager=new ViewPager(800,400);
    pager->setHorizontalFadingEdgeEnabled(true);
    pager->setFadingEdgeLength(200);

    pager->setOffscreenPageLimit(5);//must >1(This value must >=(the visible view count)+2)
    pager->setAdapter(gpAdapter);
    pager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    gpAdapter->notifyDataSetChanged();
    pager->setCurrentItem(0);//must setcurrentitem,the default item is -1.
    w->addView(pager);
    w->requestLayout();
    app.exec();
}
