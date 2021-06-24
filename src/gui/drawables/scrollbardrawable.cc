#include <drawables/scrollbardrawable.h>
#include <drawables/colordrawable.h>
#include <viewconfiguration.h>

namespace cdroid{

ScrollBarDrawable::ScrollBarDrawable(){
    mAlpha=255;
    mMutated=false;
    mRange=0;
    mOffset=0;
    mExtent=0;
    mVertical=true;
    mBoundsChanged=false;
    mRangeChanged=false;
    mAlwaysDrawHorizontalTrack=false;
    mAlwaysDrawVerticalTrack=false;
    mVerticalTrack = new ColorDrawable(0x80222222);
    mVerticalThumb = new ColorDrawable(0x80aaaaaa);
    mHorizontalTrack = nullptr;
    mHorizontalThumb = nullptr;
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
    bool vertical = mVertical;
    int extent = mExtent;
    int range = mRange;

    bool bdrawTrack = true;
    bool bdrawThumb = true;
    if (extent <= 0 || range <= extent) {
        bdrawTrack = vertical ? mAlwaysDrawVerticalTrack : mAlwaysDrawHorizontalTrack;
        bdrawThumb = false;
    }

    RECT r = getBounds();
    //if (canvas.quickReject(r.left, r.top, r.right, r.bottom, Canvas.EdgeType.AA))  return;

    if (bdrawTrack)drawTrack(canvas, r, vertical);

    if (bdrawThumb) {
        int scrollBarLength = vertical ? r.height : r.width;
        int thickness   = vertical ? r.width : r.height;
        int thumbLength = ViewConfiguration::getThumbLength(scrollBarLength, thickness, extent, range);
        int thumbOffset = ViewConfiguration::getThumbOffset(scrollBarLength, thumbLength, extent, range,mOffset);
        drawThumb(canvas, r, thumbOffset, thumbLength, vertical);
    }
}

void ScrollBarDrawable::onBoundsChange(const RECT& bounds) {
    Drawable::onBoundsChange(bounds);
    mBoundsChanged = true;
}

bool ScrollBarDrawable::isStateful()const {
    return (mVerticalTrack  && mVerticalTrack->isStateful())
            || (mVerticalThumb  && mVerticalThumb->isStateful())
            || (mHorizontalTrack && mHorizontalTrack->isStateful())
            || (mHorizontalThumb && mHorizontalThumb->isStateful())
            || Drawable::isStateful();
}

bool ScrollBarDrawable::onStateChange(const std::vector<int>&state) {
    bool changed = Drawable::onStateChange(state);
    if (mVerticalTrack )  changed |= mVerticalTrack->setState(state);
    if (mVerticalThumb )  changed |= mVerticalThumb->setState(state);
    if (mHorizontalTrack) changed |= mHorizontalTrack->setState(state);
    if (mHorizontalThumb) changed |= mHorizontalThumb->setState(state);
    return changed;
}

void ScrollBarDrawable::drawTrack(Canvas&canvas,const RECT& bounds, bool vertical) {
    Drawable* track=vertical?mVerticalTrack:mHorizontalTrack;
    if ( track ) {
        if (mBoundsChanged) 
            track->setBounds(bounds);
        track->draw(canvas);
    }
}

void ScrollBarDrawable::drawThumb(Canvas& canvas,const RECT& bounds, int offset, int length, bool vertical) {
    bool changed = mRangeChanged || mBoundsChanged;
    if (vertical) {
        if (mVerticalThumb != nullptr) {
            if (changed)
                mVerticalThumb->setBounds(bounds.x, bounds.y + offset,bounds.width, offset + length);

            mVerticalThumb->draw(canvas);
        }
    } else if ( mHorizontalThumb ) {
        if (changed) 
            mHorizontalThumb->setBounds(bounds.x+ offset, bounds.y,offset + length, bounds.height);

        mHorizontalThumb->draw(canvas);
    }
}

void ScrollBarDrawable::setVerticalThumbDrawable(Drawable* thumb) {
    if (mVerticalThumb){
        mVerticalThumb->setCallback(nullptr);
        delete mVerticalThumb;
    }

    propagateCurrentState(thumb);
    mVerticalThumb = thumb;
}

void ScrollBarDrawable::setVerticalTrackDrawable(Drawable* track) {
    if (mVerticalTrack){
        mVerticalTrack->setCallback(nullptr);
        delete mVerticalTrack;
    }

    propagateCurrentState(track);
    mVerticalTrack = track;
}

void ScrollBarDrawable::setHorizontalThumbDrawable(Drawable* thumb) {
    if (mHorizontalThumb){
        mHorizontalThumb->setCallback(nullptr);
        delete mHorizontalThumb;
    }

    propagateCurrentState(thumb);
    mHorizontalThumb = thumb;
}

void ScrollBarDrawable::setHorizontalTrackDrawable(Drawable* track) {
    if (mHorizontalTrack ){
        mHorizontalTrack->setCallback(nullptr);
        delete mHorizontalTrack;
    }

    propagateCurrentState(track);
    mHorizontalTrack = track;
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
            //d->setColorFilter(mColorFilter);
        }
    }
}

int ScrollBarDrawable::getSize(bool vertical) {
    if (vertical) {
        return mVerticalTrack  ? mVerticalTrack->getIntrinsicWidth() :
                mVerticalThumb ? mVerticalThumb->getIntrinsicWidth() : 0;
    } else {
        return mHorizontalTrack ? mHorizontalTrack->getIntrinsicHeight() :
                mHorizontalThumb? mHorizontalThumb->getIntrinsicHeight() : 0;
    }
}

ScrollBarDrawable* ScrollBarDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        if (mVerticalTrack )    mVerticalTrack->mutate();
        if (mVerticalThumb )    mVerticalThumb->mutate();

        if (mHorizontalTrack)   mHorizontalTrack->mutate();
        if (mHorizontalThumb)   mHorizontalThumb->mutate();
        mMutated = true;
    }
    return this;
}

void ScrollBarDrawable::setAlpha(int alpha) {
    mAlpha = alpha;
    mHasSetAlpha = true;

    if (mVerticalTrack)   mVerticalTrack->setAlpha(alpha);
    if (mVerticalThumb)   mVerticalThumb->setAlpha(alpha);
    if (mHorizontalTrack) mHorizontalTrack->setAlpha(alpha);
    if (mHorizontalThumb) mHorizontalThumb->setAlpha(alpha);
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

void ScrollBarDrawable::scheduleDrawable(Drawable& who, Runnable what, long when) {
    scheduleSelf(what, when);
}

void ScrollBarDrawable::unscheduleDrawable(Drawable& who,Runnable what) {
    unscheduleSelf(what);
}

}
