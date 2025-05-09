#ifndef __ABS_SPINNER_H__
#define __ABS_SPINNER_H__
#include <widget/adapterview.h>
#include <core/sparsearray.h>
namespace cdroid{
class AbsSpinner:public AdapterView{
protected:
    class RecycleBin{
    private:
        SparseArray<View*> mScrapHeap;
        AbsSpinner*ABS;
    public:
        RecycleBin(AbsSpinner*);
        void put(int position, View* v);
        View* get(int position);
        void clear();
    };
    RecycleBin*mRecycler;
    DataSetObserver* mDataSetObserver;
    int mHeightMeasureSpec;
    int mWidthMeasureSpec;
    int mSelectionLeftPadding;
    int mSelectionTopPadding;
    int mSelectionRightPadding;
    int mSelectionBottomPadding;
    Rect mSpinnerPadding;
    Rect mTouchFrame;
    void initAbsSpinner();
    void resetList();
    void recycleAllViews();
    int getChildWidth(View* child);
    virtual int getChildHeight(View* child);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    virtual void layout(int delta, bool animate)=0;
public:
    AbsSpinner(int w,int h);
    AbsSpinner(Context*,const AttributeSet&atts);
    virtual ~AbsSpinner();
    int getCount()override;
    View* getSelectedView()override;
    void setAdapter(Adapter*adapter)override;
    void setSelection(int position, bool animate);
    void setSelection(int position)override;
    virtual void setSelectionInt(int position, bool animate);
    int pointToPosition(int x, int y);
    void requestLayout()override;
};
}//namespace
#endif
