#ifndef __DAYPICKER_VIEWPAGER_H__
#define __DAYPICKER_VIEWPAGER_H__
#include <widget/viewpager.h>
namespace cdroid{

class DayPickerViewPager:public ViewPager{
private:
    std::vector<View*>mMatchParentChildren;
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
public:
    DayPickerViewPager(Context* context,const AttributeSet& attrs);
    View*findViewByPredicateTraversal(std::function<bool(View*)>predicate,View* childToSkip)override;
};
}//endof namespace
#endif
