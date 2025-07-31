#include <widgetEx/flexbox/flexboxlayout.h>
//REF:https://github.com/google/flexbox-layout/tree/main
namespace cdroid{

DECLARE_WIDGET(FlexboxLayout)

FlexboxLayout::FlexboxLayout(Context* context,const AttributeSet& attrs):ViewGroup(context,attrs){
    init();
    mFlexDirection = attrs.getInt("flexDirection",std::unordered_map<std::string,int>{
            {"column",FlexDirection::COLUMN},
            {"column_reverse",FlexDirection::COLUMN_REVERSE},
            {"row",FlexDirection::ROW},
            {"row_reverse",FlexDirection::ROW_REVERSE},
        }, (int)FlexDirection::ROW);
    mFlexWrap = attrs.getInt("flexWrap",std::unordered_map<std::string,int>{
            {"nowrap" , FlexWrap::NOWRAP},
            {"wrap" , FlexWrap::WRAP},
            {"wrap_reverse" , FlexWrap::WRAP_REVERSE}
        }, (int)FlexWrap::NOWRAP);
    mJustifyContent = attrs.getInt("justifyContent",std::unordered_map<std::string,int>{
            {"flex_start",JustifyContent::FLEX_START},
            {"flex_end", JustifyContent::FLEX_END},
            {"center"  , JustifyContent::CENTER},
            {"space_between", JustifyContent::SPACE_BETWEEN},
            {"space_around" , JustifyContent::SPACE_AROUND},
            {"space_evenly" , JustifyContent::SPACE_EVENLY}
        }, (int)JustifyContent::FLEX_START);
    mAlignItems = attrs.getInt("alignItems",std::unordered_map<std::string,int>{
            {"flex_start",(int)AlignItems::FLEX_START},
            {"flex_end", (int)AlignItems::FLEX_END},
            {"center"  , (int)AlignItems::CENTER},
            {"baseline", (int)AlignItems::BASELINE},
            {"stretch" , (int)AlignItems::STRETCH}
        }, (int)AlignItems::FLEX_START);
    mAlignContent = attrs.getInt("alignContent",std::unordered_map<std::string,int>{
            {"flex_start", (int)AlignContent::FLEX_START},
            {"flex_end"  , (int)AlignContent::FLEX_END},
            {"center" , (int)AlignContent::CENTER},
            {"space_between",(int)AlignContent::SPACE_BETWEEN},
            {"space_around" ,(int)AlignContent::SPACE_AROUND},
            {"stretch" , (int)AlignContent::STRETCH}
        }, (int)AlignContent::FLEX_START);
    mMaxLine = attrs.getInt("maxLine", NOT_SET);
    Drawable* drawable = attrs.getDrawable("dividerDrawable");
    if (drawable != nullptr) {
        setDividerDrawableHorizontal(drawable);
        setDividerDrawableVertical(drawable);
    }
    Drawable* drawableHorizontal = attrs.getDrawable("dividerDrawableHorizontal");
    if (drawableHorizontal != nullptr) {
        setDividerDrawableHorizontal(drawableHorizontal);
    }
    Drawable* drawableVertical = attrs.getDrawable("dividerDrawableVertical");
    if (drawableVertical != nullptr) {
        setDividerDrawableVertical(drawableVertical);
    }
    std::unordered_map<std::string,int>divs={
            {"beginning",(int)SHOW_DIVIDER_BEGINNING},
            {"midle",(int)SHOW_DIVIDER_MIDDLE},
            {"end",(int)SHOW_DIVIDER_END},
            {"none",(int)SHOW_DIVIDER_NONE}
        };
    int dividerMode = attrs.getInt("showDivider",divs,SHOW_DIVIDER_NONE);
    if (dividerMode != SHOW_DIVIDER_NONE) {
        mShowDividerVertical = dividerMode;
        mShowDividerHorizontal = dividerMode;
    }
    int dividerModeVertical = attrs.getInt("showDividerVertical",divs, SHOW_DIVIDER_NONE);
    if (dividerModeVertical != SHOW_DIVIDER_NONE) {
        mShowDividerVertical = dividerModeVertical;
    }
    int dividerModeHorizontal = attrs.getInt("showDividerHorizontal",divs, SHOW_DIVIDER_NONE);
    if (dividerModeHorizontal != SHOW_DIVIDER_NONE) {
        mShowDividerHorizontal = dividerModeHorizontal;
    }
}

FlexboxLayout::~FlexboxLayout(){
    delete mFlexLinesResult;
    delete mFlexboxHelper;
    delete mFlexContainer;
    if(mDividerDrawableHorizontal==mDividerDrawableVertical){
        delete mDividerDrawableHorizontal;
    }else{
        delete mDividerDrawableHorizontal;
        delete mDividerDrawableVertical;
    }
}

namespace{
class FLContainer:public FlexContainer{
private:
    FlexboxLayout*mFBL;
public:
    FLContainer(FlexboxLayout*fbl){mFBL=fbl;}
    int getFlexItemCount()override{ return mFBL->getFlexItemCount();}
    View* getFlexItemAt(int index)override{return mFBL->getFlexItemAt(index);}
    View* getReorderedFlexItemAt(int index)override{return mFBL->getReorderedFlexItemAt(index);}

    void addView(View* view)override{addView(view,-1);}
    void addView(View* view, int index)override{((ViewGroup*)mFBL)->addView(view,index);}
    void removeAllViews()override{mFBL->removeAllViews();}
    void removeViewAt(int index)override{mFBL->removeViewAt(index);}
    int getFlexDirection()override{return mFBL->getFlexDirection();}
    void setFlexDirection(int flexDirection)override{mFBL->setFlexDirection(flexDirection);}
    int getFlexWrap()override{return mFBL->getFlexWrap();}
    void setFlexWrap(int flexWrap)override{ mFBL->setFlexWrap(flexWrap);}
    int getJustifyContent()override{return mFBL->getJustifyContent();}
    void setJustifyContent(int justifyContent)override{mFBL->setJustifyContent(justifyContent);}

    int getAlignContent()override{return mFBL->getAlignContent();}
    void setAlignContent(int alignContent)override{mFBL->setAlignContent(alignContent);}

    int getAlignItems()override{return mFBL->getAlignItems();}
    void setAlignItems(int alignItems)override{mFBL->setAlignItems(alignItems);}
    std::vector<FlexLine> getFlexLines()override{return mFBL->getFlexLines();}

    bool isMainAxisDirectionHorizontal()override{return mFBL->isMainAxisDirectionHorizontal();}

    int getDecorationLengthMainAxis(View* view, int index, int indexInFlexLine)override{
        return mFBL->getDecorationLengthMainAxis(view,index,indexInFlexLine);
    }
    int getDecorationLengthCrossAxis(View* view)override{
        return mFBL->getDecorationLengthCrossAxis(view);
    }

    int getPaddingTop()override{return mFBL->getPaddingTop();}
    int getPaddingLeft()override{return mFBL->getPaddingLeft();}
    int getPaddingRight()override{return mFBL->getPaddingRight();}
    int getPaddingBottom()override{return mFBL->getPaddingBottom();}
    int getPaddingStart()override{return mFBL->getPaddingStart();}
    int getPaddingEnd()override{return mFBL->getPaddingEnd();}

    int getChildWidthMeasureSpec(int widthSpec, int padding, int childDimension){
        return mFBL->getChildWidthMeasureSpec(widthSpec,padding,childDimension);
    }
    int getChildHeightMeasureSpec(int heightSpec, int padding, int childDimension)override{
        return mFBL->getChildHeightMeasureSpec(heightSpec,padding,childDimension);
    }

    int getLargestMainSize()override{return mFBL->getLargestMainSize();}
    int getSumOfCrossSize()override{return mFBL->getSumOfCrossSize();}

    void onNewFlexItemAdded(View* view, int index, int indexInFlexLine, FlexLine& flexLine)override{
        mFBL->onNewFlexItemAdded(view,index,indexInFlexLine,flexLine);
    }
    void onNewFlexLineAdded(FlexLine& flexLine)override{mFBL->onNewFlexLineAdded(flexLine);}
    void setFlexLines(const std::vector<FlexLine>& flexLines)override{mFBL->setFlexLines(flexLines);}

    int getMaxLine()override{return mFBL->getMaxLine();}
    void setMaxLine(int maxLine)override{mFBL->setMaxLine(maxLine);}

    std::vector<FlexLine> getFlexLinesInternal()override{return mFBL->getFlexLinesInternal();}
    void updateViewCache(int position, View* view)override{mFBL->updateViewCache(position,view);}
};
}
void FlexboxLayout::init(){
    mFlexDirection= FlexDirection::ROW;
    mFlexWrap = FlexWrap::NOWRAP;
    mJustifyContent = JustifyContent::FLEX_START;
    mAlignItems  = (int)AlignItems::FLEX_START;
    mAlignContent= (int)AlignContent::FLEX_START;
    mMaxLine = NOT_SET;
    mShowDividerVertical  = SHOW_DIVIDER_NONE;
    mShowDividerHorizontal= SHOW_DIVIDER_NONE;

    mDividerVerticalWidth = 0;
    mDividerHorizontalHeight = 0;
    mFlexLinesResult = new FlexboxHelper::FlexLinesResult;
    mFlexContainer = new FLContainer(this);
    mFlexboxHelper = new FlexboxHelper(mFlexContainer);
    mDividerDrawableHorizontal = nullptr;
    mDividerDrawableVertical = nullptr;
}

void FlexboxLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    if (mFlexboxHelper->isOrderChangedFromLastMeasurement(mOrderCache)) {
        mReorderedIndices = mFlexboxHelper->createReorderedIndices(mOrderCache);
    }

    // TODO: Only calculate the children views which are affected from the last measure.

    switch (mFlexDirection) {
    case FlexDirection::ROW: // Intentional fall through
    case FlexDirection::ROW_REVERSE:
        measureHorizontal(widthMeasureSpec, heightMeasureSpec);
        break;
    case FlexDirection::COLUMN: // Intentional fall through
    case FlexDirection::COLUMN_REVERSE:
        measureVertical(widthMeasureSpec, heightMeasureSpec);
        break;
    default:
        LOGE("Invalid value for the flex direction %d is set" ,mFlexDirection);
    }
}

int FlexboxLayout::getFlexItemCount() {
    return getChildCount();
}

View* FlexboxLayout::getFlexItemAt(int index) {
    return getChildAt(index);
}

View* FlexboxLayout::getReorderedChildAt(int index) {
    if ((index < 0) || (index >= mReorderedIndices.size())) {
        return nullptr;
    }
    return getChildAt(mReorderedIndices[index]);
}

View* FlexboxLayout::getReorderedFlexItemAt(int index) {
    return getReorderedChildAt(index);
}

void FlexboxLayout::addView(View* child, int index, ViewGroup::LayoutParams* params) {
    // Create an array for the reordered indices before the View is added in the parent
    // ViewGroup since otherwise reordered indices won't be in effect before the
    // FlexboxLayout's onMeasure is called.
    // Because requestLayout is requested in the super.addView method.
    mReorderedIndices = mFlexboxHelper->createReorderedIndices(child, index, params, mOrderCache);
    ViewGroup::addView(child, index, params);
}

void FlexboxLayout::measureHorizontal(int widthMeasureSpec, int heightMeasureSpec) {
    mFlexLines.clear();

    mFlexLinesResult->reset();
    mFlexboxHelper->calculateHorizontalFlexLines(mFlexLinesResult, widthMeasureSpec,heightMeasureSpec);
    mFlexLines = mFlexLinesResult->mFlexLines;

    mFlexboxHelper->determineMainSize(widthMeasureSpec, heightMeasureSpec);

    // TODO: Consider the case any individual child's mAlignSelf is set to ALIGN_SELF_BASELINE
    if (mAlignItems == (int)AlignItems::BASELINE) {
        for (FlexLine& flexLine : mFlexLines) {
            // The largest height value that also take the baseline shift into account
            int largestHeightInLine = INT_MIN;
            for (int i = 0; i < flexLine.mItemCount; i++) {
                int viewIndex = flexLine.mFirstIndex + i;
                View* child = getReorderedChildAt(viewIndex);
                if (child == nullptr || child->getVisibility() == View::GONE) {
                    continue;
                }
                LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
                if (mFlexWrap != FlexWrap::WRAP_REVERSE) {
                    int marginTop = flexLine.mMaxBaseline - child->getBaseline();
                    marginTop = std::max(marginTop, lp->topMargin);
                    largestHeightInLine = std::max(largestHeightInLine,
                            child->getMeasuredHeight() + marginTop + lp->bottomMargin);
                } else {
                    int marginBottom = flexLine.mMaxBaseline - child->getMeasuredHeight() +
                            child->getBaseline();
                    marginBottom = std::max(marginBottom, lp->bottomMargin);
                    largestHeightInLine = std::max(largestHeightInLine,
                            child->getMeasuredHeight() + lp->topMargin + marginBottom);
                }
            }
            flexLine.mCrossSize = largestHeightInLine;
        }
    }

    mFlexboxHelper->determineCrossSize(widthMeasureSpec, heightMeasureSpec,
            getPaddingTop() + getPaddingBottom());
    // Now cross size for each flex line is determined.
    // Expand the views if alignItems (or mAlignSelf in each child view) is set to stretch
    mFlexboxHelper->stretchViews();
    setMeasuredDimensionForFlex(mFlexDirection, widthMeasureSpec, heightMeasureSpec,mFlexLinesResult->mChildState);
}

void FlexboxLayout::measureVertical(int widthMeasureSpec, int heightMeasureSpec) {
    mFlexLines.clear();
    mFlexLinesResult->reset();
    mFlexboxHelper->calculateVerticalFlexLines(mFlexLinesResult, widthMeasureSpec,heightMeasureSpec);
    mFlexLines = mFlexLinesResult->mFlexLines;

    mFlexboxHelper->determineMainSize(widthMeasureSpec, heightMeasureSpec);
    mFlexboxHelper->determineCrossSize(widthMeasureSpec, heightMeasureSpec,getPaddingLeft() + getPaddingRight());
    // Now cross size for each flex line is determined.
    // Expand the views if alignItems (or mAlignSelf in each child view) is set to stretch
    mFlexboxHelper->stretchViews();
    setMeasuredDimensionForFlex(mFlexDirection, widthMeasureSpec, heightMeasureSpec,mFlexLinesResult->mChildState);
}

void FlexboxLayout::setMeasuredDimensionForFlex(int flexDirection, int widthMeasureSpec,int heightMeasureSpec, int childState) {
    int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    int heightSize = MeasureSpec::getSize(heightMeasureSpec);
    int calculatedMaxHeight;
    int calculatedMaxWidth;
    switch (flexDirection) {
    case FlexDirection::ROW: // Intentional fall through
    case FlexDirection::ROW_REVERSE:
        calculatedMaxHeight = getSumOfCrossSize() + getPaddingTop() + getPaddingBottom();
        calculatedMaxWidth = getLargestMainSize();
        break;
    case FlexDirection::COLUMN: // Intentional fall through
    case FlexDirection::COLUMN_REVERSE:
        calculatedMaxHeight = getLargestMainSize();
        calculatedMaxWidth = getSumOfCrossSize() + getPaddingLeft() + getPaddingRight();
        break;
    default:
        LOGE("Invalid flex %d direction: " , flexDirection);
    }

    int widthSizeAndState;
    switch (widthMode) {
    case MeasureSpec::EXACTLY:
        if (widthSize < calculatedMaxWidth) {
            childState = View::combineMeasuredStates(childState, View::MEASURED_STATE_TOO_SMALL);
        }
        widthSizeAndState = View::resolveSizeAndState(widthSize, widthMeasureSpec, childState);
        break;
    case MeasureSpec::AT_MOST: {
        if (widthSize < calculatedMaxWidth) {
            childState = View::combineMeasuredStates(childState, View::MEASURED_STATE_TOO_SMALL);
        } else {
            widthSize = calculatedMaxWidth;
        }
        widthSizeAndState = View::resolveSizeAndState(widthSize, widthMeasureSpec,childState);
        break;
    }
    case MeasureSpec::UNSPECIFIED: {
        widthSizeAndState = View::resolveSizeAndState(calculatedMaxWidth, widthMeasureSpec, childState);
        break;
    }
    default:
        LOGE("Unknown width mode %x is set" , widthMode);
    }
    int heightSizeAndState;
    switch (heightMode) {
    case MeasureSpec::EXACTLY:
        if (heightSize < calculatedMaxHeight) {
            childState = View::combineMeasuredStates(childState,
                    View::MEASURED_STATE_TOO_SMALL >> View::MEASURED_HEIGHT_STATE_SHIFT);
        }
        heightSizeAndState = View::resolveSizeAndState(heightSize, heightMeasureSpec,childState);
        break;
    case MeasureSpec::AT_MOST: {
        if (heightSize < calculatedMaxHeight) {
            childState = View::combineMeasuredStates(childState,
                    View::MEASURED_STATE_TOO_SMALL >> View::MEASURED_HEIGHT_STATE_SHIFT);
        } else {
            heightSize = calculatedMaxHeight;
        }
        heightSizeAndState = View::resolveSizeAndState(heightSize, heightMeasureSpec,childState);
        break;
    }
    case MeasureSpec::UNSPECIFIED: {
        heightSizeAndState = View::resolveSizeAndState(calculatedMaxHeight,heightMeasureSpec, childState);
        break;
    }
    default:
        LOGE("Unknown height mode %x is set" ,heightMode);
    }
    setMeasuredDimension(widthSizeAndState, heightSizeAndState);
}

int FlexboxLayout::getLargestMainSize() {
    int largestSize = INT_MIN;
    for (FlexLine& flexLine : mFlexLines) {
        largestSize = std::max(largestSize, flexLine.mMainSize);
    }
    return largestSize;
}

int FlexboxLayout::getSumOfCrossSize() {
    int sum = 0;
    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
        FlexLine& flexLine = mFlexLines.at(i);

        // Judge if the beginning or middle dividers are required
        if (hasDividerBeforeFlexLine(i)) {
            if (isMainAxisDirectionHorizontal()) {
                sum += mDividerHorizontalHeight;
            } else {
                sum += mDividerVerticalWidth;
            }
        }

        // Judge if the end divider is required
        if (hasEndDividerAfterFlexLine(i)) {
            if (isMainAxisDirectionHorizontal()) {
                sum += mDividerHorizontalHeight;
            } else {
                sum += mDividerVerticalWidth;
            }
        }
        sum += flexLine.mCrossSize;
    }
    return sum;
}

bool FlexboxLayout::isMainAxisDirectionHorizontal() {
    return (mFlexDirection == FlexDirection::ROW) || (mFlexDirection == FlexDirection::ROW_REVERSE);
}

void FlexboxLayout::onLayout(bool changed, int left, int top, int width, int height) {
    const int layoutDirection = getLayoutDirection();
    bool isRtl = false;
    switch (mFlexDirection) {
        case FlexDirection::ROW:
            isRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
            layoutHorizontal(isRtl, left, top, width , height);
            break;
        case FlexDirection::ROW_REVERSE:
            isRtl = layoutDirection != View::LAYOUT_DIRECTION_RTL;
            layoutHorizontal(isRtl, left, top, width , height);
            break;
        case FlexDirection::COLUMN:
            isRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
            if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
                isRtl = !isRtl;
            }
            layoutVertical(isRtl, false, left, top, width , height);
            break;
        case FlexDirection::COLUMN_REVERSE:
            isRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
            if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
                isRtl = !isRtl;
            }
            layoutVertical(isRtl, true, left, top, width , height);
            break;
        default:
            LOGE("Invalid flex direction %d is set" , mFlexDirection);
    }
}

void FlexboxLayout::layoutHorizontal(bool isRtl, int left, int top, int width, int height) {
    int paddingLeft = getPaddingLeft();
    int paddingRight = getPaddingRight();
    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childLeft;

    // childBottom is used if the mFlexWrap is WRAP_REVERSE otherwise
    // childTop is used to align the vertical position of the children views.
    int childBottom = height - getPaddingBottom();
    int childTop = getPaddingTop();

    // Used only for RTL layout
    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childRight;
    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
        FlexLine& flexLine = mFlexLines.at(i);
        if (hasDividerBeforeFlexLine(i)) {
            childBottom -= mDividerHorizontalHeight;
            childTop += mDividerHorizontalHeight;
        }
        float spaceBetweenItem = 0.f;
        switch (mJustifyContent) {
        case JustifyContent::FLEX_START:
            childLeft = paddingLeft;
            childRight = width - paddingRight;
            break;
        case JustifyContent::FLEX_END:
            childLeft = width - flexLine.mMainSize + paddingRight;
            childRight = flexLine.mMainSize - paddingLeft;
            break;
        case JustifyContent::CENTER:
            childLeft = paddingLeft + (width - flexLine.mMainSize) / 2.f;
            childRight = width - paddingRight - (width - flexLine.mMainSize) / 2.f;
            break;
        case JustifyContent::SPACE_AROUND: {
            int visibleCount = flexLine.getItemCountNotGone();
            if (visibleCount != 0) {
                spaceBetweenItem = (width - flexLine.mMainSize) / (float) visibleCount;
            }
            childLeft = paddingLeft + spaceBetweenItem / 2.f;
            childRight = width - paddingRight - spaceBetweenItem / 2.f;
            break;
        }
        case JustifyContent::SPACE_BETWEEN: {
            childLeft = paddingLeft;
            int visibleCount = flexLine.getItemCountNotGone();
            float denominator = visibleCount != 1 ? visibleCount - 1 : 1.f;
            spaceBetweenItem = (width - flexLine.mMainSize) / denominator;
            childRight = width - paddingRight;
            break;
        }
        case JustifyContent::SPACE_EVENLY: {
            int visibleCount = flexLine.getItemCountNotGone();
            if (visibleCount != 0) {
                spaceBetweenItem = (width - flexLine.mMainSize) / (float) (visibleCount + 1);
            }
            childLeft = paddingLeft + spaceBetweenItem;
            childRight = width - paddingRight - spaceBetweenItem;
            break;
        }
        default:
            LOGE("Invalid justifyContent %d is set: ",mJustifyContent);
        }
        spaceBetweenItem = std::max(spaceBetweenItem, 0.f);

        for (int j = 0; j < flexLine.mItemCount; j++) {
            int index = flexLine.mFirstIndex + j;
            View* child = getReorderedChildAt(index);
            if (child == nullptr || child->getVisibility() == View::GONE) {
                continue;
            }
            LayoutParams* lp = ((LayoutParams*) child->getLayoutParams());
            childLeft += lp->leftMargin;
            childRight -= lp->rightMargin;
            int beforeDividerLength = 0;
            int endDividerLength = 0;
            if (hasDividerBeforeChildAtAlongMainAxis(index, j)) {
                beforeDividerLength = mDividerVerticalWidth;
                childLeft += beforeDividerLength;
                childRight -= beforeDividerLength;
            }
            if ( (j == flexLine.mItemCount - 1) && ((mShowDividerVertical & SHOW_DIVIDER_END) > 0) ) {
                endDividerLength = mDividerVerticalWidth;
            }

            if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
                if (isRtl) {
                    mFlexboxHelper->layoutSingleChildHorizontal(child, flexLine,
                            std::round(childRight) - child->getMeasuredWidth(),
                            childBottom - child->getMeasuredHeight(), std::round(childRight),
                            childBottom);
                } else {
                    mFlexboxHelper->layoutSingleChildHorizontal(child, flexLine,
                            std::round(childLeft), childBottom - child->getMeasuredHeight(),
                            std::round(childLeft) + child->getMeasuredWidth(), childBottom);
                }
            } else {
                if (isRtl) {
                    mFlexboxHelper->layoutSingleChildHorizontal(child, flexLine,
                            std::round(childRight) - child->getMeasuredWidth(),
                            childTop, std::round(childRight),
                            childTop + child->getMeasuredHeight());
                } else {
                    mFlexboxHelper->layoutSingleChildHorizontal(child, flexLine,
                            std::round(childLeft), childTop,
                            std::round(childLeft) + child->getMeasuredWidth(),
                            childTop + child->getMeasuredHeight());
                }
            }
            childLeft += child->getMeasuredWidth() + spaceBetweenItem + lp->rightMargin;
            childRight -= child->getMeasuredWidth() + spaceBetweenItem + lp->leftMargin;

            if (isRtl) {
                flexLine.updatePositionFromView(child, /*leftDecoration*/endDividerLength, 0,
                        /*rightDecoration*/ beforeDividerLength, 0);
            } else {
                flexLine.updatePositionFromView(child, /*leftDecoration*/beforeDividerLength, 0,
                        /*rightDecoration*/ endDividerLength, 0);
            }
        }
        childTop += flexLine.mCrossSize;
        childBottom -= flexLine.mCrossSize;
    }
}

void FlexboxLayout::layoutVertical(bool isRtl, bool fromBottomToTop, int left, int top,int width, int height) {
    int paddingTop = getPaddingTop();
    int paddingBottom = getPaddingBottom();

    int paddingRight = getPaddingRight();
    int childLeft = getPaddingLeft();

    // childRight is used if the mFlexWrap is WRAP_REVERSE otherwise
    // childLeft is used to align the horizontal position of the children views.
    int childRight = width - paddingRight;

    // Use float to reduce the round error that may happen in when justifyContent ==
    // SPACE_BETWEEN or SPACE_AROUND
    float childTop;

    // Used only for if the direction is from bottom to top
    float childBottom;

    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
        FlexLine& flexLine = mFlexLines.at(i);
        if (hasDividerBeforeFlexLine(i)) {
            childLeft += mDividerVerticalWidth;
            childRight -= mDividerVerticalWidth;
        }
        float spaceBetweenItem = 0.f;
        switch (mJustifyContent) {
        case JustifyContent::FLEX_START:
            childTop = paddingTop;
            childBottom = height - paddingBottom;
            break;
        case JustifyContent::FLEX_END:
            childTop = height - flexLine.mMainSize + paddingBottom;
            childBottom = flexLine.mMainSize - paddingTop;
            break;
        case JustifyContent::CENTER:
            childTop = paddingTop + (height - flexLine.mMainSize) / 2.f;
            childBottom = height - paddingBottom - (height - flexLine.mMainSize) / 2.f;
            break;
        case JustifyContent::SPACE_AROUND: {
            int visibleCount = flexLine.getItemCountNotGone();
            if (visibleCount != 0) {
                spaceBetweenItem = (height - flexLine.mMainSize) / (float) visibleCount;
            }
            childTop = paddingTop + spaceBetweenItem / 2.f;
            childBottom = height - paddingBottom - spaceBetweenItem / 2.f;
            break;
        }
        case JustifyContent::SPACE_BETWEEN: {
            childTop = paddingTop;
            int visibleCount = flexLine.getItemCountNotGone();
            float denominator = visibleCount != 1 ? visibleCount - 1 : 1.f;
            spaceBetweenItem = (height - flexLine.mMainSize) / denominator;
            childBottom = height - paddingBottom;
            break;
        }
        case JustifyContent::SPACE_EVENLY: {
            int visibleCount = flexLine.getItemCountNotGone();
            if (visibleCount != 0) {
                spaceBetweenItem = (height - flexLine.mMainSize)
                        / (float) (visibleCount + 1);
            }
            childTop = paddingTop + spaceBetweenItem;
            childBottom = height - paddingBottom - spaceBetweenItem;
            break;
        }
        default:
            LOGE("Invalid justifyContent %s is set",mJustifyContent);
        }
        spaceBetweenItem = std::max(spaceBetweenItem, 0.f);

        for (int j = 0; j < flexLine.mItemCount; j++) {
            int index = flexLine.mFirstIndex + j;
            View* child = getReorderedChildAt(index);
            if ((child == nullptr) || (child->getVisibility() == View::GONE) ) {
                continue;
            }
            LayoutParams* lp = ((LayoutParams*) child->getLayoutParams());
            childTop += lp->topMargin;
            childBottom -= lp->bottomMargin;
            int beforeDividerLength = 0;
            int endDividerLength = 0;
            if (hasDividerBeforeChildAtAlongMainAxis(index, j)) {
                beforeDividerLength = mDividerHorizontalHeight;
                childTop += beforeDividerLength;
                childBottom -= beforeDividerLength;
            }
            if ((j == flexLine.mItemCount - 1) && ((mShowDividerHorizontal & SHOW_DIVIDER_END) > 0) ) {
                endDividerLength = mDividerHorizontalHeight;
            }
            if (isRtl) {
                if (fromBottomToTop) {
                    mFlexboxHelper->layoutSingleChildVertical(child, flexLine, true,
                            childRight - child->getMeasuredWidth(),
                            std::round(childBottom) - child->getMeasuredHeight(), childRight,
                            std::round(childBottom));
                } else {
                    mFlexboxHelper->layoutSingleChildVertical(child, flexLine, true,
                            childRight - child->getMeasuredWidth(), std::round(childTop),
                            childRight, std::round(childTop) + child->getMeasuredHeight());
                }
            } else {
                if (fromBottomToTop) {
                    mFlexboxHelper->layoutSingleChildVertical(child, flexLine, false,
                            childLeft, std::round(childBottom) - child->getMeasuredHeight(),
                            childLeft + child->getMeasuredWidth(), std::round(childBottom));
                } else {
                    mFlexboxHelper->layoutSingleChildVertical(child, flexLine, false,
                            childLeft, std::round(childTop), childLeft + child->getMeasuredWidth(),
                            std::round(childTop) + child->getMeasuredHeight());
                }
            }
            childTop += child->getMeasuredHeight() + spaceBetweenItem + lp->bottomMargin;
            childBottom -= child->getMeasuredHeight() + spaceBetweenItem + lp->topMargin;

            if (fromBottomToTop) {
                flexLine.updatePositionFromView(child, 0, /*topDecoration*/endDividerLength, 0,
                        /*bottomDecoration*/ beforeDividerLength);
            } else {
                flexLine.updatePositionFromView(child, 0, /*topDecoration*/beforeDividerLength,
                        0, /*bottomDecoration*/endDividerLength);
            }
        }
        childLeft += flexLine.mCrossSize;
        childRight -= flexLine.mCrossSize;
    }
}


void FlexboxLayout::onDraw(Canvas& canvas) {
    if (mDividerDrawableVertical == nullptr && mDividerDrawableHorizontal == nullptr) {
        return;
    }
    if (mShowDividerHorizontal == SHOW_DIVIDER_NONE
            && mShowDividerVertical == SHOW_DIVIDER_NONE) {
        return;
    }

    const int layoutDirection = getLayoutDirection();
    bool isRtl;
    bool fromBottomToTop = false;
    switch (mFlexDirection) {
    case FlexDirection::ROW:
        isRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
        if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
            fromBottomToTop = true;
        }
        drawDividersHorizontal(canvas, isRtl, fromBottomToTop);
        break;
    case FlexDirection::ROW_REVERSE:
        isRtl = layoutDirection != View::LAYOUT_DIRECTION_RTL;
        if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
            fromBottomToTop = true;
        }
        drawDividersHorizontal(canvas, isRtl, fromBottomToTop);
        break;
    case FlexDirection::COLUMN:
        isRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
        if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
            isRtl = !isRtl;
        }
        drawDividersVertical(canvas, isRtl, false);
        break;
    case FlexDirection::COLUMN_REVERSE:
        isRtl = layoutDirection == View::LAYOUT_DIRECTION_RTL;
        if (mFlexWrap == FlexWrap::WRAP_REVERSE) {
            isRtl = !isRtl;
        }
        drawDividersVertical(canvas, isRtl, true);
        break;
    }
}

void FlexboxLayout::drawDividersHorizontal(Canvas& canvas, bool isRtl, bool fromBottomToTop) {
    int paddingLeft = getPaddingLeft();
    int paddingRight = getPaddingRight();
    int horizontalDividerLength = std::max(0, getWidth() - paddingRight - paddingLeft);
    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
        FlexLine& flexLine = mFlexLines.at(i);
        for (int j = 0; j < flexLine.mItemCount; j++) {
            int viewIndex = flexLine.mFirstIndex + j;
            View* view = getReorderedChildAt(viewIndex);
            if ((view == nullptr) || (view->getVisibility() == View::GONE)) {
                continue;
            }
            LayoutParams* lp = (LayoutParams*) view->getLayoutParams();

            // Judge if the beginning or middle divider is needed
            if (hasDividerBeforeChildAtAlongMainAxis(viewIndex, j)) {
                int dividerLeft;
                if (isRtl) {
                    dividerLeft = view->getRight() + lp->rightMargin;
                } else {
                    dividerLeft = view->getLeft() - lp->leftMargin - mDividerVerticalWidth;
                }

                drawVerticalDivider(canvas, dividerLeft, flexLine.mTop, flexLine.mCrossSize);
            }

            // Judge if the end divider is needed
            if (j == flexLine.mItemCount - 1) {
                if ((mShowDividerVertical & SHOW_DIVIDER_END) > 0) {
                    int dividerLeft;
                    if (isRtl) {
                        dividerLeft = view->getLeft() - lp->leftMargin - mDividerVerticalWidth;
                    } else {
                        dividerLeft = view->getRight() + lp->rightMargin;
                    }

                    drawVerticalDivider(canvas, dividerLeft, flexLine.mTop,flexLine.mCrossSize);
                }
            }
        }

        // Judge if the beginning or middle dividers are needed before the flex line
        if (hasDividerBeforeFlexLine(i)) {
            int horizontalDividerTop;
            if (fromBottomToTop) {
                horizontalDividerTop = flexLine.mBottom;
            } else {
                horizontalDividerTop = flexLine.mTop - mDividerHorizontalHeight;
            }
            drawHorizontalDivider(canvas, paddingLeft, horizontalDividerTop,horizontalDividerLength);
        }
        // Judge if the end divider is needed before the flex line
        if (hasEndDividerAfterFlexLine(i)) {
            if ((mShowDividerHorizontal & SHOW_DIVIDER_END) > 0) {
                int horizontalDividerTop;
                if (fromBottomToTop) {
                    horizontalDividerTop = flexLine.mTop - mDividerHorizontalHeight;
                } else {
                    horizontalDividerTop = flexLine.mBottom;
                }
                drawHorizontalDivider(canvas, paddingLeft, horizontalDividerTop,horizontalDividerLength);
            }
        }
    }
}

void FlexboxLayout::drawDividersVertical(Canvas& canvas, bool isRtl, bool fromBottomToTop) {
    int paddingTop = getPaddingTop();
    int paddingBottom = getPaddingBottom();
    int verticalDividerLength = std::max(0, getHeight() - paddingBottom - paddingTop);
    for (int i = 0, size = mFlexLines.size(); i < size; i++) {
        FlexLine& flexLine = mFlexLines.at(i);

        // Draw horizontal dividers if needed
        for (int j = 0; j < flexLine.mItemCount; j++) {
            int viewIndex = flexLine.mFirstIndex + j;
            View* view = getReorderedChildAt(viewIndex);
            if ((view == nullptr) || (view->getVisibility() == View::GONE)) {
                continue;
            }
            LayoutParams* lp = (LayoutParams*) view->getLayoutParams();

            // Judge if the beginning or middle divider is needed
            if (hasDividerBeforeChildAtAlongMainAxis(viewIndex, j)) {
                int dividerTop;
                if (fromBottomToTop) {
                    dividerTop = view->getBottom() + lp->bottomMargin;
                } else {
                    dividerTop = view->getTop() - lp->topMargin - mDividerHorizontalHeight;
                }
                drawHorizontalDivider(canvas, flexLine.mLeft, dividerTop, flexLine.mCrossSize);
            }

            // Judge if the end divider is needed
            if (j == flexLine.mItemCount - 1) {
                if ((mShowDividerHorizontal & SHOW_DIVIDER_END) > 0) {
                    int dividerTop;
                    if (fromBottomToTop) {
                        dividerTop = view->getTop() - lp->topMargin - mDividerHorizontalHeight;
                    } else {
                        dividerTop = view->getBottom() + lp->bottomMargin;
                    }
                    drawHorizontalDivider(canvas, flexLine.mLeft, dividerTop,flexLine.mCrossSize);
                }
            }
        }

        // Judge if the beginning or middle dividers are needed before the flex line
        if (hasDividerBeforeFlexLine(i)) {
            int verticalDividerLeft;
            if (isRtl) {
                verticalDividerLeft = flexLine.mRight;
            } else {
                verticalDividerLeft = flexLine.mLeft - mDividerVerticalWidth;
            }
            drawVerticalDivider(canvas, verticalDividerLeft, paddingTop,verticalDividerLength);
        }
        if (hasEndDividerAfterFlexLine(i)) {
            if ((mShowDividerVertical & SHOW_DIVIDER_END) > 0) {
                int verticalDividerLeft;
                if (isRtl) {
                    verticalDividerLeft = flexLine.mLeft - mDividerVerticalWidth;
                } else {
                    verticalDividerLeft = flexLine.mRight;
                }
                drawVerticalDivider(canvas, verticalDividerLeft, paddingTop,verticalDividerLength);
            }
        }
    }
}

void FlexboxLayout::drawVerticalDivider(Canvas& canvas, int left, int top, int length) {
    if (mDividerDrawableVertical == nullptr) {
        return;
    }
    mDividerDrawableVertical->setBounds(left, top, mDividerVerticalWidth, length);
    mDividerDrawableVertical->draw(canvas);
}

void FlexboxLayout::drawHorizontalDivider(Canvas& canvas, int left, int top, int length) {
    if (mDividerDrawableHorizontal == nullptr) {
        return;
    }
    mDividerDrawableHorizontal->setBounds(left, top, length, mDividerHorizontalHeight);
    mDividerDrawableHorizontal->draw(canvas);
}

bool FlexboxLayout::checkLayoutParams(const ViewGroup::LayoutParams* p) const{
    return dynamic_cast<const FlexboxLayout::LayoutParams*>(p);
}

ViewGroup::LayoutParams* FlexboxLayout::generateLayoutParams(const AttributeSet& attrs) const{
    return new FlexboxLayout::LayoutParams(getContext(), attrs);
}

ViewGroup::LayoutParams* FlexboxLayout::generateLayoutParams(const ViewGroup::LayoutParams* lp) const{
    if (dynamic_cast<const FlexboxLayout::LayoutParams*>(lp)) {
        return new FlexboxLayout::LayoutParams((const FlexboxLayout::LayoutParams&)* lp);
    } else if (dynamic_cast<const MarginLayoutParams*>(lp)) {
        return new FlexboxLayout::LayoutParams((const MarginLayoutParams&)*lp);
    }
    return new LayoutParams(*lp);
}

int FlexboxLayout::getFlexDirection() {
    return mFlexDirection;
}

void FlexboxLayout::setFlexDirection(int flexDirection) {
    if (mFlexDirection != flexDirection) {
        mFlexDirection = flexDirection;
        requestLayout();
    }
}

int FlexboxLayout::getFlexWrap() {
    return mFlexWrap;
}

void FlexboxLayout::setFlexWrap(int flexWrap) {
    if (mFlexWrap != flexWrap) {
        mFlexWrap = flexWrap;
        requestLayout();
    }
}

int FlexboxLayout::getJustifyContent() {
    return mJustifyContent;
}

void FlexboxLayout::setJustifyContent(int justifyContent) {
    if (mJustifyContent != justifyContent) {
        mJustifyContent = justifyContent;
        requestLayout();
    }
}

int FlexboxLayout::getAlignItems() {
    return mAlignItems;
}

void FlexboxLayout::setAlignItems(int alignItems) {
    if (mAlignItems != alignItems) {
        mAlignItems = alignItems;
        requestLayout();
    }
}

int FlexboxLayout::getAlignContent() {
    return mAlignContent;
}

void FlexboxLayout::setAlignContent(int alignContent) {
    if (mAlignContent != alignContent) {
        mAlignContent = alignContent;
        requestLayout();
    }
}

int FlexboxLayout::getMaxLine() {
    return mMaxLine;
}

void FlexboxLayout::setMaxLine(int maxLine) {
    if (mMaxLine != maxLine) {
        mMaxLine = maxLine;
        requestLayout();
    }
}

std::vector<FlexLine> FlexboxLayout::getFlexLines() {
    std::vector<FlexLine> result;
    for (FlexLine& flexLine : mFlexLines) {
        if (flexLine.getItemCountNotGone() == 0) {
            continue;
        }
        result.push_back(flexLine);
    }
    return result;
}

int FlexboxLayout::getDecorationLengthMainAxis(View* view, int index, int indexInFlexLine) {
    int decorationLength = 0;
    if (isMainAxisDirectionHorizontal()) {
        if (hasDividerBeforeChildAtAlongMainAxis(index, indexInFlexLine)) {
            decorationLength += mDividerVerticalWidth;
        }
        if ((mShowDividerVertical & SHOW_DIVIDER_END) > 0) {
            decorationLength += mDividerVerticalWidth;
        }
    } else {
        if (hasDividerBeforeChildAtAlongMainAxis(index, indexInFlexLine)) {
            decorationLength += mDividerHorizontalHeight;
        }
        if ((mShowDividerHorizontal & SHOW_DIVIDER_END) > 0) {
            decorationLength += mDividerHorizontalHeight;
        }
    }
    return decorationLength;
}

int FlexboxLayout::getDecorationLengthCrossAxis(View* view) {
    // Decoration along the cross axis for an individual view is not supported in the
    // FlexboxLayout.
    return 0;
}

void FlexboxLayout::onNewFlexLineAdded(FlexLine flexLine) {
    // The size of the end divider isn't added until the flexLine is added to the flex container
    // take the divider width (or height) into account when adding the flex line.
    if (isMainAxisDirectionHorizontal()) {
        if ((mShowDividerVertical & SHOW_DIVIDER_END) > 0) {
            flexLine.mMainSize += mDividerVerticalWidth;
            flexLine.mDividerLengthInMainSize += mDividerVerticalWidth;
        }
    } else {
        if ((mShowDividerHorizontal & SHOW_DIVIDER_END) > 0) {
            flexLine.mMainSize += mDividerHorizontalHeight;
            flexLine.mDividerLengthInMainSize += mDividerHorizontalHeight;
        }
    }
}

int FlexboxLayout::getChildWidthMeasureSpec(int widthSpec, int padding, int childDimension) {
    return getChildMeasureSpec(widthSpec, padding, childDimension);
}

int FlexboxLayout::getChildHeightMeasureSpec(int heightSpec, int padding, int childDimension) {
    return getChildMeasureSpec(heightSpec, padding, childDimension);
}

void FlexboxLayout::onNewFlexItemAdded(View* view, int index, int indexInFlexLine, FlexLine flexLine) {
    // Check if the beginning or middle divider is required for the flex item
    if (hasDividerBeforeChildAtAlongMainAxis(index, indexInFlexLine)) {
        if (isMainAxisDirectionHorizontal()) {
            flexLine.mMainSize += mDividerVerticalWidth;
            flexLine.mDividerLengthInMainSize += mDividerVerticalWidth;
        } else {
            flexLine.mMainSize += mDividerHorizontalHeight;
            flexLine.mDividerLengthInMainSize += mDividerHorizontalHeight;
        }
    }
}

void FlexboxLayout::setFlexLines(const std::vector<FlexLine>& flexLines) {
    mFlexLines = flexLines;
}

std::vector<FlexLine> FlexboxLayout::getFlexLinesInternal() {
    return mFlexLines;
}

void FlexboxLayout::updateViewCache(int position, View* view) {
    // No op
}

Drawable* FlexboxLayout::getDividerDrawableHorizontal() {
    return mDividerDrawableHorizontal;
}

Drawable* FlexboxLayout::getDividerDrawableVertical() {
    return mDividerDrawableVertical;
}

void FlexboxLayout::setDividerDrawable(Drawable* divider) {
    setDividerDrawableHorizontal(divider);
    setDividerDrawableVertical(divider);
}

void FlexboxLayout::setDividerDrawableHorizontal(Drawable* divider) {
    if (divider == mDividerDrawableHorizontal) {
        return;
    }
    if(mDividerDrawableHorizontal!=mDividerDrawableVertical){
        delete mDividerDrawableHorizontal;
    }
    mDividerDrawableHorizontal = divider;

    if (divider != nullptr) {
        mDividerHorizontalHeight = divider->getIntrinsicHeight();
    } else {
        mDividerHorizontalHeight = 0;
    }
    setWillNotDrawFlag();
    requestLayout();
}

void FlexboxLayout::setDividerDrawableVertical(Drawable* divider) {
    if (divider == mDividerDrawableVertical) {
        return;
    }
    if(mDividerDrawableHorizontal!=mDividerDrawableVertical){
        delete mDividerDrawableVertical;
    }
    mDividerDrawableVertical = divider;
    if (divider != nullptr) {
        mDividerVerticalWidth = divider->getIntrinsicWidth();
    } else {
        mDividerVerticalWidth = 0;
    }
    setWillNotDrawFlag();
    requestLayout();
}

int FlexboxLayout::getShowDividerVertical() {
    return mShowDividerVertical;
}

int FlexboxLayout::getShowDividerHorizontal() {
    return mShowDividerHorizontal;
}

void FlexboxLayout::setShowDivider(int dividerMode) {
    setShowDividerVertical(dividerMode);
    setShowDividerHorizontal(dividerMode);
}

void FlexboxLayout::setShowDividerVertical(int dividerMode) {
    if (dividerMode != mShowDividerVertical) {
        mShowDividerVertical = dividerMode;
        requestLayout();
    }
}

void FlexboxLayout::setShowDividerHorizontal(int dividerMode) {
    if (dividerMode != mShowDividerHorizontal) {
        mShowDividerHorizontal = dividerMode;
        requestLayout();
    }
}

void FlexboxLayout::setWillNotDrawFlag() {
    if (mDividerDrawableHorizontal == nullptr && mDividerDrawableVertical == nullptr) {
        setWillNotDraw(true);
    } else {
        setWillNotDraw(false);
    }
}

bool FlexboxLayout::hasDividerBeforeChildAtAlongMainAxis(int index, int indexInFlexLine) {
    if (allViewsAreGoneBefore(index, indexInFlexLine)) {
        if (isMainAxisDirectionHorizontal()) {
            return (mShowDividerVertical & SHOW_DIVIDER_BEGINNING) != 0;
        } else {
            return (mShowDividerHorizontal & SHOW_DIVIDER_BEGINNING) != 0;
        }
    } else {
        if (isMainAxisDirectionHorizontal()) {
            return (mShowDividerVertical & SHOW_DIVIDER_MIDDLE) != 0;
        } else {
            return (mShowDividerHorizontal & SHOW_DIVIDER_MIDDLE) != 0;
        }
    }
}

bool FlexboxLayout::allViewsAreGoneBefore(int index, int indexInFlexLine) {
    for (int i = 1; i <= indexInFlexLine; i++) {
        View* view = getReorderedChildAt(index - i);
        if (view != nullptr && view->getVisibility() != View::GONE) {
            return false;
        }
    }
    return true;
}

bool FlexboxLayout::hasDividerBeforeFlexLine(int flexLineIndex) {
    if (flexLineIndex < 0 || flexLineIndex >= mFlexLines.size()) {
        return false;
    }
    if (allFlexLinesAreDummyBefore(flexLineIndex)) {
        if (isMainAxisDirectionHorizontal()) {
            return (mShowDividerHorizontal & SHOW_DIVIDER_BEGINNING) != 0;
        } else {
            return (mShowDividerVertical & SHOW_DIVIDER_BEGINNING) != 0;
        }
    } else {
        if (isMainAxisDirectionHorizontal()) {
            return (mShowDividerHorizontal & SHOW_DIVIDER_MIDDLE) != 0;
        } else {
            return (mShowDividerVertical & SHOW_DIVIDER_MIDDLE) != 0;
        }
    }
}

bool FlexboxLayout::allFlexLinesAreDummyBefore(int flexLineIndex) {
    for (int i = 0; i < flexLineIndex; i++) {
        if (mFlexLines.at(i).getItemCountNotGone() > 0) {
            return false;
        }
    }
    return true;
}

bool FlexboxLayout::hasEndDividerAfterFlexLine(int flexLineIndex) {
    if (flexLineIndex < 0 || flexLineIndex >= mFlexLines.size()) {
        return false;
    }

    for (int i = flexLineIndex + 1; i < mFlexLines.size(); i++) {
        if (mFlexLines.at(i).getItemCountNotGone() > 0) {
            return false;
        }
    }
    if (isMainAxisDirectionHorizontal()) {
        return (mShowDividerHorizontal & SHOW_DIVIDER_END) != 0;
    } else {
        return (mShowDividerVertical & SHOW_DIVIDER_END) != 0;
    }

}

/*public static class LayoutParams extends ViewGroup.MarginLayoutParams implements FlexItem*/

FlexboxLayout::LayoutParams::LayoutParams(Context* context,const AttributeSet& attrs)
    :ViewGroup::MarginLayoutParams(context,attrs){

    mOrder = attrs.getInt("layout_order", (int)ORDER_DEFAULT);
    mFlexGrow = attrs.getFloat("layout_flexGrow", (int)FLEX_GROW_DEFAULT);
    mFlexShrink = attrs.getFloat("layout_flexShrink",(float)FLEX_SHRINK_DEFAULT);
    mAlignSelf = attrs.getInt("layout_alignSelf",std::unordered_map<std::string,int>{
            {"auto" , (int)AlignSelf::AUTO},
            {"flex_start",(int)AlignSelf::FLEX_START},
            {"flex_end", (int)AlignSelf::FLEX_END},
            {"center"  , (int)AlignSelf::CENTER},
            {"baseline", (int)AlignSelf::BASELINE},
            {"stretch" , (int)AlignSelf::STRETCH},
        }, (int)AlignSelf::AUTO);
    mFlexBasisPercent = attrs.getFraction("layout_flexBasisPercent", 1, 1,(float)FLEX_BASIS_PERCENT_DEFAULT);
    mMinWidth = attrs.getDimensionPixelSize("layout_minWidth"  ,(int)NOT_SET);
    mMinHeight = attrs.getDimensionPixelSize("layout_minHeight",(int)NOT_SET);
    mMaxWidth  = attrs.getDimensionPixelSize("layout_maxWidth" ,(int)MAX_SIZE);
    mMaxHeight = attrs.getDimensionPixelSize("layout_maxHeight",(int)MAX_SIZE);
    mWrapBefore= attrs.getBoolean("layout_wrapBefore", false);
}

FlexboxLayout::LayoutParams::LayoutParams(const LayoutParams& source):ViewGroup::MarginLayoutParams(source){

    mOrder = source.mOrder;
    mFlexGrow = source.mFlexGrow;
    mFlexShrink = source.mFlexShrink;
    mAlignSelf = source.mAlignSelf;
    mFlexBasisPercent = source.mFlexBasisPercent;
    mMinWidth = source.mMinWidth;
    mMinHeight = source.mMinHeight;
    mMaxWidth = source.mMaxWidth;
    mMaxHeight = source.mMaxHeight;
    mWrapBefore = source.mWrapBefore;
}

FlexboxLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source):ViewGroup::MarginLayoutParams(source){
}

FlexboxLayout::LayoutParams::LayoutParams(int width, int height):ViewGroup::MarginLayoutParams(ViewGroup::LayoutParams(width, height)){
}

FlexboxLayout::LayoutParams::LayoutParams(const MarginLayoutParams& source):ViewGroup::MarginLayoutParams(source){
}

int FlexboxLayout::LayoutParams::getWidth() {
    return width;
}

void FlexboxLayout::LayoutParams::setWidth(int width) {
    this->width = width;
}

int FlexboxLayout::LayoutParams::getHeight() {
    return height;
}

void FlexboxLayout::LayoutParams::setHeight(int height) {
    this->height = height;
}

int FlexboxLayout::LayoutParams::getOrder() {
    return mOrder;
}

void FlexboxLayout::LayoutParams::setOrder(int order) {
    mOrder = order;
}

float FlexboxLayout::LayoutParams::getFlexGrow() {
    return mFlexGrow;
}

void FlexboxLayout::LayoutParams::setFlexGrow(float flexGrow) {
    this->mFlexGrow = flexGrow;
}

float FlexboxLayout::LayoutParams::getFlexShrink() {
    return mFlexShrink;
}

void FlexboxLayout::LayoutParams::setFlexShrink(float flexShrink) {
    mFlexShrink = flexShrink;
}

int FlexboxLayout::LayoutParams::getAlignSelf() {
    return mAlignSelf;
}

void FlexboxLayout::LayoutParams::setAlignSelf(int alignSelf) {
    mAlignSelf = alignSelf;
}

int FlexboxLayout::LayoutParams::getMinWidth() {
    return mMinWidth;
}

void FlexboxLayout::LayoutParams::setMinWidth(int minWidth) {
    this->mMinWidth = minWidth;
}

int FlexboxLayout::LayoutParams::getMinHeight() {
    return mMinHeight;
}

void FlexboxLayout::LayoutParams::setMinHeight(int minHeight) {
    mMinHeight = minHeight;
}

int FlexboxLayout::LayoutParams::getMaxWidth() {
    return mMaxWidth;
}

void FlexboxLayout::LayoutParams::setMaxWidth(int maxWidth) {
    mMaxWidth = maxWidth;
}

int FlexboxLayout::LayoutParams::getMaxHeight() {
    return mMaxHeight;
}

void FlexboxLayout::LayoutParams::setMaxHeight(int maxHeight) {
    mMaxHeight = maxHeight;
}

bool FlexboxLayout::LayoutParams::isWrapBefore() {
    return mWrapBefore;
}

void FlexboxLayout::LayoutParams::setWrapBefore(bool wrapBefore) {
    mWrapBefore = wrapBefore;
}

float FlexboxLayout::LayoutParams::getFlexBasisPercent() {
    return mFlexBasisPercent;
}

void FlexboxLayout::LayoutParams::setFlexBasisPercent(float flexBasisPercent) {
    mFlexBasisPercent = flexBasisPercent;
}

int FlexboxLayout::LayoutParams::getMarginLeft() {
    return leftMargin;
}

int FlexboxLayout::LayoutParams::getMarginTop() {
    return topMargin;
}

int FlexboxLayout::LayoutParams::getMarginRight() {
    return rightMargin;
}

int FlexboxLayout::LayoutParams::getMarginBottom() {
    return bottomMargin;
}

}
