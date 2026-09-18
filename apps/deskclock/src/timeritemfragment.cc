#include <timeritemfragment.h>

#include <R.h>

#include <core/bundle.h>
#include <view/layoutinflater.h>
#include <widget/textview.h>

#include <datamodel.h>
#include <labeldialogfragment.h>
#include <timeritem.h>
#include <timerstringformatter.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace timer {

namespace {
constexpr const char* KEY_TIMER_ID = "KEY_TIMER_ID";
} // namespace

TimerItemFragment* TimerItemFragment::newInstance(const data::Timer& timer) {
    TimerItemFragment* fragment = new TimerItemFragment();
    Bundle* args = new Bundle();
    args->putInt(KEY_TIMER_ID, timer.id);
    fragment->setArguments(args);
    return fragment;
}

void TimerItemFragment::onCreate(cdroid::Bundle* savedInstanceState) {
    Fragment::onCreate(savedInstanceState);

    const Bundle* args = getArguments();
    if (args != nullptr) {
        mTimerId = args->getInt(KEY_TIMER_ID, 0);
    }
}

View* TimerItemFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
                                      Bundle* savedInstanceState) {
    data::Timer timer;
    if (!data::DataModel::getDataModel().getTimer(mTimerId, timer)) {
        return nullptr;
    }

    View* view = inflater->inflate(R::layout::timer_item, container, false);
    view->findViewById(R::id::reset_add)->setOnClickListener(
            [this](View& v) { onResetAddClick(v); });
    view->findViewById(R::id::timer_label)->setOnClickListener([this](View&) {
        data::Timer timer = getTimer();
        LabelDialogFragment::showDialog(getParentFragmentManager(),
                LabelDialogFragment::newInstance(timer));
    });
    view->findViewById(R::id::timer_time_text)->setOnClickListener([this](View&) {
        // TimeTextListener: tap toggles start/pause.
        data::Timer clickedTimer = getTimer();
        if (clickedTimer.isPaused() || clickedTimer.isReset()) {
            data::DataModel::getDataModel().startTimer(clickedTimer);
        } else if (clickedTimer.isRunning()) {
            data::DataModel::getDataModel().pauseTimer(clickedTimer);
        }
    });
    ((TimerItem*) view)->update(timer);

    return view;
}

bool TimerItemFragment::updateTime() {
    TimerItem* view = (TimerItem*) getView();
    if (view != nullptr) {
        data::Timer timer = getTimer();
        view->update(timer);
        return !timer.isReset();
    }
    return false;
}

data::Timer TimerItemFragment::getTimer() const {
    data::Timer out;
    data::DataModel::getDataModel().getTimer(mTimerId, out);
    return out;
}

void TimerItemFragment::onResetAddClick(View& view) {
    data::Timer timer = getTimer();
    if (timer.isPaused()) {
        data::Timer outTimer;
        data::DataModel::getDataModel().resetOrDeleteTimer(timer,
                R::string::label_deskclock, outTimer);
    } else if (timer.isRunning() || timer.isExpired() || timer.isMissed()) {
        data::DataModel::getDataModel().addTimerMinute(timer);

        // Must re-retrieve timer because old timer is no longer accurate.
        data::Timer fresh = getTimer();
        const int64_t currentTime = fresh.getRemainingTime();
        if (currentTime > 0) {
            // announceForAccessibility(TimerStringFormatter.formatString(
            //     context, R.string.timer_accessibility_one_minute_added, currentTime, true))
            // — announcement face TODO; keep the formatter exercised for parity.
            (void) data::TimerStringFormatter::formatString(*view.getContext(),
                    R::string::timer_accessibility_one_minute_added, currentTime, true);
        }
    }
}

} // namespace timer
} // namespace deskclock
} // namespace cdroid
