#include <uidata.h>

#include <porting/cdlog.h>

#include <R.h>
#include <cmath>
#include <cstdio>

#include <content/resources.h>
#include <content/sharedpreferences.h>
#include <core/context.h>
#include <core/systemclock.h>
#include <text/textutils.h>
#include <widget/internal_R.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace uidata {

namespace {

const Tab TABS[] = {
    {Tab::ALARMS,     "AlarmClockFragment",  R::drawable::ic_tab_alarm,      R::string::menu_alarm},
    {Tab::CLOCKS,     "ClockFragment",       R::drawable::ic_tab_clock,      R::string::menu_clock},
    {Tab::TIMERS,     "TimerFragment",       R::drawable::ic_tab_timer,      R::string::menu_timer},
    {Tab::STOPWATCH,  "StopwatchFragment",   R::drawable::ic_tab_stopwatch,  R::string::menu_stopwatch},
};
constexpr int TAB_COUNT = 4;

} // namespace

//
// TabDAO
//

int TabDAO::getSelectedTab(SharedPreferences& prefs) {
    return prefs.getInt(KEY_SELECTED_TAB, Tab::CLOCKS);
}

void TabDAO::setSelectedTab(SharedPreferences& prefs, int tab) {
    prefs.edit().putInt(KEY_SELECTED_TAB, tab).apply();
}

//
// TabModel
//

TabModel::TabModel(SharedPreferences& prefs) : mPrefs(prefs) {
    mTabScrolledToTop.assign(TAB_COUNT, true);
}

void TabModel::addTabListener(const TabListener& tabListener) {
    mTabListeners.push_back(tabListener);
}

void TabModel::removeTabListener(const TabListener& tabListener) {
    for (auto it = mTabListeners.begin(); it != mTabListeners.end(); ++it) {
        if (*it == tabListener) {
            mTabListeners.erase(it);
            return;
        }
    }
}

int TabModel::getTabCount() const {
    return TAB_COUNT;
}

Tab TabModel::getTab(int ordinal) const {
    return TABS[ordinal];
}

Tab TabModel::getTabAt(int position) const {
    const int layoutDirection = TextUtils::getLayoutDirectionFromLocale(Locale::getDefault());
    const int ordinal = (layoutDirection == View::LAYOUT_DIRECTION_RTL)
            ? TAB_COUNT - position - 1 : position;
    return getTab(ordinal);
}

int TabModel::getSelectedTab() {
    if (mSelectedTab == -1) {
        mSelectedTab = TabDAO::getSelectedTab(mPrefs);
    }
    return mSelectedTab;
}

void TabModel::setSelectedTab(int tab) {
    const int oldSelectedTab = getSelectedTab();
    if (oldSelectedTab != tab) {
        mSelectedTab = tab;
        TabDAO::setSelectedTab(mPrefs, tab);

        // Notify of the tab change.
        for (TabListener& tl : mTabListeners) {
            if (tl) tl(oldSelectedTab, tab);
        }

        // Notify of the vertical scroll position change if there is one.
        const bool tabScrolledToTop = isTabScrolledToTop(tab);
        if (isTabScrolledToTop(oldSelectedTab) != tabScrolledToTop) {
            for (TabScrollListener& tsl : mTabScrollListeners) {
                if (tsl) tsl(tab, tabScrolledToTop);
            }
        }
    }
}

void TabModel::addTabScrollListener(const TabScrollListener& tabScrollListener) {
    mTabScrollListeners.push_back(tabScrollListener);
}

void TabModel::removeTabScrollListener(const TabScrollListener& tabScrollListener) {
    for (auto it = mTabScrollListeners.begin(); it != mTabScrollListeners.end(); ++it) {
        if (*it == tabScrollListener) {
            mTabScrollListeners.erase(it);
            return;
        }
    }
}

void TabModel::setTabScrolledToTop(int tab, bool scrolledToTop) {
    if (isTabScrolledToTop(tab) != scrolledToTop) {
        mTabScrolledToTop[tab] = scrolledToTop;
        if (tab == getSelectedTab()) {
            for (TabScrollListener& tsl : mTabScrollListeners) {
                if (tsl) tsl(tab, scrolledToTop);
            }
        }
    }
}

bool TabModel::isTabScrolledToTop(int tab) const {
    return mTabScrolledToTop[tab];
}

//
// PeriodicCallbackModel
//

namespace {
constexpr int64_t MINUTE_IN_MILLIS = 60LL * 1000;
constexpr int64_t QUARTER_HOUR_IN_MILLIS = 15 * MINUTE_IN_MILLIS;
constexpr int64_t HOUR_IN_MILLIS = 60 * MINUTE_IN_MILLIS;
} // namespace

PeriodicCallbackModel::PeriodicRunnable::PeriodicRunnable(const Runnable& delegate,
        Period period, int64_t offset, const Handler& handler)
    : mDelegate(delegate), mPeriod(period), mOffset(offset), mHandler(handler) {
}

void PeriodicCallbackModel::PeriodicRunnable::run() {
    mDelegate();
    schedule();
}

void PeriodicCallbackModel::PeriodicRunnable::runAndReschedule() {
    unSchedule();
    mDelegate();
    schedule();
}

void PeriodicCallbackModel::PeriodicRunnable::schedule() {
    const int64_t delay = getDelay(SystemClock::currentTimeMillis(), mPeriod, mOffset);
    mHandler.postDelayed(mDelegate, delay);
}

void PeriodicCallbackModel::PeriodicRunnable::unSchedule() {
    mHandler.removeCallbacks(mDelegate);
}

PeriodicCallbackModel::PeriodicCallbackModel(Context& /*context*/) {
    // Upstream registers a TIME/DATE/TIMEZONE_CHANGED receiver to reschedule on
    // device time changes; CDROID has no system broadcasts, so callbacks simply
    // follow the wall clock.
}

void PeriodicCallbackModel::addMinuteCallback(const Runnable& runnable, int64_t offset) {
    PeriodicRunnable* periodicRunnable =
            new PeriodicRunnable(runnable, MINUTE, offset, Handler(Looper::getMainLooper()));
    mPeriodicRunnables.push_back(periodicRunnable);
    periodicRunnable->schedule();
}

void PeriodicCallbackModel::addQuarterHourCallback(const Runnable& runnable) {
    // Callbacks *can* occur early so pad in an extra 100ms on the quarter-hour callback
    // to ensure the sampled wallclock time reflects the subsequent quarter-hour.
    PeriodicRunnable* periodicRunnable = new PeriodicRunnable(runnable, QUARTER_HOUR, 100,
            Handler(Looper::getMainLooper()));
    mPeriodicRunnables.push_back(periodicRunnable);
    periodicRunnable->schedule();
}

void PeriodicCallbackModel::addHourCallback(const Runnable& runnable) {
    // Callbacks *can* occur early so pad in an extra 100ms on the hour callback to ensure
    // the sampled wallclock time reflects the subsequent hour.
    PeriodicRunnable* periodicRunnable =
            new PeriodicRunnable(runnable, HOUR, 100, Handler(Looper::getMainLooper()));
    mPeriodicRunnables.push_back(periodicRunnable);
    periodicRunnable->schedule();
}

void PeriodicCallbackModel::addMidnightCallback(const Runnable& runnable) {
    // Callbacks *can* occur early so pad in an extra 100ms on the midnight callback to ensure
    // the sampled wallclock time reflects the subsequent day.
    PeriodicRunnable* periodicRunnable =
            new PeriodicRunnable(runnable, MIDNIGHT, 100, Handler(Looper::getMainLooper()));
    mPeriodicRunnables.push_back(periodicRunnable);
    periodicRunnable->schedule();
}

void PeriodicCallbackModel::removePeriodicCallback(const Runnable& runnable) {
    for (auto it = mPeriodicRunnables.begin(); it != mPeriodicRunnables.end(); ++it) {
        if ((*it)->mDelegate == runnable) {
            (*it)->unSchedule();
            delete *it;
            mPeriodicRunnables.erase(it);
            return;
        }
    }
}

int64_t PeriodicCallbackModel::getDelay(int64_t now, Period period, int64_t offset) {
    const int64_t periodStart = now - offset;

    switch (period) {
        case MINUTE: {
            const int64_t lastMinute = periodStart - periodStart % MINUTE_IN_MILLIS;
            const int64_t nextMinute = lastMinute + MINUTE_IN_MILLIS;
            return nextMinute - now + offset;
        }
        case QUARTER_HOUR: {
            const int64_t lastQuarterHour = periodStart - periodStart % QUARTER_HOUR_IN_MILLIS;
            const int64_t nextQuarterHour = lastQuarterHour + QUARTER_HOUR_IN_MILLIS;
            return nextQuarterHour - now + offset;
        }
        case HOUR: {
            const int64_t lastHour = periodStart - periodStart % HOUR_IN_MILLIS;
            const int64_t nextHour = lastHour + HOUR_IN_MILLIS;
            return nextHour - now + offset;
        }
        case MIDNIGHT: {
            std::unique_ptr<Calendar> nextMidnight = Calendar::getInstance();
            nextMidnight->setTimeInMillis(periodStart);
            nextMidnight->add(Calendar::DATE, 1);
            nextMidnight->set(Calendar::HOUR_OF_DAY, 0);
            nextMidnight->set(Calendar::MINUTE, 0);
            nextMidnight->set(Calendar::SECOND, 0);
            nextMidnight->set(Calendar::MILLISECOND, 0);
            return nextMidnight->getTimeInMillis() - now + offset;
        }
    }
    return 0;
}

//
// FormattedStringModel
//

std::string FormattedStringModel::getFormattedNumber(int value) {
    const int length = (value == 0) ? 1 : (int) log10((double) value) + 1;
    return getFormattedNumber(false, value, length);
}

std::string FormattedStringModel::getFormattedNumber(int value, int length) {
    return getFormattedNumber(false, value, length);
}

std::string FormattedStringModel::getFormattedNumber(bool negative, int value, int length) {
    if (value < 0) {
        // TEMP INSTRUMENTATION (remove after the DeskClock sweep diagnosis): upstream
        // throws here, which kills the app mid-sweep; log-and-clamp to trace the source.
        LOGW("getFormattedNumber: negative value %d (length=%d) clamped to 0", value, length);
        value = 0;
    }

    // Look up the value cache using the length; -ve and +ve values are cached separately.
    const int lengthCacheKey = negative ? -length : length;
    std::map<int, std::string>& valueCache = mNumberFormatCache[lengthCacheKey];

    // Look up the cached formatted value using the value.
    std::string& formatted = valueCache[value];
    if (formatted.empty()) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%s%0*d", negative ? "\xE2\x88\x92" : "", length, value);
        formatted = buf;
    }
    return formatted;
}

std::string FormattedStringModel::getShortWeekday(int calendarDay) {
    if (mShortWeekdayNames.empty()) {
        SimpleDateFormat format("ccccc", Locale::getDefault());
        for (int i = Calendar::SUNDAY; i <= Calendar::SATURDAY; i++) {
            std::unique_ptr<Calendar> calendar = Calendar::getInstance();
            calendar->set(2014, Calendar::JULY, 20 + i - 1);
            mShortWeekdayNames[i] = format.format(calendar->getTimeInMillis());
        }
    }

    auto it = mShortWeekdayNames.find(calendarDay);
    return it != mShortWeekdayNames.end() ? it->second : std::string();
}

std::string FormattedStringModel::getLongWeekday(int calendarDay) {
    if (mLongWeekdayNames.empty()) {
        std::unique_ptr<Calendar> calendar = Calendar::getInstance();
        calendar->set(2014, Calendar::JULY, 20);
        SimpleDateFormat format("EEEE", Locale::getDefault());
        for (int i = Calendar::SUNDAY; i <= Calendar::SATURDAY; i++) {
            mLongWeekdayNames[i] = format.format(calendar->getTimeInMillis());
            calendar->add(Calendar::DAY_OF_YEAR, 1);
        }
    }

    auto it = mLongWeekdayNames.find(calendarDay);
    return it != mLongWeekdayNames.end() ? it->second : std::string();
}

//
// UiDataModel
//

UiDataModel& UiDataModel::getUiDataModel() {
    static UiDataModel sUiDataModel;
    return sUiDataModel;
}

void UiDataModel::init(Context& context, SharedPreferences& prefs) {
    if (mContext != &context) {
        mContext = &context;
        mPrefs = &prefs;

        delete mPeriodicCallbackModel;
        delete mFormattedStringModel;
        delete mTabModel;
        mPeriodicCallbackModel = new PeriodicCallbackModel(context);
        mFormattedStringModel = new FormattedStringModel();
        mTabModel = new TabModel(prefs);
    }
}

std::string UiDataModel::getFormattedNumber(int value) {
    return mFormattedStringModel->getFormattedNumber(value);
}

std::string UiDataModel::getFormattedNumber(int value, int length) {
    return mFormattedStringModel->getFormattedNumber(value, length);
}

std::string UiDataModel::getFormattedNumber(bool negative, int value, int length) {
    return mFormattedStringModel->getFormattedNumber(negative, value, length);
}

std::string UiDataModel::getShortWeekday(int calendarDay) {
    return mFormattedStringModel->getShortWeekday(calendarDay);
}

std::string UiDataModel::getLongWeekday(int calendarDay) {
    return mFormattedStringModel->getLongWeekday(calendarDay);
}

int64_t UiDataModel::getShortAnimationDuration() const {
    return mContext->getResources().getInteger(internal::R::integer::config_shortAnimTime);
}

int64_t UiDataModel::getLongAnimationDuration() const {
    return mContext->getResources().getInteger(internal::R::integer::config_longAnimTime);
}

void UiDataModel::addTabListener(const TabListener& tabListener) {
    mTabModel->addTabListener(tabListener);
}

void UiDataModel::removeTabListener(const TabListener& tabListener) {
    mTabModel->removeTabListener(tabListener);
}

int UiDataModel::getTabCount() const {
    return mTabModel->getTabCount();
}

Tab UiDataModel::getTab(int ordinal) const {
    return mTabModel->getTab(ordinal);
}

Tab UiDataModel::getTabAt(int position) const {
    return mTabModel->getTabAt(position);
}

int UiDataModel::getSelectedTab() {
    return mTabModel->getSelectedTab();
}

void UiDataModel::setSelectedTab(int tab) {
    mTabModel->setSelectedTab(tab);
}

void UiDataModel::addTabScrollListener(const TabScrollListener& tabScrollListener) {
    mTabModel->addTabScrollListener(tabScrollListener);
}

void UiDataModel::removeTabScrollListener(const TabScrollListener& tabScrollListener) {
    mTabModel->removeTabScrollListener(tabScrollListener);
}

void UiDataModel::setTabScrolledToTop(int tab, bool scrolledToTop) {
    mTabModel->setTabScrolledToTop(tab, scrolledToTop);
}

bool UiDataModel::isSelectedTabScrolledToTop() {
    return mTabModel->isTabScrolledToTop(getSelectedTab());
}

void UiDataModel::addMinuteCallback(const Runnable& runnable, int64_t offset) {
    mPeriodicCallbackModel->addMinuteCallback(runnable, offset);
}

void UiDataModel::addQuarterHourCallback(const Runnable& runnable) {
    mPeriodicCallbackModel->addQuarterHourCallback(runnable);
}

void UiDataModel::addHourCallback(const Runnable& runnable) {
    mPeriodicCallbackModel->addHourCallback(runnable);
}

void UiDataModel::addMidnightCallback(const Runnable& runnable) {
    mPeriodicCallbackModel->addMidnightCallback(runnable);
}

void UiDataModel::removePeriodicCallback(const Runnable& runnable) {
    mPeriodicCallbackModel->removePeriodicCallback(runnable);
}

} // namespace uidata
} // namespace deskclock
} // namespace cdroid
