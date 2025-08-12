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
#ifndef __NINE_PATCH_H__
#define __NINE_PATCH_H__
#include <core/canvas.h>
#include <core/insets.h>
namespace cdroid{
class Context;
class NinePatch {
private:
    int mWidth = -1;/*CacheImage.Width*/
    int mHeight= -1;/*CacheImage.Height*/
    int mOpacity;
    int mRadius;
    Rect mContentArea;
    Rect mPadding;
    Insets mOpticalInsets;
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
    int getCornerRadius(Cairo::RefPtr<Cairo::ImageSurface> bitmap,int start,int step);
    Insets getOpticalInsets(Cairo::RefPtr<Cairo::ImageSurface>bitmap)const;
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
    Insets getOpticalInsets()const;
    int getRadius()const;
    Rect getOutlineRect() const;
    int getOutlineRadius() const;
};
}/*endof namespace*/
#endif

