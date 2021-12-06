#include <windows.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <string.h>
#include <lyricsview.h>
#include <core/textutils.h>

class MediaItem{
public:
    bool isDir;
    std::string name;
    std::string fullpath;
    MediaItem(){
    }
    MediaItem(const MediaItem&o){
        isDir=o.isDir;
        name=o.name;
        fullpath=o.fullpath;
    }
};
class MediaAdapter:public ArrayAdapter<MediaItem>{
public:
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        const MediaItem& mi=getItemAt(position);
        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            tv=new TextView("",600,20);
            tv->setPadding(20,3,0,3);
        }
        tv->setId(position);
        tv->setText(mi.name);
        tv->setTextColor(mi.isDir?0xFFFFFFFF:0xFF88FF00);
        tv->setBackgroundColor(0x80111111);
        tv->setTextSize(28);
        return tv;
    }
    int loadMedias(const std::string&path);
};
std::string SimplifyPath(const std::string & path) {
    char rpath[1024];
    realpath(path.c_str(),rpath);
    return std::string(rpath);
}

static int file_filter(const struct dirent*ent){
    return (ent->d_type==DT_REG)||(ent->d_type==DT_DIR);
}

int MediaAdapter::loadMedias(const std::string&filepath){
    std::string path=SimplifyPath(filepath);
    DIR*dir=opendir(filepath.c_str());
    struct dirent*ent;
    int count=0;
    LOGD("%s scaned=%d",filepath.c_str(),count);
    while((ent=readdir(dir))!=nullptr){
        MediaItem mi;
        mi.name=ent->d_name;
        mi.fullpath=path+"/";
        mi.fullpath+=mi.name;
        LOGV("%s",mi.fullpath.c_str());
        switch(ent->d_type){
        case DT_DIR:
        case DT_REG:
            mi.isDir=(ent->d_type==DT_DIR);
            add(mi);
            count++;
            break;
        default:break;
        }
    }
    if(dir)closedir(dir);
    return count;
}

class MediaWindow:public Window{
protected:
    //ToolBar*mdtype;
    //ToolBar*header;
    ListView*mdlist;
    TextView*mFilePath;
    MediaAdapter*mAdapter;
    AnalogClock*clock;
    LyricsView*lyrics;
    BitmapDrawable*drbmp;
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
        player=nullptr;
    }
    int processMedia(const MediaItem itm);
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
};

MediaWindow::MediaWindow(int x,int y,int w,int h):Window(x,y,w,h){
    player=nullptr;
    filter_type=VIDEO;
    sort_revert=false;
    drbmp=new BitmapDrawable(nullptr,"");
    setBackgroundDrawable(drbmp);
    RelativeLayout*layout=new RelativeLayout(w,h);//LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    layout->setBackgroundColor(0x000000);
    RelativeLayout::LayoutParams*lp=new RelativeLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
    lp->addRule(RelativeLayout::ALIGN_PARENT_BOTTOM);
    mFilePath=new TextView("",600,30);
    mFilePath->setSingleLine(true);
    mFilePath->setEllipsize(Layout::ELLIPSIS_MIDDLE);
    mFilePath->setTextSize(28);
    layout->addView(mFilePath,lp).setId(100);
    //mdtype=new ToolBar(1280,30);


    //header=new ToolBar(1280,30);
    
    mdlist=new ListView(600,520);
    lp=new RelativeLayout::LayoutParams(600,LayoutParams::MATCH_PARENT);
    lp->addRule(RelativeLayout::ABOVE,100);
    mdlist->setSelector(new ColorDrawable(0x8000ff00));
    mdlist->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    mdlist->setVerticalScrollBarEnabled(true);
    mdlist->setDrawSelectorOnTop(true);
    mdlist->setDivider(new ColorDrawable(0x40FFFFFF));
    mdlist->setDividerHeight(1);
    layout->addView(mdlist,lp).setId(1);


    mAdapter=new MediaAdapter();
    mAdapter->loadMedias("/");
    mdlist->setAdapter(mAdapter);
    mdlist->setBackgroundColor(0x000000);
    mAdapter->notifyDataSetChanged();
    mdlist->setOnItemClickListener([&](AdapterView&lv,View&v,int pos,long id){
        const MediaItem mdi=mAdapter->getItemAt(pos);
        const std::string fname=SimplifyPath(mdi.fullpath);
        processMedia(mdi);
        mFilePath->setText(fname);
        auto image=getContext()->getImage(fname,false);
        LOGD("getImage(%s) %p",fname.c_str(),image.get());
        if(image)drbmp->setBitmap(image);
    });

    lyrics=new LyricsView("",440,mdlist->getHeight()+30);
    lp=new RelativeLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    lp->addRule(RelativeLayout::RIGHT_OF,1);
    lp->addRule(RelativeLayout::ABOVE,100);
    lyrics->setVisibility(VISIBLE);
    layout->addView(lyrics,lp).setBackgroundColor(0x80000000);
    addView(layout);
    requestLayout();
}

int MediaWindow::processMedia(const MediaItem mdi){
    if(mdi.isDir){
        LOGD("%s [%s]",mdi.fullpath.c_str(),mdi.name.c_str());
        mAdapter->clear();
        mAdapter->loadMedias(mdi.fullpath);
        mAdapter->notifyDataSetChanged();
    }else{
        if(TextUtils::endWith(mdi.fullpath,"mp3")){
             lyrics->setText(mdi.fullpath);
             lyrics->invalidate(); 
        }
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
