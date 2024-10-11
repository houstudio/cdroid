#include <animation/cliprectanimation.h>
#include <core/rect.h>
namespace cdroid{

ClipRectAnimation::ClipRectAnimation(const ClipRectAnimation&o):Animation(o){
    mFromLeftValue  = o.mFromLeftValue;
    mFromTopValue   = o.mFromTopValue;
    mFromRightValue = o.mFromRightValue;
    mFromBottomValue= o.mFromBottomValue;
    mToLeftValue    = o.mToLeftValue;
    mToTopValue     = o.mToTopValue;
    mToRightValue   = o.mToRightValue;
    mToBottomValue  = o.mToBottomValue;
}

ClipRectAnimation::ClipRectAnimation(Context* context, const AttributeSet& attrs)
:Animation(context,attrs){
    mFromLeftValue  = getPivotType(attrs.getString("fromLeft"),mFromLeftType);
    mFromTopValue   = getPivotType(attrs.getString("fromTop"),mFromTopType);
    mFromRightValue = getPivotType(attrs.getString("fromRight"),mFromRightType);
    mFromBottomValue= getPivotType(attrs.getString("fromBottom"),mFromBottomType);

    mToLeftValue  = getPivotType(attrs.getString("toLeft"),mToLeftType);
    mToTopValue   = getPivotType(attrs.getString("toTop"),mToTopType);
    mToRightValue = getPivotType(attrs.getString("toRight"),mToRightType);
    mToBottomValue= getPivotType(attrs.getString("tpBottom"),mToBottomType);
}

ClipRectAnimation::ClipRectAnimation(const Rect& fromClip,const Rect& toClip)
:Animation(){
    mFromLeftValue = (float)fromClip.left;
    mFromTopValue = (float)fromClip.top;
    mFromRightValue= (float)fromClip.right();
    mFromBottomValue = (float)fromClip.bottom();

    mToLeftValue = (float)toClip.left;
    mToTopValue = (float)toClip.top;
    mToRightValue= (float)toClip.right();
    mToBottomValue = (float)toClip.bottom();    
}

ClipRectAnimation::ClipRectAnimation(int fromL, int fromT, int fromR, int fromB,
        int toL, int toT, int toR, int toB)
: ClipRectAnimation(Rect::MakeLTRB(fromL, fromT, fromR, fromB),Rect::MakeLTRB(toL, toT, toR, toB)){
}

ClipRectAnimation* ClipRectAnimation::clone()const{
    return new ClipRectAnimation(*this);
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
