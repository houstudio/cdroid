/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <assets.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <thread>
#include <mutex>

#include <porting/cdlog.h>
#include <porting/cdgraph.h>
#include <core/app.h>
#include <core/cxxopts.h>
#include <core/build.h>
#include <core/atexit.h>
#include <core/inputeventsource.h>
#include <core/windowmanager.h>
#include <core/inputmethodmanager.h>

#if defined(__linux__)||defined(__unix__)
extern "C" char *__progname;
#elif defined(_WIN32)||defined(_WIN64)
extern "C" unsigned long  GetModuleFileNameA(void* hModule, char* lpFilename, unsigned long nSize);
#endif

namespace cdroid{

App*App::mInst = nullptr;

App::App(int argc,const char*argv[]){
    int alpha=255,rotation=0,density=0,frameDelay=0;
    bool debug=false,showFPS=false,help=false;
    std::string logo,monkey,record,datapath;
    LogParseModules(argc,argv);
    mQuitFlag = false;
    mExitCode = 0;
    mInst = this;
    cxxopts::Options options("cdroid","cdroid application");
    options.add_options()
        ("d,debug","enable debuig mode",cxxopts::value<bool>(debug))
        ("h,help","print helps",cxxopts::value<bool>(help))
        ("fps", "show fps info",cxxopts::value<bool>(showFPS))
        ("a,alpha","UI layer global alpha[0,255]",cxxopts::value<int>(alpha)->default_value("255"))
        ("f,framedelay","animation frame delay",cxxopts::value<int>(frameDelay))
        ("density","UI Density",cxxopts::value<int>(density))
        ("R,rotate","display rotate(90*n)",cxxopts::value<int>(rotation)->default_value("0"))
        ("l,logo","show logo",cxxopts::value<std::string>(logo))
        ("m,monkey","events playback path",cxxopts::value<std::string>(monkey))
        ("r,record","events record path",cxxopts::value<std::string>(record))
        ("data","data directory",cxxopts::value<std::string>(datapath));

    Looper::prepareMainLooper();
    options.allow_unrecognised_options();
    cxxopts::ParseResult result;
    std::string name;
#if defined(__linux__)||defined(__unix__)
    name= std::string(argc?argv[0]:__progname);
#elif (defined(_WIN32)||defined(_WIN64))
    char progName[260];
    GetModuleFileNameA(nullptr,progName,sizeof(progName));
    name = progName;
#endif
    try{
        if(argv==nullptr){
            const char*dummy[]={name.c_str(),nullptr};
            result = options.parse(1,dummy);
        }else{
            result = options.parse(argc,argv);
        }
        mArgsResult = std::make_unique<cxxopts::ParseResult>(result);
    }catch(std::exception&e){
        LOGE("%s",e.what());
    }
    if(help){
        std::cout<<options.help()<<std::endl;
        exit(EXIT_SUCCESS);
        LogSetModuleLevel(nullptr,LOG_FATAL);
        mQuitFlag = true;
        return;
    }
    Typeface::setContext(this);
    onInit();
    setName(name);
    LOGI("\033[1;35m          ┏━┓┏┓╋╋╋┏┓┏┓");
    LOGI("\033[1;35m          ┃┏╋┛┣┳┳━╋╋┛┃");
    LOGI("\033[1;35m          ┃┗┫╋┃┏┫╋┃┃╋┃");
    LOGI("\033[1;35m          ┗━┻━┻┛┗━┻┻━┛");

    LOGI("cdroid %s on %s [%s] Build:%d Commit:%s",Build::VERSION::Release,Build::VERSION::BASE_OS,
            Build::VERSION::CODENAME,Build::VERSION::BuildNumber,Build::VERSION::CommitID);
    LOGI("https://www.gitee.com/houstudio/cdroid");

    GraphDevice& graph =GraphDevice::getInstance();
    if(rotation){
        rotation = (rotation/90)%4;
        WindowManager::getInstance().setDisplayRotation(0,rotation);
        graph.setRotation(rotation);
    }
    if(!logo.empty()) graph.setLogo(logo);
    graph.showFPS(showFPS).init();
    View::VIEW_DEBUG = result.count("debug");
    DisplayMetrics::DENSITY_DEVICE = DisplayMetrics::getDeviceDensity();
    if(alpha!=255) setOpacity(alpha);
    if(density) DisplayMetrics::DENSITY_DEVICE = density;
    if(frameDelay) Choreographer::setFrameDelay(frameDelay);
    Typeface::loadPreinstalledSystemFontMap();
    Typeface::loadFaceFromResource(this);

    AtExit::registerCallback([this](){
        LOGD("Exit...");
        mQuitFlag = true;
    });

    InputEventSource*inputsource=&InputEventSource::getInstance();//(getArg("record",""));
    addEventHandler(inputsource);
    if(!monkey.empty()){
        inputsource->playback(monkey);
    }
}

App::~App(){
    WindowManager::getInstance().shutDown();
    InputMethodManager::getInstance().shutDown();
    delete Looper::getMainLooper();
    delete &GraphDevice::getInstance();
}

void App::onInit(){
    LOGD("onInit");
    GFXInit();
    mDisplayMetrics.setToDefaults();
    addResource(getDataPath()+std::string("cdroid.pak"),"cdroid");
}

const std::string App::getDataPath()const{
    std::string path=getArg("data","./");
    if(path.back()!='/')path+='/';
    return path;
}

App& App::getInstance(){
    if(mInst==nullptr)
        mInst = new App;
    return *mInst;
}

const std::string App::getArg(const std::string&key,const std::string&def)const{
    std::string value = def;
    if(mArgsResult->count(key)){
        value = (*mArgsResult)[key].as<std::string>();
    }
    return value;
}

bool App::hasArg(const std::string&key)const{
    return mArgsResult->count(key)!=0;
}

bool App::hasSwitch(const std::string&key)const{
    return mArgsResult->count(key)!=0;
}

void App::setArg(const std::string&key,const std::string&value){
    //mArgsResult.setArgument(key,value);
}

int App::getArgAsInt(const std::string&key,int def)const{
    int value = def;
    if(mArgsResult->count(key)){
        value = (*mArgsResult)[key].as<int>();
    }
    return value;
}

float App::getArgAsFloat(const std::string&key,float def)const{
    float value = def;
    if(mArgsResult->count(key)){
        value = (*mArgsResult)[key].as<float>();
    }
    return value;
}

double App::getArgAsDouble(const std::string&key,double def)const{
    double value = def;
    if(mArgsResult->count(key)){
        value = (*mArgsResult)[key].as<double>();
    }
    return value;
}

size_t App::getParamCount()const{
    return mArgsResult->arguments().size();//getParamCount();
}

std::string App::getParam(int idx,const std::string&def)const{
    std::string value = def;
    //mArgsResult.getParam(idx,value);
    const auto& args = mArgsResult->arguments();
    if((idx<args.size())&&(idx>=0)){
        const std::string  key = args[idx].key();
        value = (*mArgsResult)[key].as<std::string>();
    }
    return value;
}

void App::setOpacity(unsigned char alpha){
    auto primarySurface = GraphDevice::getInstance().getPrimarySurface();
    if(primarySurface){
        GFXSurfaceSetOpacity(primarySurface,alpha);
        LOGD("alpha=%d",alpha);
    }
}

void App::addEventHandler(const EventHandler*handler){
    Looper::getMainLooper()->addEventHandler(handler);
}

void App::removeEventHandler(const EventHandler*handler){
    Looper::getMainLooper()->removeEventHandler(handler);
}

int App::exec(){
    Looper*looper = Looper::getMainLooper();
    while(!mQuitFlag)looper->pollAll(1);
    return mExitCode;
}

void App::exit(int code){
    mQuitFlag = true;
    mExitCode = code;
}

void App::setName(const std::string&appname){
    mName = appname;
    size_t pos = mName.find_last_of("/");
    if(pos!=std::string::npos)
        mName = mName.substr(pos+1);
    addResource(getDataPath()+mName+std::string(".pak"));
}

const std::string& App::getName(){
    return mName;
}

}

