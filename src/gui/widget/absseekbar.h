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
#ifndef __ABS_SEEK_BAR_H__
#define __ABS_SEEK_BAR_H__
#include <widget/progressbar.h>

namespace cdroid{

class AbsSeekBar:public ProgressBar{
private:
    bool mSplitTrack;
    bool mHasThumbTint;
    bool mHasThumbTintMode;
    bool mIsDragging;
    bool mHasTickMarkTint = false;
    bool mHasTickMarkTintMode = false;
    ColorStateList*mThumbTintList;
    ColorStateList*mTickMarkTintList;

    float mDisabledAlpha;
    float mTouchDownX;
    float mTouchDownY;
    int  mScaledTouchSlop;
    int  mTouchThumbOffset;
    int  mTickMarkTintMode;
    int  mThumbExclusionMaxSize;
    Rect mThumbRect;
    std::vector<Rect>mUserGestureExclusionRects;
    std::vector<Rect>mGestureExclusionRects;
    void initSeekBar();
    float getScale()const;
    void startDrag(MotionEvent& event);
    void attemptClaimDrag();
    void applyThumbTint();
    void updateThumbAndTrackPos(int w, int h);
    void setThumbPos(int w, Drawable* thumb, float scale, int offset);
    void updateGestureExclusionRects();
    void applyTickMarkTint();
protected:
    int mKeyProgressIncrement;
    int mThumbOffset;
    bool mIsUserSeekable;
    float mTouchProgressOffset;
    Drawable*mThumb;
    Drawable*mTickMark;
    void setHotspot(float x,float y);
    void trackTouchEvent(MotionEvent& event);
    virtual void onStartTrackingTouch();
    virtual void onStopTrackingTouch();
    virtual void onKeyChange();
    void onSizeChanged(int w,int h,int oldw,int oldh)override;
    bool verifyDrawable(Drawable* who)const override;
    virtual void drawThumb(Canvas&canvas);
    void drawTrack(Canvas&canvas)override;
    virtual void drawTickMarks(Canvas&canvas);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onVisualProgressChanged(int id, float scale)override;
    void drawableStateChanged()override;

    void onDraw(Canvas&canvas)override;
    bool onKeyDown(int keycode,KeyEvent&event)override;
    bool onTouchEvent(MotionEvent& event)override;
public:
    AbsSeekBar(int w,int h);
    AbsSeekBar(Context*ctx,const AttributeSet&attrs);
    ~AbsSeekBar();
    void setKeyProgressIncrement(int increment);
    int getKeyProgressIncrement()const;
    void setThumbOffset(int thumbOffset);
    int getThumbOffset()const;
    Drawable*getThumb()const;
    void setThumb(Drawable*thumb);
    void setTickMark(Drawable* tickMark);
    Drawable* getTickMark()const;
    virtual void setMin(int min);
    virtual void setMax(int max);
    void jumpDrawablesToCurrentState()override;
    virtual bool canUserSetProgress()const;
    void onRtlPropertiesChanged(int layoutDirection)override;
    void drawableHotspotChanged(float x,float y)override;
    void setSystemGestureExclusionRects(const std::vector<Rect>&rects)override;
    void growRectTo(Rect& r, int minimumSize);
    void onResolveDrawables(int layoutDirection)override;
    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    bool performAccessibilityActionInternal(int action, Bundle* arguments)override;
};

}//namespace


#endif
