#ifndef __NINE_PATCH_H__
#define __NINE_PATCH_H__
#include <core/canvas.h>

namespace cdroid{

class NinePatch {
private:
    int mWidth = -1;/*CacheImage.Width*/
    int mHeight= -1;/*CacheImage.Height*/
    RECT mContentArea;
    RECT mPadding;
    std::vector<std::pair< int, int >>mResizeDistancesY;
    std::vector<std::pair< int, int >>mResizeDistancesX;
    Cairo::RefPtr<Cairo::ImageSurface> mCachedImage;
public:
    Cairo::RefPtr<Cairo::ImageSurface> mImage;
private:
    RECT getContentArea();
    void getResizeArea();
    void getFactor(int width, int height, double& factorX, double& factorY);
    void updateCachedImage(int width, int height);
public:
    NinePatch(Cairo::RefPtr<Cairo::ImageSurface> image);
    ~NinePatch();
    void draw(Canvas& painter, int x, int y);
    void drawScaledPart(const RECT& oldRect,const RECT& newRect,Cairo::Context&painter);
    void drawConstPart (const RECT& oldRect,const RECT& newRect,Cairo::Context&painter);
    void setImageSize(int width, int height);
    RECT getContentArea(int  widht, int  height);
    RECT getPadding()const;
};
}/*endof namespace*/
#endif

