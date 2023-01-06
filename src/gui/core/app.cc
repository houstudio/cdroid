#include <app.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
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
#include <mutex>
#include <cla.h>
void spt_init(int argc, char *argv[]);
void setproctitle(const char *fmt, ...);
namespace cdroid{

App*App::mInst=nullptr;

static CLA::Argument ARGS[]={
   {CLA::EntryType::Option, "a", "alpha",  "UI layer global alpha[0,255]", CLA::ValueType::Int, (int)CLA::EntryFlags::Manditory},
   {CLA::EntryType::Option, "", "data",   "app data path",        CLA::ValueType::String, (int)CLA::EntryFlags::Optional },
   {CLA::EntryType::Option, "m", "monkey", "events playback path",  CLA::ValueType::String, (int)CLA::EntryFlags::Optional },
   {CLA::EntryType::Option, "r", "record", "events record path", CLA::ValueType::String,   (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Option, "R", "rotate", "display rotate ", CLA::ValueType::Int,   (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Switch, "h", "help", "display help info ", CLA::ValueType::None,   (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Switch, "d", "debug", "open debug", CLA::ValueType::None,   (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Switch, "", "fps"  , "Show FPS ",CLA::ValueType::None,   (int)CLA::EntryFlags::Optional}
};

App::App(int argc,const char*argv[],const struct option*extoptions){
    int option_index=-1,c=-1;
    std::string optstring;
    LogParseModules(argc,argv);
    mQuitFlag = false;
    mExitCode = 0;
    mInst = this;

    LOGD("App %s started",(argc&&argv)?argv[0]:"");
    cla.addArguments(ARGS,sizeof(ARGS)/sizeof(CLA::Argument));
    cla.setSwitchChars("-");
    cla.parse(argc,argv);
    if(hasSwitch("debug")){
        ViewGroup::DEBUG_DRAW=true;
        View::DEBUG_DRAW=true;
    }
    if(hasSwitch("help")){
	std::cout<<cla.getUsageString()<<std::endl;
	std::cout<<"params.count="<<getParamCount()<<std::endl;
	exit(0);
    }
    GFXInit();
    GFXSetRotation(0,(GFX_ROTATION)((getArgAsInt("rotate",0)/90)%4));
    setOpacity(getArgAsInt("alpha",255));
    GraphDevice::getInstance().showFPS(hasSwitch("fps"));
    InputEventSource*inputsource=new InputEventSource(getArg("record",""));
    addEventHandler(inputsource);
    inputsource->playback(getArg("monkey",""));

    signal(SIGINT,[](int sig){
        LOGD("SIG %d...",sig);
        App::mInst->mQuitFlag = true;
        signal(sig,SIG_DFL);
    });
}

App::~App(){
    WindowManager::getInstance().shutDown();
    InputMethodManager::getInstance().shutDown();
    delete Looper::getDefault();
    delete &GraphDevice::getInstance();
}

const std::string App::getDataPath()const{
    std::string path=getArg("data","./");
    if(path.back()!='/')path+='/';
    return path;
}

App& App::getInstance(){
    if(mInst==nullptr)
        mInst=new App;
    return *mInst;
}

const std::string App::getArg(const std::string&key,const std::string&def)const{
    std::string value=def;
    cla.find(key,value);
    return value;
}

bool App::hasArg(const std::string&key)const{
    return cla.find(key);
}

bool App::hasSwitch(const std::string&key)const{
    return cla.findSwitch(key);
}

void App::setArg(const std::string&key,const std::string&value){
    cla.setArgument(key,value);
}

int App::getArgAsInt(const std::string&key,int def)const{
    int value=def;
    cla.find(key,value);
    return value;
}

float App::getArgAsFloat(const std::string&key,float def)const{
    float value=def;
    cla.find(key,value);
    return value;
}

double App::getArgAsDouble(const std::string&key,double def)const{
    double value=def;
    cla.find(key,value);
    return value;
}

int App::getParamCount()const{
    return cla.getParamCount();
}

std::string App::getParam(int idx,const std::string&def)const{
    std::string value=def;
    cla.getParam(idx,value);
    return value;
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
    Looper::getDefault()->removeEventHandler(handler);
}

int App::exec(){
    while(!mQuitFlag)Looper::getDefault()->pollAll(1);
    return mExitCode;
}

void App::exit(int code){
    mQuitFlag = true;
    mExitCode = code;
}

void App::setName(const std::string&appname){
    mName = appname;
    addResource(getAssetsPath());
}

const std::string& App::getName(){
    return mName;
}

}

