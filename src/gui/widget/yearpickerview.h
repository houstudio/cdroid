#ifndef __YEARPICKERVIEW_H__
#define __YEARPICKERVIEW_H__
#include <widget/listview.h>

namespace cdroid{

class YearPickerView:public ListView{
public:
    DECLARE_UIEVENT(void,OnYearSelectedListener,YearPickerView& view, int year);
private:
    class YearAdapter* mAdapter;
    int mViewSize;
    int mChildSize;
    OnYearSelectedListener mOnYearSelectedListener;
public:
    YearPickerView(int w,int h);
    YearPickerView(Context* context, const AttributeSet& attrs);
    void setOnYearSelectedListener(OnYearSelectedListener listener);
    void setYear(int year);
    void setSelectionCentered(int position);
    void setRange(int min,int max/*Calendar min, Calendar max*/);
    int getFirstPositionOffset();
};
    
}//namespace

#endif
