#pragma once
#include <animation/objectanimator.h>

namespace cdroid{
class RippleComponent{
private:
    bool mHasMaxRadius;
    static float getTargetRadius(const Rect& bounds);
protected:
    class RippleDrawable* mOwner;
    Rect mBounds;
    /** How big this ripple should be when fully entered. */
    float mTargetRadius;
    /** Screen density used to adjust pixel-based constants. */
    float mDensityScale;

    void invalidateSelf();
    virtual void onTargetRadiusChanged(float targetRadius);
public:
    RippleComponent(RippleDrawable* owner,const Rect& bounds);
    virtual void onHotspotBoundsChanged();
    virtual void onBoundsChange();
    void setup(float maxRadius, int densityDpi);
    void getBounds(Rect& bounds);
};


class RippleBackground:public RippleComponent{
private:
    static constexpr  int OPACITY_DURATION = 80;
    ValueAnimator* mAnimator;
    float mOpacity = 0;
    bool mIsBounded;
    bool mFocused = false;
    bool mHovered = false;

    void onStateChanged();
public:
    RippleBackground(RippleDrawable* owner,const Rect& bounds, bool isBounded);
    bool isVisible()const;
    void draw(Canvas& c,float alpha);
    void setState(bool focused, bool hovered, bool pressed);
    void jumpToFinal();
};

}
