#ifndef __ALPHA_ANIMAION_H__
#define __ALPHA_ANIMAION_H__
#include <animation/animation.h>
namespace cdroid{

class AlphaAnimation:public Animation{
private:
    float mFromAlpha;
    float mToAlpha;
protected:
    AlphaAnimation(const AlphaAnimation&other);
    void applyTransformation(float interpolatedTime, Transformation& t)override;
public:
    AlphaAnimation(Context* context,const AttributeSet& attrs);
    AlphaAnimation(float fromAlpha, float toAlpha);
    bool willChangeTransformationMatrix()const override;
    bool willChangeBounds()const override;
    bool hasAlpha() override;
    Animation*clone()override;
};
}
#endif//__ALPHA_ANIMAION_H__
