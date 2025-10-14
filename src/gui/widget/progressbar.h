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
#ifndef __PROGRESS_BAR_H__
#define __PROGRESS_BAR_H__
#include <view/view.h>
#include <core/sparsearray.h>
#include <animation/objectanimator.h>
#include <animation/alphaanimation.h>

namespace cdroid{

class ProgressBar:public View{
private:
    static constexpr int MAX_LEVEL = 10000;
    static constexpr int TIMEOUT_SEND_ACCESSIBILITY_EVENT = 200;
    static constexpr int PROGRESS_ANIM_DURATION = 80;
    friend class VISUAL_PROGRESS;    
    bool mAttached;
    float mVisualProgress;
    int mBehavior;
    int mDuration;
    bool mOnlyIndeterminate;
    Transformation* mTransformation;
    AlphaAnimation* mAnimation;
    bool mHasAnimation;
    bool mInDrawing;
    bool mRefreshIsPosted;
    SparseArray<class RefreshData*>mRefreshData;
    Runnable mRefreshProgressRunnable;
    Runnable mAccessibilityEventSender;
    Animator::AnimatorListener mAnimtorListener;
    bool mShouldStartAnimationDrawable;
    class ProgressTintInfo*mProgressTintInfo;
    ObjectAnimator*mLastProgressAnimator;
    bool mNoInvalidate;
    const Interpolator* mInterpolator;

    void initProgressBar();
    void swapCurrentDrawable(Drawable*d);
    void updateDrawableBounds(int width,int height);
    void updateDrawableState();
    void setVisualProgress(int id, float progress);

    static bool needsTileify(Drawable* dr);
    Drawable* tileify(Drawable* drawable, bool clip);
    Drawable* tileifyIndeterminate(Drawable* drawable);
    void applyProgressTints();
    void applyIndeterminateTint();
    void applyPrimaryProgressTint();
    void applyProgressBackgroundTint();
    void applySecondaryProgressTint();
    void scheduleAccessibilityEventSender();
    float getPercent(int progress)const;
    std::string formatStateDescription(int progress)const;
protected:
    static constexpr int HORIZONTAL= 0;
    static constexpr int VERTICAL  = 1;
    int mMin;
    int mMax;
    int mMinWidth;
    int mMaxWidth;
    int mMinHeight;
    int mMaxHeight;
    int mProgress;
    int mSecondaryProgress;
    int indeterminatePos;
    int mSampleWidth;
    bool mIndeterminate;
    bool mMirrorForRtl;
    bool mAggregatedIsVisible;
    Drawable*mCurrentDrawable;
    Drawable*mProgressDrawable;
    Drawable*mIndeterminateDrawable;
    void startAnimation();
    void stopAnimation();
    Drawable*getCurrentDrawable()const;
    int  getProgressGravity()const;
    int  getProgressOrientation()const;
    void drawableStateChanged()override;
    bool verifyDrawable(Drawable* who)const override;
    void jumpDrawablesToCurrentState()override;
    void onResolveDrawables(int layoutDirection)override;
    virtual void drawTrack(Canvas&canvas);
    void refreshProgress(int id, int progress, bool from,bool animate);
    void doRefreshProgress(int id, int progress, bool fromUser,bool callBackToApp, bool animate);
    void onVisibilityAggregated(bool isVisible)override;
    void invalidateDrawable(Drawable& dr)override;
    void onSizeChanged(int w,int h,int ow,int oh)override;
    virtual bool setProgressInternal(int progress, bool fromUser=false,bool animate=false);
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    virtual void onProgressRefresh(float scale, bool fromUser, int progress);
    virtual void onVisualProgressChanged(int id, float progress);
    virtual void onDraw(Canvas&canvas)override;
public:
    ProgressBar(int width, int height);
    ProgressBar(Context*ctx,const AttributeSet&attrs);
    ~ProgressBar()override;
    void setMin(int value);
    void setMax(int value);
    int getMin()const {return mMin;}
    int getMax()const {return mMax;}
    void setRange(int vmin,int vmax);

    void setProgress(int value);
    void setProgress(int progress, bool animate);
    int getProgress()const;
    void setSecondaryProgress(int secondaryProgress);
    int getSecondaryProgress()const;

    void setMinWidth(int minWidth);
    int  getMinWidth() const;
    void setMaxWidth(int maxWidth);
    int getMaxWidth() const;
    void setMinHeight(int minHeight);
    int getMinHeight() const;
    void setMaxHeight(int maxHeight);
    int getMaxHeight() const;

    void incrementProgressBy(int diff);
    void incrementSecondaryProgressBy(int diff);
    bool isIndeterminate()const;
    void setIndeterminate(bool indeterminate);
    void setMirrorForRtl(bool mirrorRtl);
    bool getMirrorForRtl()const;
    void setProgressDrawable(Drawable*d);
    Drawable*getProgressDrawable()const;
    void setIndeterminateDrawable(Drawable*d);
    void setIndeterminateDrawableTiled(Drawable* d);
    Drawable*getIndeterminateDrawable()const;

    void setProgressTintList(const ColorStateList*tint);
    const ColorStateList*getProgressTintList()const;
    void setProgressTintMode(int);
    int getProgressTintMode()const;
    void setProgressBackgroundTintList(const ColorStateList*tint);
    const ColorStateList*getProgressBackgroundTintList()const;
    void setProgressBackgroundTintMode(int);
    int getProgressBackgroundTintMode()const;
    void setSecondaryProgressTintList(const ColorStateList*tintt);
    const ColorStateList*getSecondaryProgressTintList()const;
    void setSecondaryProgressTintMode(int tintMode);
    int getSecondaryProgressTintMode()const;
    Drawable* getTintTarget(int layerId, bool shouldFallback);
    void setProgressDrawableTiled(Drawable* d);
    void drawableHotspotChanged(float x, float y)override;
    void setStateDescription(const std::string& stateDescription)override;
    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityEventInternal(AccessibilityEvent& event)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
};

}
#endif
