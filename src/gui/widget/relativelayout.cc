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
#include <widget/relativelayout.h>
#include <string.h>
#include <cdlog.h>
namespace cdroid{

static constexpr int RULES_VERTICAL[] = {
    RelativeLayout::ABOVE, 
    RelativeLayout::BELOW, 
    RelativeLayout::ALIGN_BASELINE, 
    RelativeLayout::ALIGN_TOP, 
    RelativeLayout::ALIGN_BOTTOM
};

static constexpr int RULES_HORIZONTAL[] = {
    RelativeLayout::LEFT_OF,
    RelativeLayout::RIGHT_OF,
    RelativeLayout::ALIGN_LEFT,
    RelativeLayout::ALIGN_RIGHT,
    RelativeLayout::START_OF,
    RelativeLayout::END_OF,
    RelativeLayout::ALIGN_START,
    RelativeLayout::ALIGN_END
};

DECLARE_WIDGET(RelativeLayout)

RelativeLayout::RelativeLayout(int w,int h):ViewGroup(w,h){
    mIgnoreGravity  = NO_ID;
    mDirtyHierarchy = true;
    mGraph = new DependencyGraph();
}

RelativeLayout::RelativeLayout(Context* context,const AttributeSet& attrs)
 :ViewGroup(context,attrs){
    mDirtyHierarchy = true;
    mIgnoreGravity = attrs.getResourceId("ignoreGravity", View::NO_ID);
    mGravity=attrs.getGravity("gravity",mGravity);
    mGraph = new DependencyGraph();
}

RelativeLayout::~RelativeLayout(){
    delete mGraph;
}

bool RelativeLayout::shouldDelayChildPressedState(){
    return false;
}

void RelativeLayout::setIgnoreGravity(int viewId){
    mIgnoreGravity = viewId;
}

int RelativeLayout::getIgnoreGravity()const{
    return mIgnoreGravity;
}

int RelativeLayout::getGravity()const{
    return mGravity;
}

void RelativeLayout::setGravity(int gravity) {
    if (mGravity == gravity)return;

    if ((gravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK) == 0) {
        gravity |= Gravity::START;
    }

    if ((gravity & Gravity::VERTICAL_GRAVITY_MASK) == 0) {
        gravity |= Gravity::TOP;
    }

    mGravity = gravity;
    requestLayout();
}

void RelativeLayout::setHorizontalGravity(int horizontalGravity) {
    const int gravity = horizontalGravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK;
    if ((mGravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK) != gravity) {
        mGravity = (mGravity & ~Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK) | gravity;
        requestLayout();
    }
}

void RelativeLayout::setVerticalGravity(int verticalGravity) {
    const int gravity = verticalGravity & Gravity::VERTICAL_GRAVITY_MASK;
    if ((mGravity & Gravity::VERTICAL_GRAVITY_MASK) != gravity) {
        mGravity = (mGravity & ~Gravity::VERTICAL_GRAVITY_MASK) | gravity;
        requestLayout();
    }
}

int RelativeLayout::getBaseline() {
    return mBaselineView ? mBaselineView->getBaseline() : ViewGroup::getBaseline();
}

void RelativeLayout::requestLayout() {
    ViewGroup::requestLayout();
    mDirtyHierarchy = true;
}

void RelativeLayout::sortChildren(){
    const int count = getChildCount();

    mSortedVerticalChildren.resize(count);
    mSortedHorizontalChildren.resize(count);

    mGraph->clear();

    for (int i = 0; i < count; i++) {
        mGraph->add(getChildAt(i));
    }
    mGraph->getSortedViews(mSortedVerticalChildren, (const int*)RULES_VERTICAL,sizeof(RULES_VERTICAL)/sizeof(RULES_VERTICAL[0]));
    mGraph->getSortedViews(mSortedHorizontalChildren,(const int*)RULES_HORIZONTAL,sizeof(RULES_HORIZONTAL)/sizeof(RULES_HORIZONTAL[0]));
}

void RelativeLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    if (mDirtyHierarchy) {
        mDirtyHierarchy = false;
        sortChildren();
    }

    int myWidth = -1;
    int myHeight = -1;

    int width = 0;
    int height = 0;

    int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    int heightMode= MeasureSpec::getMode(heightMeasureSpec);
    int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightSize= MeasureSpec::getSize(heightMeasureSpec);

    // Record our dimensions if they are known;
    if (widthMode != MeasureSpec::UNSPECIFIED)  myWidth = widthSize;

    if (heightMode != MeasureSpec::UNSPECIFIED) myHeight = heightSize;

    if (widthMode == MeasureSpec::EXACTLY) width = myWidth;

    if (heightMode == MeasureSpec::EXACTLY)height = myHeight;

    View* ignore = nullptr;
    int gravity = mGravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK;
    bool horizontalGravity = gravity != Gravity::START && gravity != 0;
    gravity = mGravity & Gravity::VERTICAL_GRAVITY_MASK;
    bool verticalGravity = gravity != Gravity::TOP && gravity != 0;

    int left = INT_MAX;
    int top  = INT_MAX;
    int right = INT_MIN;
    int bottom= INT_MIN;

    bool offsetHorizontalAxis = false;
    bool offsetVerticalAxis = false;

    if ((horizontalGravity || verticalGravity) && mIgnoreGravity != NO_ID){
        ignore = findViewById(mIgnoreGravity);
    }

    bool isWrapContentWidth = widthMode != MeasureSpec::EXACTLY;
    bool isWrapContentHeight = heightMode != MeasureSpec::EXACTLY;
    // We need to know our size for doing the correct computation of children positioning in RTL
    // mode but there is no practical way to get it instead of running the code below.
    // So, instead of running the code twice, we just set the width to a "default display width"
    // before the computation and then, as a last pass, we will update their real position with
    // an offset equals to "DEFAULT_WIDTH - width".
    const int layoutDirection = getLayoutDirection();
    if (isLayoutRtl() && myWidth == -1) {
        myWidth = DEFAULT_WIDTH;
    }

    std::vector<View*>&views = mSortedHorizontalChildren;
    int count = views.size();
    for (int i = 0; i < count; i++) {
        View* child = views[i];
        if (child->getVisibility() != GONE) {
            LayoutParams* params = (LayoutParams*) child->getLayoutParams();
            const int*rules = params->getRules(layoutDirection);

            applyHorizontalSizeRules(params, myWidth, rules);
            measureChildHorizontal(child, params, myWidth, myHeight);

            if (positionChildHorizontal(child, params, myWidth, isWrapContentWidth)) {
                offsetHorizontalAxis = true;
            }
        }
    }

    views = mSortedVerticalChildren;
    count = views.size();

    for (int i = 0; i < count; i++) {
        View* child = views[i];
        if (child->getVisibility() != GONE) {
            LayoutParams* params = (LayoutParams*) child->getLayoutParams();

            applyVerticalSizeRules(params, myHeight, child->getBaseline());
            measureChild(child, params, myWidth, myHeight);

            if (positionChildVertical(child, params, myHeight, isWrapContentHeight)) {
                offsetVerticalAxis = true;
            }

            if (isWrapContentWidth) {
                if (isLayoutRtl()) {
                    width = std::max(width, myWidth - params->mLeft + params->leftMargin);
                } else {
                    width = std::max(width, params->mRight + params->rightMargin);
                }
            }

            if (isWrapContentHeight) {
                height = std::max(height, params->mBottom + params->bottomMargin);
            }

            if (child != ignore || verticalGravity) {
                left= std::min(left, params->mLeft - params->leftMargin);
                top = std::min(top, params->mTop - params->topMargin);
            }

            if (child != ignore || horizontalGravity) {
                right = std::max(right, params->mRight + params->rightMargin);
                bottom= std::max(bottom, params->mBottom + params->bottomMargin);
            }
        }
    }

    // Use the top-start-most laid out view as the baseline. RTL offsets are
    // applied later, so we can use the left-most edge as the starting edge.
    View* baselineView = nullptr;
    LayoutParams* baselineParams = nullptr;
    for (int i = 0; i < count; i++) {
        View* child = views[i];
        if (child->getVisibility() != GONE) {
            LayoutParams* childParams = (LayoutParams*) child->getLayoutParams();
            if ((baselineView == nullptr) || (baselineParams == nullptr)
                    || compareLayoutPosition(childParams, baselineParams) < 0) {
                baselineView = child;
                baselineParams = childParams;
            }
        }
    }
    mBaselineView = baselineView;
    if (isWrapContentWidth) {
        // Width already has left padding in it since it was calculated by looking at
        // the right of each child view
        width += mPaddingRight;
        if (mLayoutParams && mLayoutParams->width >= 0) {
            width = std::max(width, mLayoutParams->width);
        }

        width = std::max(width, getSuggestedMinimumWidth());
        width = resolveSize(width, widthMeasureSpec);

        if (offsetHorizontalAxis) {
            for (int i = 0; i < count; i++) {
                View* child = views[i];
                if (child->getVisibility() != GONE) {
                    LayoutParams* params = (LayoutParams*) child->getLayoutParams();
                    const int* rules = params->getRules(layoutDirection);
                    if ((rules[CENTER_IN_PARENT] != 0) || (rules[CENTER_HORIZONTAL] != 0)) {
                        centerHorizontal(child, params, width);
                    } else if (rules[ALIGN_PARENT_RIGHT] != 0) {
                        const int childWidth = child->getMeasuredWidth();
                        params->mLeft = width - mPaddingRight - childWidth;
                        params->mRight = params->mLeft + childWidth;
                    }
                }
            }
        }
    }

    if (isWrapContentHeight) {
        // Height already has top padding in it since it was calculated by looking at
        // the bottom of each child view
        height += mPaddingBottom;

        if (mLayoutParams && mLayoutParams->height >= 0) {
            height = std::max(height, mLayoutParams->height);
        }

        height = std::max(height, getSuggestedMinimumHeight());
        height = resolveSize(height, heightMeasureSpec);

        if (offsetVerticalAxis) {
            for (int i = 0; i < count; i++) {
                View* child = views[i];
                if (child->getVisibility() != GONE) {
                    LayoutParams* params = (LayoutParams*) child->getLayoutParams();
                    const int* rules = params->getRules(layoutDirection);
                    if ((rules[CENTER_IN_PARENT] != 0) || (rules[CENTER_VERTICAL] != 0)) {
                        centerVertical(child, params, height);
                    } else if (rules[ALIGN_PARENT_BOTTOM] != 0) {
                        const int childHeight = child->getMeasuredHeight();
                        params->mTop = height - mPaddingBottom - childHeight;
                        params->mBottom = params->mTop + childHeight;
                    }
                }
            }
        }
    }

    if (horizontalGravity || verticalGravity) {
        Rect selfBounds = mSelfBounds;
        selfBounds.set(mPaddingLeft, mPaddingTop, width - mPaddingLeft - mPaddingRight,
                height - mPaddingTop - mPaddingBottom);

        Rect contentBounds = mContentBounds;
        Gravity::apply(mGravity, right - left, bottom - top, selfBounds, contentBounds,
                layoutDirection);

        const int horizontalOffset = contentBounds.left - left;
        const int verticalOffset = contentBounds.top - top;
        if (horizontalOffset != 0 || verticalOffset != 0) {
            for (int i = 0; i < count; i++) {
                View* child = views[i];
                if ((child->getVisibility() != GONE) && (child != ignore)) {
                    LayoutParams* params = (LayoutParams*) child->getLayoutParams();
                    if (horizontalGravity) {
                        params->mLeft += horizontalOffset;
                        params->mRight += horizontalOffset;
                    }
                    if (verticalGravity) {
                        params->mTop += verticalOffset;
                        params->mBottom += verticalOffset;
                    }
                }
            }
        }
    }

    if (isLayoutRtl()) {
        const int offsetWidth = myWidth - width;
        for (int i = 0; i < count; i++) {
            View* child = views[i];
            if (child->getVisibility() != GONE) {
                LayoutParams* params = (LayoutParams*) child->getLayoutParams();
                params->mLeft -= offsetWidth;
                params->mRight -= offsetWidth;
            }
        }
    }

    setMeasuredDimension(width, height);
}

int RelativeLayout::compareLayoutPosition(const LayoutParams* p1,const LayoutParams* p2){
    const int topDiff = p1->mTop - p2->mTop;
    if (topDiff != 0) {
        return topDiff;
    }
    return p1->mLeft - p2->mLeft;
}

void RelativeLayout::measureChild(View* child, LayoutParams* params, int myWidth, int myHeight){
    const int childWidthMeasureSpec = getChildMeasureSpec(params->mLeft,
            params->mRight, params->width,
            params->leftMargin, params->rightMargin,
            mPaddingLeft, mPaddingRight, myWidth);
    const int childHeightMeasureSpec = getChildMeasureSpec(params->mTop,
            params->mBottom, params->height,
            params->topMargin, params->bottomMargin,
            mPaddingTop, mPaddingBottom,myHeight);
    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void RelativeLayout::measureChildHorizontal(View* child, LayoutParams* params, int myWidth, int myHeight){
    const int childWidthMeasureSpec = getChildMeasureSpec(params->mLeft, params->mRight, params->width, 
            params->leftMargin, params->rightMargin, mPaddingLeft, mPaddingRight,myWidth);

    int childHeightMeasureSpec;
    if (myHeight < 0 && !mAllowBrokenMeasureSpecs) {
        if (params->height >= 0) {
            childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(
                    params->height, MeasureSpec::EXACTLY);
        } else {
            // Negative values in a mySize/myWidth/myWidth value in
            // RelativeLayout measurement is code for, "we got an
            // unspecified mode in the RelativeLayout's measure spec."
            // Carry it forward.
            childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED);
        }
    } else {
        int maxHeight;
        if (mMeasureVerticalWithPaddingMargin) {
            maxHeight = std::max(0, myHeight - mPaddingTop - mPaddingBottom
                    - params->topMargin - params->bottomMargin);
        } else {
            maxHeight = std::max(0, myHeight);
        }

        int heightMode;
        if (params->height == LayoutParams::MATCH_PARENT) {
            heightMode = MeasureSpec::EXACTLY;
        } else {
            heightMode = MeasureSpec::AT_MOST;
        }
        childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(maxHeight, heightMode);
    }

    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

int RelativeLayout::getChildMeasureSpec(int childStart, int childEnd, int childSize,
     int startMargin, int endMargin, int startPadding,int endPadding, int mySize){
    int childSpecMode = 0;
    int childSpecSize = 0;

    // Negative values in a mySize value in RelativeLayout
    // measurement is code for, "we got an unspecified mode in the
    // RelativeLayout's measure spec."
    const bool isUnspecified = mySize < 0;
    if (isUnspecified && !mAllowBrokenMeasureSpecs) {
        if ((childStart != VALUE_NOT_SET) && (childEnd != VALUE_NOT_SET)) {
            // Constraints fixed both edges, so child has an exact size.
            childSpecSize = std::max(0, childEnd - childStart);
            childSpecMode = MeasureSpec::EXACTLY;
        } else if (childSize >= 0) {
            // The child specified an exact size.
            childSpecSize = childSize;
            childSpecMode = MeasureSpec::EXACTLY;
        } else {
            // Allow the child to be whatever size it wants.
            childSpecSize = 0;
            childSpecMode = MeasureSpec::UNSPECIFIED;
        }
         return MeasureSpec::makeMeasureSpec(childSpecSize, childSpecMode);
    }

    // Figure out start and end bounds.
    int tempStart = childStart;
    int tempEnd = childEnd;

    // If the view did not express a layout constraint for an edge, use
    // view's margins and our padding
    if (tempStart == VALUE_NOT_SET) {
        tempStart = startPadding + startMargin;
    }
    if (tempEnd == VALUE_NOT_SET) {
        tempEnd = mySize - endPadding - endMargin;
    }

    // Figure out maximum size available to this view
    const int maxAvailable = tempEnd - tempStart;

    if ((childStart != VALUE_NOT_SET) && (childEnd != VALUE_NOT_SET)) {
        // Constraints fixed both edges, so child must be an exact size.
        childSpecMode = isUnspecified ? MeasureSpec::UNSPECIFIED : MeasureSpec::EXACTLY;
        childSpecSize = std::max(0, maxAvailable);
    } else {
        if (childSize >= 0) {
            // Child wanted an exact size. Give as much as possible.
            childSpecMode = MeasureSpec::EXACTLY;
             if (maxAvailable >= 0) {
                // We have a maximum size in this dimension.
                childSpecSize = std::min(maxAvailable, childSize);
            } else {
                // We can grow in this dimension.
                childSpecSize = childSize;
            }
        } else if (childSize == LayoutParams::MATCH_PARENT) {
            // Child wanted to be as big as possible. Give all available
            // space.
            childSpecMode = isUnspecified ? MeasureSpec::UNSPECIFIED : MeasureSpec::EXACTLY;
            childSpecSize = std::max(0, maxAvailable);
    } else if (childSize == LayoutParams::WRAP_CONTENT) {
            // Child wants to wrap content. Use AT_MOST to communicate
            // available space if we know our max size.
            if (maxAvailable >= 0) {
                // We have a maximum size in this dimension.
                childSpecMode = MeasureSpec::AT_MOST;
                childSpecSize = maxAvailable;
            } else {
                // We can grow in this dimension. Child can be as big as it
                // wants.
                childSpecMode = MeasureSpec::UNSPECIFIED;
                childSpecSize = 0;
            }
        }
    }

    return MeasureSpec::makeMeasureSpec(childSpecSize, childSpecMode);
}

bool RelativeLayout::positionChildHorizontal(View* child, LayoutParams* params, int myWidth,bool wrapContent){
    const int layoutDirection = getLayoutDirection();
    const int* rules = params->getRules(layoutDirection);

    if ((params->mLeft == VALUE_NOT_SET) && (params->mRight != VALUE_NOT_SET)) {
        // Right is fixed, but left varies
        params->mLeft = params->mRight - child->getMeasuredWidth();
    } else if ((params->mLeft != VALUE_NOT_SET) && (params->mRight == VALUE_NOT_SET)) {
        // Left is fixed, but right varies
        params->mRight = params->mLeft + child->getMeasuredWidth();
    } else if ((params->mLeft == VALUE_NOT_SET) && (params->mRight == VALUE_NOT_SET)) {
        // Both left and right vary
        if ((rules[CENTER_IN_PARENT] != 0) || (rules[CENTER_HORIZONTAL] != 0)) {
            if (!wrapContent) {
                centerHorizontal(child, params, myWidth);
            } else {
                positionAtEdge(child, params, myWidth);
            }
            return true;
        } else {
            // This is the default case. For RTL we start from the right and for LTR we start
            // from the left. This will give LEFT/TOP for LTR and RIGHT/TOP for RTL.
            positionAtEdge(child, params, myWidth);
        }
    }
    return rules[ALIGN_PARENT_END] != 0;
}

void RelativeLayout::positionAtEdge(View* child, LayoutParams* params, int myWidth){
    if (isLayoutRtl()) {
        params->mRight = myWidth - mPaddingRight - params->rightMargin;
        params->mLeft = params->mRight - child->getMeasuredWidth();
    } else {
        params->mLeft = mPaddingLeft + params->leftMargin;
        params->mRight = params->mLeft + child->getMeasuredWidth();
    }
}

bool RelativeLayout::positionChildVertical(View* child, LayoutParams* params, int myHeight, bool wrapContent){
    const int* rules = params->getRules();

    if ((params->mTop == VALUE_NOT_SET) && (params->mBottom != VALUE_NOT_SET)) {
        // Bottom is fixed, but top varies
        params->mTop = params->mBottom - child->getMeasuredHeight();
    } else if ((params->mTop != VALUE_NOT_SET) && (params->mBottom == VALUE_NOT_SET)) {
        // Top is fixed, but bottom varies
        params->mBottom = params->mTop + child->getMeasuredHeight();
    } else if ((params->mTop == VALUE_NOT_SET) && (params->mBottom == VALUE_NOT_SET)) {
        // Both top and bottom vary
        if ((rules[CENTER_IN_PARENT] != 0) || (rules[CENTER_VERTICAL] != 0)) {
            if (!wrapContent) {
                centerVertical(child, params, myHeight);
            } else {
                params->mTop = mPaddingTop + params->topMargin;
                params->mBottom = params->mTop + child->getMeasuredHeight();
            }
            return true;
        } else {
            params->mTop = mPaddingTop + params->topMargin;
            params->mBottom = params->mTop + child->getMeasuredHeight();
        }
    }
    return rules[ALIGN_PARENT_BOTTOM] != 0;
}

void RelativeLayout::applyHorizontalSizeRules(RelativeLayout::LayoutParams* childParams, int myWidth,const int* rules){
    RelativeLayout::LayoutParams* anchorParams;

    // VALUE_NOT_SET indicates a "soft requirement" in that direction. For example:
    // left=10, right=VALUE_NOT_SET means the view must start at 10, but can go as far as it
    // wants to the right
    // left=VALUE_NOT_SET, right=10 means the view must end at 10, but can go as far as it
    // wants to the left
    // left=10, right=20 means the left and right ends are both fixed
    childParams->mLeft = VALUE_NOT_SET;
    childParams->mRight = VALUE_NOT_SET;

    anchorParams = getRelatedViewParams(rules, LEFT_OF);
    if (anchorParams) {
        childParams->mRight = anchorParams->mLeft - (anchorParams->leftMargin +
                childParams->rightMargin);
    } else if (childParams->alignWithParent && (rules[LEFT_OF] != 0)) {
        if (myWidth >= 0) {
            childParams->mRight = myWidth - mPaddingRight - childParams->rightMargin;
        }
    }

    anchorParams = getRelatedViewParams(rules, RIGHT_OF);
    if (anchorParams) {
        childParams->mLeft = anchorParams->mRight + (anchorParams->rightMargin +
                childParams->leftMargin);
    } else if (childParams->alignWithParent && (rules[RIGHT_OF] != 0)) {
        childParams->mLeft = mPaddingLeft + childParams->leftMargin;
    }

    anchorParams = getRelatedViewParams(rules, ALIGN_LEFT);
    if (anchorParams) {
        childParams->mLeft = anchorParams->mLeft + childParams->leftMargin;
    } else if (childParams->alignWithParent && (rules[ALIGN_LEFT] != 0)) {
        childParams->mLeft = mPaddingLeft + childParams->leftMargin;
    }

    anchorParams = getRelatedViewParams(rules, ALIGN_RIGHT);
    if (anchorParams) {
        childParams->mRight = anchorParams->mRight - childParams->rightMargin;
    } else if (childParams->alignWithParent && (rules[ALIGN_RIGHT] != 0)) {
        if (myWidth >= 0) {
            childParams->mRight = myWidth - mPaddingRight - childParams->rightMargin;
        }
    }

    if (0 != rules[ALIGN_PARENT_LEFT]) {
        childParams->mLeft = mPaddingLeft + childParams->leftMargin;
    }

    if (0 != rules[ALIGN_PARENT_RIGHT]) {
        if (myWidth >= 0) {
            childParams->mRight = myWidth - mPaddingRight - childParams->rightMargin;
        }
    }
}

void RelativeLayout::applyVerticalSizeRules(RelativeLayout::LayoutParams* childParams, int myHeight, int myBaseline){
    const int* rules = childParams->getRules();

    // Baseline alignment overrides any explicitly specified top or bottom.
    int baselineOffset = getRelatedViewBaselineOffset(rules);
    if (baselineOffset != -1) {
        if (myBaseline != -1) {
            baselineOffset -= myBaseline;
        }
        childParams->mTop = baselineOffset;
        childParams->mBottom = VALUE_NOT_SET;
        return;
    }

    RelativeLayout::LayoutParams* anchorParams;

    childParams->mTop = VALUE_NOT_SET;
    childParams->mBottom = VALUE_NOT_SET;

    anchorParams = getRelatedViewParams(rules, ABOVE);
    if (anchorParams) {
        childParams->mBottom = anchorParams->mTop - (anchorParams->topMargin +
                childParams->bottomMargin);
    } else if (childParams->alignWithParent && (rules[ABOVE] != 0)) {
        if (myHeight >= 0) {
            childParams->mBottom = myHeight - mPaddingBottom - childParams->bottomMargin;
        }
    }

    anchorParams = getRelatedViewParams(rules, BELOW);
    if (anchorParams) {
        childParams->mTop = anchorParams->mBottom + (anchorParams->bottomMargin +
                childParams->topMargin);
    } else if (childParams->alignWithParent && (rules[BELOW] != 0)) {
        childParams->mTop = mPaddingTop + childParams->topMargin;
    }

    anchorParams = getRelatedViewParams(rules, ALIGN_TOP);
    if (anchorParams) {
        childParams->mTop = anchorParams->mTop + childParams->topMargin;
    } else if (childParams->alignWithParent && (rules[ALIGN_TOP] != 0)) {
        childParams->mTop = mPaddingTop + childParams->topMargin;
    }

    anchorParams = getRelatedViewParams(rules, ALIGN_BOTTOM);
    if (anchorParams) {
        childParams->mBottom = anchorParams->mBottom - childParams->bottomMargin;
    } else if (childParams->alignWithParent && (rules[ALIGN_BOTTOM] != 0)) {
        if (myHeight >= 0) {
            childParams->mBottom = myHeight - mPaddingBottom - childParams->bottomMargin;
        }
    }

    if (0 != rules[ALIGN_PARENT_TOP]) {
        childParams->mTop = mPaddingTop + childParams->topMargin;
    }

    if (0 != rules[ALIGN_PARENT_BOTTOM]) {
        if (myHeight >= 0) {
            childParams->mBottom = myHeight - mPaddingBottom - childParams->bottomMargin;
        }
    }
}

View* RelativeLayout::getRelatedView(const int* rules, int relation){
    const int id = rules[relation];
    if (id != 0) {
        DependencyGraph::Node* node = mGraph->mKeyNodes.get(id);
        if (node == nullptr) return nullptr;
        View* v = node->view;

        // Find the first non-GONE view up the chain
        while (v->getVisibility() == View::GONE) {
            rules = ((LayoutParams*) v->getLayoutParams())->getRules(v->getLayoutDirection());
            node = mGraph->mKeyNodes.get((rules[relation]));
            // ignore self dependency. for more info look in git commit: da3003
            if ((node == nullptr) || (v == node->view)) return nullptr;
            v = node->view;
        }
        return v;
    }

    return nullptr;
}

RelativeLayout::LayoutParams* RelativeLayout::getRelatedViewParams(const int* rules, int relation){
    View* v = getRelatedView(rules, relation);
    if (v != nullptr) {
        ViewGroup::LayoutParams* params = v->getLayoutParams();
        if (dynamic_cast<LayoutParams*>(params)) {
            return (LayoutParams*) v->getLayoutParams();
        }
    }
    return nullptr;
}

int RelativeLayout::getRelatedViewBaselineOffset(const int* rules){
    View* v = getRelatedView(rules, ALIGN_BASELINE);
    if (v != nullptr) {
        int baseline = v->getBaseline();
        if (baseline != -1) {
            ViewGroup::LayoutParams* params = v->getLayoutParams();
            if (dynamic_cast<LayoutParams*>(params)) {
                LayoutParams* anchorParams = (LayoutParams*) v->getLayoutParams();
                return anchorParams->mTop + baseline;
            }
        }
    }
    return -1;
}

void RelativeLayout::centerHorizontal(View* child, LayoutParams* params, int myWidth){
    const int childWidth = child->getMeasuredWidth();
    const int left = (myWidth - childWidth) / 2;
    params->mLeft = left;
    params->mRight= left + childWidth;
}

void RelativeLayout::centerVertical(View* child, LayoutParams* params, int myHeight){
    const int childHeight = child->getMeasuredHeight();
    const int top = (myHeight - childHeight) / 2;
    params->mTop = top;
    params->mBottom = top + childHeight;
}

void RelativeLayout::onLayout(bool changed, int l, int t, int w, int h) {
    //  The layout has actually already been performed and the positions
    //  cached.  Apply the cached values to the children.
    const int count = getChildCount();

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            const RelativeLayout::LayoutParams* st =
                    (RelativeLayout::LayoutParams*) child->getLayoutParams();
            child->layout(st->mLeft, st->mTop, st->mRight-st->mLeft, st->mBottom-st->mTop);
        }
    }
}

RelativeLayout::LayoutParams* RelativeLayout::generateLayoutParams(const AttributeSet& attrs)const{
    return new RelativeLayout::LayoutParams(getContext(), attrs);
}

RelativeLayout::LayoutParams* RelativeLayout::generateDefaultLayoutParams()const{
    return new LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
}

bool RelativeLayout::checkLayoutParams(const ViewGroup::LayoutParams* p)const{
    return dynamic_cast<const RelativeLayout::LayoutParams*>(p);
}

RelativeLayout::LayoutParams* RelativeLayout::generateLayoutParams(const ViewGroup::LayoutParams* lp)const{
    if (sPreserveMarginParamsInLayoutParamConversion) {
        if (dynamic_cast<const LayoutParams*>(lp)) {
            return new LayoutParams((const LayoutParams&)*lp);
        } else if (dynamic_cast<const MarginLayoutParams*>(lp)) {
            return new LayoutParams((const MarginLayoutParams&)*lp);
        }
    }
    return new LayoutParams(*lp);
}

bool RelativeLayout::dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event){
    /*if (mTopToBottomLeftToRightSet == null) {
        mTopToBottomLeftToRightSet = new TreeSet<View>(new TopToBottomLeftToRightComparator());
    }*/

    // sort children top-to-bottom and left-to-right
    for (int i = 0, count = getChildCount(); i < count; i++) {
        mTopToBottomLeftToRightSet.insert(getChildAt(i));
    }

    for (View* view : mTopToBottomLeftToRightSet) {
        if ((view->getVisibility() == View::VISIBLE)
                && view->dispatchPopulateAccessibilityEvent(event)) {
            mTopToBottomLeftToRightSet.clear();
            return true;
        }
    }
    mTopToBottomLeftToRightSet.clear();
    return false;
}

std::string RelativeLayout::getAccessibilityClassName()const {
    return "RelativeLayout";
}

bool RelativeLayout::TopToBottomLeftToRightComparator::operator()(const View* first, const View* second)const{
    const int topDifference = first->getTop() - second->getTop();
    if (topDifference != 0) {
        return topDifference;
    }
    // left - right
    const int leftDifference = first->getLeft() - second->getLeft();
    if (leftDifference != 0) {
        return leftDifference;
    }
    // break tie by height
    const int heightDiference = first->getHeight() - second->getHeight();
    if (heightDiference != 0) {
        return heightDiference;
    }
    // break tie by width
    const int widthDiference = first->getWidth() - second->getWidth();
    if (widthDiference != 0) {
        return widthDiference;
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
RelativeLayout::LayoutParams::LayoutParams(int w, int h)
  :MarginLayoutParams(w,h){
    alignWithParent= false;
    mRulesChanged  = false;
    mIsRtlCompatibilityMode= false;
    memset(mRules,0,sizeof(mRules));
    memset(mInitialRules,0,sizeof(mInitialRules));
    mLeft = mTop = mRight = mBottom = VALUE_NOT_SET;
    mNeedsLayoutResolution = false;
}

RelativeLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
  :MarginLayoutParams(source){
    mRulesChanged  = false;
    alignWithParent= false;
    memset(mRules,0,sizeof(mRules));
    memset(mInitialRules,0,sizeof(mInitialRules));
    mLeft = mTop = mRight = mBottom = 0;//VALUE_NOT_SET;
    mIsRtlCompatibilityMode= false;
    mNeedsLayoutResolution = false;
}

RelativeLayout::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams& source)
  :MarginLayoutParams(source){
    mRulesChanged  = false;
    alignWithParent= false;
    mIsRtlCompatibilityMode=false;
    memset(mRules,0,sizeof(mRules));
    memset(mInitialRules,0,sizeof(mInitialRules));
    mLeft = mTop = mRight = mBottom = VALUE_NOT_SET;
    mNeedsLayoutResolution = false;
}

RelativeLayout::LayoutParams::LayoutParams(const RelativeLayout::LayoutParams& source)
  :MarginLayoutParams(source){
    mIsRtlCompatibilityMode = source.mIsRtlCompatibilityMode;
    mRulesChanged = source.mRulesChanged;
    alignWithParent= source.alignWithParent;
    memcpy(mRules,source.mRules,sizeof(mRules));
    memcpy(mInitialRules,source.mInitialRules,sizeof(mInitialRules)); 
    mLeft = source.mLeft;
    mTop  = source.mTop;
    mRight  = source.mRight;
    mBottom = source.mBottom;;
    mNeedsLayoutResolution = false;
}

RelativeLayout::LayoutParams::LayoutParams(Context*ctx,const AttributeSet&atts):MarginLayoutParams(ctx,atts){
    alignWithParent = atts.getBoolean("alignWithParentIfMissing",false);
    mLeft = mTop = mRight = mBottom = VALUE_NOT_SET;
    mRules[LEFT_OF] = atts.getResourceId("layout_toLeftOf",0);
    mRules[RIGHT_OF]= atts.getResourceId("layout_toRightOf",0);
    mRules[ABOVE]   = atts.getResourceId("layout_above",0);
    mRules[BELOW]   = atts.getResourceId("layout_below",0);
    mRules[ALIGN_BASELINE]= atts.getResourceId("layout_alignBaseline",0);
    mRules[ALIGN_LEFT]    = atts.getResourceId("layout_alignLeft",0);
    mRules[ALIGN_TOP]     = atts.getResourceId("layout_alignTop",0);
    mRules[ALIGN_RIGHT]   = atts.getResourceId("layout_alignRight",0);
    mRules[ALIGN_BOTTOM]  = atts.getResourceId("layout_alignBottom",0);

    mRules[ALIGN_PARENT_LEFT]  = atts.getBoolean("layout_alignParentLeft"  , false) ? LTRUE : 0;
    mRules[ALIGN_PARENT_TOP]   = atts.getBoolean("layout_alignParentTop"   , false) ? LTRUE : 0;    
    mRules[ALIGN_PARENT_RIGHT] = atts.getBoolean("layout_alignParentRight" , false) ? LTRUE : 0;    
    mRules[ALIGN_PARENT_BOTTOM]= atts.getBoolean("layout_alignParentBottom", false) ? LTRUE : 0;    
    
    mRules[CENTER_IN_PARENT] = atts.getBoolean("layout_centerInParent"  , false) ? LTRUE : 0;
    mRules[CENTER_HORIZONTAL]= atts.getBoolean("layout_centerHorizontal", false) ? LTRUE : 0;
    mRules[CENTER_VERTICAL]  = atts.getBoolean("layout_centerVertical"  , false) ? LTRUE : 0;

    mRules[START_OF]   = atts.getResourceId("layout_toStartOf",0);
    mRules[END_OF]     = atts.getResourceId("layout_toEndOf",0);
    mRules[ALIGN_START]= atts.getResourceId("layout_alignStart",0);
    mRules[ALIGN_END]  = atts.getResourceId("layout_alignEnd",0);

    mRules[ALIGN_PARENT_START] = atts.getBoolean("layout_alignParentStart", false) ? LTRUE : 0;
    mRules[ALIGN_PARENT_END]   = atts.getBoolean("layout_alignParentEnd"  , false) ? LTRUE : 0;
    mRulesChanged = true;
    mNeedsLayoutResolution = false;
    memcpy(mInitialRules,mRules,sizeof(mRules));
}

void RelativeLayout::LayoutParams::addRule(int verb) {
    addRule(verb, RelativeLayout::LTRUE);
}

void RelativeLayout::LayoutParams::addRule(int verb, int subject) {
    // If we're removing a relative rule, we'll need to force layout
    // resolution the next time it's requested.
    if (!mNeedsLayoutResolution && isRelativeRule(verb)
            && mInitialRules[verb] != 0 && subject == 0) {
        mNeedsLayoutResolution = true;
    }

    mRules[verb] = subject;
    mInitialRules[verb] = subject;
    mRulesChanged = true;
}

void RelativeLayout::LayoutParams::removeRule(int verb) {
    addRule(verb, 0);
}

int RelativeLayout::LayoutParams::getRule(int verb) {
    return mRules[verb];
}

bool RelativeLayout::LayoutParams::hasRelativeRules() {
    return (mInitialRules[START_OF] != 0 || mInitialRules[END_OF] != 0 ||
            mInitialRules[ALIGN_START] != 0 || mInitialRules[ALIGN_END] != 0 ||
            mInitialRules[ALIGN_PARENT_START] != 0 || mInitialRules[ALIGN_PARENT_END] != 0);
}

bool RelativeLayout::LayoutParams::isRelativeRule(int rule) {
    return rule == START_OF || rule == END_OF
            || rule == ALIGN_START || rule == ALIGN_END
            || rule == ALIGN_PARENT_START || rule == ALIGN_PARENT_END;
}

void RelativeLayout::LayoutParams::resolveRules(int layoutDirection) {
    const bool isLayoutRtl = (layoutDirection == View::LAYOUT_DIRECTION_RTL);

    // Reset to initial state
    memcpy(mRules,mInitialRules,sizeof(mRules));//  System.arraycopy(mInitialRules, LEFT_OF, mRules, LEFT_OF, VERB_COUNT);

    // Apply rules depending on direction and if we are in RTL compatibility mode
    if (mIsRtlCompatibilityMode) {
        if (mRules[ALIGN_START] != 0) {
            if (mRules[ALIGN_LEFT] == 0) {
                // "left" rule is not defined but "start" rule is: use the "start" rule as
                // the "left" rule
                mRules[ALIGN_LEFT] = mRules[ALIGN_START];
            }
            mRules[ALIGN_START] = 0;
        }

        if (mRules[ALIGN_END] != 0) {
            if (mRules[ALIGN_RIGHT] == 0) {
                // "right" rule is not defined but "end" rule is: use the "end" rule as the
                // "right" rule
                mRules[ALIGN_RIGHT] = mRules[ALIGN_END];
            }
            mRules[ALIGN_END] = 0;
        }

        if (mRules[START_OF] != 0) {
            if (mRules[LEFT_OF] == 0) {
                // "left" rule is not defined but "start" rule is: use the "start" rule as
                // the "left" rule
                mRules[LEFT_OF] = mRules[START_OF];
            }
            mRules[START_OF] = 0;
        }

        if (mRules[END_OF] != 0) {
            if (mRules[RIGHT_OF] == 0) {
                // "right" rule is not defined but "end" rule is: use the "end" rule as the
                // "right" rule
                mRules[RIGHT_OF] = mRules[END_OF];
            }
            mRules[END_OF] = 0;
        }

        if (mRules[ALIGN_PARENT_START] != 0) {
            if (mRules[ALIGN_PARENT_LEFT] == 0) {
                // "left" rule is not defined but "start" rule is: use the "start" rule as
                // the "left" rule
                mRules[ALIGN_PARENT_LEFT] = mRules[ALIGN_PARENT_START];
            }
            mRules[ALIGN_PARENT_START] = 0;
        }

        if (mRules[ALIGN_PARENT_END] != 0) {
            if (mRules[ALIGN_PARENT_RIGHT] == 0) {
                // "right" rule is not defined but "end" rule is: use the "end" rule as the
                // "right" rule
                mRules[ALIGN_PARENT_RIGHT] = mRules[ALIGN_PARENT_END];
            }
            mRules[ALIGN_PARENT_END] = 0;
        }
    } else {
        // JB MR1+ case
        if ((mRules[ALIGN_START] != 0 || mRules[ALIGN_END] != 0) &&
                (mRules[ALIGN_LEFT] != 0 || mRules[ALIGN_RIGHT] != 0)) {
            // "start"/"end" rules take precedence over "left"/"right" rules
            mRules[ALIGN_LEFT] = 0;
            mRules[ALIGN_RIGHT] = 0;
        }
        if (mRules[ALIGN_START] != 0) {
            // "start" rule resolved to "left" or "right" depending on the direction
            mRules[isLayoutRtl ? ALIGN_RIGHT : ALIGN_LEFT] = mRules[ALIGN_START];
            mRules[ALIGN_START] = 0;
        }
        if (mRules[ALIGN_END] != 0) {
            // "end" rule resolved to "left" or "right" depending on the direction
            mRules[isLayoutRtl ? ALIGN_LEFT : ALIGN_RIGHT] = mRules[ALIGN_END];
            mRules[ALIGN_END] = 0;
        }

        if ((mRules[START_OF] != 0 || mRules[END_OF] != 0) &&
                (mRules[LEFT_OF] != 0 || mRules[RIGHT_OF] != 0)) {
            // "start"/"end" rules take precedence over "left"/"right" rules
            mRules[LEFT_OF] = 0;
            mRules[RIGHT_OF] = 0;
        }
        if (mRules[START_OF] != 0) {
            // "start" rule resolved to "left" or "right" depending on the direction
            mRules[isLayoutRtl ? RIGHT_OF : LEFT_OF] = mRules[START_OF];
            mRules[START_OF] = 0;
        }
        if (mRules[END_OF] != 0) {
            // "end" rule resolved to "left" or "right" depending on the direction
            mRules[isLayoutRtl ? LEFT_OF : RIGHT_OF] = mRules[END_OF];
            mRules[END_OF] = 0;
        }

        if ((mRules[ALIGN_PARENT_START] != 0 || mRules[ALIGN_PARENT_END] != 0) &&
                (mRules[ALIGN_PARENT_LEFT] != 0 || mRules[ALIGN_PARENT_RIGHT] != 0)) {
            // "start"/"end" rules take precedence over "left"/"right" rules
            mRules[ALIGN_PARENT_LEFT] = 0;
            mRules[ALIGN_PARENT_RIGHT] = 0;
        }
        if (mRules[ALIGN_PARENT_START] != 0) {
            // "start" rule resolved to "left" or "right" depending on the direction
            mRules[isLayoutRtl ? ALIGN_PARENT_RIGHT : ALIGN_PARENT_LEFT] = mRules[ALIGN_PARENT_START];
            mRules[ALIGN_PARENT_START] = 0;
        }
        if (mRules[ALIGN_PARENT_END] != 0) {
            // "end" rule resolved to "left" or "right" depending on the direction
            mRules[isLayoutRtl ? ALIGN_PARENT_LEFT : ALIGN_PARENT_RIGHT] = mRules[ALIGN_PARENT_END];
            mRules[ALIGN_PARENT_END] = 0;
        }
    }

    mRulesChanged = false;
    mNeedsLayoutResolution = false;
}

const int* RelativeLayout::LayoutParams::getRules(int layoutDirection) {
    resolveLayoutDirection(layoutDirection);
    return mRules;
}

const int* RelativeLayout::LayoutParams::getRules() {
    return mRules;
}

void RelativeLayout::LayoutParams::resolveLayoutDirection(int layoutDirection) {
    if (shouldResolveLayoutDirection(layoutDirection)) {
        resolveRules(layoutDirection);
    }
    // This will set the layout direction.
    ViewGroup::MarginLayoutParams::resolveLayoutDirection(layoutDirection);
}

bool RelativeLayout::LayoutParams::shouldResolveLayoutDirection(int layoutDirection) {
    return (mNeedsLayoutResolution || hasRelativeRules())
            && (mRulesChanged || layoutDirection != getLayoutDirection());
}

////////////////////////////////////////////////////////////////////////////////////////////

RelativeLayout::DependencyGraph::Node::Node(View*v){
    view=v;
}

RelativeLayout::DependencyGraph::Node::~Node(){
    dependents.clear();
    dependencies.clear();
}

RelativeLayout::DependencyGraph::DependencyGraph(){
}

RelativeLayout::DependencyGraph::~DependencyGraph(){
    clear();
}

void RelativeLayout::DependencyGraph::clear(){
    for (Node* node:mNodes) {
        delete node;
    }
    mNodes.clear();
    mKeyNodes.clear();
    mRoots.clear();
}

void RelativeLayout::DependencyGraph::add(View* view) {
    const int id = view->getId();
    Node* node = new Node(view);//Node::acquire(view);
    if (id != NO_ID) {
        mKeyNodes.put(id, node);
    }
    mNodes.push_back(node);
}

/**
 * Builds a sorted list of views. The sorting order depends on the dependencies
 * between the view. For instance, if view C needs view A to be processed first
 * and view A needs view B to be processed first, the dependency graph
 * is: B -> A -> C. The sorted array will contain views B, A and C in this order.
 *
 * @param sorted The sorted list of views. The length of this array must
 *        be equal to getChildCount().
 * @param rules The list of rules to take into account.
 */
void RelativeLayout::DependencyGraph::getSortedViews(std::vector<View*>&sorted,const int* rules,size_t rulesCount){
    std::list<Node*> roots = findRoots(rules,rulesCount);
    int index = 0;
    const int count=roots.size();
 
    Node* node = nullptr;
    while ((roots.empty()==false)&&(node = roots.back()) != nullptr) {
        View* view = node->view;
        const int key = view->getId();
        sorted[index++] = view;
        roots.pop_back();

        std::unordered_map<Node*, DependencyGraph*>& dependents = node->dependents;
        for (auto dep:dependents) {
            Node* dependent = dep.first;//dependents.keyAt(i);
            SparseArray<Node*>& dependencies = dependent->dependencies;
            
            dependencies.remove(key);
            if (dependencies.size() == 0) {
                roots.push_back(dependent);
            }
        }
    }
    LOGE_IF(index < sorted.size(),"Circular dependencies cannot exist in RelativeLayout %d,%d",index,sorted.size());
}

std::list<RelativeLayout::DependencyGraph::Node*> RelativeLayout::DependencyGraph::findRoots(const int* rulesFilter,size_t rulesCount){

    // Find roots can be invoked several times, so make sure to clear
    // all dependents and dependencies before running the algorithm
    for (Node*node:mNodes) {
        node->dependents.clear();
        node->dependencies.clear();
    }

    // Builds up the dependents and dependencies for each node of the graph
    for (Node*node:mNodes) {

        LayoutParams* layoutParams = (LayoutParams*) node->view->getLayoutParams();
        const int* rules = layoutParams->mRules;

        // Look only the the rules passed in parameter, this way we build only the
        // dependencies for a specific set of rules
        for (int j = 0; j < rulesCount; j++) {
            int rule = rules[rulesFilter[j]];
            if (rule > 0) {
                // The node this node depends on
                Node* dependency = mKeyNodes.get(rule);
                // Skip unknowns and self dependencies
                if (dependency == nullptr || dependency == node) {
                    continue;
                }
                // Add the current node as a dependent
                dependency->dependents.emplace(node, this);
                // Add a dependency to the current node
                node->dependencies.put(rule, dependency);
            }
        }
    }

    std::list<Node*>& roots = mRoots;
    roots.clear();

    // Finds all the roots in the graph: all nodes with no dependencies
    for (Node*node:mNodes) {
        LOGV("Roots::node %p:%8d  depends=%8d %8d %s",node->view,node->view->getId(),
            node->dependents.size(),node->dependencies.size(),
            (rulesCount==sizeof(RULES_VERTICAL)/sizeof(RULES_VERTICAL[0]))?"Vertical":"Horizontal");
        if (node->dependencies.size() == 0)
            roots.push_back(node);
    }
    return roots;
}

}//endof namespace
