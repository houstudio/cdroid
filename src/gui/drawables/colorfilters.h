#ifndef __COLOR_FILTERS_H__
#define __COLOR_FILTERS_H__
#include <drawables/colormatrix.h>
#include <core/canvas.h>
#include <core/porterduff.h>
namespace cdroid{

class ColorFilter{
public:
    virtual void apply(Canvas&canvas,const Rect&)=0;
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
