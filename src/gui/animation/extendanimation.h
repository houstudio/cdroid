#ifndef __EXTEND_ANIMATION_H__
#define __EXTEND_ANIMATION_H__
#include <animation/animation.h>
#include <view/windowinsets.h>
namespace cdroid{
class ExtendAnimation:public Animation {
private:
    int mFromLeftType = ABSOLUTE;
    int mFromTopType = ABSOLUTE;
    int mFromRightType = ABSOLUTE;
    int mFromBottomType = ABSOLUTE;

    int mToLeftType = ABSOLUTE;
    int mToTopType = ABSOLUTE;
    int mToRightType = ABSOLUTE;
    int mToBottomType = ABSOLUTE;

    float mFromLeftValue;
    float mFromTopValue;
    float mFromRightValue;
    float mFromBottomValue;

    float mToLeftValue;
    float mToTopValue;
    float mToRightValue;
    float mToBottomValue;
protected:
    Insets mFromInsets;
    Insets mToInsets;
protected:
    void applyTransformation(float it, Transformation& tr) override;
public:
    /**
     * Constructor used when an ExtendAnimation is loaded from a resource.
     *
     * @param context Application context to use
     * @param attrs Attribute set from which to read values
     */
    ExtendAnimation(Context* context,const AttributeSet& attrs);

    /**
     * Constructor to use when building an ExtendAnimation from code
     *
     * @param fromInsets the insets to animate from
     * @param toInsets the insets to animate to
     */
    ExtendAnimation(const Insets& fromInsets,const Insets& toInsets);

    ExtendAnimation(int fromL, int fromT, int fromR, int fromB, int toL, int toT, int toR, int toB);


    bool willChangeTransformationMatrix() const override;

    int getExtensionEdges() const override;

    void initialize(int width, int height, int parentWidth, int parentHeight) override;
};
}/*endof namespace*/
#endif/*__EXTEND_ANIMATION_H__*/
