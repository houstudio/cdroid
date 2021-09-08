#include <windows.h>
#include <dirent.h>

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
        }
        if(dir)closedir(dir);
    }
    int getCount()override{return urls.size();}
    bool isViewFromObject(View* view, void*object)override{ return view==object;}
    void* instantiateItem(ViewGroup* container, int position)override {
        ImageView*iv=new  ImageView(100,100);
        container->addView(iv,new ViewPager::LayoutParams());
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
     
    MyPageAdapter*gpAdapter=new MyPageAdapter(app.hasArg("path")?app.getArg("path"):std::string("/home/houzh/images"));
    ViewPager*pager=new ViewPager(800,560);
    pager->setOffscreenPageLimit(gpAdapter->getCount());
    pager->setAdapter(gpAdapter);
    ViewPager::OnPageChangeListener listener;
    listener.onPageSelected=[&](int position){
        LOGD("Page %d Selected",position);
    };
    pager->addOnPageChangeListener(listener);
    pager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    constexpr float CENTER_PAGE_SCALE=.8f; 
    constexpr int pagerWidth=800;
    const float MIN_SCALE = 0.5f;
    const float MIN_ALPHA = 0.5f;
    if(argc>1)
    pager->setPageTransformer(true,[&](View&view,float position){
         int pageWidth = view.getWidth();
	 int pageHeight = view.getHeight();

         if (position < -1){ // [-Infinity,-1)
   	     // This page is way off-screen to the left.
	     view.setAlpha(0);
         } else if (position <= 1){ //a页滑动至b页 ； a页从 0.0 -1 ；b页从1 ~ 0.0
	     // [-1,1]
	     // Modify the default slide transition to shrink the page as well
	     float scaleFactor = std::max(MIN_SCALE, 1 - std::abs(position));
	     float vertMargin = pageHeight * (1 - scaleFactor) / 2;
	     float horzMargin = pageWidth * (1 - scaleFactor) / 2;
	    if (position < 0){
		view.setTranslationX(horzMargin - vertMargin / 2);
	    } else{
		view.setTranslationX(-horzMargin + vertMargin / 2);
	    }
 
	    // Scale the page down (between MIN_SCALE and 1)
	    view.setScaleX(scaleFactor);
	    view.setScaleY(scaleFactor);
 
	    // Fade the page relative to its size.
	    view.setAlpha(MIN_ALPHA + (scaleFactor - MIN_SCALE)
		/ (1 - MIN_SCALE) * (1 - MIN_ALPHA));
 
	} else{ // (1,+Infinity]
	    // This page is way off-screen to the right.
	    view.setAlpha(0);
	}
    });
    layout->addView(pager);
    tab->setupWithViewPager(pager);
    w->addView(layout);
    w->requestLayout();
    app.exec();
}
