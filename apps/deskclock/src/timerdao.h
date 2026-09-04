#ifndef __DESKCLOCK_TIMERDAO_H__
#define __DESKCLOCK_TIMERDAO_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.TimerDAO — transfer of data between Timer
 * domain objects and their permanent storage in SharedPreferences.
 *********************************************************************************/
#include <set>
#include <string>
#include <vector>

#include <timer.h>

namespace cdroid {

class SharedPreferences;

namespace deskclock {
namespace data {

class TimerDAO {
public:
    /** @return the timers from permanent storage. */
    static std::vector<Timer> getTimers(SharedPreferences& prefs);

    /** @param timer the timer to be added. */
    static Timer addTimer(SharedPreferences& prefs, const Timer& timer);

    /** @param timer the timer to be updated. */
    static void updateTimer(SharedPreferences& prefs, const Timer& timer);

    /** @param timer the timer to be removed. */
    static void removeTimer(SharedPreferences& prefs, const Timer& timer);

private:
    static std::set<std::string> getTimerIds(SharedPreferences& prefs);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMERDAO_H__
