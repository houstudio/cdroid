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
    //if returned calue <1 OffscreenPageLimit must be larger to workfine 
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,800,600);
    HorizontalScrollView* hs=new HorizontalScrollView(800,400);
    LinearLayout*layout=new LinearLayout(400,100);
    ColorStateList*cl=ColorStateList::inflate(nullptr,"/home/houzh/Miniwin/src/gui/res/color/textview.xml");
    layout->setId(10);
    hs->addView(layout);
    hs->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    w->addView(hs).setId(1);

    MyPageAdapter*gpAdapter=new MyPageAdapter(argc==1?std::string("/home/houzh/images"):argv[1]);
    for(int i=0;i<gpAdapter->getCount();i++){
        Button*btn=new Button("Hello Button"+std::to_string(i),150,30);
        btn->setPadding(5,5,5,5);
        btn->setTextColor(cl);
        btn->setTextSize(30);
        btn->setBackgroundColor(0xFF000000|i*20<<8|i*8);
        btn->setId(100+i);
        btn->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
        layout->addView(btn,new LinearLayout::LayoutParams(800,LayoutParams::WRAP_CONTENT));
    }
    ViewPager*pager=new ViewPager(800,560);
    pager->setOffscreenPageLimit(3);
    pager->setAdapter(gpAdapter);
    ViewPager::OnPageChangeListener listener={nullptr,nullptr,nullptr};
    listener.onPageSelected=[&](int position){
        //hs->
    };
    listener.onPageScrolled=[&](int position, float positionOffset, int positionOffsetPixels){
        hs->scrollTo(position*pager->getWidth()+positionOffsetPixels,0);
    };
    pager->setOnPageChangeListener(listener);
    pager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    w->addView(pager).setPos(0,40);
    gpAdapter->notifyDataSetChanged();
    pager->setCurrentItem(0);
    w->requestLayout();
    app.exec();
}
