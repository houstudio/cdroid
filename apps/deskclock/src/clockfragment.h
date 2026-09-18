#ifndef __DESKCLOCK_CLOCKFRAGMENT_H__
#define __DESKCLOCK_CLOCKFRAGMENT_H__
/*********************************************************************************
 * Port of com.android.deskclock.ClockFragment — the CLOCKS tab: main clock
 * frame (or list header) + selected-world-cities RecyclerView.
 *
 * Cut from upstream: the pre-L alarm ContentObserver, the L+ next-alarm
 * broadcast receiver (no system broadcasts on cdroid; refreshAlarm is invoked
 * from the periodic/scroll paths), and the long-press→screensaver gesture
 * (screensaver module pending).
 *********************************************************************************/
#include <string>

#include <widget/textclock.h>
#include <widgetEx/recyclerview/recyclerview.h>

#include <analogclock.h>
#include <deskclockfragment.h>

namespace cdroid {
namespace deskclock {

class ClockFragment : public DeskClockFragment {
private:
    /** Updates dates in the UI on every quarter-hour. */
    Runnable mQuarterHourUpdater;

    TextClock* mDigitalClock = nullptr;
    AnalogClock* mAnalogClock = nullptr;
    View* mClockFrame = nullptr;

    class SelectedCitiesAdapter;
    SelectedCitiesAdapter* mCityAdapter = nullptr;
    RecyclerView* mCityList = nullptr;

    std::string mDateFormat;
    std::string mDateFormatForAccessibility;

    RecyclerView::OnScrollListener mScrollPositionWatcher;

public:
    ClockFragment();
    ~ClockFragment() override;

    View* onCreateView(LayoutInflater* inflater, ViewGroup* container,
                       Bundle* savedInstanceState) override;
    void onResume() override;
    void onDestroyView() override;

    void onFabClick(ImageView& fab) override;
    void onUpdateFab(ImageView& fab) override;
    void onUpdateFabButtons(Button& left, Button& right) override;

private:
    void refreshAlarm();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_CLOCKFRAGMENT_H__
