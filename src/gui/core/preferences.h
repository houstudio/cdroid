#ifndef __PREFERENCE_H__
#define __PREFERENCE_H__
#include<string>
#include <json/json.h>

namespace cdroid{

class Preferences{
protected:
   int updates;
   std::string pref_file;
   Json::Value d;
//   void getSection(const std::string&section,Json::Value&value);
public:
   Preferences();
   ~Preferences();
   void load(const std::string&fname);
   void save(const std::string&fname);
   int getSections(std::vector<std::string>*mbs);
   int getUpdates()const;
   bool getBool(const std::string&section,const std::string&key,bool def=false);
   int getInt(const std::string&section,const std::string&key,int def=0);
   float getFloat(const std::string&section,const std::string&key,float def=.0);
   double getDouble(const std::string&section,const std::string&key,double def=.0);
   const char* getString(const std::string&section,const std::string&key,const char*);

   void setValue(const std::string&section,const std::string&key,bool v);
   void setValue(const std::string&section,const std::string&key,int v);
   void setValue(const std::string&section,const std::string&key,float v);
   void setValue(const std::string&section,const std::string&key,const char* v);
   void setValue(const std::string&section,const std::string&key,double v);
};

}//namespace

#endif
