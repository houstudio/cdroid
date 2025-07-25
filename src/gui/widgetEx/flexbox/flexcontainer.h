#ifndef __FLEX_CONTAINER_H__
#define __FLEX_CONTAINER_H__
#include <view/view.h>
#include <widgetEx/flexbox/flexline.h>
namespace cdroid{
class FlexContainer {
public:
    static constexpr int NOT_SET = -1;
    virtual ~FlexContainer()=default;
    virtual int getFlexItemCount()=0;
    virtual View* getFlexItemAt(int index)=0;
    virtual View* getReorderedFlexItemAt(int index)=0;

    virtual void addView(View* view)=0;
    virtual void addView(View* view, int index)=0;
    virtual void removeAllViews()=0;
    virtual void removeViewAt(int index)=0;

    virtual int getFlexDirection()=0;
    virtual void setFlexDirection( int flexDirection)=0;

    virtual int getFlexWrap()=0;
    virtual void setFlexWrap(int flexWrap)=0;

    virtual int getJustifyContent()=0;
    virtual void setJustifyContent(int justifyContent)=0;

    virtual int getAlignContent()=0;
    virtual void setAlignContent(int alignContent)=0;
    virtual int getAlignItems()=0;

    virtual void setAlignItems(int alignItems)=0;

    virtual std::vector<FlexLine> getFlexLines()=0;

    virtual bool isMainAxisDirectionHorizontal()=0;

    virtual int getDecorationLengthMainAxis(View* view, int index, int indexInFlexLine)=0;
    virtual int getDecorationLengthCrossAxis(View* view)=0;

    virtual int getPaddingTop()=0;
    virtual int getPaddingLeft()=0;
    virtual int getPaddingRight()=0;
    virtual int getPaddingBottom()=0;
    virtual int getPaddingStart()=0;
    virtual int getPaddingEnd()=0;

    virtual int getChildWidthMeasureSpec(int widthSpec, int padding, int childDimension)=0;
    virtual int getChildHeightMeasureSpec(int heightSpec, int padding, int childDimension)=0;

    virtual int getLargestMainSize()=0;

    virtual int getSumOfCrossSize()=0;

    virtual void onNewFlexItemAdded(View* view, int index, int indexInFlexLine, FlexLine& flexLine)=0;
    virtual void onNewFlexLineAdded(FlexLine& flexLine)=0;
    virtual void setFlexLines(const std::vector<FlexLine>& flexLines)=0;

    virtual int getMaxLine()=0;
    virtual void setMaxLine(int maxLine)=0;

    virtual std::vector<FlexLine> getFlexLinesInternal()=0;
    virtual void updateViewCache(int position, View* view)=0;
};
}/*endof namespace*/
#endif/*__FLEX_CONTAINER_H__*/
