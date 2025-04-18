#ifndef __FLEX_ITEM_H__
#define __FLEX_ITEM_H__
#include <climits>
#include <view/view.h>
namespace cdroid{
class FlexItem{
public:
    /** The default value for the order attribute */
    static constexpr int ORDER_DEFAULT = 1;

    /** The default value for the flex grow attribute */
    static constexpr float FLEX_GROW_DEFAULT = 0.f;

    /** The default value for the flex shrink attribute */
    static constexpr float FLEX_SHRINK_DEFAULT = 1.f;

    /** The value representing the flex shrink attribute is not set  */
    static constexpr float FLEX_SHRINK_NOT_SET = 0.f;

    /** The default value for the flex basis percent attribute */
    static constexpr float FLEX_BASIS_PERCENT_DEFAULT = -1.f;

    /** The maximum size of the max width and max height attributes */
    static constexpr int MAX_SIZE = INT_MAX & View::MEASURED_SIZE_MASK;
public:
    virtual int getWidth()=0;

    virtual void setWidth(int width)=0;

    virtual int getHeight()=0;

    virtual void setHeight(int height)=0;

    virtual int getOrder()=0;

    virtual void setOrder(int order)=0;

    virtual float getFlexGrow()=0;

    virtual void setFlexGrow(float flexGrow)=0;

    virtual float getFlexShrink()=0;

    virtual void setFlexShrink(float flexShrink)=0;

    virtual int getAlignSelf()=0;

    virtual void setAlignSelf(int alignSelf)=0;

    virtual int getMinWidth()=0;

    virtual void setMinWidth(int minWidth)=0;

    virtual int getMinHeight()=0;

    virtual void setMinHeight(int minHeight)=0;

    virtual int getMaxWidth()=0;

    virtual void setMaxWidth(int maxWidth)=0;

    virtual int getMaxHeight()=0;

    virtual void setMaxHeight(int maxHeight)=0;

    virtual bool isWrapBefore()=0;

    virtual void setWrapBefore(bool wrapBefore)=0;

    virtual float getFlexBasisPercent()=0;

    virtual void setFlexBasisPercent(float flexBasisPercent)=0;

    virtual int getMarginLeft()=0;

    virtual int getMarginTop()=0;

    virtual int getMarginRight()=0;

    virtual int getMarginBottom()=0;

    virtual int getMarginStart()=0;

    virtual int getMarginEnd()=0;
};
}/*endof namespace*/
#endif/*__FLEX_ITEM_H__*/ 
