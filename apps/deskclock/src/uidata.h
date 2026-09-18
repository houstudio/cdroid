#ifndef __DESKCLOCK_UIDATA_H__
#define __DESKCLOCK_UIDATA_H__
/*********************************************************************************
 * Port of com.android.deskclock.uidata — Tab{Model,DAO,Listener,ScrollListener},
 * PeriodicCallbackModel, FormattedStringModel and the UiDataModel singleton.
 *
 * CDROID has no system broadcasts: the time-changed/locale-changed receivers
 * upstream are not ported (callbacks simply run on their wall-clock schedule;
 * locale-sensitive caches rebuild lazily per process life).
 *********************************************************************************/
#include <map>
#include <string>
#include <vector>

#include <core/callbackbase.h>
#include <core/calendar.h>
#include <content/Locale.h>
#include <content/simpledateformat.h>
#include <core/handler.h>
#include <view/view.h>

namespace cdroid {

class Context;
class SharedPreferences;

namespace deskclock {
namespace uidata {

class UiDataModel;

/** The interface through which interested parties are notified of selected-tab changes. */
typedef CallbackBase<void, int /*oldSelectedTab*/, int /*newSelectedTab*/> TabListener;

/** Notifies of changes to the vertical scroll position of the selected tab. */
typedef CallbackBase<void, int /*selectedTab*/, bool /*scrolledToTop*/> TabScrollListener;

/** Identifies each of the primary tabs within the application. */
struct Tab {
    enum Value { ALARMS = 0, CLOCKS = 1, TIMERS = 2, STOPWATCH = 3 };

    Value value;
    const char* fragmentClassName;
    int iconResId;
    int labelResId;
};

/** Encapsulates the storage of tab data in SharedPreferences. */
class TabDAO {
public:
    static const constexpr char* KEY_SELECTED_TAB = "selected_tab";

    /** @return an enumerated value indicating the currently selected primary tab. */
    static int getSelectedTab(SharedPreferences& prefs);

    /** @param tab an enumerated value indicating the newly selected primary tab. */
    static void setSelectedTab(SharedPreferences& prefs, int tab);
};

/** All tab data is accessed via this model. */
class TabModel {
private:
    SharedPreferences& mPrefs;

    /** The listeners to notify when the selected tab is changed. */
    std::vector<TabListener> mTabListeners;

    /** The listeners to notify when the vertical scroll state of the selected tab changes. */
    std::vector<TabScrollListener> mTabScrollListeners;

    /** The scrolled-to-top state of each tab. */
    std::vector<bool> mTabScrolledToTop;

    /** An enumerated value indicating the currently selected tab; -1 before first read. */
    int mSelectedTab = -1;

public:
    explicit TabModel(SharedPreferences& prefs);

    void addTabListener(const TabListener& tabListener);
    void removeTabListener(const TabListener& tabListener);

    int getTabCount() const;
    Tab getTab(int ordinal) const;
    /** @param position the position of the tab in the UI (RTL-aware). */
    Tab getTabAt(int position) const;

    int getSelectedTab();
    void setSelectedTab(int tab);

    void addTabScrollListener(const TabScrollListener& tabScrollListener);
    void removeTabScrollListener(const TabScrollListener& tabScrollListener);

    void setTabScrolledToTop(int tab, bool scrolledToTop);
    bool isTabScrolledToTop(int tab) const;
};

/** All callbacks to be delivered at requested times on the main thread. */
class PeriodicCallbackModel {
public:
    enum Period { MINUTE, QUARTER_HOUR, HOUR, MIDNIGHT };

    explicit PeriodicCallbackModel(Context& context);

    void addMinuteCallback(const Runnable& runnable, int64_t offset);
    void addQuarterHourCallback(const Runnable& runnable);
    void addHourCallback(const Runnable& runnable);
    void addMidnightCallback(const Runnable& runnable);

    void removePeriodicCallback(const Runnable& runnable);

    /** Return the delay until the given period elapses adjusted by the given offset. */
    static int64_t getDelay(int64_t now, Period period, int64_t offset);

private:
    class PeriodicRunnable {
    public:
        Runnable mDelegate;
        Period mPeriod;
        int64_t mOffset;
        Handler mHandler; // main-looper handler that owns this schedule

        PeriodicRunnable(const Runnable& delegate, Period period, int64_t offset,
                         const Handler& handler);
        void run();
        void runAndReschedule();
        void schedule();
        void unSchedule();
    };

    std::vector<PeriodicRunnable*> mPeriodicRunnables;
};

/** All formatted strings that are cached for performance are accessed via this model. */
class FormattedStringModel {
private:
    /** length (negated when negative-sign) -> value -> formatted string. */
    std::map<int, std::map<int, std::string>> mNumberFormatCache;

    std::map<int, std::string> mShortWeekdayNames;
    std::map<int, std::string> mLongWeekdayNames;

public:
    std::string getFormattedNumber(int value);
    std::string getFormattedNumber(int value, int length);
    std::string getFormattedNumber(bool negative, int value, int length);

    std::string getShortWeekday(int calendarDay);
    std::string getLongWeekday(int calendarDay);
};

/** All application-wide user interface data is accessible through this singleton. */
class UiDataModel {
private:
    Context* mContext = nullptr;
    SharedPreferences* mPrefs = nullptr;

    TabModel* mTabModel = nullptr;
    FormattedStringModel* mFormattedStringModel = nullptr;
    PeriodicCallbackModel* mPeriodicCallbackModel = nullptr;

    UiDataModel() = default;

public:
    static UiDataModel& getUiDataModel();

    /** The context may be set precisely once during the application life. */
    void init(Context& context, SharedPreferences& prefs);

    // --- Formatted Strings ---
    std::string getFormattedNumber(int value);
    std::string getFormattedNumber(int value, int length);
    std::string getFormattedNumber(bool negative, int value, int length);
    std::string getShortWeekday(int calendarDay);
    std::string getLongWeekday(int calendarDay);

    // --- Animations ---
    int64_t getShortAnimationDuration() const;
    int64_t getLongAnimationDuration() const;

    // --- Tabs ---
    void addTabListener(const TabListener& tabListener);
    void removeTabListener(const TabListener& tabListener);
    int getTabCount() const;
    Tab getTab(int ordinal) const;
    Tab getTabAt(int position) const;
    int getSelectedTab();
    void setSelectedTab(int tab);
    void addTabScrollListener(const TabScrollListener& tabScrollListener);
    void removeTabScrollListener(const TabScrollListener& tabScrollListener);
    void setTabScrolledToTop(int tab, bool scrolledToTop);
    bool isSelectedTabScrolledToTop();

    // --- Timed Callbacks ---
    void addMinuteCallback(const Runnable& runnable, int64_t offset);
    void addQuarterHourCallback(const Runnable& runnable);
    void addHourCallback(const Runnable& runnable);
    void addMidnightCallback(const Runnable& runnable);
    void removePeriodicCallback(const Runnable& runnable);
};

} // namespace uidata
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_UIDATA_H__
