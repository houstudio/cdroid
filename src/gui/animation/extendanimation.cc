#include <animation/extendanimation.h>

namespace cdroid{

ExtendAnimation::ExtendAnimation(Context* context,const AttributeSet& attrs)
    :Animation(context, attrs){
    mFromLeftValue = getPivotType(attrs.getString("fromExtendLeft"), mFromLeftType);

    mFromTopValue  = getPivotType(attrs.getString("fromExtendTop") , mFromTopType);

    mFromRightValue= getPivotType(attrs.getString("fromExtendRight"),mFromRightType);

    mFromBottomValue=getPivotType(attrs.getString("fromExtendBottom"),mFromBottomType);

    mToLeftValue = getPivotType(attrs.getString("toExtendLeft"), mToLeftType);

    mToTopValue  = getPivotType(attrs.getString("toExtendTop") , mToTopType);

    mToRightValue= getPivotType(attrs.getString("toExtendRight"),mToRightType);

    mToBottomValue=getPivotType(attrs.getString("toExtendBottom"),mToBottomType);
}

/**
 * Constructor to use when building an ExtendAnimation from code
 *
 * @param fromInsets the insets to animate from
 * @param toInsets the insets to animate to
 */
ExtendAnimation::ExtendAnimation(const Insets& fromInsets,const Insets& toInsets) {
    mFromLeftValue = -fromInsets.left;
    mFromTopValue  = -fromInsets.top;
    mFromRightValue= -fromInsets.right;
    mFromBottomValue= -fromInsets.bottom;

    mToLeftValue = -toInsets.left;
    mToTopValue  = -toInsets.top;
    mToRightValue = -toInsets.right;
    mToBottomValue= -toInsets.bottom;
}

/**
 * Constructor to use when building an ExtendAnimation from code
 */
ExtendAnimation::ExtendAnimation(int fromL, int fromT, int fromR, int fromB,int toL, int toT, int toR, int toB)
    :ExtendAnimation(Insets::of(-fromL, -fromT, -fromR, -fromB), Insets::of(-toL, -toT, -toR, -toB)){
}

void ExtendAnimation::ExtendAnimation::applyTransformation(float it, Transformation& tr) {
    int l = mFromInsets.left + (int) ((mToInsets.left - mFromInsets.left) * it);
    int t = mFromInsets.top + (int) ((mToInsets.top - mFromInsets.top) * it);
    int r = mFromInsets.right + (int) ((mToInsets.right - mFromInsets.right) * it);
    int b = mFromInsets.bottom + (int) ((mToInsets.bottom - mFromInsets.bottom) * it);
    tr.setInsets(l, t, r, b);
}

bool ExtendAnimation::willChangeTransformationMatrix() const{
    return false;
}

int ExtendAnimation::getExtensionEdges() const{
    return (mFromInsets.left < 0 || mToInsets.left < 0 ?  WindowInsets::Side::LEFT : 0)
        | (mFromInsets.right < 0 || mToInsets.right < 0 ?  WindowInsets::Side::RIGHT : 0)
        | (mFromInsets.top < 0 || mToInsets.top < 0 ?  WindowInsets::Side::TOP : 0)
        | (mFromInsets.bottom < 0 || mToInsets.bottom < 0 ? WindowInsets::Side::BOTTOM : 0);
}

void ExtendAnimation::initialize(int width, int height, int parentWidth, int parentHeight) {
    Animation::initialize(width, height, parentWidth, parentHeight);
    // We remove any negative extension (i.e. positive insets) and set those to 0
    mFromInsets = Insets::min(Insets::of(
                -(int) resolveSize(mFromLeftType, mFromLeftValue, width, parentWidth),
                -(int) resolveSize(mFromTopType, mFromTopValue, height, parentHeight),
                -(int) resolveSize(mFromRightType, mFromRightValue, width, parentWidth),
                -(int) resolveSize(mFromBottomType, mFromBottomValue, height, parentHeight)
            ), Insets::NONE);
    mToInsets = Insets::min(Insets::of(
                -(int) resolveSize(mToLeftType, mToLeftValue, width, parentWidth),
                -(int) resolveSize(mToTopType, mToTopValue, height, parentHeight),
                -(int) resolveSize(mToRightType, mToRightValue, width, parentWidth),
                -(int) resolveSize(mToBottomType, mToBottomValue, height, parentHeight)
            ), Insets::NONE);
}
}/*endof namespace*/
