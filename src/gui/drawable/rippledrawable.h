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
#ifndef __RIPPLE_DRAWABLE_H__
#define __RIPPLE_DRAWABLE_H__
#include <drawable/layerdrawable.h>
#include <drawable/rippleforeground.h>

namespace cdroid{

class RippleDrawable:public LayerDrawable{
private:
    static constexpr int BACKGROUND_OPACITY_DURATION= 80;
    static constexpr int RADIUS_AUTO = -1;
    static constexpr int MASK_UNKNOWN = -1;
    static constexpr int MASK_NONE = 0;
    static constexpr int MASK_CONTENT = 1;
    static constexpr int MASK_EXPLICIT = 2;
    static constexpr int MAX_RIPPLES = 10;
    static constexpr int STYLE_SOLID = 0;
    static constexpr int STYLE_PATTERNED = 1;
    static constexpr bool FORCE_PATTERNED_STYLE = true;
private:
    class RippleState:public LayerDrawable::LayerState{
    public:
        std::vector<int>mTouchThemeAttrs;
        int mMaxRadius;
        int mRippleStyle=FORCE_PATTERNED_STYLE?STYLE_PATTERNED:STYLE_SOLID;
        const ColorStateList*mColor;
        RippleState(LayerState* orig, RippleDrawable* owner);
        ~RippleState();
        void onDensityChanged(int sourceDensity, int targetDensity)override;
        void applyDensityScaling(int sourceDensity, int targetDensity);
        RippleDrawable*newDrawable()override;
        int getChangingConfigurations()const override;
    };

    Rect mHotspotBounds;
    Rect mDrawingBounds;
    Rect mDirtyBounds;
    std::shared_ptr<RippleState>mState;
    Drawable* mMask;/*The masking layer, e.g. the layer with id R.id.mask. */
    RippleBackground* mBackground;
 
    bool mHasValidMask;
    bool mRippleActive;
    bool mHasPending;
    bool mOverrideBounds;
    float mPendingX;
    float mPendingY;
    std::vector<RippleForeground*>mExitingRipples;
    int  mDensity;
    float mBackgroundOpacity;
    float mTargetBackgroundOpacity;
    bool mForceSoftware;
    bool mAddRipple;
    bool mRunBackgroundAnimation;
    bool mExitingAnimation;
    RippleForeground* mRipple;
    ValueAnimator*mBackgroundAnimation;
private:
    RippleDrawable(std::shared_ptr<RippleState> state);
    void cancelExitingRipples();
    void setRippleActive(bool active);
    void setBackgroundActive(bool hovered, bool focused, bool pressed);
    bool isBounded()const;
    void tryRippleEnter();
    void tryRippleExit();
    void clearHotspots();
    void onHotspotBoundsChanged();
    void updateLocalState();
    int  getMaskType();
    void drawContent(Canvas& canvas);
    void drawBackgroundAndRipples(Canvas& canvas);
    void drawMask(Canvas& canvas);
    void drawSolid(Canvas& canvas);
    void exitPatternedBackgroundAnimation();
    void startPatternedAnimation();
    void exitPatternedAnimation();
    void enterPatternedBackgroundAnimation(bool focused, bool hovered);
    void startBackgroundAnimation();
protected:
    bool onStateChange(const std::vector<int>&stateSet)override;
    void onBoundsChange(const Rect& bounds)override;
public:
    RippleDrawable();
    RippleDrawable(const ColorStateList* color,Drawable* content,Drawable* mask);
    ~RippleDrawable()override;
    void jumpToCurrentState()override;
    int  getOpacity()override;
    bool setVisible(bool visible, bool restart)override;
    bool isProjected();
    bool isStateful()const override;
    bool hasFocusStateSpecified()const override;
    void setColor(const ColorStateList* color);
    void setRadius(int radius);
    int  getRadius()const;
    bool setDrawableByLayerId(int id, Drawable* drawable)override;
    void setPaddingMode(int mode)override;
    bool canApplyTheme()override;
    void getHotspotBounds(Rect&out)const override;
    void setHotspot(float x,float y)override;
    void setHotspotBounds(int left,int top,int w,int h)override;
    void draw(Canvas& canvas)override;
    void invalidateSelf()override;
    void invalidateSelf(bool invalidateMask);
    void pruneRipples();
    Rect getDirtyBounds() const override;
    void inflate(XmlPullParser&,const AttributeSet&atts)override;
};

}
#endif
