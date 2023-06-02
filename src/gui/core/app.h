#ifndef __APPLICATION_H__
#define __APPLICATION_H__
#include <string>
#include <map>
#include <istream>
#include <cairomm/surface.h>
#include <core/looper.h>
#include <core/context.h>
#include <core/assets.h>
#include <core/cla.h>
struct option;

namespace cdroid{

class App:public Assets{
private:
    const std::string getAssetsPath();
    bool mQuitFlag;
    int mExitCode;
protected:
    CLA cla;
    static App*mInst;
public:
     App(int argc=0,const char*argv[]=NULL,const std::vector<CLA::Argument>&extoptions={});
     ~App();
     static App&getInstance();
     const std::string getDataPath()const;
     virtual void setOpacity(unsigned char alpha);
     virtual void setName(const std::string&appname);
     virtual const std::string&getName();
     void setArg(const std::string&key,const std::string&value);
     bool hasArg(const std::string&key)const;
     bool hasSwitch(const std::string&key)const;
     const std::string getArg(const std::string&key,const std::string&def="")const;
     int getArgAsInt(const std::string&key,int def)const;
     float getArgAsFloat(const std::string&key,float def)const;
     double getArgAsDouble(const std::string&key,double def)const;
     int getParamCount()const;
     std::string getParam(int idx,const std::string&def="")const;
     virtual void addEventHandler(const EventHandler* handler);
     virtual void removeEventHandler(const EventHandler*handler);
     virtual int exec();
     virtual void exit(int code=0);
};

}//namespace
#endif
