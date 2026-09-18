#ifndef __DESKCLOCK_TIMERITEMFRAGMENT_H__
#define __DESKCLOCK_TIMERITEMFRAGMENT_H__
/*********************************************************************************
 * Port of com.android.deskclock.timer.TimerItemFragment — one page of the
 * vertical timer pager: a TimerItem bound to the timer with the given id.
 *********************************************************************************/
#include <fragment/fragment.h>

#include <timer.h>

namespace cdroid {
namespace deskclock {
namespace timer {

class TimerItemFragment : public Fragment {
private:
    int mTimerId = 0;

public:
    void onCreate(cdroid::Bundle* savedInstanceState) override;
    View* onCreateView(LayoutInflater* inflater, ViewGroup* container,
                       cdroid::Bundle* savedInstanceState) override;

    /**
     * @return true iff the timer is in a state that requires continuous updates
     */
    bool updateTime();

    /** @return the timer this page displays (id-invalid Timer when gone). */
    data::Timer getTimer() const;

    int getTimerId() const { return mTimerId; }

    static TimerItemFragment* newInstance(const data::Timer& timer);

private:
    void onResetAddClick(View& view);
};

} // namespace timer
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERITEMFRAGMENT_H__
