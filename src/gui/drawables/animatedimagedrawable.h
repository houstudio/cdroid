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
#ifndef __ANIMATED_IMAGE_DRAWABLE_H__
#define __ANIMATED_IMAGE_DRAWABLE_H__
#include <drawables/drawable.h>
#include <core/handler.h>
namespace cdroid{
class FrameSequence;
class FrameSequenceState;
/*for drawing animated images (like GIFi/apng/webp)*/
class AnimatedImageDrawable:public Drawable,public Animatable2{
private:
    class AnimatedImageState:public std::enable_shared_from_this<AnimatedImageState>,public ConstantState{
    public:
        bool mAutoMirrored;
        int mFrameCount;
        int mRepeatCount;
        int mAlpha;
        FrameSequence*mFrameSequence;
        AnimatedImageState();
        AnimatedImageState(const AnimatedImageState& state);
        ~AnimatedImageState();
        AnimatedImageDrawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
    
    int mIntrinsicWidth;
    int mIntrinsicHeight;
    bool mStarting;
    bool mFrameScheduled;
    bool mMutated;
    int mCurrentFrame;
    int mNextFrame;
    int mFrameDelay;
    int mRepeatCount;
    int mRepeated;/*repeated played rounds*/
    float mAlpha;
    std::shared_ptr<AnimatedImageState> mAnimatedImageState;
    Cairo::RefPtr<Cairo::ImageSurface>mImage;
    FrameSequenceState*mFrameSequenceState;
    Runnable mRunnable;
    ColorFilter* mColorFilter;
    std::vector<Animatable2::AnimationCallback> mAnimationCallbacks;
    void* mImageHandler;
    void postOnAnimationStart();
    void postOnAnimationEnd();
    void updateStateFromTypedArray(const AttributeSet&atts,int srcDensityOverride);
    AnimatedImageDrawable(std::shared_ptr<AnimatedImageState> state);
protected:
    void onBoundsChange(const Rect& bounds)override;
public:
    static constexpr int REPEAT_INFINITE=-1;
    static constexpr int LOOP_INFINITE = REPEAT_INFINITE;
    static constexpr int REPEAT_UNDEFINED = -2;
public:
    AnimatedImageDrawable();
    AnimatedImageDrawable(cdroid::Context*,const std::string&res);
    ~AnimatedImageDrawable();
    std::shared_ptr<ConstantState>getConstantState()override;
    void setRepeatCount(int repeatCount);
    int getRepeatCount()const;
    int getIntrinsicWidth() override;
    int getIntrinsicHeight() override;
    void setAlpha(int alpha)override;
    int getAlpha()const override;
    int getOpacity()override;
    void setAutoMirrored(bool)override;
    bool isAutoMirrored()const override;
    bool onLayoutDirectionChanged(int layoutDirection)override;
    void draw(Canvas& canvas)override;
    bool isRunning()override;
    void start()override;
    void stop()override;
    void restart(int fromFrame=0);
    AnimatedImageDrawable*mutate()override;
    void registerAnimationCallback(const Animatable2::AnimationCallback& callback)override;
    bool unregisterAnimationCallback(const Animatable2::AnimationCallback& callback)override;
    void clearAnimationCallbacks();
    void inflate(XmlPullParser&,const AttributeSet&atts)override;
};

}//end namespace
#endif
