#include <cdroid.h>
#include <dirent.h>
#include <widget/pagetransformers.h>
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
            if(ent->d_type==DT_REG)  urls.push_back(fullpath);
	    printf("%s\r\n",fullpath.c_str());
        }
        if(dir)closedir(dir);
    }
    int getCount()override{return urls.size();}
    bool isViewFromObject(View* view, void*object)override{ return view==object;}
    void* instantiateItem(ViewGroup* container, int position)override {
        ImageView*iv=new  ImageView(200,200);
        ViewPager::LayoutParams*lp=new ViewPager::LayoutParams();
        //lp->isDecor=true;
        lp->gravity=Gravity::LEFT;
        container->addView(iv,lp).setId(position);
        iv->setScaleType(FIT_XY);
        RefPtr<Cairo::ImageSurface>img;
        if(imgs.find(position)==imgs.end()){
            img=container->getContext()->getImage(urls[position]);
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
    float getPageWidth(int position)override{return 1.f;}

    std::string getPageTitle(int position){
        std::string url=urls[position];
        size_t pos=url.find_last_of('/');
        return url.substr(pos+1);
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,800,600);
    LinearLayout*layout=new LinearLayout(800,600);
    layout->setOrientation(LinearLayout::VERTICAL);
    TabLayout* tab=new TabLayout(1280,36);
    layout->addView(tab);
    printf("params=%d params[0]=%s\r\n",app.getParamCount(),app.getParam(0,"/home/houzh/images").c_str()); 
    MyPageAdapter*gpAdapter=new MyPageAdapter(app.getParam(0,"/home/houzh/images"));
    ViewPager*pager=new ViewPager(800,560);
    pager->setOffscreenPageLimit(gpAdapter->getCount());
    pager->setAdapter(gpAdapter);
    ViewPager::OnPageChangeListener listener;
    listener.onPageSelected=[&](int position){
        LOGD("Page %d Selected",position);
    };
    pager->addOnPageChangeListener(listener);
    pager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    if(argc>1){
        ViewPager::PageTransformer*pt=nullptr;
        switch(atoi(argv[1])){
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
        }  
        if(pt)pager->setPageTransformer(true,pt);
    }
    layout->addView(pager);
    tab->setupWithViewPager(pager);
    w->addView(layout);
    w->requestLayout();
    app.exec();
}
