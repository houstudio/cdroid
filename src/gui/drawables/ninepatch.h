#ifndef __NINE_PATCH_H__
#define __NINE_PATCH_H__
#include <core/canvas.h>

namespace cdroid{
class Context;
class NinePatch {
private:
    int mWidth = -1;/*CacheImage.Width*/
    int mHeight= -1;/*CacheImage.Height*/
    Rect mContentArea;
    Rect mPadding;
    Rect mOpticalInsets;
    int mOpacity;
    float mAlpha;
    std::vector<std::pair< int, int >>mResizeDistancesY;
    std::vector<std::pair< int, int >>mResizeDistancesX;
    Cairo::RefPtr<Cairo::ImageSurface> mCachedImage;
public:
    Cairo::RefPtr<Cairo::ImageSurface> mImage;
private:
    Rect getContentArea();
    void getResizeArea();
    void getFactor(int width, int height, double& factorX, double& factorY);
    void updateCachedImage(int width, int height,Cairo::Context*);
    int analyzeEdge(Cairo::RefPtr<Cairo::ImageSurface>img, int fixedIndex, int start, int end, bool isBottom);
    Rect getOpticalInsetsFromBitmap(Cairo::RefPtr<Cairo::ImageSurface>bitmap);
public:
    NinePatch(Cairo::RefPtr<Cairo::ImageSurface> image);
    NinePatch(cdroid::Context*ctx,const std::string&resid);
    ~NinePatch();
    void draw(Canvas& painter, int x, int y,float alpha=1.f);
    void draw(Canvas& painter, const Rect&,float alpha=1.f);
    void drawScaledPart(const Rect& oldRect,const Rect& newRect,Cairo::Context&painter);
    void drawConstPart (const Rect& oldRect,const Rect& newRect,Cairo::Context&painter);
    void setImageSize(int width, int height);
    Rect getContentArea(int  widht, int  height);
    Rect getPadding()const;
    Rect getOpticalInsets()const;
};
}/*endof namespace*/
#endif

