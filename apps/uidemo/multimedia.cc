#include <cdroid.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <string.h>
#include <core/textutils.h>
#include <fileadapter.h>
#include <R.h>
class FileTypeAdapter:public PagerAdapter{
public:
    int getCount()override{return 3;}
    bool isViewFromObject(View* view, void*object)override{ return view==object;}
    void* instantiateItem(ViewGroup* container, int position)override{
        ListView*lv=new ListView(480,400);
        lv->setDivider(new ColorDrawable(0x80224422));
        lv->setDividerHeight(1);
        lv->setSelector(new ColorDrawable(0x8800FF00));
        lv->setVerticalScrollBarEnabled(true);
        lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS); 
        FileAdapter*adapter=new FileAdapter();
        adapter->loadFiles("/"); 
        lv->setAdapter(adapter);
        lv->setBackgroundColor(0xFF000000|(0xFF<<position*8));
        container->addView(lv);
        lv->setOnItemClickListener([](AdapterView&lv,View&v,int pos,long id){
            FileAdapter*adp=(FileAdapter*)lv.getAdapter();
            FileItem&f=adp->getItemAt(pos);
            LOG(DEBUG)<<"clicked "<<pos<<" "<<f.fileName;
            if(f.isDir){
                adp->clear();
                adp->loadFiles(f.fullpath);
                adp->notifyDataSetChanged();
            }
        });

        LOGV("instantiateItem %d to %p",position,lv);
        return lv;
    }
    void destroyItem(ViewGroup* container, int position,void* object)override{
        container->removeView((View*)object);
        LOGV("destroyItem[%d]: %p",position,object);
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
public:
    MediaWindow(int x,int y,int w,int h);
    ~MediaWindow(){
        player=nullptr;
    }
};

MediaWindow::MediaWindow(int x,int y,int w,int h):Window(x,y,w,h){
    ViewGroup*vg=(ViewGroup*)LayoutInflater::from(getContext())->inflate("layout/main.xml",this);
    mAdapter=new FileTypeAdapter();
    mTabLayout=(TabLayout*)vg->findViewById(uidemo::R::id::tablayout);
    mPager = (ViewPager*)vg->findViewById(uidemo::R::id::viewpager);
    
    mTabLayout->setSelectedTabIndicatorColor(0x8000FF00);
    mTabLayout->setSelectedTabIndicatorHeight(4);
    mTabLayout->setTabIndicatorGravity(Gravity::BOTTOM);//TOP/BOTTOM/CENTER_VERTICAL/FILL_VERTICAL
    LOGD("pager=%p tab=%p this=%p:%p",mPager,mTabLayout,this,vg);
    mPager->setAdapter(mAdapter);
    mPager->setPageMargin(20);
    mTabLayout->setupWithViewPager(mPager);
    mTabLayout->requestLayout();
    requestLayout();
}

Window*CreateMultiMedia(){
    MediaWindow*w=new MediaWindow(0,0,800,640);
    w->setText(TEXT("Media"));
    w->show();
    LOGD("CreateMultiMedia");
    return w;
}
