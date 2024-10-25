#ifndef __COLOR_H__
#define __COLOR_H__
#include <string>
#include <cstdint>

#define RGBA(r,g,b,a) (((a)<<24)||((r)<<16)||((g)<<8)||(b))
#if !(defined(RGB)||defined(_WIN32)||defined(_WIN64))
#define RGB(r,g,b) RGBA(r,g,b,0xFF)
#endif

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
      ORANGE    = 0xFFFFA500,
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

    void setRed(float r){mComponents[0]=r;}
    void setGreen(float g){mComponents[1]=g;}
    void setBlue(float b){mComponents[2]=b;}
    void setAlpha(float a){mComponents[3]=a;}

    void setRed(uint8_t r){mComponents[0]=float(r)/255.f;}
    void setGreen(uint8_t g){mComponents[1]=float(g)/255.f;}
    void setBlue(uint8_t b){mComponents[2]=float(b)/255.f;}
    void setAlpha(uint8_t a){mComponents[3]=float(a)/255.f;}

    unsigned int toArgb()const;

    static uint8_t red ( int c){return (c>>16)&0xFF;}
    static uint8_t green(int c){return (c>> 8)&0xFF;}
    static uint8_t blue (int c){return (c&0xFF);}
    static uint8_t alpha(int c){return c>>24;}
    static unsigned int toArgb(float r, float g, float b, float a);
    static unsigned int toArgb(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
    static Color* valueOf(float r,float g,float b,float a=1.0);
    static unsigned int parseColor(const std::string&colorstring);
    static unsigned int getHtmlColor(const std::string&color);
};

}
#endif
