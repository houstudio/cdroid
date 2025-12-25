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
#include <widget/scrollbardrawable.h>
#include <widget/scrollbarutils.h>
#include <drawable/colordrawable.h>
#include <view/viewconfiguration.h>

namespace cdroid{

ScrollBarDrawable::ScrollBarDrawable(){
    mAlpha = 255;
    mMutated=false;
    mRange = 0;
    mOffset= 0;
    mExtent= 0;
    mVertical = true;
    mHasSetAlpha  = false;
    mBoundsChanged= false;
    mRangeChanged = false;
    mHasSetColorFilter = false;
    mAlwaysDrawHorizontalTrack=false;
    mAlwaysDrawVerticalTrack = false;
    mVerticalTrack = nullptr;
    mVerticalThumb = nullptr;
    mHorizontalTrack=nullptr;
    mHorizontalThumb=nullptr;
}

ScrollBarDrawable::~ScrollBarDrawable(){
    delete mVerticalTrack;
    delete mVerticalThumb;
    delete mHorizontalTrack;
    delete mHorizontalThumb;
}

void ScrollBarDrawable::setAlwaysDrawHorizontalTrack(bool alwaysDrawTrack) {
    mAlwaysDrawHorizontalTrack = alwaysDrawTrack;
}

void ScrollBarDrawable::setAlwaysDrawVerticalTrack(bool alwaysDrawTrack) {
    mAlwaysDrawVerticalTrack = alwaysDrawTrack;
}

bool ScrollBarDrawable::getAlwaysDrawVerticalTrack()const{
    return mAlwaysDrawVerticalTrack;
}

bool ScrollBarDrawable::getAlwaysDrawHorizontalTrack()const{
    return mAlwaysDrawHorizontalTrack;
}

void ScrollBarDrawable::setParameters(int range, int offset, int extent, bool vertical) {
    if (mVertical != vertical) {
        mVertical = vertical;
        mBoundsChanged = true;
    }

    if (mRange != range || mOffset != offset || mExtent != extent) {
        mRange = range;
        mOffset = offset;
        mExtent = extent;
        mRangeChanged = true;
    }
}

void ScrollBarDrawable::draw(Canvas&canvas) {
    bool bDrawTrack = true;
    bool bDrawThumb = true;
    if (mExtent <= 0 || mRange <= mExtent) {
        bDrawTrack = mVertical ? mAlwaysDrawVerticalTrack : mAlwaysDrawHorizontalTrack;
        bDrawThumb = false;
    }

    Rect r = getBounds();

    double x1,y1,x2,y2;
    canvas.get_clip_extents(x1,y1,x2,y2);
    Rect rcClip;
    rcClip.set(int(x1),int(y1),int(std::abs(x2-x1)),int(std::abs(y2-y1)));
    if(!rcClip.intersect(r))return;

    if (bDrawTrack)drawTrack(canvas, r, mVertical);

    if (bDrawThumb) {
        const int scrollBarLength = mVertical ? r.height : r.width;
        const int thickness   = mVertical ? r.width : r.height;
        const int thumbLength = ScrollBarUtils::getThumbLength(scrollBarLength, thickness, mExtent, mRange);
        const int thumbOffset = ScrollBarUtils::getThumbOffset(scrollBarLength, thumbLength, mExtent, mRange,mOffset);
        drawThumb(canvas, r, thumbOffset, thumbLength, mVertical);
    }
}

void ScrollBarDrawable::onBoundsChange(const Rect& bounds) {
    Drawable::onBoundsChange(bounds);
    mBoundsChanged = true;
}

bool ScrollBarDrawable::isStateful()const {
    return ((mVerticalTrack!=nullptr)  && mVerticalTrack->isStateful())
            || ((mVerticalThumb!=nullptr)  && mVerticalThumb->isStateful())
            || ((mHorizontalTrack!=nullptr) && mHorizontalTrack->isStateful())
            || ((mHorizontalThumb!=nullptr) && mHorizontalThumb->isStateful())
            || Drawable::isStateful();
}

bool ScrollBarDrawable::onStateChange(const std::vector<int>&state) {
    bool changed = Drawable::onStateChange(state);
    if (mVerticalTrack!=nullptr) changed |= mVerticalTrack->setState(state);
    if (mVerticalThumb!=nullptr) changed |= mVerticalThumb->setState(state);
    if (mHorizontalTrack!=nullptr) changed |= mHorizontalTrack->setState(state);
    if (mHorizontalThumb!=nullptr) changed |= mHorizontalThumb->setState(state);
    return changed;
}

void ScrollBarDrawable::drawTrack(Canvas&canvas,const Rect& bounds, bool vertical) {
    Drawable* track = vertical ? mVerticalTrack:mHorizontalTrack;
    if ( track!=nullptr) {
        if (mBoundsChanged)
            track->setBounds(bounds);
        track->draw(canvas);
    }
}

void ScrollBarDrawable::drawThumb(Canvas& canvas,const Rect& bounds, int offset, int length, bool vertical) {
    const bool changed = mRangeChanged || mBoundsChanged;
    if (vertical) {
        if (mVerticalThumb != nullptr) {
            if (changed)
                mVerticalThumb->setBounds(bounds.left, bounds.top + offset,bounds.width, length);
            mVerticalThumb->draw(canvas);
        }
    } else if ( mHorizontalThumb!=nullptr) {
        if (changed)
            mHorizontalThumb->setBounds(bounds.left+ offset, bounds.top,length, bounds.height);
        mHorizontalThumb->draw(canvas);
    }
}

void ScrollBarDrawable::setVerticalThumbDrawable(Drawable* thumb) {
    if (mVerticalThumb!=nullptr){
        mVerticalThumb->setCallback(nullptr);
        delete mVerticalThumb;
    }

    propagateCurrentState(thumb);
    mVerticalThumb = thumb;
}

void ScrollBarDrawable::setVerticalTrackDrawable(Drawable* track) {
    if (mVerticalTrack!=nullptr){
        mVerticalTrack->setCallback(nullptr);
        delete mVerticalTrack;
    }

    propagateCurrentState(track);
    mVerticalTrack = track;
}

void ScrollBarDrawable::setHorizontalThumbDrawable(Drawable* thumb) {
    if (mHorizontalThumb!=nullptr){
        mHorizontalThumb->setCallback(nullptr);
        delete mHorizontalThumb;
    }

    propagateCurrentState(thumb);
    mHorizontalThumb = thumb;
}

void ScrollBarDrawable::setHorizontalTrackDrawable(Drawable* track) {
    if (mHorizontalTrack!=nullptr){
        mHorizontalTrack->setCallback(nullptr);
        delete mHorizontalTrack;
    }

    propagateCurrentState(track);
    mHorizontalTrack = track;
}

Drawable*ScrollBarDrawable::getVerticalThumbDrawable()const{
    return mVerticalThumb;
}

Drawable*ScrollBarDrawable::getVerticalTrackDrawable()const{
    return mVerticalTrack;
}

Drawable*ScrollBarDrawable::getHorizontalThumbDrawable()const{
    return mHorizontalThumb;
}

Drawable*ScrollBarDrawable::getHorizontalTrackDrawable()const{
    return mHorizontalTrack;
}

void ScrollBarDrawable::propagateCurrentState(Drawable* d) {
    if (d != nullptr) {
        if (mMutated) {
            d->mutate();
        }

        d->setState(getState());
        d->setCallback(this);

        if (mHasSetAlpha)d->setAlpha(mAlpha);

        if (mHasSetColorFilter) {
            d->setColorFilter(mColorFilter);
        }
    }
}

int ScrollBarDrawable::getSize(bool vertical) {
    if (vertical) {
        return mVerticalTrack!=nullptr ? mVerticalTrack->getIntrinsicWidth() :
                mVerticalThumb!=nullptr ? mVerticalThumb->getIntrinsicWidth() : 0;
    } else {
        return mHorizontalTrack!=nullptr ? mHorizontalTrack->getIntrinsicHeight() :
                mHorizontalThumb!=nullptr ? mHorizontalThumb->getIntrinsicHeight() : 0;
    }
}

ScrollBarDrawable* ScrollBarDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        if (mVerticalTrack!=nullptr) mVerticalTrack->mutate();
        if (mVerticalThumb!=nullptr) mVerticalThumb->mutate();

        if (mHorizontalTrack!=nullptr) mHorizontalTrack->mutate();
        if (mHorizontalThumb!=nullptr) mHorizontalThumb->mutate();
        mMutated = true;
    }
    return this;
}

void ScrollBarDrawable::setAlpha(int alpha) {
    mAlpha = alpha;
    mHasSetAlpha = true;

    if (mVerticalTrack!=nullptr) mVerticalTrack->setAlpha(alpha);
    if (mVerticalThumb!=nullptr) mVerticalThumb->setAlpha(alpha);
    if (mHorizontalTrack!=nullptr) mHorizontalTrack->setAlpha(alpha);
    if (mHorizontalThumb!=nullptr) mHorizontalThumb->setAlpha(alpha);
}

int ScrollBarDrawable::getAlpha()const{
    return mAlpha;
}

int ScrollBarDrawable::getOpacity() {
    return TRANSLUCENT;
}

void ScrollBarDrawable::invalidateDrawable(Drawable& who) {
    invalidateSelf();
}

void ScrollBarDrawable::scheduleDrawable(Drawable& who,const Runnable& what, int64_t when) {
    scheduleSelf(what, when);
}

void ScrollBarDrawable::unscheduleDrawable(Drawable& who,const Runnable& what) {
    unscheduleSelf(what);
}

}
