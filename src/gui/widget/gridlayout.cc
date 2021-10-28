#include <widget/gridlayout.h>
#include <widget/measurespec.h>
#include <cdlog.h>


namespace cdroid{

GridLayoutParams::GridLayoutParams()
    :MarginLayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT){
}
GridLayoutParams::GridLayoutParams(const LayoutParams& params)
    :MarginLayoutParams(params){
}
GridLayoutParams::GridLayoutParams(const MarginLayoutParams& params)
    :MarginLayoutParams(params){
}
GridLayoutParams::GridLayoutParams(const GridLayoutParams& source)
    :MarginLayoutParams(source){
}
GridLayoutParams::GridLayoutParams(Context* context,const AttributeSet& attrs)
    :MarginLayoutParams(context,attrs){

}

void GridLayoutParams::setGravity(int gravity){

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GridLayout::GridLayout(int w,int h)
    :ViewGroup(w,h){
    columnCount=1;
    verticalSpace=0;
    horizontalSpace=0;
    childWidth=0;
}

GridLayout::GridLayout(Context*ctx,const AttributeSet&attrs)
    :ViewGroup(ctx,attrs){
}

void GridLayout::refreshNotGoneChildList() {
    notGoneViewList.clear();
    int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() != GONE) {
            notGoneViewList.push_back(child);
        }
    }
}

int GridLayout::getVerticalSpace()const{
    return verticalSpace;
}

void GridLayout::setVerticalSpace(int verticalSpace) {
    this->verticalSpace = verticalSpace;
    requestLayout();
}

int GridLayout::getHorizontalSpace()const {
    return horizontalSpace;
}

void GridLayout::setHorizontalSpace(int horizontalSpace) {
    this->horizontalSpace = horizontalSpace;
    requestLayout();
}

int GridLayout::getColumnCount()const {
    return columnCount;
}

void GridLayout::setColumnCount(int columnCount) {
    this->columnCount = columnCount;
    requestLayout();
}

bool GridLayout::checkLayoutParams(const LayoutParams* p)const {
    return dynamic_cast<const GridLayoutParams*>(p);
}

LayoutParams* GridLayout::generateDefaultLayoutParams()const {
    return new LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT);
}

LayoutParams* GridLayout::generateLayoutParams(const AttributeSet& attrs)const {
    return new GridLayoutParams(getContext(), attrs);
}

LayoutParams* GridLayout::generateLayoutParams(const LayoutParams* p)const {
    return new GridLayoutParams(*p);
}

void GridLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    refreshNotGoneChildList();
    if (childWidth <= 0) {
        int parentWidth = MeasureSpec::getSize(widthMeasureSpec);
        childWidth = (int) ((parentWidth - (columnCount - 1) * horizontalSpace * 1.0f) / columnCount + 0.5f);
    }
    int childCount = notGoneViewList.size();
    int line = childCount % columnCount == 0 ? childCount / columnCount: childCount / columnCount + 1;
    int totalHeight = 0;
    int childIndex = 0;
    for (int i = 0; i < line; i++) {
        int inlineHeight = 0;
        for (int j = 0; j < columnCount; j++) {
            childIndex = i * columnCount + j;
            if (childIndex < childCount) {
                View* child = notGoneViewList.at(childIndex);
                int childWidthWithPadding = childWidth;
                if (j == 0) {
                    // measureChild会在size的基础上减掉paddingLeft和paddingRight，对于每一行第一个元素加上paddingRight抵消
                    childWidthWithPadding += getPaddingRight();
                } else if (j == columnCount -1){
                    // measureChild会在size的基础上减掉paddingLeft和paddingRight，对于每一行最后一个元素加上paddingLeft抵消
                    childWidthWithPadding += getPaddingLeft();
                }
                measureChild(child,
                             MeasureSpec::makeMeasureSpec(childWidthWithPadding, MeasureSpec::EXACTLY),
                             MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED));
                int totalInlineChildHeight = child->getMeasuredHeight();
                if (totalInlineChildHeight > inlineHeight) {
                    inlineHeight = totalInlineChildHeight;
                }
            }
        }
        totalHeight += inlineHeight;
        totalHeight += verticalSpace;
    }
    totalHeight -= verticalSpace;
    totalHeight += getPaddingTop() + getPaddingBottom();
    setMeasuredDimension(getDefaultSize(getSuggestedMinimumWidth(), widthMeasureSpec),totalHeight);
}

void GridLayout::onLayout(bool changed, int left, int top, int w, int h){
    int childCount = notGoneViewList.size();
    int line = childCount % columnCount == 0 ? childCount / columnCount: childCount / columnCount + 1;
    int childIndex = 0;
    int lastLeft = getPaddingLeft();
    int lastTop = getPaddingTop();
    for (int i = 0; i < line; i++) {
        int inlineHeight = 0;
        for (int j = 0; j < columnCount; j++) {
            childIndex = i * columnCount + j;
            if (childIndex < childCount) {
                View* child = notGoneViewList.at(childIndex);
                int childWidth = child->getMeasuredWidth();
                int childHeight = child->getMeasuredHeight();

                child->layout(lastLeft, lastTop, childWidth, childHeight);
                lastLeft += (childWidth + horizontalSpace);
                int totalInlineChildHeight = child->getMeasuredHeight();
                if (totalInlineChildHeight > inlineHeight) {
                    inlineHeight = totalInlineChildHeight;
                }
            }
        }
        lastLeft = getPaddingLeft();
        lastTop += (inlineHeight + verticalSpace);
    }
}

}
