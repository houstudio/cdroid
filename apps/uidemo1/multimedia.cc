#include <cdroid.h>
#include <porting/cdlog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <string.h>
#include <core/textutils.h>
#include <fileadapter.h>
#include <widget/toolbar.h>
#include <R.h>
class FileTypeAdapter:public PagerAdapter{
private:
    FileAdapter*adapter1 = nullptr;
    FileAdapter*adapter2 = nullptr;
public:
    ~FileTypeAdapter(){
        delete adapter1;
        delete adapter2;
    }
    int getCount()override{return 5;}
    bool isViewFromObject(View* view, void*object)override{ return view==object;}
    void* instantiateItem(ViewGroup* container, int position)override{
        std::string res[]={"@layout/layout1.xml","@layout/layout2.xml","@layout/layout3.xml"};
        switch(position){
        case 2:
        case 3:
        case 4:
            {View*v=LayoutInflater::from(container->getContext())->inflate(res[position-2],nullptr,false);
            container->addView(v);
            v->requestLayout();
            View*btn=v->findViewById(uidemo1::R::id::button1);
            if(btn)btn->setOnClickListener([](View&v){
                 Window*w=new Window(0,0,400,400);
                 w->setBackgroundColor(0x80FF0000);
            });
            Toolbar*tb=(Toolbar*)v->findViewById(uidemo1::R::id::toolbar);
            if(tb){
                tb->setNavigationOnClickListener([](View&){
                    LOGD("Navigation Clicked");
                });
            }
            return v;
            }break;
        case 0:
            {
            ListView*lv=new ListView(800,480);
            lv->setDivider(new ColorDrawable(0x80224422));
            lv->setDividerHeight(1);
            //lv->setFastScrollEnabled(true);
            lv->setSelector(new ColorDrawable(0x8800FF00));
            lv->setVerticalScrollBarEnabled(true);
            lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS); 
            if(adapter1==nullptr){
                adapter1=new FileAdapter("@layout/fileitem.xml");
                adapter1->loadFiles("/");
            } 
            lv->setAdapter(adapter1);
            lv->setBackgroundColor(0xFF000000|(0xFF<<position*8));
            container->addView(lv);
            lv->setId(12345);
            lv->setOnItemClickListener([](AdapterView&lv,View&v,int pos,long id){
                FileAdapter*adp=(FileAdapter*)lv.getAdapter();
                FileItem f=adp->getItemAt(pos);
                //LOG(DEBUG)<<"clicked "<<pos<<" "<<f.fileName;
                if(f.isDir){
                    adp->clear();
                    adp->loadFiles(f.fullpath);
                    adp->notifyDataSetChanged();
                    lv.setSelection(0);
                }
            });
            //LOGV("instantiateItem %d to %p",position,lv);
            return lv;
            }   
        case 1:{//LOGD("===========1111");
            GridView*gv=new GridView(800,480);
            if(adapter2==nullptr)
                adapter2=new FileAdapter("@layout/fileitem2.xml");
            gv->setOnItemClickListener([](AdapterView&lv,View&v,int pos,long id){
                FileAdapter*adp=(FileAdapter*)lv.getAdapter();
                FileItem f=adp->getItemAt(pos);
                //LOG(DEBUG)<<"clicked "<<pos<<" "<<f.fileName;
                if(f.isDir){
                    adp->clear();
                    adp->loadFiles(f.fullpath);
                    adp->notifyDataSetChanged();
                    lv.setSelection(0);
                }
            });
            gv->setVerticalScrollBarEnabled(true);
            gv->setScrollbarFadingEnabled(false);
            gv->setNumColumns(2);
            gv->setAdapter(adapter2);
            gv->setHorizontalSpacing(2); 
            gv->setVerticalSpacing(2);
            container->addView(gv);
            gv->setId(12345);
            adapter2->loadFiles("/");
            adapter2->notifyDataSetChanged();
            return gv;
            }
        }
    }
    void destroyItem(ViewGroup* container, int position,void* object)override{
        container->removeView((View*)object);
        //LOGV("destroyItem[%d]: %p",position,object);
        delete (View*)object;
    }
    std::string getPageTitle(int position)override{
        return std::string("Tab")+std::to_string(position);
    }
    float getPageWidth(int position)override{return 1.f;}//if returned calue <1 OffscreenPageLimit must be larger to workfine
};


class MediaWindow:public Window{
protected:
    ListView*mdlist;
    TabLayout*mTabLayout;
    ViewPager*mPager;
    TextView*mFilePath;
    FileTypeAdapter*mAdapter;
    std::string media_path;
    HANDLE player;
    Runnable run;
public:
    MediaWindow(int x,int y,int w,int h);
    ~MediaWindow(){
        delete mAdapter;
        player=nullptr;
    }
    
};

MediaWindow::MediaWindow(int x,int y,int w,int h):Window(x,y,w,h){
    ViewGroup*vg=(ViewGroup*)LayoutInflater::from(getContext())->inflate("layout/main.xml",this);
    mAdapter=new FileTypeAdapter();
    mTabLayout=(TabLayout*)vg->findViewById(uidemo1::R::id::tablayout);
    mPager = (ViewPager*)vg->findViewById(uidemo1::R::id::viewpager);
    mTabLayout->setSelectedTabIndicatorColor(0x8000FF00);
    mTabLayout->setSelectedTabIndicatorHeight(5);
    mTabLayout->setTabTextColors(0xFFFF0000,0xFF00FF00);
    mTabLayout->setTabIndicatorGravity(Gravity::BOTTOM);//TOP/BOTTOM/CENTER_VERTICAL/FILL_VERTICAL
    //LOGD("pager=%p tab=%p this=%p:%p",mPager,mTabLayout,this,vg);
    mPager->setAdapter(mAdapter);
    mPager->setOffscreenPageLimit(1);//mAdapter->getCount());
    mPager->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    mTabLayout->setupWithViewPager(mPager);
    mTabLayout->requestLayout();
    run = [this](){
        static int count=0;
        TextView*tv=(TextView*)findViewById(uidemo1::R::id::light);
        if(tv)tv->setText(std::to_string(count++));
        postDelayed(run,1000);
    };
    //postDelayed(run,1000);
}

Window*CreateMultiMedia(){
    MediaWindow*w=new MediaWindow(0,0,-1,-1);
    w->setText("Media");
    //LOGD("CreateMultiMedia");
    return w;
}
