#ifndef __FLEXBOX_LAYOUT_H__
#define __FLEXBOX_LAYOUT_H__
#include <view/viewgroup.h>
#include <widgetEx/flexbox/flexline.h>
#include <widgetEx/flexbox/flexitem.h>
#include <widgetEx/flexbox/flexwrap.h>
#include <widgetEx/flexbox/aligndefs.h>
#include <widgetEx/flexbox/justifycontent.h>
#include <widgetEx/flexbox/flexdirection.h>
#include <widgetEx/flexbox/flexboxhelper.h>
#include <widgetEx/flexbox/flexcontainer.h>
namespace cdroid{
class FlexboxLayout:public ViewGroup{
public:
    static constexpr int NOT_SET = -1;
    static constexpr int SHOW_DIVIDER_NONE = 0;

    /** Constant to show a divider at the beginning of the flex lines (or flex items). */
    static constexpr int SHOW_DIVIDER_BEGINNING = 1;

    /** Constant to show dividers between flex lines or flex items. */
    static constexpr int SHOW_DIVIDER_MIDDLE = 1 << 1;

    /** Constant to show a divider at the end of the flex lines or flex items. */
    static constexpr int SHOW_DIVIDER_END = 1 << 2;
private:
    int mFlexDirection;
    int mFlexWrap;
    int mJustifyContent;
    int mAlignItems;
    int mAlignContent;
    int mMaxLine = NOT_SET;
    /** Constant to show no dividers */

    /** The drawable to be drawn for the horizontal dividers. */
    Drawable* mDividerDrawableHorizontal;

    /** The drawable to be drawn for the vertical dividers. */
    Drawable* mDividerDrawableVertical;

    int mShowDividerHorizontal;
    int mShowDividerVertical;

    int mDividerHorizontalHeight;

    int mDividerVerticalWidth;
    std::vector<int> mReorderedIndices;
    SparseIntArray mOrderCache;

    FlexboxHelper* mFlexboxHelper;
    FlexContainer* mFlexContainer;
    std::vector<FlexLine> mFlexLines;
    FlexboxHelper::FlexLinesResult* mFlexLinesResult;
private:
    void init();
    void measureHorizontal(int widthMeasureSpec, int heightMeasureSpec);
    void measureVertical(int widthMeasureSpec, int heightMeasureSpec);
    void setMeasuredDimensionForFlex(int flexDirection, int widthMeasureSpec,int heightMeasureSpec, int childState);
    void layoutHorizontal(bool isRtl, int left, int top, int width, int height);
    void layoutVertical(bool isRtl, bool fromBottomToTop, int left, int top,int width, int height);
    void drawDividersHorizontal(Canvas& canvas, bool isRtl, bool fromBottomToTop);
    void drawDividersVertical(Canvas& canvas, bool isRtl, bool fromBottomToTop);
    void drawVerticalDivider(Canvas& canvas, int left, int top, int length);
    void drawHorizontalDivider(Canvas& canvas, int left, int top, int length);
    void setWillNotDrawFlag();
    bool hasDividerBeforeChildAtAlongMainAxis(int index, int indexInFlexLine);
    bool allViewsAreGoneBefore(int index, int indexInFlexLine);
    bool hasDividerBeforeFlexLine(int flexLineIndex);
    bool allFlexLinesAreDummyBefore(int flexLineIndex);
    bool hasEndDividerAfterFlexLine(int flexLineIndex);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int left, int top, int right, int bottom)override;
    void onDraw(Canvas& canvas) override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    ViewGroup::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* lp)const override;
public:
    class LayoutParams;
    FlexboxLayout(Context* context,const AttributeSet& attrs);
    ~FlexboxLayout()override;
    void addView(View* child, int index,ViewGroup::LayoutParams* params)override;
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    
    ////////////////////////////////////////////////////////////////////////////////////////////
    //FlexContainer's impl.
    int getFlexItemCount();// override;
    View* getFlexItemAt(int index);// override;
    View* getReorderedChildAt(int index);
    View* getReorderedFlexItemAt(int index);//override;
    int getLargestMainSize();//override;
    int getSumOfCrossSize();//override;
    bool isMainAxisDirectionHorizontal();//override;

    int getFlexDirection();//override;
    void setFlexDirection(int flexDirection);//override;

    int getFlexWrap();//override;
    void setFlexWrap(int flexWrap);//override;

    int getJustifyContent();//override;
    void setJustifyContent(int justifyContent);//override;

    int getAlignItems();//override;
    void setAlignItems(int alignItems);//override;
    int getAlignContent();//override;

    void setAlignContent(int alignContent);//override;
    int getMaxLine();//override;

    void setMaxLine(int maxLine);//override;
    std::vector<FlexLine> getFlexLines();//override;
    int getDecorationLengthMainAxis(View* view, int index, int indexInFlexLine);//override;

    int getDecorationLengthCrossAxis(View* view);//override;
    void onNewFlexLineAdded(FlexLine flexLine);//override;

    int getChildWidthMeasureSpec(int widthSpec, int padding, int childDimension);//override;
    int getChildHeightMeasureSpec(int heightSpec, int padding, int childDimension);//override;
    void onNewFlexItemAdded(View* view, int index, int indexInFlexLine, FlexLine flexLine);//override;

    void setFlexLines(const std::vector<FlexLine>& flexLines);//override;
    std::vector<FlexLine> getFlexLinesInternal();//override;
    void updateViewCache(int position, View* view);//override;
    ////////////////////////////////////////////////////////////////////////////////////////////

    Drawable* getDividerDrawableHorizontal();
    Drawable* getDividerDrawableVertical();
    void setDividerDrawable(Drawable* divider);
    void setDividerDrawableHorizontal(Drawable* divider);
    void setDividerDrawableVertical(Drawable* divider);
    int getShowDividerVertical();
    int getShowDividerHorizontal();
    void setShowDivider(int dividerMode);
    void setShowDividerVertical(int dividerMode);
    void setShowDividerHorizontal(int dividerMode);
};

class FlexboxLayout::LayoutParams:public ViewGroup::MarginLayoutParams{//,public FlexItem {
private:
   static constexpr int ORDER_DEFAULT = 1;
   static constexpr float FLEX_GROW_DEFAULT = 0.f;
   static constexpr float FLEX_SHRINK_DEFAULT = 1.f;
   static constexpr float FLEX_SHRINK_NOT_SET = 0.f;
   static constexpr float FLEX_BASIS_PERCENT_DEFAULT = -1.f;
   static constexpr int MAX_SIZE = INT_MAX & View::MEASURED_SIZE_MASK;
   int mOrder = ORDER_DEFAULT;
   float mFlexGrow = FLEX_GROW_DEFAULT;
   float mFlexShrink = FLEX_SHRINK_DEFAULT;
   int mAlignSelf = (int)AlignSelf::AUTO;
   float mFlexBasisPercent = FLEX_BASIS_PERCENT_DEFAULT;
   int mMinWidth = NOT_SET;
   int mMinHeight = NOT_SET;
   int mMaxWidth = MAX_SIZE;
   int mMaxHeight = MAX_SIZE;
   bool mWrapBefore;
public:
   LayoutParams(Context* context,const AttributeSet& attrs);
   LayoutParams(const LayoutParams& source);
   LayoutParams(const ViewGroup::LayoutParams& source);
   LayoutParams(int width, int height);
   LayoutParams(const MarginLayoutParams& source);

   int getWidth();//override;
   void setWidth(int width);//override;
   int getHeight();//override;
   void setHeight(int height);//override;
   int getOrder();//override;
   void setOrder(int order);//override;
   float getFlexGrow();//override;
   void setFlexGrow(float flexGrow);//override;
   float getFlexShrink();//override;
   void setFlexShrink(float flexShrink);//override;
   int getAlignSelf();//override;
   void setAlignSelf(int alignSelf);//override;
   int getMinWidth();//override;
   void setMinWidth(int minWidth);//override;
   int getMinHeight();//override;
   void setMinHeight(int minHeight);//override;
   int getMaxWidth();//override;
   void setMaxWidth(int maxWidth);//override;
   int getMaxHeight();//override;
   void setMaxHeight(int maxHeight);//override;
   bool isWrapBefore();//override;
   void setWrapBefore(bool wrapBefore);//override;
   float getFlexBasisPercent();//override;
   void setFlexBasisPercent(float flexBasisPercent);//override;
   int getMarginLeft();//override;
   int getMarginTop();//override;
   int getMarginRight();//override;
   int getMarginBottom();//override;
};
}/*endof namespace*/
#endif/*__FLEXBOX_LAYOUT_H__*/
