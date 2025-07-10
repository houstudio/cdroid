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
#ifndef __PREFERENCE_H__
#define __PREFERENCE_H__
#include <string>
#include <vector>
#include <map>

namespace cdroid{

class Preferences{
protected:
   int updates;
   std::string mFileName;
   std::map<std::string,std::map<std::string,std::string>>mPrefs;
public:
   Preferences();
   ~Preferences();
   void load(const std::string&fname);
   void save(const std::string&fname);
   void load(std::istream&istream);
   void save(std::ostream&ostream);
   void load(const char*,size_t);
   int getSectionCount()const;
   int getSections(std::vector<std::string>&mbs);
   void removeSection(const std::string&section);
   bool hasSection(const std::string&)const;
   int getUpdates()const;
   bool getBool(const std::string&section,const std::string&key,bool def=false);
   int getInt(const std::string&section,const std::string&key,int def=0);
   int getInt(const std::string&section,const std::string&key,const std::map<std::string,int>&kv,int def);
   float getFloat(const std::string&section,const std::string&key,float def=.0);
   double getDouble(const std::string&section,const std::string&key,double def=.0);
   std::string getString(const std::string&section,const std::string&key,const std::string&def="");

   void setValue(const std::string&section,const std::string&key,bool v);
   void setValue(const std::string&section,const std::string&key,int v);
   void setValue(const std::string&section,const std::string&key,float v);
   void setValue(const std::string&section,const std::string&key,const std::string& v);
   void setValue(const std::string&section,const std::string&key,double v);
};

}//namespace

#endif
