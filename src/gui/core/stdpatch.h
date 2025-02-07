#ifndef __STD_PATCH_H__
#define __STD_PATCH_H__
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#if defined(__GNUC__) && (__GNUC__ <= 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 9))
namespace std{
inline double round(double __x)
  { return ::round(__x); }

inline float
  round(float __x)
  { return ::round(__x); }

inline int
  stoi(const string& __str, size_t* __idx = 0, int __base = 10)
  { return atoi(__str.c_str());}

inline float
  strtof(const string& __str, size_t* __idx = 0)
  { return ::strtof(__str.c_str(), NULL); }

inline float
  stof(const string& __str, size_t* __idx = 0)
  {  return ::strtof(__str.c_str(), NULL); }

inline unsigned long strtoul(const string&__str,char**__index=0,int __base=10)
  { return ::strtoul(__str.c_str(),__index,__base);}

inline string to_string(int value){
   char sbuff[32];
   sprintf(sbuff,"%d",value);
   return std::string(sbuff);
}

inline string to_string(float value){
   char sbuff[32];
   sprintf(sbuff,"%f",value);
   return std::string(sbuff);
}

inline string to_string(double value){
   char sbuff[32];
   sprintf(sbuff,"%f",value);
   return std::string(sbuff);
}

inline const char*get_time(struct tm*,const char*){return "";}
/*constexpr inline double fmod(double x, double y){
   return ::fmod(x,y);
}

constexpr inline double fmod(float x, float y){
   return fmodf(x,y);
}*/
/*constexpr inline int abs(int j){return ::abs(j);}
constexpr inline long labs(long j){return ::labs(j);}
constexpr inline long long llabs(long long j){return llabs(j);}*/

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}
#endif
#endif/*__STD_PATCH_H__*/
