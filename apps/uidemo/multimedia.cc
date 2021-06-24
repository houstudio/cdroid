#include <windows.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <assets.h>
#include <fstream>
#include <string.h>
#include <lyricsview.h>
#include <ntvutils.h>

class MediaItem{
public:
    std::string path;
};
class MediaAdapter:public ArrayAdapter<MediaItem>{
public:
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        const MediaItem* mi=(MediaItem*)getItem(position);
        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            tv=new TextView("",600,20);
            tv->setPadding(20,3,0,3);
        }
        tv->setId(position);
        tv->setText(mi->path);
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0x80002222);
        tv->setTextSize(24);
        return tv;
    }
    int loadMedias(const std::string&path);
};
int MediaAdapter::loadMedias(const std::string&filepath){
    int count=0;
    std::string path=SimplifyPath(filepath);
    DIR *dir=opendir(path.c_str());
    struct dirent **namelist;
    int n=scandir(filepath.c_str(),&namelist,NULL/*filter*/,alphasort);
    for(int i=0;i<n;i++){
        MediaItem mi;
        LOGD("%d: %s",i,namelist[i]->d_name);
        mi.path=namelist[i]->d_name;
        add(mi);
    }
    LOGD("%s loaded %d",filepath.c_str(),n);
    free(namelist);
    return count;
}

class MediaWindow:public Window{
protected:
    //ToolBar*mdtype;
    //ToolBar*header;
    ListView*mdlist;
    MediaAdapter*mAdapter;
    AnalogClock*clock;
    LyricsView*lyrics;
    RefPtr<ImageSurface>image;
    std::string media_path;
    bool sort_revert;
    HANDLE player;
    int filter_type;
    std::vector<std::string> split(const std::string& s,const std::string& delim);
public:
    enum{
        VIDEO=0,
        MUSIC=1,
        PICTURE=2,
        FOLDERS=3
    }Filter;
public:
    MediaWindow(int x,int y,int w,int h);
    ~MediaWindow(){
        //if(player)nglMPClose(player);
        player=nullptr;
    }
    int loadMedia(const std::string&path,int filter);
    int processMedia(MediaItem&itm);
    virtual bool onKeyDown(int,KeyEvent&k)override;
    std::string filename2URL(const std::string&name){
        std::string url;
        url.append("file://");
        for(int i=0;i<name.length();i++){
            BYTE cc=name.at(i);
            if((cc&0x80)||(cc==' ')){
               char tmp[8];
               sprintf(tmp,"%%%02x",cc);
               url.append(tmp);
            }else url.append(1,(char)cc);
        }
        return url;
    }
    void onDraw(Canvas&canvas)override{
        RECT rect=getClientRect();
        if(image==nullptr)
            Window::onDraw(canvas);
        else canvas.draw_image(image,rect,nullptr); 
    }
};

MediaWindow::MediaWindow(int x,int y,int w,int h):Window(x,y,w,h){
    player=nullptr;
    filter_type=VIDEO;
    sort_revert=false;
    setBackgroundColor(0x000000);
    //mdtype=new ToolBar(1280,30);


    //header=new ToolBar(1280,30);
       
    mdlist=new ListView(600,520);
    mdlist->setPos(40,130);
    mdlist->setSelector(new ColorDrawable(0xFF800000));
    mdlist->setVerticalScrollBarEnabled(true);
    mdlist->setDrawSelectorOnTop(false);
    addView(mdlist);
    mAdapter=new MediaAdapter();
    mAdapter->loadMedias("/");
    mdlist->setAdapter(mAdapter);
    mAdapter->notifyDataSetChanged();

    clock=new AnalogClock(227,227); 
    addView(clock).setPos(1000,100).setBackgroundColor(0x80123456);
    lyrics=new LyricsView("",440,mdlist->getHeight()+30);
    lyrics->setVisibility(INVISIBLE);
    addView(lyrics).setPos(840,330).setBackgroundColor(0x80000000);

}
int MediaWindow::loadMedia(const std::string&path,int filter){
    return 0;
}
int MediaWindow::processMedia(MediaItem&item){
   switch(filter_type){
   case VIDEO: 
   case MUSIC: 
        if(player){
             //nglMPStop(player);
             //nglMPClose(player);
        }
        player=nullptr;
        if(filter_type==MUSIC){
           //lyrics->setText(url);
        }
        break;
   case PICTURE:{
        }break;
   case FOLDERS:
        break;
   }
   return 0;
}

bool MediaWindow::onKeyDown(int keyCode,KeyEvent&k){
    switch(keyCode){
    case KEY_LEFT:
    case KEY_RIGHT:
         //lyrics->setVisibility(filter_type==MUSIC?VISIBLE:INVISIBLE);
         break;
    case KEY_RED:
         break;
    case KEY_VOLUMEDOWN:
    case KEY_VOLUMEUP:
         LOGD("KEY_VOLUME");
         break;
    default: return Window::onKeyDown(keyCode,k);
    }
    return false;
}

std::vector<std::string> MediaWindow::split(const std::string& s,const std::string& delim){
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len){
        int find_pos = s.find(delim, pos);
        if (find_pos < 0){
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}


Window*CreateMultiMedia(){
    MediaWindow*w=new MediaWindow(0,0,1280,720);
    w->setText(TEXT("Media"));
    w->show();
    LOGD("CreateMultiMedia");
    return w;
}
