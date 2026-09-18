#ifndef __DESKCLOCK_ALARMRECYCLERVIEW_H__
#define __DESKCLOCK_ALARMRECYCLERVIEW_H__
/*********************************************************************************
 * Port of com.android.deskclock.AlarmRecyclerView — thin RecyclerView wrapper
 * that prevents simultaneous layout passes, particularly during animations.
 *********************************************************************************/
#include <widgetEx/recyclerview/recyclerview.h>

namespace cdroid {
namespace deskclock {

class AlarmRecyclerView : public RecyclerView {
private:
    bool mIgnoreRequestLayout = false;

    RecyclerView::OnItemTouchListener mDisallowScrollListener;

public:
    AlarmRecyclerView(Context* context, const AttributeSet* attrs);

protected:
    void onLayout(bool changed, int left, int top, int right, int bottom) override;
    void requestLayout() override;
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMRECYCLERVIEW_H__
