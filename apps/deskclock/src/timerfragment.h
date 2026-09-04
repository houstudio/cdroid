#ifndef __DESKCLOCK_TIMERFRAGMENT_H__
#define __DESKCLOCK_TIMERFRAGMENT_H__
/*********************************************************************************
 * Port of com.android.deskclock.timer.TimerFragment — the TIMERS tab: a
 * vertical pager of running timers plus the keypad setup view, with the
 * translate/fade cross-animation between the two.
 *********************************************************************************/
#include <widget/button.h>
#include <widget/imageview.h>
#include <widget/viewpager.h>
#include <view/viewtreeobserver.h>

#include <datalisteners.h>
#include <deskclockfragment.h>
#include <timerpageradapter.h>
#include <timersetupview.h>

namespace cdroid {
namespace deskclock {
namespace timer {

class TimerFragment : public DeskClockFragment {
private:
    /** Notified when the user swipes vertically to change the visible timer. */
    ViewPager::OnPageChangeListener mTimerPageChangeListener;

    /** Scheduled to update the timers while at least one is running. */
    Runnable mTimeUpdateRunnable;

    /** Updates the page indicators in response to timers being added or removed. */
    data::TimerListener mTimerWatcher;

    TimerSetupView* mCreateTimerView = nullptr;
    ViewPager* mViewPager = nullptr;
    TimerPagerAdapter* mAdapter = nullptr;
    View* mTimersView = nullptr;
    View* mCurrentView = nullptr;
    ImageView* mPageIndicators[4] = {nullptr, nullptr, nullptr, nullptr};

    std::vector<int> mTimerSetupState;
    bool mHasTimerSetupState = false;

    /** True while this fragment is creating a new timer; false otherwise. */
    bool mCreatingTimer = false;

    // animateToView's OnPreDraw callback. Upstream captures per-call params in an
    // anonymous listener removed on first fire; cdroid removes by identity unreliably,
    // so the listener stays installed as a one-shot guarded by mPreDrawArmed and the
    // per-call params live in the members below.
    ViewTreeObserver::OnPreDrawListener mPreDrawListener;
    bool mPreDrawArmed = false;
    View* mAnimToView = nullptr;
    const data::Timer* mAnimTimerToRemove = nullptr;
    bool mAnimAnimateDown = false;

public:
    TimerFragment();
    ~TimerFragment() override;

    View* onCreateView(LayoutInflater* inflater, ViewGroup* container,
                       Bundle* savedInstanceState) override;
    void onStart() override;
    void onResume() override;
    void onStop() override;
    void onDestroyView() override;
    void onSaveInstanceState(Bundle* outState) override;

    void onUpdateFab(ImageView& fab) override;
    void onMorphFab(ImageView& fab) override;
    void onUpdateFabButtons(Button& left, Button& right) override;
    void onFabClick(ImageView& fab) override;
    void onLeftButtonClick(Button& left) override;
    void onRightButtonClick(Button& right) override;
    bool onKeyDown(int keyCode, KeyEvent& event) override;

    /**
     * @param page the selected page; value between 0 and pageCount
     * @param pageIndicatorCount the number of indicators displaying the page location
     * @param pageCount the number of pages that exist
     * @return an array of length pageIndicatorCount specifying which image to display for
     *         each page indicator or 0 if the page indicator should be hidden
     */
    static std::vector<int> computePageIndicatorStates(int page, int pageIndicatorCount,
                                                       int pageCount);

    /** @return an Intent that selects the timers tab with a new-timer setup screen. */
    static Intent createTimerSetupIntent();

private:
    // Expose the inherited updateFab(@UpdateFabFlag) alongside the (ImageView, bool) overload.
    using DeskClockFragment::updateFab;
    void updateFab(ImageView& fab, bool animate);
    void updatePageIndicators();
    void showCreateTimerView(int updateTypes);
    void showTimersView(int updateTypes);
    void animateTimerRemove(const data::Timer& timerToRemove);
    void animateToView(View* toView, const data::Timer* timerToRemove, bool animateDown);
    bool hasTimers() const;
    bool getCurrentTimer(data::Timer& outTimer) const;
    void startUpdatingTime();
    void stopUpdatingTime();
};

} // namespace timer
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERFRAGMENT_H__
