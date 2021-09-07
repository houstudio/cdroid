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
            if(ent->d_type==DT_REG)
                urls.push_back(fullpath);
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
    void destroyItem(ViewGroup* container, int position,void* object)override{
        container->removeView((View*)object);
    }
    float getPageWidth(int position)override{return 1.f;}

    std::string getPageTitle(int position){
        std::string url=urls[position];
        size_t pos=url.find_last_of('/');
        return url.substr(pos+1);
    }

    //if returned calue <1 OffscreenPageLimit must be larger to workfine 
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,800,600);
    LinearLayout*layout=new LinearLayout(800,600);
    layout->setOrientation(LinearLayout::VERTICAL);
    TabLayout* tab=new TabLayout(1280,36);
    layout->addView(tab);
    
    MyPageAdapter*gpAdapter=new MyPageAdapter(argc==1?std::string("/home/houzh/images"):argv[1]);
    ViewPager*pager=new ViewPager(800,560);
    pager->setOffscreenPageLimit(3);
    pager->setAdapter(gpAdapter);
    ViewPager::OnPageChangeListener listener;
    listener.onPageSelected=[&](int position){
        //hs->
    };
    pager->addOnPageChangeListener(listener);
    pager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    layout->addView(pager);
    tab->setupWithViewPager(pager);
    w->addView(layout);
    gpAdapter->notifyDataSetChanged();
    w->requestLayout();
    app.exec();
}
