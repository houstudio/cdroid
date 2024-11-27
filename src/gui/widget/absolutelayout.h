#ifndef __ABSOLUTELAYOUT_H__
#define __ABSOLUTELAYOUT_H__
#include <view/viewgroup.h>
namespace cdroid{


class AbsoluteLayout:public ViewGroup{
public:
    class LayoutParams:public ViewGroup::LayoutParams{
    public:
        int x;
        int y;
        LayoutParams(int width, int height, int x, int y);
        LayoutParams(Context* c,const AttributeSet& attrs);
        LayoutParams(const ViewGroup::LayoutParams& source);
        LayoutParams(const LayoutParams& source);
    };
protected:
    ViewGroup::LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    ViewGroup::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t,int w, int h)override;
public:
    AbsoluteLayout(int w,int h);
    AbsoluteLayout(Context* context,const AttributeSet& attrs);
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    std::string getAccessibilityClassName()const override;
};

}

#endif
