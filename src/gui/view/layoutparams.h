#ifndef __LAYOUT_PARAMS_H__
#define __LAYOUT_PARAMS_H__
#include <string>
#include <limits.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <animation/layoutanimationcontroller.h>

namespace cdroid{
class View;
class Canvas;
class LayoutParams{
public:
    static constexpr int MATCH_PARENT = -1;
    static constexpr int WRAP_CONTENT = -2;
protected:
    static const std::string sizeToString(int size);
    void setBaseAttributes(const AttributeSet& a, int widthAttr, int heightAttr);
public:
    int width;
    int height;
    LayoutAnimationController::AnimationParameters* layoutAnimationParameters;
    LayoutParams();
    LayoutParams(Context* c,const AttributeSet& attrs);
    LayoutParams(int width, int height);
    LayoutParams(const LayoutParams& source);
    virtual ~LayoutParams();
    virtual void resolveLayoutDirection(int layoutDirection);
    virtual void onDebugDraw(View&view, Canvas&canvas);
};

class MarginLayoutParams:public LayoutParams{
private:
    static constexpr int DEFAULT_MARGIN_RELATIVE=INT_MIN;
    static constexpr int LAYOUT_DIRECTION_MASK = 0x00000003;
    static constexpr int LEFT_MARGIN_UNDEFINED_MASK = 0x00000004;
    static constexpr int RIGHT_MARGIN_UNDEFINED_MASK = 0x00000008;
    static constexpr int RTL_COMPATIBILITY_MODE_MASK = 0x00000010;
    static constexpr int NEED_RESOLUTION_MASK = 0x00000020;
    static constexpr int DEFAULT_MARGIN_RESOLVED = 0;
    static constexpr int UNDEFINED_MARGIN = DEFAULT_MARGIN_RELATIVE;
private:
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
    void setLayoutDirection(int layoutDirection);
    int getLayoutDirection()const;
    void resolveLayoutDirection(int layoutDirection)override;
    bool isLayoutRtl()const;
    void onDebugDraw(View&view, Canvas&canvas)override;
};

}//namespace
#endif
