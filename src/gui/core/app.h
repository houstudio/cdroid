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
#ifndef __APPLICATION_H__
#define __APPLICATION_H__
#include <string>
#include <map>
#include <istream>
#include <cairomm/surface.h>
#include <core/looper.h>
#include <core/context.h>
#include <core/assets.h>

namespace cxxopts{
    class ParseResult;
}
namespace cdroid{

class App:public Assets{
private:
    bool mQuitFlag;
    int mExitCode;
protected:
    std::unique_ptr<cxxopts::ParseResult> mArgsResult;
    static App*mInst;
    void onInit();
public:
     App(int argc=0,const char*argv[]=NULL);
     ~App()override;
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
     size_t getParamCount()const;
     std::string getParam(int idx,const std::string&def="")const;
     virtual void addEventHandler(const EventHandler* handler);
     virtual void removeEventHandler(const EventHandler*handler);
     virtual int exec();
     virtual void exit(int code=0);
};

}/*end ofnamespace*/
#endif/*__APPLICATION_H__*/
