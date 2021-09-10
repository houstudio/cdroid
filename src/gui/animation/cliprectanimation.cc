#include <animation/cliprectanimation.h>
#include <core/rect.h>
namespace cdroid{

ClipRectAnimation::ClipRectAnimation(Context* context, const AttributeSet& attrs)
:Animation(context,attrs){
}

ClipRectAnimation::ClipRectAnimation(const Rect& fromClip,const Rect& toClip)
:Animation(){
    mFromLeftValue = fromClip.left;
    mFromTopValue = fromClip.top;
    mFromRightValue= fromClip.right();
    mFromBottomValue = fromClip.bottom();

    mToLeftValue = toClip.left;
    mToTopValue = toClip.top;
    mToRightValue= toClip.right();
    mToBottomValue = toClip.bottom();    
}

ClipRectAnimation::ClipRectAnimation(int fromL, int fromT, int fromR, int fromB,
        int toL, int toT, int toR, int toB)
: ClipRectAnimation(Rect::MakeLTRB(fromL, fromT, fromR, fromB),Rect::MakeLTRB(toL, toT, toR, toB)){
}

void ClipRectAnimation::applyTransformation(float it, Transformation& tr){
    int l = mFromRect.left + (int) ((mToRect.left - mFromRect.left) * it);
    int t = mFromRect.top + (int) ((mToRect.top - mFromRect.top) * it);
    int r = mFromRect.right() + (int) ((mToRect.right() - mFromRect.right()) * it);
    int b = mFromRect.bottom() + (int) ((mToRect.bottom() - mFromRect.bottom()) * it);
    tr.setClipRect(l, t, r, b);
}

bool ClipRectAnimation::willChangeTransformationMatrix()const{
    return false;
}

void ClipRectAnimation::initialize(int width, int height, int parentWidth, int parentHeight) {
     Animation::initialize(width, height, parentWidth, parentHeight);

     mFromRect.set((int) resolveSize(mFromLeftType, mFromLeftValue, width, parentWidth),
            (int) resolveSize(mFromTopType, mFromTopValue, height, parentHeight),
            (int) resolveSize(mFromRightType, mFromRightValue, width, parentWidth),
            (int) resolveSize(mFromBottomType, mFromBottomValue, height, parentHeight));
     mToRect.set((int) resolveSize(mToLeftType, mToLeftValue, width, parentWidth),
            (int) resolveSize(mToTopType, mToTopValue, height, parentHeight),
            (int) resolveSize(mToRightType, mToRightValue, width, parentWidth),
            (int) resolveSize(mToBottomType, mToBottomValue, height, parentHeight));
}

}
