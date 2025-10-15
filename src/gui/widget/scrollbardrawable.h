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
#ifndef __SCROLLBAR_DRAWABLE_H__
#define __SCROLLBAR_DRAWABLE_H__
#include <drawable/drawable.h>
namespace cdroid{

class ScrollBarDrawable:public Drawable,Drawable::Callback{
private:
    Drawable* mVerticalTrack;
    Drawable* mHorizontalTrack;
    Drawable* mVerticalThumb;
    Drawable* mHorizontalThumb;
    int mRange;
    int mOffset;
    int mExtent;

    bool mVertical;
    bool mBoundsChanged;
    bool mRangeChanged;
    bool mAlwaysDrawHorizontalTrack;
    bool mAlwaysDrawVerticalTrack;
    bool mMutated;

    int mAlpha;
    bool mHasSetAlpha;
    //ColorFilter mColorFilter;
    bool mHasSetColorFilter;

    void drawTrack(Canvas&canvas,const Rect& bounds, bool vertical);
    void drawThumb(Canvas& canvas,const Rect& bounds, int offset, int length, bool vertical);
    void propagateCurrentState(Drawable* d);
protected:
    void onBoundsChange(const Rect& bounds)override;
    bool onStateChange(const std::vector<int>&state)override;
public:
    ScrollBarDrawable();
    virtual ~ScrollBarDrawable();
    void setAlwaysDrawHorizontalTrack(bool alwaysDrawTrack);
    void setAlwaysDrawVerticalTrack(bool alwaysDrawTrack);
    bool getAlwaysDrawVerticalTrack()const;
    bool getAlwaysDrawHorizontalTrack()const;
    void setParameters(int range, int offset, int extent, bool vertical);
    void draw(Canvas&canvas)override;
    bool isStateful()const override;
    void setVerticalThumbDrawable(Drawable* thumb);
    void setVerticalTrackDrawable(Drawable* track);
    void setHorizontalThumbDrawable(Drawable* thumb);
    void setHorizontalTrackDrawable(Drawable* track);
    Drawable*getVerticalThumbDrawable()const;
    Drawable*getVerticalTrackDrawable()const;
    Drawable*getHorizontalThumbDrawable()const;
    Drawable*getHorizontalTrackDrawable()const;
    int getSize(bool vertical);
    ScrollBarDrawable* mutate()override;
    void setAlpha(int alpha)override;
    int getAlpha() const override;
    int getOpacity()override;
    void invalidateDrawable(Drawable& who)override;
    void scheduleDrawable(Drawable& who,const Runnable& what, int64_t when)override;
    void unscheduleDrawable(Drawable& who,const Runnable& what)override;
};

}
#endif
