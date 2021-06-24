/*************************************************************************
	> File Name: absolutelayout.h
	> Author: 
	> Mail: 
	> Created Time: Wed 06 Jan 2021 12:54:07 PM UTC
 ************************************************************************/

#ifndef __ABSOLUTELAYOUT_H__
#define __ABSOLUTELAYOUT_H__
#include <widget/viewgroup.h>
namespace cdroid{

class AbsoluteLayoutParams:public LayoutParams{
public:
    int x;
    int y;
    AbsoluteLayoutParams(int width, int height, int x, int y);
    AbsoluteLayoutParams(Context* c,const AttributeSet& attrs);
    AbsoluteLayoutParams(const LayoutParams& source);
};

class AbsoluteLayout:public ViewGroup{
protected:
    LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const LayoutParams* p)const override;
    LayoutParams* generateLayoutParams(const LayoutParams* p)const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t,int w, int h)override;
public:
    AbsoluteLayout(int w,int h);
    AbsoluteLayout(Context* context,const AttributeSet& attrs);
    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
};

}

#endif
