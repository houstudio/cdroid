#include <app.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cdgraph.h>
#include <windowmanager.h>
#include <assets.h>
#include <cairomm/surface.h>
#include <inputmethodmanager.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <thread>
#include <cdinput.h>
#include <inputeventsource.h>
#include <uieventsource.h>
#include <mutex>


void spt_init(int argc, char *argv[]);
void setproctitle(const char *fmt, ...);
namespace cdroid{

App*App::mInst=nullptr;

static struct option app_options[]={
   {"alpha"   ,required_argument,0,0},
   {"data"    ,required_argument,0,0},
   {"config"  ,required_argument,0,0},
   {"language",required_argument,0,0},
   {"record"  ,required_argument,0,0},
   {"monkey"  ,required_argument,0,0},
   {"debug"   ,no_argument      ,0,0},
   {0,0,0,0}
};

App::App(int argc,const char*argv[],const struct option*extoptions){
    int option_index=-1,c=-1;
    std::string optstring;
    std::vector<struct option>all;
    LogParseModules(argc,argv);    
    mInst=this;

    LOGD("App %s started",(argc&&argv)?argv[0]:"");
    GFXInit();
    all.insert(all.begin(),app_options,app_options+(sizeof(app_options)/sizeof(option)-1));
    for(;extoptions&&extoptions->name;)all.push_back(*extoptions++);
    for(auto o:all){//::optional :must has arg 
        LOGV("%s hasarg=%d %c/%d",o.name,o.has_arg,o.val,o.val);
        if(o.val==0)continue;
        optstring.append(1,o.val);
        if(o.has_arg)optstring.append(o.has_arg,':');
    }

    do{
        c=getopt_long_only(argc,(char*const*)argv,optstring.c_str(),all.data(),&option_index);
        LOGV_IF(c>=0,"option_index=%d  c=%c/%d",option_index,c,c);
        if(c>0){
            for(int i=0;i<all.size();i++)if(all[i].val==c){option_index=i;break;}
            std::string key=all[option_index].name;
            args[key]=optarg?:"";
            LOGD("args[%d]%s:%s",option_index,key.c_str(),optarg);
        }else if(c==0){
            std::string key=all[option_index].name;
            args[key]=optarg?:"";
            LOGD("args[%d]%s:%s",option_index,key.c_str(),optarg);
        }
    }while(c>=0);

    if(argc&&argv){
        spt_init(argc,(char**)argv);
        setName(argv[0]);
    }
    if(hasArg("debug")){
        ViewGroup::DEBUG_DRAW=true;
        View::DEBUG_DRAW=true;
    }
    setOpacity(getArgAsInt("alpha",255));
    InputEventSource*inputsource=new InputEventSource(getArg("record",""));
    addEventHandler(inputsource);
    inputsource->playback(getArg("monkey",""));
}

App::~App(){
    InputMethodManager::getInstance().shutDown();
    WindowManager::getInstance().shutDown();
    delete Looper::getDefault();
    delete &GraphDevice::getInstance();
    LOGD("%p Destroied",this);
}

const std::string App::getDataPath()const{
    std::string path=getArg("data","./");
    if(path.back()!='/')path+='/';
    return path;
}

App&App::getInstance(){
    if(mInst==nullptr)
        mInst=new App;
    return *mInst;
}

const std::string&App::getArg(const std::string&key,const std::string&def)const{
    auto itr=args.find(key);
    if(itr==args.end()||itr->second.empty())
        return def;
    return itr->second;
}

bool App::hasArg(const std::string&key)const{
    auto itr=args.find(key);
    return itr!=args.end();
}

void App::setArg(const std::string&key,const std::string&value){
    args[key]=value;
}

int App::getArgAsInt(const std::string&key,int def)const{
    auto itr=args.find(key);
    if(itr==args.end()||itr->second.empty())
        return def;
    const char*arg=itr->second.c_str();
    return strtoul(arg,NULL,(strpbrk(arg,"xX")?16:10));
}

void App::setOpacity(unsigned char alpha){
    GFXSurfaceSetOpacity(GraphDevice::getInstance().getPrimarySurface(),alpha);
    LOGD("alpha=%d",alpha);
}

const std::string App::getAssetsPath(){
    std::string path=mName;
    size_t pos = path.find_last_of("\\/");
    if(pos!=std::string::npos)
        path=path.substr(pos+1);
    path=getDataPath()+path+".pak";
    return path;
}

void App::addEventHandler(const EventHandler*handler){
    Looper::getDefault()->addEventHandler(handler);
}

void App::removeEventHandler(const EventHandler*handler){
    //Looper::getDefault()->remove_event_source(source);
}

int App::exec(){
    while(1)Looper::getDefault()->pollAll(5);
}

void App::exit(int code){
    //Looper::getDefault()->quit(code);
}

void App::setName(const std::string&appname){
    mName = appname;
    addResource(getAssetsPath());
}

const std::string& App::getName(){
    return mName;
}

}

