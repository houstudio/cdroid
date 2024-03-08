#include <cdroid.h>
#include <dirent.h>
#include <widget/pagetransformers.h>
#include <cdlog.h>
using namespace Cairo;
class MyPageAdapter:public PagerAdapter{
    std::vector<std::string>urls;
    std::map< int,RefPtr<Cairo::ImageSurface> >imgs;
public:
    MyPageAdapter(const std::string&path){
        DIR*dir=opendir(path.c_str());
        struct dirent*ent;
        while(dir&&(ent=readdir(dir))){
            std::string fullpath=path+"/"+ent->d_name;
            if(ent->d_type==DT_REG/*||ent->d_type!=DT_DIR*/)  urls.push_back(fullpath);
            LOGV("%s",fullpath.c_str());
        }
        if(dir)closedir(dir);
    }
    int getCount()override{return urls.size();}
    bool isViewFromObject(View* view, void*object)override{ return view==object;}
    void* instantiateItem(ViewGroup* container, int position)override {
        ImageView*iv=new  ImageView(200,200);
        ViewPager::LayoutParams*lp=new ViewPager::LayoutParams();
        lp->gravity=Gravity::CENTER;
        container->addView(iv).setId(position);
        //iv->setScaleType(FIT_XY);
        RefPtr<Cairo::ImageSurface>img;
        if(imgs.find(position)==imgs.end()){
            img=container->getContext()->loadImage(urls[position]);
            imgs[position]=img;
        }else img=imgs[position];
        iv->setImageBitmap(img);
        return iv;
    }
    void setPrimaryItem(ViewGroup* container, int position, void* object)override{
    }
    void destroyItem(ViewGroup* container, int position,void* object)override{
        container->removeView((View*)object);
        delete (View*)object;
    }
    float getPageWidth(int position)override{return 0.2f;}

    std::string getPageTitle(int position){
        std::string url=urls[position];
        size_t pos=url.find_last_of('/');
        return url.substr(pos+1);
    }
};

class ScalePageTransformer:public ViewPager::PageTransformer {
public:
    static constexpr float MAX_SCALE = 1.2f;
    static constexpr float MIN_SCALE = 0.6f;
    cdroid::ViewPager*mVP;
    ScalePageTransformer(cdroid::ViewPager*vp){mVP=vp;}
    void transformPage(View& page, float position) {

        if (position < -1) {
            position = -1;
        } else if (position > 1) {
            position = 1;
        }

        float tempScale = position < 0 ? 1 + position : 1 - position;

        float slope = (MAX_SCALE - MIN_SCALE) / 1;
        //一个公式
	int dist=page.getId()>mVP->getCurrentItem();
        float scaleValue = 1.3f - 0.3f*position;
        page.setScaleX(scaleValue);
        page.setScaleY(scaleValue);
	page.setTranslationY(40*std::abs(dist));
	page.setTranslationX(dist>0?(-10*dist):(10*dist));
	ViewPager::LayoutParams*lp=(ViewPager::LayoutParams*)page.getLayoutParams();
	lp->childIndex=(position<1.f)?((1.f-position)*mVP->getChildCount()-1):page.getId();

        //mVP->requestLayout();
	LOGD("page:%p:%d position=%f",&page,page.getId(),position);
    }
};

class CascadeZoomPageTransformer:public ViewPager::PageTransformer {
public:
   void transformPage(View& page, float position) {
        if (position < -1) { /* [-Infinity,-1)*/
        /*页面已经在屏幕左侧且不可视*/
        } else if (position <= 0) { /* [-1,0]*/
            /*页面从左侧进入或者向左侧滑出的状态*/
            page.setAlpha(1 + position);
        } else if (position <= 1) {/* (0,1]*/
            /*页面从右侧进入或者向右侧滑出的状态*/
            page.setTranslationX(page.getWidth() * -position);
            page.setScaleX(1-position*0.5f);
            page.setScaleY(1-position*0.5f);
            page.setAlpha(1 - position);
        }else if (position >1){
        /*页面已经在屏幕右侧且不可视*/
        }
    }
};
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    LinearLayout*layout=new LinearLayout(800,600);
    layout->setOrientation(LinearLayout::VERTICAL);
    TabLayout* tab=new TabLayout(1280,36);
    layout->addView(tab);
    printf("params=%d params[0]=%s\r\n",app.getParamCount(),app.getParam(0,"/home/houzh/images").c_str()); 
    MyPageAdapter*gpAdapter=new MyPageAdapter(app.getParam(0,"/home/houzh/images"));
    ViewPager*pager=new ViewPager(800,560);
    pager->setOffscreenPageLimit(15);//gpAdapter->getCount());
    pager->setAdapter(gpAdapter);
    ViewPager::OnPageChangeListener listener;
    pager->setPageMargin(5);
    listener.onPageSelected=[&](int position){
        LOGD("Page %d Selected",position);
    };
    pager->addOnPageChangeListener(listener);
    pager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    if(app.getParamCount()>1){
        ViewPager::PageTransformer*pt=nullptr;
        LOGD("choice=%d",atoi(app.getParam(1,"0").c_str()));
        switch(atoi(app.getParam(1,"0").c_str())){
        case  0:pt=new ScaleInOutTransformer(); break;
        case  1:pt=new AccordionTransformer();  break;
        case  2:pt=new CubeInTransformer();break;
        case  3:pt=new CubeOutTransformer();break;
        case  4:pt=new FlipHorizontalTransformer();break;
        case  5:pt=new FlipVerticalTransformer();break;
        case  6:pt=new ParallaxTransformer();break;
        case  7:pt=new RotateDownTransformer();break;
        case  8:pt=new RotateUpTransformer();break;
        case  9:pt=new ScaleInOutTransformer();break;
        case 10:pt=new StackTransformer();break;
        case 11:pt=new ZoomInTransformer();break;
        case 12:pt=new ZoomOutTransformer();break;
        case 13:pt=new ZoomOutSlideTransformer();break;
	case 14:pt=new ScalePageTransformer(pager);break;
	case 15:pt=new CascadeZoomPageTransformer();break;
        } 
        if(pt)pager->setPageTransformer(true,pt);
    }
    layout->addView(pager);
    tab->setupWithViewPager(pager);
    w->addView(layout);
    w->requestLayout();
    app.exec();
}
