#ifndef __LAYOUT_PARAMS_H__
#define __LAYOUT_PARAMS_H__
#include <string>
#include <attributeset.h>
#include <context.h>
#include <limits.h>

namespace cdroid{
class View;
class Canvas;
class LayoutParams{
public:
    enum{
        MATCH_PARENT=-1,
        WRAP_CONTENT=-2,
    };
protected:
    static const std::string sizeToString(int size);
    void setBaseAttributes(const AttributeSet& a, int widthAttr, int heightAttr);
public:
    int width;
    int height;
    LayoutParams();
    LayoutParams(Context* c,const AttributeSet& attrs);
    LayoutParams(int width, int height);
    LayoutParams(const LayoutParams& source);
    virtual ~LayoutParams(){}
    virtual void resolveLayoutDirection(int layoutDirection);
    virtual void onDebugDraw(View&view, Canvas&canvas);
};

class MarginLayoutParams:public LayoutParams{
private:
    enum{
        DEFAULT_MARGIN_RELATIVE=INT_MIN,
        LAYOUT_DIRECTION_MASK = 0x00000003,
        LEFT_MARGIN_UNDEFINED_MASK = 0x00000004,
        RIGHT_MARGIN_UNDEFINED_MASK = 0x00000008,
        RTL_COMPATIBILITY_MODE_MASK = 0x00000010,
        NEED_RESOLUTION_MASK = 0x00000020,
        DEFAULT_MARGIN_RESOLVED = 0,
        UNDEFINED_MARGIN = DEFAULT_MARGIN_RELATIVE
    };
    void doResolveMargins();
protected:
    int mMarginFlags;
public:
    int leftMargin;
    int topMargin;
    int rightMargin;
    int bottomMargin;
    int startMargin;
    int endMargin;
    MarginLayoutParams(Context*c,const AttributeSet& attrs);
    MarginLayoutParams(int width, int height);
    MarginLayoutParams(const LayoutParams& source);
    MarginLayoutParams(const MarginLayoutParams& source);
    void copyMarginsFrom(const MarginLayoutParams& source);
    void setMargins(int left, int top, int right, int bottom);
    void setMarginsRelative(int start, int top, int end, int bottom);
    void setMarginStart(int start);
    int getMarginStart();
    void setMarginEnd(int end);
    int getMarginEnd();
    bool isMarginRelative()const;
    void setLayoutDirection(int layoutDirection);//override;
    int getLayoutDirection()const;
    void resolveLayoutDirection(int layoutDirection)override;
    bool isLayoutRtl()const;
    void onDebugDraw(View&view, Canvas&canvas)override;
};

}//namespace
#endif
