#ifndef __DESKCLOCK_STOPWATCHFRAGMENT_H__
#define __DESKCLOCK_STOPWATCHFRAGMENT_H__
/*********************************************************************************
 * Port of com.android.deskclock.stopwatch.StopwatchFragment — the STOPWATCH
 * tab: circle + time text (blink while paused) + laps list with gradient
 * decoration; fab toggles run/pause, left button resets, right button
 * adds a lap (running) or shares (paused).
 *
 * CDROID facades: share chooser opens nothing (no system share targets);
 * the keep-screen-on window flag is a no-op (no WindowManager flags).
 *********************************************************************************/
#include <widget/button.h>
#include <widget/imageview.h>
#include <widget/textview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/recyclerview.h>

#include <datalisteners.h>
#include <deskclockfragment.h>
#include <lapsadapter.h>
#include <stopwatchcircleview.h>
#include <stopwatchtextcontroller.h>

namespace cdroid {
namespace deskclock {
namespace stopwatch {

class StopwatchFragment : public DeskClockFragment {
private:
    /** Updates the UI in response to changes to the selected tab. */
    uidata::TabListener mTabWatcher;

    /** Periodically updates the time text while this fragment is shown. */
    Runnable mTimeUpdateRunnable;

    /** Updates the UI in response to stopwatch and lap changes. */
    data::StopwatchListener mStopwatchWatcher;

    class GradientItemDecoration;
    GradientItemDecoration* mGradientItemDecoration = nullptr;

    LapsAdapter* mLapsAdapter = nullptr;
    LinearLayoutManager* mLapsLayoutManager = nullptr;

    StopwatchCircleView* mTime = nullptr;

    View* mStopwatchWrapper = nullptr;
    RecyclerView* mLapsList = nullptr;
    TextView* mMainTimeText = nullptr;
    TextView* mHundredthsTimeText = nullptr;
    StopwatchTextController* mStopwatchTextController = nullptr;

    RecyclerView::OnScrollListener mScrollPositionWatcher;

public:
    StopwatchFragment();
    ~StopwatchFragment() override;

    View* onCreateView(LayoutInflater* inflater, ViewGroup* container,
                       Bundle* savedInstanceState) override;
    void onStart() override;
    void onStop() override;
    void onDestroyView() override;

    void onFabClick(ImageView& fab) override;
    void onLeftButtonClick(Button& left) override;
    void onRightButtonClick(Button& right) override;

    void onMorphFab(ImageView& fab) override;
    void onUpdateFab(ImageView& fab) override;
    void onUpdateFabButtons(Button& left, Button& right) override;

private:
    // Expose the inherited updateFab(@UpdateFabFlag) alongside the (ImageView, bool) overload.
    using DeskClockFragment::updateFab;
    void updateFab(ImageView& fab, bool animate);
    void doStart();
    void doPause();
    void doReset();
    void doShare();
    void doAddLap();

    /** Shows/hides the laps list (and bottom padding) accordingly. */
    void showOrHideLaps(bool clearLaps);

    void toggleStopwatchState();

    void startUpdatingTime();
    void stopUpdatingTime();
    void updateTime();
    void updateUI(int updateTypes);
};

} // namespace stopwatch
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_STOPWATCHFRAGMENT_H__
