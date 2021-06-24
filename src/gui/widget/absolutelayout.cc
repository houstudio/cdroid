#include <widget/absolutelayout.h>
namespace cdroid{

AbsoluteLayoutParams::AbsoluteLayoutParams(int width, int height, int x, int y)
    :LayoutParams(width,height){
    this->x=x;
    this->y=y;
}

AbsoluteLayoutParams::AbsoluteLayoutParams(Context* c,const AttributeSet& attrs)
    :LayoutParams(c,attrs){
    x=attrs.getDimensionPixelOffset("layout_x");
    y=attrs.getDimensionPixelOffset("layout_y");
}

AbsoluteLayoutParams::AbsoluteLayoutParams(const LayoutParams& source)
    :LayoutParams(source){
    //NOTHING
}

////////////////////////////////////////////////////////////////////////////////////////////

AbsoluteLayout::AbsoluteLayout(int w,int h):ViewGroup(w,h){
}

AbsoluteLayout::AbsoluteLayout(Context* context,const AttributeSet& attrs)
    :ViewGroup(context,attrs){
}

void AbsoluteLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
     int count = getChildCount();

    int maxHeight = 0;
    int maxWidth = 0;

    // Find out how big everyone wants to be
    measureChildren(widthMeasureSpec, heightMeasureSpec);

    // Find rightmost and bottom-most child
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            int childRight;
            int childBottom;

            AbsoluteLayoutParams* lp = (AbsoluteLayoutParams*) child->getLayoutParams();

            childRight = lp->x + child->getMeasuredWidth();
            childBottom= lp->y + child->getMeasuredHeight();

            maxWidth = std::max(maxWidth, childRight);
            maxHeight= std::max(maxHeight, childBottom);
        }
    }

    // Account for padding too
    maxWidth += mPaddingLeft + mPaddingRight;
    maxHeight += mPaddingTop + mPaddingBottom;

    // Check against minimum height and width
    maxHeight= std::max(maxHeight, getSuggestedMinimumHeight());
    maxWidth = std::max(maxWidth, getSuggestedMinimumWidth());
        
    setMeasuredDimension(resolveSizeAndState(maxWidth, widthMeasureSpec, 0),
        resolveSizeAndState(maxHeight, heightMeasureSpec, 0));
}

LayoutParams* AbsoluteLayout::generateDefaultLayoutParams()const{
    return new AbsoluteLayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT, 0, 0);
}

LayoutParams* AbsoluteLayout::generateLayoutParams(const AttributeSet& attrs)const {
    return new AbsoluteLayoutParams(getContext(),attrs);
}

bool AbsoluteLayout::checkLayoutParams(const LayoutParams* p)const {
    return dynamic_cast<const AbsoluteLayoutParams*>(p);
}

LayoutParams* AbsoluteLayout::generateLayoutParams(const LayoutParams* p)const {
    return new LayoutParams(*p);
}

void AbsoluteLayout::onLayout(bool changed, int l, int t,int w, int h){
    int count = getChildCount();

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            AbsoluteLayoutParams* lp =(AbsoluteLayoutParams*) child->getLayoutParams();

            int childLeft = mPaddingLeft + lp->x;
            int childTop = mPaddingTop + lp->y;
            child->layout(childLeft, childTop,child->getMeasuredWidth(),child->getMeasuredHeight());
        }
    }
}

}
