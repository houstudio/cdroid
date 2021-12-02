#include <widget/framelayout.h>
#include <widget/measurespec.h>
#include <cdlog.h>


#define DEFAULT_CHILD_GRAVITY (Gravity::TOP | Gravity::START)
namespace cdroid{

DECLARE_WIDGET(FrameLayout)

FrameLayout::FrameLayout(int w,int h):ViewGroup(w,h){
    mMeasureAllChildren=false;
    mForegroundPaddingLeft=mForegroundPaddingRight=0;
    mForegroundPaddingTop =mForegroundPaddingBottom=0;
}

FrameLayout::FrameLayout(Context* context,const AttributeSet& attrs)
    :ViewGroup(context,attrs){
    mMeasureAllChildren=false;
    mForegroundPaddingLeft=mForegroundPaddingRight=0;
    mForegroundPaddingTop =mForegroundPaddingBottom=0;
}
//@android.view.RemotableViewMethod
void FrameLayout::setForegroundGravity(int foregroundGravity){
    #if 0
    if (getForegroundGravity() != foregroundGravity) {
        ViewGroup::setForegroundGravity(foregroundGravity);
        // calling get* again here because the set above may apply default constraints
        Drawable* foreground = getForeground();
        if (getForegroundGravity() == Gravity.FILL && foreground != null) {
            Rect padding;
            if (foreground.getPadding(padding)) {
                mForegroundPaddingLeft = padding.x;
                mForegroundPaddingTop = padding.y;
                mForegroundPaddingRight = padding.right();
                mForegroundPaddingBottom = padding.bottom();
            }
        } else {
            mForegroundPaddingLeft = 0;
            mForegroundPaddingTop = 0;
            mForegroundPaddingRight = 0;
            mForegroundPaddingBottom = 0;
        }
        requestLayout();
    }
    #endif
}

LayoutParams* FrameLayout::generateDefaultLayoutParams()const {
    return new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT);
}

LayoutParams* FrameLayout::generateLayoutParams(const AttributeSet& attrs)const {
    return new FrameLayoutParams(getContext(), attrs);
}

bool FrameLayout::checkLayoutParams(const LayoutParams* p)const{
    return dynamic_cast<const FrameLayoutParams*>(p);
}

LayoutParams* FrameLayout::generateLayoutParams(const LayoutParams* lp)const {
    if (true/*sPreserveMarginParamsInLayoutParamConversion*/) {
        if (dynamic_cast<const FrameLayoutParams*>(lp)) {
            return new LayoutParams(*(FrameLayoutParams*) lp);
        } else if (dynamic_cast<const MarginLayoutParams*>(lp)) {
            return new LayoutParams(*(MarginLayoutParams*)lp);
        }
    }
    return new FrameLayoutParams(*lp);
}

int FrameLayout::getPaddingLeftWithForeground() {
    return isForegroundInsidePadding() ? std::max(mPaddingLeft, mForegroundPaddingLeft) :
        mPaddingLeft + mForegroundPaddingLeft;
}

int FrameLayout::getPaddingRightWithForeground() {
    return isForegroundInsidePadding() ? std::max(mPaddingRight, mForegroundPaddingRight) :
        mPaddingRight + mForegroundPaddingRight;
}

int FrameLayout::getPaddingTopWithForeground() {
    return isForegroundInsidePadding() ? std::max(mPaddingTop, mForegroundPaddingTop) :
            mPaddingTop + mForegroundPaddingTop;
}

int FrameLayout::getPaddingBottomWithForeground() {
    return isForegroundInsidePadding() ? std::max(mPaddingBottom, mForegroundPaddingBottom) :
        mPaddingBottom + mForegroundPaddingBottom;
}

void FrameLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int count = getChildCount();

    bool measureMatchParentChildren =MeasureSpec::getMode(widthMeasureSpec) != MeasureSpec::EXACTLY ||
        MeasureSpec::getMode(heightMeasureSpec) != MeasureSpec::EXACTLY;
    mMatchParentChildren.clear();

    int maxHeight = 0;
    int maxWidth = 0;
    int childState = 0;

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (mMeasureAllChildren || child->getVisibility() != GONE) {
            measureChildWithMargins(child, widthMeasureSpec, 0, heightMeasureSpec, 0);
            FrameLayoutParams* lp = (FrameLayoutParams*) child->getLayoutParams();
            maxWidth = std::max(maxWidth,child->getMeasuredWidth() + lp->leftMargin + lp->rightMargin);
            maxHeight = std::max(maxHeight,child->getMeasuredHeight() + lp->topMargin + lp->bottomMargin);
            LOGV("%p margin:%d,%d-%d,%d size:%dx%d",child,lp->leftMargin,lp->topMargin,lp->rightMargin,lp->bottomMargin,maxWidth,maxHeight);
            childState = combineMeasuredStates(childState, child->getMeasuredState());
            if (measureMatchParentChildren) {
                if (lp->width == LayoutParams::MATCH_PARENT ||
                    lp->height == LayoutParams::MATCH_PARENT) {
                    mMatchParentChildren.push_back(child);
                }
            }
        }
    }

    // Account for padding too
    maxWidth += getPaddingLeftWithForeground() + getPaddingRightWithForeground();
    maxHeight += getPaddingTopWithForeground() + getPaddingBottomWithForeground();

    // Check against our minimum height and width
    maxHeight= std::max(maxHeight, getSuggestedMinimumHeight());
    maxWidth = std::max(maxWidth, getSuggestedMinimumWidth());

    // Check against our foreground's minimum height and width
    Drawable* drawable = getForeground();
    if (drawable != nullptr) {
        maxHeight = std::max(maxHeight, drawable->getMinimumHeight());
        maxWidth = std::max(maxWidth, drawable->getMinimumWidth());
    }

    setMeasuredDimension(resolveSizeAndState(maxWidth, widthMeasureSpec, childState),
                resolveSizeAndState(maxHeight, heightMeasureSpec,childState << MEASURED_HEIGHT_STATE_SHIFT));

    count = mMatchParentChildren.size();
    for (int i = 0; i < count; i++) {
        View* child = mMatchParentChildren.at(i);
        MarginLayoutParams* lp = (MarginLayoutParams*) child->getLayoutParams();

        int childWidthMeasureSpec;
        if (lp->width == LayoutParams::MATCH_PARENT) {
            int width = std::max(0, getMeasuredWidth()
                        - getPaddingLeftWithForeground() - getPaddingRightWithForeground()
                        - lp->leftMargin - lp->rightMargin);
            childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(width, MeasureSpec::EXACTLY);
        } else {
            childWidthMeasureSpec = getChildMeasureSpec(widthMeasureSpec,
                        getPaddingLeftWithForeground() + getPaddingRightWithForeground() +
                        lp->leftMargin + lp->rightMargin,lp->width);
        }

        int childHeightMeasureSpec;
        if (lp->height == LayoutParams::MATCH_PARENT) {
            int height = std::max(0, getMeasuredHeight()
                        - getPaddingTopWithForeground() - getPaddingBottomWithForeground()
                        - lp->topMargin - lp->bottomMargin);
            childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(height, MeasureSpec::EXACTLY);
        } else {
            childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec,
                        getPaddingTopWithForeground() + getPaddingBottomWithForeground() +
                        lp->topMargin + lp->bottomMargin,lp->height);
        }

        child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
    }
}

void FrameLayout::onLayout(bool changed, int left, int top, int width, int height) {
    LOGV("FrameLayout::onLayout %d,%d,%d,%d",left,top,width,height);
    layoutChildren(left, top, width, height, false /* no force left gravity */);
}

void FrameLayout::layoutChildren(int left, int top, int right, int bottom, bool forceLeftGravity) {
    int count = getChildCount();

    int parentLeft = getPaddingLeftWithForeground();
    int parentRight = right - left - getPaddingRightWithForeground();

    int parentTop = getPaddingTopWithForeground();
    int parentBottom = bottom - top - getPaddingBottomWithForeground();

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            FrameLayoutParams* lp = (FrameLayoutParams*) child->getLayoutParams();

            int width = child->getMeasuredWidth();
            int height = child->getMeasuredHeight();

            int childLeft;
            int childTop;

            int gravity = lp->gravity;
            if (gravity == -1) {
                gravity = DEFAULT_CHILD_GRAVITY;
            }

            int layoutDirection = getLayoutDirection();
            int absoluteGravity = Gravity::getAbsoluteGravity(gravity, layoutDirection);
            int verticalGravity = gravity & Gravity::VERTICAL_GRAVITY_MASK;

            switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
            case Gravity::CENTER_HORIZONTAL:
                childLeft = parentLeft + (parentRight - parentLeft - width) / 2 +
                    lp->leftMargin - lp->rightMargin;
                break;
            case Gravity::RIGHT:
                if (!forceLeftGravity) {
                    childLeft = parentRight - width - lp->rightMargin;
                    break;
                }
            case Gravity::LEFT:
            default: childLeft = parentLeft + lp->leftMargin;
            }

            switch (verticalGravity) {
            case Gravity::TOP:
                childTop = parentTop + lp->topMargin;
                break;
            case Gravity::CENTER_VERTICAL:
                childTop = parentTop + (parentBottom - parentTop - height) / 2 +
                    lp->topMargin - lp->bottomMargin;
                break;
            case Gravity::BOTTOM:
                childTop = parentBottom - height - lp->bottomMargin;
                break;
            default:  childTop = parentTop + lp->topMargin;
            }

            LOGV("child %p marin: %d,%d,%d,%d bounds:%d,%d-%d,%d",child,lp->leftMargin,lp->topMargin ,
                        lp->rightMargin,lp->bottomMargin,childLeft, childTop, width,height);
            child->layout(childLeft, childTop, width,height);
        }
    }
}

void FrameLayout::setMeasureAllChildren(bool measureAll) {
    mMeasureAllChildren = measureAll;
}

bool FrameLayout::getMeasureAllChildren()const {
    return mMeasureAllChildren;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FrameLayoutParams::FrameLayoutParams(Context* c,const AttributeSet& attrs)
    :MarginLayoutParams(c,attrs){
    gravity=attrs.getGravity("layout_gravity",UNSPECIFIED_GRAVITY);
}

FrameLayoutParams::FrameLayoutParams(int width, int height)
    :FrameLayoutParams(width,height,UNSPECIFIED_GRAVITY){
}

FrameLayoutParams::FrameLayoutParams(int width, int height, int gravity)
    :MarginLayoutParams(width,height){
    this->gravity=gravity;
}

FrameLayoutParams::FrameLayoutParams(const LayoutParams& source)
    :MarginLayoutParams(source){
    gravity=UNSPECIFIED_GRAVITY;
}

FrameLayoutParams::FrameLayoutParams(const MarginLayoutParams& source)
    :MarginLayoutParams(source){
    gravity= UNSPECIFIED_GRAVITY;
}

FrameLayoutParams::FrameLayoutParams(const FrameLayoutParams& source)
    :MarginLayoutParams(source){
    this->gravity = source.gravity;;
}

}
