#ifndef __ANIMATIONSET_H__
#define __ANIMATIONSET_H__
#include <animation/animation.h>

namespace cdroid{

class AnimationSet:public Animation{
private:
    int mFlags = 0;
    bool mDirty;
    bool mHasAlpha;
    std::vector<Animation*> mAnimations ;
    long mLastEnd;
    std::vector<long>mStoredOffsets;
 
    void setFlag(int mask, bool value);
    void init();
protected:
    AnimationSet(const AnimationSet&);
public:
    AnimationSet(Context* context,const AttributeSet& attrs);
    AnimationSet(bool shareInterpolator);
    ~AnimationSet()override;
    AnimationSet* clone()const override;
    void setFillAfter(bool fillAfter);
    void setFillBefore(bool fillBefore);
    void setRepeatMode(int repeatMode);
    void setStartOffset(long startOffset);
    bool hasAlpha() override;
    void setDuration(long durationMillis)override;
    void addAnimation(Animation* a);
    void setStartTime(int64_t startTimeMillis)override;
    int64_t getStartTime()const override;
    void restrictDuration(long durationMillis)override;
    long getDuration()const override;
    long computeDurationHint()override;
    void initializeInvalidateRegion(int left, int top, int right, int bottom);
    bool getTransformation(int64_t currentTime, Transformation& t)override;
    void scaleCurrentDuration(float scale);
    void initialize(int width, int height, int parentWidth, int parentHeight)override;
    void reset()override;
    void restoreChildrenStartOffset();
    std::vector<Animation*> getAnimations();
    bool willChangeTransformationMatrix()const override;
    bool willChangeBounds()const override;
};

}//endof namespace
#endif //__ANIMATIONSET_H__
