#ifndef __COLOR_H__
#define __COLOR_H_
#include <string>

namespace cdroid{
class Color{
public:
    enum{
      BLACK     = 0xFF000000,
      DKGRAY    = 0xFF444444,
      GRAY      = 0xFF888888,
      LTGRAY    = 0xFFCCCCCC,
      WHITE     = 0xFFFFFFFF,
      RED       = 0xFFFF0000,
      GREEN     = 0xFF00FF00,
      BLUE      = 0xFF0000FF,
      YELLOW    = 0xFFFFFF00,
      CYAN      = 0xFF00FFFF,
      MAGENTA   = 0xFFFF00FF,
      TRANSPARENT = 0
    };
private:
    float  mComponents[4];
public:
    Color(unsigned int colorargb=BLACK);
    Color(float r, float g, float b, float a);
    float red () const{return mComponents[0];}
    float green()const{return mComponents[1];}
    float blue() const{return mComponents[2];}
    float alpha()const{return mComponents[3];}
    unsigned int toArgb()const;

    static int red ( int c){return (c>>16)&0xFF;}
    static int green(int c){return (c>> 8)&0xFF;}
    static int blue (int c){return (c&0xFF);}
    static int alpha(int c){return c>>24;}
    static unsigned int toArgb(float r, float g, float b, float a);
    static unsigned int toArgb(int r,int g,int b,int a);
    static Color* valueOf(float r,float g,float b,float a=1.0);
    static unsigned int parseColor(const std::string&colorstring);
    static unsigned int getHtmlColor(const std::string&color);
};

}
#endif
