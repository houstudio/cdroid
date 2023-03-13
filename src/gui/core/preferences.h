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
   int getSectionCount()const;
   int getSections(std::vector<std::string>&mbs);
   int getUpdates()const;
   bool getBool(const std::string&section,const std::string&key,bool def=false);
   int getInt(const std::string&section,const std::string&key,int def=0);
   float getFloat(const std::string&section,const std::string&key,float def=.0);
   double getDouble(const std::string&section,const std::string&key,double def=.0);
   std::string getString(const std::string&section,const std::string&key,const char*);

   void setValue(const std::string&section,const std::string&key,bool v);
   void setValue(const std::string&section,const std::string&key,int v);
   void setValue(const std::string&section,const std::string&key,float v);
   void setValue(const std::string&section,const std::string&key,const std::string& v);
   void setValue(const std::string&section,const std::string&key,double v);
};

}//namespace

#endif
