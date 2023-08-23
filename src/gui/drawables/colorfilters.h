#ifndef __COLOR_FILTERS_H__
#define __COLOR_FILTERS_H__
#include <drawables/colormatrix.h>
#include <core/canvas.h>
namespace cdroid{
enum TintMode{
    NOOP  = -1,
    CLEAR = 0,
    SRC   = 1,
    DST   = 2,
    SRC_OVER=3,
    DST_OVER=4,
    SRC_IN  =5,
    DST_IN  =6,
    SRC_OUT =7,
    DST_OUT =8,
    SRC_ATOP=9,
    DST_ATOP=10,
    XOR     =11,
    ADD     =12,
    MULTIPLY=13,
    SCREEN  =14,
    OVERLAY =15,
    DARKEN  =16,
    LIGHTEN =17,
};
typedef TintMode PorterDuffMode;

class ColorFilter{
public:
    virtual void apply(Canvas&canvas,const Rect&)=0;
    static int tintMode2CairoOperator(int tintMode);
};

class ColorMatrixColorFilter:public ColorFilter{
protected:
    ColorMatrix mCM;
public:    
    ColorMatrixColorFilter(const float(&v)[20]);
    void apply(Canvas&canvas,const Rect&)override;
};

class PorterDuffColorFilter:public ColorFilter{
protected:
    int mColor;
    int mMode;
public:
    PorterDuffColorFilter(int color,int mode);
    void apply(Canvas&canvas,const Rect&)override;
    void setColor(int c);
    void setMode(int m);
    int getColor()const;
    int getMode()const;
};

/* R' = R * colorMultiply.R + colorAdd.R
 * G' = G * colorMultiply.G + colorAdd.G
 * B' = B * colorMultiply.B + colorAdd.B
 * The result is pinned to the <code>[0..255]</code> range for each channel*/
class LightingColorFilter:public ColorMatrixColorFilter{
protected:
    int mMul;
    int mAdd;
public:
    LightingColorFilter(int mul,int add);
    void apply(Canvas&canvas,const Rect&)override;
};
}//end namespace

#endif//
