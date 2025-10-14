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
#include <widget/linearlayout.h>
#include <cdlog.h>
 
namespace cdroid {

DECLARE_WIDGET(LinearLayout)

LinearLayout::LayoutParams::LayoutParams(Context* c,const AttributeSet&attrs)
    :ViewGroup::MarginLayoutParams(c,attrs){
    weight = attrs.getFloat("layout_weight", 0);
    gravity= attrs.getGravity("layout_gravity", -1);
    LOGV("width=%d,height=%d weight=%.2f gravity=%x margin=%d,%d,%d,%d",width,height,
	    weight,gravity,topMargin,bottomMargin,leftMargin,rightMargin);
}

LinearLayout::LayoutParams::LayoutParams(int width, int height)
    :ViewGroup::MarginLayoutParams(width,height){
    weight  = 0;
    gravity = -1;
}

LinearLayout::LayoutParams::LayoutParams(int width, int height, float weight)
    :ViewGroup::MarginLayoutParams(width,height){
    this->weight = weight;
    gravity = -1;
}

LinearLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& p)
    :ViewGroup::MarginLayoutParams(p){
    this->weight = .0f;
    gravity = -1;
}

LinearLayout::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams&source)
    :ViewGroup::MarginLayoutParams(source){
    this->weight = .0f;
    gravity = -1;
}

LinearLayout::LayoutParams::LayoutParams(const LayoutParams&source)
    :ViewGroup::MarginLayoutParams(source){
    weight = source.weight;
    gravity= source.gravity;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LinearLayout::LinearLayout(int w,int h):LinearLayout(0,0,w,h){
    initView();
}

LinearLayout::LinearLayout(int x,int y,int w,int h)
  :ViewGroup(x,y,w,h){
    initView();
}

void LinearLayout::initView(){
    mWeightSum   = -1.0;
    mOrientation = HORIZONTAL;
    mGravity     = Gravity::START | Gravity::TOP;
    mTotalLength = 0;
    mDividerPadding = 0;
    mBaselineChildTop = 0;
    mDivider = nullptr;
    mShowDividers= SHOW_DIVIDER_NONE;
    mDividerWidth= mDividerHeight = 0;
    mUseLargestChild = false;
    mBaselineAligned = false;
    mBaselineAlignedChildIndex   = -1;
    mLayoutDirection = View::LAYOUT_DIRECTION_UNDEFINED;
    mAllowInconsistentMeasurement= false;//version <= Build::VERSION_CODES::M;
}

LinearLayout::LinearLayout(Context* context,const AttributeSet& attrs)
  :ViewGroup(context,attrs){
    initView();

    int index = attrs.getInt("orientation",std::unordered_map<std::string,int>{
             {"horizontal",LinearLayout::HORIZONTAL},
             {"vertical",LinearLayout::VERTICAL} },-1);
    if(index>=0)setOrientation(index);
    index = attrs.getGravity("gravity",-1);
    if(index>=0)setGravity(index);

    const bool baselineAligned=attrs.getBoolean("baselineAligned",true);
    if(baselineAligned)setBaselineAligned(baselineAligned);

    mWeightSum = attrs.getFloat("weightSum",-1.f);
    mBaselineAlignedChildIndex=attrs.getInt("baselineAlignedChildIndex",-1);
    mUseLargestChild = attrs.getBoolean("measureWithLargestChild",false);

    mShowDividers = attrs.getInt("showDividers",std::unordered_map<std::string,int>{
	   {"none",SHOW_DIVIDER_NONE},
	   {"beginning",SHOW_DIVIDER_BEGINNING},
	   {"middle",SHOW_DIVIDER_MIDDLE},
	   {"end",SHOW_DIVIDER_END} },SHOW_DIVIDER_NONE);
    mDividerPadding = attrs.getInt("dividerPadding",0);
    setDividerDrawable(attrs.getDrawable("divider"));
}

LinearLayout::~LinearLayout() {
    delete mDivider;
}

int LinearLayout::getBaseline(){
    if (mBaselineAlignedChildIndex < 0) {
        return ViewGroup::getBaseline();
    }

    if (getChildCount() <= mBaselineAlignedChildIndex) {
        LOGD("mBaselineAlignedChildIndex of LinearLayout set to an index that is out of bounds.");
    }

    View* child = getChildAt(mBaselineAlignedChildIndex);
    const int childBaseline = child->getBaseline();

    if (childBaseline == -1) {
        if (mBaselineAlignedChildIndex == 0) {
            // this is just the default case, safe to return -1
            return -1;
        }
        // the user picked an index that points to something that doesn't
        // know how to calculate its baseline.
        LOGD("mBaselineAlignedChildIndex of LinearLayout points to a View that doesn't know how to get its baseline.");
    }

    // TODO: This should try to take into account the virtual offsets
    // (See getNextLocationOffset and getLocationOffset)
    // We should add to childTop:
    // sum([getNextLocationOffset(getChildAt(i)) / i < mBaselineAlignedChildIndex])
    // and also add:
    // getLocationOffset(child)
    int childTop = mBaselineChildTop;

    if (mOrientation == VERTICAL) {
        const int majorGravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;
        if (majorGravity != Gravity::TOP) {
            switch (majorGravity) {
            case Gravity::BOTTOM:
                childTop = getHeight() - mPaddingBottom - mTotalLength;
                break;

            case Gravity::CENTER_VERTICAL:
                childTop += ((getHeight() - mPaddingTop - mPaddingBottom) -mTotalLength) / 2;
                break;
            }
        }
    }

    const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    return childTop + lp->topMargin + childBaseline;
}

std::string LinearLayout::getAccessibilityClassName()const{
    return "LinearLayout";
}

bool LinearLayout::isBaselineAligned()const {
    return mBaselineAligned;
}

void LinearLayout::setBaselineAligned(bool baselineAligned) {
    mBaselineAligned = baselineAligned;
}

bool LinearLayout::isMeasureWithLargestChildEnabled()const{
    return mUseLargestChild;
}

void LinearLayout::setMeasureWithLargestChildEnabled(bool enabled) {
    mUseLargestChild = enabled;
}

bool LinearLayout::isShowingDividers()const{
    return (mShowDividers != SHOW_DIVIDER_NONE) && (mDivider != nullptr);
}

void LinearLayout::setShowDividers(int showDividers) {
    if (showDividers == mShowDividers)
        return;
    mShowDividers = showDividers;

    setWillNotDraw(!isShowingDividers());
    requestLayout();
}

int LinearLayout::getShowDividers()const{
   return mShowDividers;
}

LinearLayout::LayoutParams* LinearLayout::generateDefaultLayoutParams()const{
    if (mOrientation == HORIZONTAL) {
        return new LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    } else if (mOrientation == VERTICAL) {
        return new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT);
    }
    return nullptr;
}

LinearLayout::LayoutParams* LinearLayout::generateLayoutParams(const ViewGroup::LayoutParams* lp)const{
    if (sPreserveMarginParamsInLayoutParamConversion) {
        if (dynamic_cast<const LayoutParams*>(lp)) {
            return new LayoutParams((const LayoutParams&)*lp);
        } else if (dynamic_cast<const MarginLayoutParams*>(lp)) {
            return new LayoutParams((const MarginLayoutParams&)*lp);
        }
    }
    return new LayoutParams(*lp);
}

LinearLayout::LayoutParams* LinearLayout::generateLayoutParams(const AttributeSet&atts)const{
    return new LayoutParams(getContext(),atts); 
}

bool LinearLayout::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
     return dynamic_cast<const LayoutParams*>(p);
}

int LinearLayout::getGravity()const{
    return mGravity;
}

void LinearLayout::setGravity(int gravity){
    if (mGravity != gravity) {
        if ((gravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK) == 0) {
            gravity |= Gravity::START;
        }

        if ((gravity & Gravity::VERTICAL_GRAVITY_MASK) == 0) {
            gravity |= Gravity::TOP;
        }

        mGravity = gravity;
        requestLayout();
    }
}

void LinearLayout::setHorizontalGravity(int horizontalGravity) {
    const int gravity = horizontalGravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK;
    if ((mGravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK) != gravity) {
        mGravity = (mGravity & ~Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK) | gravity;
        requestLayout();
    }
}

void LinearLayout::setVerticalGravity(int verticalGravity) {
    const int gravity = verticalGravity & Gravity::VERTICAL_GRAVITY_MASK;
    if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) != gravity) {
        mGravity = (mGravity & ~Gravity::VERTICAL_GRAVITY_MASK) | gravity;
        requestLayout();
    }
}

void LinearLayout::setOrientation(int orientation){
    if (mOrientation != orientation) {
        mOrientation = orientation;
        requestLayout();
    }
}

int LinearLayout::getOrientation()const{
    return mOrientation;
}

Drawable*LinearLayout::getDividerDrawable(){
   return mDivider;
}

void LinearLayout::setDividerDrawable(Drawable*divider){
    if (divider == mDivider) return;
    delete mDivider;
    mDivider = divider;
    if (divider != nullptr) {
        mDividerWidth = divider->getIntrinsicWidth();
        mDividerHeight = divider->getIntrinsicHeight();
    } else {
        mDividerWidth = 0;
        mDividerHeight = 0;
    }
    setWillNotDraw(!isShowingDividers());
    requestLayout();
}

void LinearLayout::setDividerPadding(int padding){
    if (padding == mDividerPadding) return;
    mDividerPadding = padding;

    if (isShowingDividers()) {
        requestLayout();
        invalidate(true);
    }
}

int LinearLayout::getDividerWidth()const{
    return mDividerWidth;
}

View* LinearLayout::getLastNonGoneChild() {
    for (int i = getVirtualChildCount() - 1; i >= 0; i--) {
        View* child = getVirtualChildAt(i);
        if (child && (child->getVisibility() != View::GONE)) {
            return child;
        }
    }
    return nullptr;
}

void LinearLayout::drawDividersHorizontal(Canvas& canvas){
    const int count = getVirtualChildCount();
    const bool bisLayoutRtl = isLayoutRtl();
    for (int i = 0; i < count; i++) {
        View* child = getVirtualChildAt(i);
        if (child && (child->getVisibility() != View::GONE)) {
            if (hasDividerBeforeChildAt(i)) {
                const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
                int position;
                if (bisLayoutRtl) {
                    position = child->getRight() + lp->rightMargin;
                } else {
                    position = child->getLeft() - lp->leftMargin - mDividerWidth;
                }
                drawVerticalDivider(canvas, position);
            }
        }
    }

    if (hasDividerBeforeChildAt(count)) {
        View* child = getLastNonGoneChild();
        int position;
        if (child == nullptr) {
            if (bisLayoutRtl) {
                position = getPaddingLeft();
            } else {
                position = getWidth() - getPaddingRight() - mDividerWidth;
            }
        } else {
            const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            if (bisLayoutRtl) {
                position = child->getLeft() - lp->leftMargin - mDividerWidth;
            } else {
                position = child->getRight() + lp->rightMargin;
            }
        }
        drawVerticalDivider(canvas, position);
    }       
}

void LinearLayout::drawHorizontalDivider(Canvas& canvas, int top){
    mDivider->setBounds(getPaddingLeft() + mDividerPadding, top,
          getWidth() - getPaddingRight() - getPaddingLeft() - 2*mDividerPadding,  mDividerHeight);
    mDivider->draw(canvas);
}

void LinearLayout::drawVerticalDivider(Canvas& canvas, int left){
    mDivider->setBounds(left, getPaddingTop() + mDividerPadding,mDividerWidth,
          getHeight() - getPaddingBottom() -  getPaddingTop() -2*mDividerPadding);
    mDivider->draw(canvas);
}

void LinearLayout::drawDividersVertical(Canvas& canvas){
    const int count = getVirtualChildCount();
    for (int i = 0; i < count; i++) {
        View* child = getVirtualChildAt(i);
        if (child && (child->getVisibility() != View::GONE)) {
            if (hasDividerBeforeChildAt(i)) {
                const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
                const int top = child->getTop() - lp->topMargin - mDividerHeight;
                drawHorizontalDivider(canvas, top);
            }
        }
    }

    if (hasDividerBeforeChildAt(count)) {
        View* child = getLastNonGoneChild();
        int bottom = 0;
        if (child == nullptr) {
            bottom = getHeight() - getPaddingBottom() - mDividerHeight;
        } else {
            const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            bottom = child->getBottom() + lp->bottomMargin;
        }
        drawHorizontalDivider(canvas, bottom);
    }
}

void LinearLayout::onDraw(Canvas&canvas) {
    if (mDivider == nullptr)return;
    if (mOrientation == VERTICAL) {
        drawDividersVertical(canvas);
    } else {
        drawDividersHorizontal(canvas);
    }
}

int LinearLayout::getDividerPadding()const{
    return mDividerPadding;
}

bool LinearLayout::shouldDelayChildPressedState(){
    return false;
}

void LinearLayout::setWeightSum(float weightSum){
    mWeightSum = std::max(0.0f, weightSum);
}

float LinearLayout::getWeightSum()const{
    return mWeightSum;
}

void LinearLayout::setChildFrame(View*child, int left, int top, int width, int height){
    child->layout(left, top,  width, height);
}

int LinearLayout::getChildrenSkipCount(View* child, int index){
    return 0;
}

int LinearLayout::getLocationOffset(View* child){
    return 0;
}

int LinearLayout::getNextLocationOffset(View* child){
    return 0;
}

void LinearLayout::measureChildBeforeLayout(View* child, int childIndex,int widthMeasureSpec,
             int totalWidth, int heightMeasureSpec,int totalHeight){
    measureChildWithMargins(child, widthMeasureSpec, totalWidth,heightMeasureSpec, totalHeight);
}

bool LinearLayout::allViewsAreGoneBefore(int childIndex){
    for (int i = childIndex - 1; i >= 0; i--) {
        View* child = getVirtualChildAt(i);
        if (child != nullptr && child->getVisibility() != GONE) {
            return false;
        }
    }
    return true;
}

bool LinearLayout::allViewsAreGoneAfter(int childIndex) {
    const int count = getVirtualChildCount();
    for (int i = childIndex + 1; i < count; i++) {
        View* child = getVirtualChildAt(i);
        if (child != nullptr && child->getVisibility() != GONE) {
            return false;
        }
    }
    return true;
}

bool LinearLayout::hasDividerBeforeChildAt(int childIndex){
    if (mShowDividers == SHOW_DIVIDER_NONE) {
        // Short-circuit to save iteration over child views.
        return false;
    }
    if (childIndex == getVirtualChildCount()) {
        // Check whether the end divider should draw.
        return (mShowDividers & SHOW_DIVIDER_END) != 0;
    }
    const bool ballViewsAreGoneBefore = allViewsAreGoneBefore(childIndex);
    if (ballViewsAreGoneBefore) {
        // This is the first view that's not gone, check if beginning divider is enabled.
        return (mShowDividers & SHOW_DIVIDER_BEGINNING) != 0;
    } else {
        return (mShowDividers & SHOW_DIVIDER_MIDDLE) != 0;
    }
}

bool LinearLayout::hasDividerAfterChildAt(int childIndex) {
    if (mShowDividers == SHOW_DIVIDER_NONE) {
        // Short-circuit to save iteration over child views.
        return false;
    }
    if (allViewsAreGoneAfter(childIndex)) {
        // This is the last view that's not gone, check if end divider is enabled.
        return (mShowDividers & SHOW_DIVIDER_END) != 0;
    }
    return (mShowDividers & SHOW_DIVIDER_MIDDLE) != 0;
}

void LinearLayout::forceUniformHeight(int count, int widthMeasureSpec) {
    // Pretend that the linear layout has an exact size. This is the measured height of
    // ourselves. The measured height should be the max height of the children, changed
    // to accommodate the heightMeasureSpec from the parent
    const int uniformMeasureSpec = MeasureSpec::makeMeasureSpec(getMeasuredHeight(), MeasureSpec::EXACTLY);
    for (int i = 0; i < count; ++i) {
        View* child = getVirtualChildAt(i);
        if (child && (child->getVisibility() != View::GONE)) {
            LayoutParams*lp = (LayoutParams*) child->getLayoutParams();

            if (lp->height == LayoutParams::MATCH_PARENT) {
                // Temporarily force children to reuse their old measured width
                // FIXME: this may not be right for something like wrapping text?
                int oldWidth = lp->width;
                lp->width = child->getMeasuredWidth();

                // Remeasure with new dimensions
                measureChildWithMargins(child, widthMeasureSpec, 0, uniformMeasureSpec, 0);
                lp->width = oldWidth;
            }
        }
    }
}

void LinearLayout::forceUniformWidth(int count, int heightMeasureSpec) {
    // Pretend that the linear layout has an exact size.
    const int uniformMeasureSpec = MeasureSpec::makeMeasureSpec(getMeasuredWidth(), MeasureSpec::EXACTLY);
    for (int i = 0; i< count; ++i) {
        View* child = getVirtualChildAt(i);
        if (child && (child->getVisibility() != View::GONE)) {
            LayoutParams* lp = (LayoutParams*)child->getLayoutParams();

            if (lp->width == LayoutParams::MATCH_PARENT) {
                // Temporarily force children to reuse their old measured height
                // FIXME: this may not be right for something like wrapping text?
                const int oldHeight = lp->height;
                lp->height = child->getMeasuredHeight();

                // Remeasue with new dimensions
                measureChildWithMargins(child, uniformMeasureSpec, 0, heightMeasureSpec, 0);
                lp->height = oldHeight;
            }
        }
    }
}

void LinearLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    if (mOrientation == VERTICAL) 
        measureVertical(widthMeasureSpec, heightMeasureSpec);
    else
        measureHorizontal(widthMeasureSpec, heightMeasureSpec);
}

void LinearLayout::measureHorizontal(int widthMeasureSpec, int heightMeasureSpec){
    mTotalLength = 0;
    int maxHeight = 0;
    int childState = 0;
    int alternativeMaxHeight = 0;
    int weightedMaxHeight = 0;
    bool allFillParent = true;
    float totalWeight = 0;

    const int count = getVirtualChildCount();

    const int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);

    bool matchHeight = false;
    bool skippedMeasure = false;

    if ((mMaxAscent.size() == 0) || (mMaxDescent.size() == 0)) {
        mMaxAscent.resize(4);
        mMaxDescent.resize(4);
    }


    mMaxAscent[0] = mMaxAscent[1] = mMaxAscent[2] = mMaxAscent[3] = -1;
    mMaxDescent[0]= mMaxDescent[1]= mMaxDescent[2]= mMaxDescent[3] = -1;

    const bool baselineAligned = mBaselineAligned;
    const bool useLargestChild = mUseLargestChild;

    const bool isExactly = widthMode == MeasureSpec::EXACTLY;

    int largestChildWidth = INT_MIN;//Integer.MIN_VALUE;
    int usedExcessSpace = 0;

    int nonSkippedChildCount = 0;

    // See how wide everyone is. Also remember max height.
    for (int i = 0; i < count; ++i) {
        View* child = getVirtualChildAt(i);
        if (child == nullptr) {
            mTotalLength += measureNullChild(i);
            continue;
        }

        if (child->getVisibility() == View::GONE) {
            i += getChildrenSkipCount(child, i);
            continue;
        }

        nonSkippedChildCount++;
        if (hasDividerBeforeChildAt(i)) {
            mTotalLength += mDividerWidth;
        }

        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

        totalWeight += lp->weight;

        const bool useExcessSpace = (lp->width == 0) && (lp->weight > 0);
        if (widthMode == MeasureSpec::EXACTLY && useExcessSpace) {
            // Optimization: don't bother measuring children who are only
            // laid out using excess space. These views will get measured
            // later if we have space to distribute.
            if (isExactly) {
                mTotalLength += lp->leftMargin + lp->rightMargin;
            } else {
                const int totalLength = mTotalLength;
                mTotalLength = std::max(totalLength, totalLength +
                        lp->leftMargin + lp->rightMargin);
            }

            // Baseline alignment requires to measure widgets to obtain the
            // baseline offset (in particular for TextViews). The following
            // defeats the optimization mentioned above. Allow the child to
            // use as much space as it wants because we can shrink things
            // later (and re-measure).
            if (baselineAligned) {
                const int freeWidthSpec = MeasureSpec::makeSafeMeasureSpec(
                        MeasureSpec::getSize(widthMeasureSpec), MeasureSpec::UNSPECIFIED);
                const int freeHeightSpec = MeasureSpec::makeSafeMeasureSpec(
                        MeasureSpec::getSize(heightMeasureSpec), MeasureSpec::UNSPECIFIED);
                child->measure(freeWidthSpec, freeHeightSpec);
            } else {
                skippedMeasure = true;
            }
        } else {
            if (useExcessSpace) {
                // The widthMode is either UNSPECIFIED or AT_MOST, and
                // this child is only laid out using excess space. Measure
                // using WRAP_CONTENT so that we can find out the view's
                // optimal width. We'll restore the original width of 0
                // after measurement.
                lp->width = LayoutParams::WRAP_CONTENT;
            }

            // Determine how big this child would like to be. If this or
            // previous children have given a weight, then we allow it to
            // use all available space (and we will shrink things later
            // if needed).
            const int usedWidth = (totalWeight == 0) ? mTotalLength : 0;
            measureChildBeforeLayout(child, i, widthMeasureSpec, usedWidth,heightMeasureSpec, 0);

            const int childWidth = child->getMeasuredWidth();
            if (useExcessSpace) {
                // Restore the original width and record how much space
                // we've allocated to excess-only children so that we can
                // match the behavior of EXACTLY measurement.
                lp->width = 0;
                usedExcessSpace += childWidth;
            }

            if (isExactly) {
                mTotalLength += childWidth + lp->leftMargin + lp->rightMargin
                    + getNextLocationOffset(child);
            } else {
                const int totalLength = mTotalLength;
                mTotalLength = std::max(totalLength, totalLength + childWidth + lp->leftMargin
                        + lp->rightMargin + getNextLocationOffset(child));
            }

            if (useLargestChild) {
                largestChildWidth = std::max(childWidth, largestChildWidth);
            }
        }

        bool matchHeightLocally = false;
        if (heightMode != MeasureSpec::EXACTLY && lp->height == LayoutParams::MATCH_PARENT) {
            // The height of the linear layout will scale, and at least one
            // child said it wanted to match our height. Set a flag indicating that
            // we need to remeasure at least that view when we know our height.
            matchHeight = true;
            matchHeightLocally = true;
        }

        const int margin = lp->topMargin + lp->bottomMargin;
        const int childHeight = child->getMeasuredHeight() + margin;
        childState=combineMeasuredStates(childState, child->getMeasuredState());

        if (baselineAligned) {
            const int childBaseline = child->getBaseline();
            if (childBaseline != -1) {
                // Translates the child's vertical gravity into an index
                // in the range 0..VERTICAL_GRAVITY_COUNT
                const int gravity = (lp->gravity < 0 ? mGravity : lp->gravity) & Gravity::VERTICAL_GRAVITY_MASK;
                const int index = ((gravity >> Gravity::AXIS_Y_SHIFT) & ~Gravity::AXIS_SPECIFIED) >> 1;

                mMaxAscent[index] = std::max(mMaxAscent[index], childBaseline);
                mMaxDescent[index]= std::max(mMaxDescent[index], childHeight - childBaseline);
            }
        }

        maxHeight = std::max(maxHeight, childHeight);

        allFillParent = allFillParent && (lp->height == LayoutParams::MATCH_PARENT);
        if (lp->weight > 0) {
            /*
            * Heights of weighted Views are bogus if we end up
            * remeasuring, so keep them separate.
            */
            weightedMaxHeight = std::max(weightedMaxHeight, matchHeightLocally ? margin : childHeight);
        } else {
            alternativeMaxHeight = std::max(alternativeMaxHeight,matchHeightLocally ? margin : childHeight);
        }

        i += getChildrenSkipCount(child, i);
    }

    if (nonSkippedChildCount > 0 && hasDividerBeforeChildAt(count)) {
        mTotalLength += mDividerWidth;
    }

    // Check mMaxAscent[INDEX_TOP] first because it maps to Gravity.TOP,
    // the most common case
    if (mMaxAscent[INDEX_TOP] != -1 ||
        mMaxAscent[INDEX_CENTER_VERTICAL] != -1 ||
            mMaxAscent[INDEX_BOTTOM] != -1 ||
            mMaxAscent[INDEX_FILL] != -1) {
        const int ascent = std::max(mMaxAscent[INDEX_FILL],
                    std::max(mMaxAscent[INDEX_CENTER_VERTICAL],
                    std::max(mMaxAscent[INDEX_TOP], mMaxAscent[INDEX_BOTTOM])));
        const int descent = std::max(mMaxDescent[INDEX_FILL],
                    std::max(mMaxDescent[INDEX_CENTER_VERTICAL],
                    std::max(mMaxDescent[INDEX_TOP], mMaxDescent[INDEX_BOTTOM])));
        maxHeight = std::max(maxHeight, ascent + descent);
    }

    if (useLargestChild && (widthMode == MeasureSpec::AT_MOST || widthMode == MeasureSpec::UNSPECIFIED)) {
        mTotalLength = 0;
        nonSkippedChildCount = 0;
        for (int i = 0; i < count; ++i) {
            View* child = getVirtualChildAt(i);
            if (child == nullptr) {
                mTotalLength += measureNullChild(i);
                continue;
            }

            if (child->getVisibility() == View::GONE) {
                i += getChildrenSkipCount(child, i);
                continue;
            }

            nonSkippedChildCount++;
            if (hasDividerBeforeChildAt(i)) {
                mTotalLength += mDividerWidth;
            }
            LayoutParams* lp = (LayoutParams*)child->getLayoutParams();
            if (isExactly) {
                mTotalLength += largestChildWidth + lp->leftMargin + lp->rightMargin +
                        getNextLocationOffset(child);
            } else {
                int totalLength = mTotalLength;
                mTotalLength = std::max(totalLength, totalLength + largestChildWidth +
                        lp->leftMargin + lp->rightMargin + getNextLocationOffset(child));
            }

        }
        if (nonSkippedChildCount > 0 && hasDividerBeforeChildAt(count)) {
            mTotalLength += mDividerWidth;
        }
    }

    // Add in our padding
    mTotalLength += mPaddingLeft + mPaddingRight;

    int widthSize = mTotalLength;

    // Check against our minimum width
    widthSize = std::max(widthSize, getSuggestedMinimumWidth());

        // Reconcile our calculated size with the widthMeasureSpec
    int widthSizeAndState = resolveSizeAndState(widthSize, widthMeasureSpec, 0);
    widthSize = widthSizeAndState & MEASURED_SIZE_MASK;

    // Either expand children with weight to take up available space or
    // shrink them if they extend beyond our current bounds. If we skipped
    // measurement on any children, we need to measure them now.
    int remainingExcess = widthSize - mTotalLength
            + (mAllowInconsistentMeasurement ? 0 : usedExcessSpace);
    if (skippedMeasure || ((/*sRemeasureWeightedChildren*/true || remainingExcess != 0) && totalWeight > 0.0f)) {
        float remainingWeightSum = mWeightSum > 0.0f ? mWeightSum : totalWeight;

        mMaxAscent[0] = mMaxAscent[1] = mMaxAscent[2] = mMaxAscent[3] = -1;
        mMaxDescent[0]= mMaxDescent[1]= mMaxDescent[2]= mMaxDescent[3] = -1;
        maxHeight = -1;

        mTotalLength = 0;
        nonSkippedChildCount = 0;

        for (int i = 0; i < count; ++i) {
            View* child = getVirtualChildAt(i);
            if ((child == nullptr) || (child->getVisibility() == View::GONE)) {
                continue;
            }

            nonSkippedChildCount++;
            if (hasDividerBeforeChildAt(i)) {
                mTotalLength += mDividerWidth;
            }

            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            float childWeight = lp->weight;
            if (childWeight > 0) {
                int share = (int) (childWeight * remainingExcess / remainingWeightSum);
                remainingExcess -= share;
                remainingWeightSum -= childWeight;

                int childWidth;
                if (mUseLargestChild && widthMode != MeasureSpec::EXACTLY) {
                    childWidth = largestChildWidth;
                } else if (lp->width == 0 && (!mAllowInconsistentMeasurement
                            || widthMode == MeasureSpec::EXACTLY)) {
                    // This child needs to be laid out from scratch using
                    // only its share of excess space.
                    childWidth = share;
                } else {
                    // This child had some intrinsic width to which we
                    // need to add its share of excess space.
                    childWidth = child->getMeasuredWidth() + share;
                }

                int childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(
                            std::max(0, childWidth), MeasureSpec::EXACTLY);
                int childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec,
                            mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin,
                            lp->height);
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

                // Child may now not fit in horizontal dimension.
                childState|=(child->getMeasuredState() & MEASURED_STATE_MASK);
            }

            if (isExactly) {
                mTotalLength += child->getMeasuredWidth() + lp->leftMargin + lp->rightMargin +
                            getNextLocationOffset(child);
            } else {
                const int totalLength = mTotalLength;
                mTotalLength = std::max(totalLength, totalLength + child->getMeasuredWidth() +
                            lp->leftMargin + lp->rightMargin + getNextLocationOffset(child));
            }

            const bool matchHeightLocally = (heightMode != MeasureSpec::EXACTLY) &&
                    (lp->height == LayoutParams::MATCH_PARENT);

            const int margin = lp->topMargin + lp->bottomMargin;
            const int childHeight = child->getMeasuredHeight() + margin;
            maxHeight = std::max(maxHeight, childHeight);
            alternativeMaxHeight = std::max(alternativeMaxHeight,
                    matchHeightLocally ? margin : childHeight);

            allFillParent = allFillParent && (lp->height == LayoutParams::MATCH_PARENT);

            if (baselineAligned) {
                const int childBaseline = child->getBaseline();
                if (childBaseline != -1) {
                    // Translates the child's vertical gravity into an index in the range 0..2
                    const int gravity = (lp->gravity < 0 ? mGravity : lp->gravity)
                                & Gravity::VERTICAL_GRAVITY_MASK;
                    const int index = ((gravity >> Gravity::AXIS_Y_SHIFT)
                                & ~Gravity::AXIS_SPECIFIED) >> 1;

                    mMaxAscent[index] = std::max(mMaxAscent[index], childBaseline);
                    mMaxDescent[index] = std::max(mMaxDescent[index],childHeight - childBaseline);
                }
            }
        }

        if (nonSkippedChildCount > 0 && hasDividerBeforeChildAt(count)) {
            mTotalLength += mDividerWidth;
        }
        // Add in our padding
        mTotalLength += mPaddingLeft + mPaddingRight;
        // TODO: Should we update widthSize with the new total length?

        // Check mMaxAscent[INDEX_TOP] first because it maps to Gravity.TOP,
        // the most common case
        if (mMaxAscent[INDEX_TOP] != -1 ||
                mMaxAscent[INDEX_CENTER_VERTICAL] != -1 ||
                mMaxAscent[INDEX_BOTTOM] != -1 ||
                mMaxAscent[INDEX_FILL] != -1) {
            const int ascent = std::max(mMaxAscent[INDEX_FILL],
                        std::max(mMaxAscent[INDEX_CENTER_VERTICAL],
                        std::max(mMaxAscent[INDEX_TOP], mMaxAscent[INDEX_BOTTOM])));
            const int descent =std::max(mMaxDescent[INDEX_FILL],
                        std::max(mMaxDescent[INDEX_CENTER_VERTICAL],
                        std::max(mMaxDescent[INDEX_TOP], mMaxDescent[INDEX_BOTTOM])));
            maxHeight = std::max(maxHeight, ascent + descent);
        }
    } else {
        alternativeMaxHeight = std::max(alternativeMaxHeight, weightedMaxHeight);

        // We have no limit, so make all weighted views as wide as the largest child.
        // Children will have already been measured once.
        if (useLargestChild && (widthMode != MeasureSpec::EXACTLY)) {
            for (int i = 0; i < count; i++) {
                View* child = getVirtualChildAt(i);
                if ((child == nullptr) || (child->getVisibility() == View::GONE)) {
                    continue;
                }

                LayoutParams* lp =(LayoutParams*) child->getLayoutParams();

                const float childExtra = lp->weight;
                if (childExtra > 0) {
                    child->measure(
                        MeasureSpec::makeMeasureSpec(largestChildWidth, MeasureSpec::EXACTLY),
                        MeasureSpec::makeMeasureSpec(child->getMeasuredHeight(),MeasureSpec::EXACTLY));
                }
            }
        }
    }

    if (!allFillParent && (heightMode != MeasureSpec::EXACTLY)) {
        maxHeight = alternativeMaxHeight;
    }

    maxHeight += mPaddingTop + mPaddingBottom;

    // Check against our minimum height
    maxHeight =std::max(maxHeight, getSuggestedMinimumHeight());

    setMeasuredDimension(widthSizeAndState | (childState&MEASURED_STATE_MASK),
            resolveSizeAndState(maxHeight, heightMeasureSpec,
            (childState<<MEASURED_HEIGHT_STATE_SHIFT)));

    if (matchHeight)forceUniformHeight(count, widthMeasureSpec);
}

void LinearLayout::measureVertical(int widthMeasureSpec, int heightMeasureSpec){
    mTotalLength = 0;
    int maxWidth = 0;
    int childState = 0;
    int alternativeMaxWidth = 0;
    int weightedMaxWidth = 0;
    bool allFillParent = true;
    float totalWeight = 0;

    const int count = getVirtualChildCount();

    const int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);

    bool matchWidth = false;
    bool skippedMeasure = false;

    int baselineChildIndex = mBaselineAlignedChildIndex;
    bool useLargestChild = mUseLargestChild;

    int largestChildHeight = INT_MIN;
    int consumedExcessSpace = 0;

    int nonSkippedChildCount = 0;

    // See how tall everyone is. Also remember max width.
    for (int i = 0; i < count; ++i) {
        View* child = getVirtualChildAt(i);
        if (child == nullptr) {
            mTotalLength += measureNullChild(i);
            continue;
        }

        if (child->getVisibility() == View::GONE) {
            i += getChildrenSkipCount(child, i);
            continue;
        }

        nonSkippedChildCount++;
        if (hasDividerBeforeChildAt(i)) {
            mTotalLength += mDividerHeight;
        }

        LayoutParams*lp = (LayoutParams*) child->getLayoutParams();

        totalWeight += lp->weight;

        const bool useExcessSpace = (lp->height == 0) && (lp->weight > 0);
        if (heightMode == MeasureSpec::EXACTLY && useExcessSpace) {
            // Optimization: don't bother measuring children who are only
            // laid out using excess space. These views will get measured
            // later if we have space to distribute.
            const int totalLength = mTotalLength;
            mTotalLength = std::max(totalLength, totalLength + lp->topMargin + lp->bottomMargin);
            skippedMeasure = true;
        } else {
            if (useExcessSpace) {
                // The heightMode is either UNSPECIFIED or AT_MOST, and
                // this child is only laid out using excess space. Measure
                // using WRAP_CONTENT so that we can find out the view's
                // optimal height. We'll restore the original height of 0
                // after measurement.
                lp->height = LayoutParams::WRAP_CONTENT;
            }

            // Determine how big this child would like to be. If this or
            // previous children have given a weight, then we allow it to
            // use all available space (and we will shrink things later
            // if needed).
            const int usedHeight = (totalWeight == 0) ? mTotalLength : 0;
            measureChildBeforeLayout(child, i, widthMeasureSpec, 0,
                        heightMeasureSpec, usedHeight);

            int childHeight = child->getMeasuredHeight();
            if (useExcessSpace) {
                // Restore the original height and record how much space
                // we've allocated to excess-only children so that we can
                // match the behavior of EXACTLY measurement.
                lp->height = 0;
                consumedExcessSpace += childHeight;
            }

            const int totalLength = mTotalLength;
            mTotalLength = std::max(totalLength, totalLength + childHeight + lp->topMargin +
                    lp->bottomMargin + getNextLocationOffset(child));

            if (useLargestChild) {
                largestChildHeight = std::max(childHeight, largestChildHeight);
            }
        }

        /*If applicable, compute the additional offset to the child's baseline
        * we'll need later when asked {@link #getBaseline}.*/
        if ((baselineChildIndex >= 0) && (baselineChildIndex == i + 1)) {
            mBaselineChildTop = mTotalLength;
        }

        // if we are trying to use a child index for our baseline, the above
        // book keeping only works if there are no children above it with
        // weight.  fail fast to aid the developer.
        if ((i < baselineChildIndex) && (lp->weight > 0)) {
            LOGE("A child of LinearLayout with index "
                      "less than mBaselineAlignedChildIndex has weight > 0, which "
                      "won't work.  Either remove the weight, or don't set "
                      "mBaselineAlignedChildIndex.");
        }

        bool matchWidthLocally = false;
        if ((widthMode != MeasureSpec::EXACTLY) && (lp->width == LayoutParams::MATCH_PARENT)) {
            // The width of the linear layout will scale, and at least one
            // child said it wanted to match our width. Set a flag
            // indicating that we need to remeasure at least that view when
            // we know our width.
            matchWidth = true;
            matchWidthLocally = true;
        }

        const int margin = lp->leftMargin + lp->rightMargin;
        const int measuredWidth = child->getMeasuredWidth() + margin;
        maxWidth = std::max(maxWidth, measuredWidth);
        childState |=  child->getMeasuredState();

        allFillParent = allFillParent && (lp->width == LayoutParams::MATCH_PARENT);
        if (lp->weight > 0) {
            /*
            * Widths of weighted Views are bogus if we end up
            * remeasuring, so keep them separate.
            */
            weightedMaxWidth = std::max(weightedMaxWidth,
                    matchWidthLocally ? margin : measuredWidth);
        } else {
            alternativeMaxWidth = std::max(alternativeMaxWidth,
                    matchWidthLocally ? margin : measuredWidth);
        }

        i += getChildrenSkipCount(child, i);
    }

    if ((nonSkippedChildCount > 0) && hasDividerBeforeChildAt(count)) {
        mTotalLength += mDividerHeight;
    }

    if (useLargestChild &&
            (heightMode == MeasureSpec::AT_MOST || heightMode == MeasureSpec::UNSPECIFIED)) {
        mTotalLength = 0;

        for (int i = 0; i < count; ++i) {
            View* child = getVirtualChildAt(i);
            if (child == nullptr) {
                mTotalLength += measureNullChild(i);
                continue;
            }

            if (child->getVisibility() == GONE) {
                i += getChildrenSkipCount(child, i);
                continue;
            }

            LayoutParams* lp = (LayoutParams*)child->getLayoutParams();
            // Account for negative margins
            const int totalLength = mTotalLength;
            mTotalLength = std::max(totalLength, totalLength + largestChildHeight +
                    lp->topMargin + lp->bottomMargin + getNextLocationOffset(child));
        }
    }

    // Add in our padding
    mTotalLength += mPaddingTop + mPaddingBottom;

    int heightSize = mTotalLength;

    // Check against our minimum height
    heightSize = std::max(heightSize, getSuggestedMinimumHeight());

    // Reconcile our calculated size with the heightMeasureSpec
    const int heightSizeAndState = resolveSizeAndState(heightSize, heightMeasureSpec, 0);
    heightSize = heightSizeAndState & MEASURED_SIZE_MASK;
    // Either expand children with weight to take up available space or
    // shrink them if they extend beyond our current bounds. If we skipped
    // measurement on any children, we need to measure them now.
    int remainingExcess = heightSize - mTotalLength
        + (mAllowInconsistentMeasurement ? 0 : consumedExcessSpace);
    if (skippedMeasure || (remainingExcess != 0) && (totalWeight > 0.0f)) {
        float remainingWeightSum = (mWeightSum > 0.0f) ? mWeightSum : totalWeight;

        mTotalLength = 0;

        for (int i = 0; i < count; ++i) {
            View* child = getVirtualChildAt(i);
            if ((child == nullptr) || (child->getVisibility() ==GONE)) {
                continue;
            }

            LayoutParams*lp = (LayoutParams*) child->getLayoutParams();
            float childWeight = lp->weight;
            if (childWeight > 0) {
                const int share = (int) (childWeight * remainingExcess / remainingWeightSum);
                remainingExcess -= share;
                remainingWeightSum -= childWeight;

                int childHeight;
                if (mUseLargestChild && (heightMode != MeasureSpec::EXACTLY)) {
                    childHeight = largestChildHeight;
                } else if ((lp->height == 0) && (!mAllowInconsistentMeasurement
                        || heightMode == MeasureSpec::EXACTLY)) {
                    // This child needs to be laid out from scratch using
                    // only its share of excess space.
                    childHeight = share;
                } else {
                    // This child had some intrinsic height to which we
                    // need to add its share of excess space.
                    childHeight = child->getMeasuredHeight() + share;
                }

                const int childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(
                        std::max(0, childHeight), MeasureSpec::EXACTLY);
                const int childWidthMeasureSpec = getChildMeasureSpec(widthMeasureSpec,
                        mPaddingLeft + mPaddingRight + lp->leftMargin + lp->rightMargin, lp->width);
                child->measure(childWidthMeasureSpec, childHeightMeasureSpec);

                // Child may now not fit in vertical dimension.
                childState |=( child->getMeasuredState() & (MEASURED_STATE_MASK>>MEASURED_HEIGHT_STATE_SHIFT));
            }

            const int margin =  lp->leftMargin + lp->rightMargin;
            const int measuredWidth = child->getMeasuredWidth() + margin;
            maxWidth = std::max(maxWidth, measuredWidth);

            const bool matchWidthLocally = (widthMode != MeasureSpec::EXACTLY) &&
                (lp->width == LayoutParams::MATCH_PARENT);

            alternativeMaxWidth = std::max(alternativeMaxWidth,
                    matchWidthLocally ? margin : measuredWidth);

            allFillParent = allFillParent && (lp->width == LayoutParams::MATCH_PARENT);

            const int totalLength = mTotalLength;
            mTotalLength = std::max(totalLength, totalLength + child->getMeasuredHeight() +
                lp->topMargin + lp->bottomMargin + getNextLocationOffset(child));
        }

        // Add in our padding
        mTotalLength += mPaddingTop + mPaddingBottom;
        // TODO: Should we recompute the heightSpec based on the new total length?
    } else {
        alternativeMaxWidth = std::max(alternativeMaxWidth,weightedMaxWidth);
        
        // We have no limit, so make all weighted views as tall as the largest child.
        // Children will have already been measured once.
        if (useLargestChild && (heightMode != MeasureSpec::EXACTLY)) {
            for (int i = 0; i < count; i++) {
                View* child = getVirtualChildAt(i);
                if ((child == nullptr) || (child->getVisibility() == View::GONE)) {
                    continue;
                }

                LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

                const float childExtra = lp->weight;
                if (childExtra > 0) {
                    child->measure(MeasureSpec::makeMeasureSpec(child->getMeasuredWidth(),MeasureSpec::EXACTLY),
                            MeasureSpec::makeMeasureSpec(largestChildHeight,MeasureSpec::EXACTLY));
                }
            }
        }
    }

    if (!allFillParent && (widthMode != MeasureSpec::EXACTLY)) {
        maxWidth = alternativeMaxWidth;
    }

    maxWidth += mPaddingLeft + mPaddingRight;

    // Check against our minimum width
    maxWidth = std::max(maxWidth, getSuggestedMinimumWidth());

    setMeasuredDimension(resolveSizeAndState(maxWidth, widthMeasureSpec, childState), heightSizeAndState);

    if (matchWidth) forceUniformWidth(count, heightMeasureSpec);
}

void LinearLayout::layoutVertical(int left, int top, int width, int height){
    const int paddingLeft = mPaddingLeft;

    int childTop;
    int childLeft;

    // Where right end of child should go
    const int childRight = width - mPaddingRight;

    // Space available for child
    const int childSpace = width - paddingLeft - mPaddingRight;

    const int count = getVirtualChildCount();

    const int majorGravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;
    const int minorGravity = mGravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK;

    switch (majorGravity) {
    case Gravity::BOTTOM:
        // mTotalLength contains the padding already
        childTop = mPaddingTop + height - mTotalLength;
        break;
        // mTotalLength contains the padding already
    case Gravity::CENTER_VERTICAL:
        childTop = mPaddingTop + (height - mTotalLength) / 2;
        break;

    case Gravity::TOP:
    default:
        childTop = mPaddingTop;
        break;
    }

    for (int i = 0; i < count; i++) {
        View* child = getVirtualChildAt(i);
        if (child == nullptr) {
            childTop += measureNullChild(i);
        } else if (child->getVisibility() != GONE) {
            const int childWidth = child->getMeasuredWidth();
            const int childHeight = child->getMeasuredHeight();

            LayoutParams* lp =(LayoutParams*) child->getLayoutParams();

            int gravity = lp->gravity;
            if (gravity < 0) {
                gravity = minorGravity;
            }
            const int layoutDirection = getLayoutDirection();
            const int absoluteGravity = Gravity::getAbsoluteGravity(gravity, layoutDirection);
            switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
            case Gravity::CENTER_HORIZONTAL:
                childLeft = paddingLeft + ((childSpace - childWidth) / 2)
                        + lp->leftMargin - lp->rightMargin;
                break;

            case Gravity::RIGHT:
                childLeft = childRight - childWidth - lp->rightMargin;
                break;

            case Gravity::LEFT:
            default:
                childLeft = paddingLeft + lp->leftMargin;
                break;
            }

            if (hasDividerBeforeChildAt(i)) childTop += mDividerHeight;

            childTop += lp->topMargin;
            setChildFrame(child, childLeft, childTop + getLocationOffset(child),
                    childWidth, childHeight);
            childTop += childHeight + lp->bottomMargin + getNextLocationOffset(child);

            i += getChildrenSkipCount(child, i);
        }
    }
}

void LinearLayout::onRtlPropertiesChanged(int layoutDirection) {
    ViewGroup::onRtlPropertiesChanged(layoutDirection);
    if (layoutDirection != mLayoutDirection) {
        mLayoutDirection = layoutDirection;
        if (mOrientation == HORIZONTAL) {
            requestLayout();
        }
    }
}

void LinearLayout::layoutHorizontal(int left, int top, int width, int height){
    const bool bLayoutRtl = isLayoutRtl();
    const int paddingTop = mPaddingTop;

    int childTop;
    int childLeft;

    // Where bottom of child should go
    const int childBottom = height - mPaddingBottom;

    // Space available for child
    const int childSpace = height - paddingTop - mPaddingBottom;

    const int count = getVirtualChildCount();

    const int majorGravity = mGravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK;
    const int minorGravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;

    const bool baselineAligned = mBaselineAligned;

    const int layoutDirection = getLayoutDirection();
    switch (Gravity::getAbsoluteGravity(majorGravity, layoutDirection)) {
    case Gravity::RIGHT:
        // mTotalLength contains the padding already
        childLeft = mPaddingLeft + width - mTotalLength;
        break;

    case Gravity::CENTER_HORIZONTAL:
        // mTotalLength contains the padding already
        childLeft = mPaddingLeft + (width - mTotalLength) / 2;
        break;

    case Gravity::LEFT:
    default:
        childLeft = mPaddingLeft;
        break;
    }

    int start = 0;
    int dir = 1;
    //In case of RTL, start drawing from the last child.
    if (bLayoutRtl) {
        start = count - 1;
        dir = -1;
    }

    for (int i = 0; i < count; i++) {
        int childIndex = start + dir * i;
        View* child = getVirtualChildAt(childIndex);
        if (child == nullptr) {
            childLeft += measureNullChild(childIndex);
        } else if (child->getVisibility() != GONE) {
            const int childWidth = child->getMeasuredWidth();
            const int childHeight = child->getMeasuredHeight();
            int childBaseline = -1;

            LayoutParams*lp =(LayoutParams*) child->getLayoutParams();

            if (baselineAligned && (lp->height != LayoutParams::MATCH_PARENT)) {
                childBaseline = child->getBaseline();
            }

            int gravity = lp->gravity;
            if (gravity < 0) {
                gravity = minorGravity;
            }

            switch (gravity & Gravity::VERTICAL_GRAVITY_MASK) {
            case Gravity::TOP:
                childTop = paddingTop + lp->topMargin;
                if (childBaseline != -1) {
                    childTop +=mMaxAscent[INDEX_TOP] - childBaseline;
                }
                break;

            case Gravity::CENTER_VERTICAL:
                // Removed support for baseline alignment when layout_gravity or
                // gravity == center_vertical. See bug #1038483.
                // Keep the code around if we need to re-enable this feature
                // if (childBaseline != -1) {
                //     // Align baselines vertically only if the child is smaller than us
                //     if (childSpace - childHeight > 0) {
                //         childTop = paddingTop + (childSpace / 2) - childBaseline;
                //     } else {
                //         childTop = paddingTop + (childSpace - childHeight) / 2;
                //     }
                // } else {
                childTop = paddingTop + ((childSpace - childHeight) / 2)
                        + lp->topMargin - lp->bottomMargin;
                break;

            case Gravity::BOTTOM:
                childTop = childBottom - childHeight - lp->bottomMargin;
                if (childBaseline != -1) {
                    int descent = child->getMeasuredHeight() - childBaseline;
                    childTop -= (mMaxDescent[INDEX_BOTTOM] - descent);
                }
                break;
            default:
                childTop = paddingTop;
                break;
            }
            if(bLayoutRtl){
                // Because rtl rendering occurs in the reverse direction, we need to check
                // after the child rather than before (since after=left in this context)
                if (hasDividerAfterChildAt(childIndex)) {
                    childLeft += mDividerWidth;
                }
            }else if (hasDividerBeforeChildAt(childIndex)) {
                childLeft += mDividerWidth;
            }

            childLeft += lp->leftMargin;
            setChildFrame(child, childLeft + getLocationOffset(child), childTop, childWidth, childHeight);
            childLeft += childWidth + lp->rightMargin +getNextLocationOffset(child);

            i += getChildrenSkipCount(child, childIndex);
        }
    }
}

void LinearLayout::onLayout(bool changed, int l, int t, int w, int h) {
    if (mOrientation == VERTICAL) {
        layoutVertical(l, t, w, h);
    } else {
        layoutHorizontal(l, t, w, h);
    }
}//  endof onLayout

}//endof namespace
