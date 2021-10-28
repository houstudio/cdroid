/*************************************************************************
	> File Name: ../../src/gui/widget/framelayout.h
	> Author: 
	> Mail: 
	> Created Time: Wed 06 Jan 2021 05:07:47 AM UTC
 ************************************************************************/

#ifndef __FRAMELAYOUT_H__
#define __FRAMELAYOUT_H__
#include <widget/viewgroup.h>
namespace cdroid{

class FrameLayoutParams:public MarginLayoutParams{
public:
    enum{
        UNSPECIFIED_GRAVITY=-1
    };
    int gravity;
    FrameLayoutParams(Context* c,const AttributeSet& attrs);
    FrameLayoutParams(int width, int height);
    FrameLayoutParams(int width, int height, int gravity);
    FrameLayoutParams(const LayoutParams& source);
    FrameLayoutParams(const MarginLayoutParams& source) ;
    FrameLayoutParams(const FrameLayoutParams& source);
};

class FrameLayout:public ViewGroup{
private:
    bool mMeasureAllChildren;
    int mForegroundPaddingLeft;
    int mForegroundPaddingTop;
    int mForegroundPaddingRight;
    int mForegroundPaddingBottom;
    std::vector<View*>mMatchParentChildren;

protected:
    int getPaddingLeftWithForeground();
    int getPaddingRightWithForeground();
    int getPaddingTopWithForeground();
    int getPaddingBottomWithForeground();

    LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams( const LayoutParams* p)const override;
    LayoutParams* generateLayoutParams( const LayoutParams* lp)const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int left, int top, int width, int height)override;
    void layoutChildren(int left, int top, int width, int height, bool forceLeftGravity);

public:
    FrameLayout(int w,int h);
    FrameLayout(Context* context,const AttributeSet& attrs);
    void setForegroundGravity(int foregroundGravity);
    void setMeasureAllChildren(bool measureAll);
    bool getMeasureAllChildren()const;
    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
};
}//namespace
#endif
