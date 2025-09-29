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
#ifndef __ANIMATIONSET_H__
#define __ANIMATIONSET_H__
#include <animation/animation.h>

namespace cdroid{

class AnimationSet:public Animation{
private:
    int mFlags = 0;
    bool mDirty;
    bool mHasAlpha;
    std::vector<Animation*> mAnimations;
    int64_t mLastEnd;
    std::vector<int64_t>mStoredOffsets;
 
    void setFlag(int mask, bool value);
    void init();
protected:
    AnimationSet(const AnimationSet&);
public:
    AnimationSet(Context* context,const AttributeSet& attrs);
    AnimationSet(bool shareInterpolator);
    ~AnimationSet()override;
    AnimationSet* clone()const override;
    void setFillAfter(bool fillAfter)override;
    void setFillBefore(bool fillBefore)override;
    void setRepeatMode(int repeatMode)override;
    void setStartOffset(int64_t startOffset)override;
    bool hasAlpha() override;
    void setDuration(int64_t durationMillis)override;
    void addAnimation(Animation* a);
    void setStartTime(int64_t startTimeMillis)override;
    int64_t getStartTime()const override;
    void restrictDuration(int64_t durationMillis)override;
    int64_t getDuration()const override;
    int64_t computeDurationHint()override;
    void initializeInvalidateRegion(int left, int top, int right, int bottom);
    bool getTransformation(int64_t currentTime, Transformation& t)override;
    void scaleCurrentDuration(float scale)override;
    void initialize(int width, int height, int parentWidth, int parentHeight)override;
    void reset()override;
    void restoreChildrenStartOffset();
    std::vector<Animation*> getAnimations();
    bool willChangeTransformationMatrix()const override;
    bool willChangeBounds()const override;
};

}//endof namespace
#endif //__ANIMATIONSET_H__
