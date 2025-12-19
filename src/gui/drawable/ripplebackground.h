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
#ifndef __RIPPLE_COMPONENT_H__
#define __RIPPLE_COMPONENT_H__
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
    virtual ~RippleComponent()=default;
    virtual void onHotspotBoundsChanged();
    virtual void onBoundsChange();
    void setup(float maxRadius, int densityDpi);
    void getBounds(Rect& bounds);
};

class RippleBackground:public RippleComponent{
private:
    class COPACITY;
    static const COPACITY OPACITY;
    static constexpr  int OPACITY_DURATION = 80;
    ValueAnimator* mAnimator;
    float mOpacity = 0;
    bool mIsBounded;
    bool mFocused = false;
    bool mHovered = false;

    void onStateChanged();
public:
    RippleBackground(RippleDrawable* owner,const Rect& bounds, bool isBounded);
    ~RippleBackground()override;
    bool isVisible()const;
    void draw(Canvas& c,float alpha);
    void setState(bool focused, bool hovered, bool pressed);
    void jumpToFinal();
};

}/*endof namespace*/
#endif/*__RIPPLE_COMPONENT_H__*/
