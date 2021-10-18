#ifndef __GRID_LAYOUT_H__
#define __GRID_LAYOUT_H__
#include <widget/viewgroup.h>
#include <widget/layoutparams.h>
namespace cdroid{

class GridLayoutParams:public MarginLayoutParams{
private:
public:
    GridLayoutParams();
    GridLayoutParams(const LayoutParams& params);
    GridLayoutParams(const MarginLayoutParams& params);
    GridLayoutParams(const GridLayoutParams& source);
    GridLayoutParams(Context* context,const AttributeSet& attrs);
    void setGravity(int gravity);
};

class GridLayout:public ViewGroup{
private:
    int verticalSpace;
    int horizontalSpace;
    int columnCount;
    int childWidth;
    std::vector<View*> notGoneViewList;
    void refreshNotGoneChildList();
protected:
    bool checkLayoutParams(const LayoutParams* p)const override;
    LayoutParams* generateDefaultLayoutParams()const override;
    LayoutParams* generateLayoutParams(const LayoutParams* p)const override;
    void onMeasure(int widthSpec, int heightSpec)override;
    void onLayout(bool changed, int left, int top, int w, int h)override;
public:
    GridLayout(int w,int h);
    GridLayout(Context*ctx,const AttributeSet&attrs,const std::string&defstyle=nullptr);
    int getVerticalSpace()const;
    void setVerticalSpace(int verticalSpace);
    int getHorizontalSpace()const;
    void setHorizontalSpace(int horizontalSpace);
    int getColumnCount()const;
    void setColumnCount(int columnCount);
    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
};

}
#endif
