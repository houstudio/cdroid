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

class FrameLayout:public ViewGroup{
public:
    class LayoutParams:public MarginLayoutParams{
    public:
        enum{
            UNSPECIFIED_GRAVITY=-1
        };
        int gravity;
        LayoutParams(Context* c,const AttributeSet& attrs);
        LayoutParams(int width, int height);
        LayoutParams(int width, int height, int gravity);
        LayoutParams(const MarginLayoutParams& source) ;
        LayoutParams(const LayoutParams& source);
    };


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

    ViewGroup::LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams( const ViewGroup::LayoutParams* p)const override;
    ViewGroup::LayoutParams* generateLayoutParams( const ViewGroup::LayoutParams* lp)const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int left, int top, int width, int height)override;
    void layoutChildren(int left, int top, int width, int height, bool forceLeftGravity);

public:
    FrameLayout(int w,int h);
    FrameLayout(Context* context,const AttributeSet& attrs);
    void setForegroundGravity(int foregroundGravity);
    void setMeasureAllChildren(bool measureAll);
    bool getMeasureAllChildren()const;
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
};
}//namespace
#endif
