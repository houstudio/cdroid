#ifndef __TAB_LAYOUT_H__
#define __TAB_LAYOUT_H__
#include <widget/tablerow.h>
#include <unordered_map>

namespace cdroid{

class TableLayout:public LinearLayout{
public:
    class LayoutParams:public LinearLayout::LayoutParams{
    public:
        LayoutParams();
        LayoutParams(Context* c, const AttributeSet&attrs);
        LayoutParams(int w, int h);
        LayoutParams(int w, int h, float initWeight);
        LayoutParams(const ViewGroup::LayoutParams& p);
        LayoutParams(const ViewGroup::MarginLayoutParams& source);
    };
private:
    bool mInitialized;
    bool mShrinkAllColumns;
    bool mStretchAllColumns;
    std::vector<int>mMaxWidths;
    SparseBooleanArray mCollapsedColumns;
    SparseBooleanArray mStretchableColumns;
    SparseBooleanArray mShrinkableColumns;
    static SparseBooleanArray parseColumns(const std::string& sequence);
    void initTableLayout();
    void requestRowsLayout();
    void trackCollapsedColumns(View* child);
    void findLargestCells(int widthMeasureSpec, int heightMeasureSpec);
    void shrinkAndStretchColumns(int widthMeasureSpec);
    void mutateColumnsWidth(SparseBooleanArray & columns,bool allColumns, int size, int totalWidth);
protected:
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void measureChildBeforeLayout(View* child, int childIndex,int widthMeasureSpec, 
            int totalWidth,int heightMeasureSpec, int totalHeight)override;
    void measureVertical(int widthMeasureSpec, int heightMeasureSpec)override;

    ViewGroup::LayoutParams* generateDefaultLayoutParams() const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p) const override;
    ViewGroup::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
public:
    TableLayout(int w,int h);
    TableLayout(Context*,const AttributeSet&atts);
    void requestLayout()override;

    bool isShrinkAllColumns()const;
    void setShrinkAllColumns(bool shrinkAllColumns);

    bool isStretchAllColumns()const;
    void setStretchAllColumns(bool stretchAllColumns);

    bool isColumnCollapsed(int columnIndex)const;
    void setColumnCollapsed(int columnIndex, bool isCollapsed);

    bool isColumnStretchable(int columnIndex)const;
    void setColumnStretchable(int columnIndex, bool isStretchable);

    bool isColumnShrinkable(int columnIndex)const;
    void setColumnShrinkable(int columnIndex, bool isShrinkable);

    View& addView(View* child)override;
    View& addView(View*child, int index)override;
    View& addView(View* child, ViewGroup::LayoutParams* params)override;
    View& addView(View* child, int index, ViewGroup::LayoutParams* params)override;

    std::string getAccessibilityClassName()const override;
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
};
}
#endif
