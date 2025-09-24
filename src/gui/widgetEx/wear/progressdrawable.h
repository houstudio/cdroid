#ifndef __PROGRESS_DRAWABLE_H__
#define __PROGRESS_DRAWABLE_H__
#include <drawables/drawable.h>
namespace cdroid{
class ObjectAnimator;
class ProgressDrawable:public Drawable {
private:
    static constexpr int MAX_LEVEL = 10000;

    static constexpr int NUMBER_OF_SEGMENTS = 5;

    static constexpr int LEVELS_PER_SEGMENT = MAX_LEVEL / NUMBER_OF_SEGMENTS;
    static constexpr float STARTING_ANGLE = -90.f;
    static constexpr long ANIMATION_DURATION = 6000;
    static constexpr int FULL_CIRCLE = 360;
    static constexpr int MAX_SWEEP = 306;
    static constexpr int CORRECTION_ANGLE = FULL_CIRCLE - MAX_SWEEP;
    /**
     * How far through each cycle does the bar stop growing and start shrinking, half way. *
     */
    static constexpr float GROW_SHRINK_RATIO = 0.5f;

    RectF mInnerCircleBounds;
    ObjectAnimator* mAnimator;
    float mCircleBorderWidth;
    int mCircleBorderColor;
private:    
    static float lerpInv(float a, float b, float value);
protected:
    bool onLevelChange(int level)override;;
public:
    ProgressDrawable();

    void setRingColor(int color);

    void setRingWidth(float width);

    void startAnimation();
    void stopAnimation();

    void draw(Canvas& canvas) override;

    void setAlpha(int i);

    void setColorFilter(ColorFilter* colorFilter)override;

    int getOpacity() override;
};
}/*endof namespace*/
#endif/*__PROGRESS_DRAWABLE_H__*/
