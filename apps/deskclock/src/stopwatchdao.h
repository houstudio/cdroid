#ifndef __DESKCLOCK_STOPWATCHDAO_H__
#define __DESKCLOCK_STOPWATCHDAO_H__
/*********************************************************************************
 * Port of com.android.deskclock.data.StopwatchDAO — transfer of data between
 * Stopwatch/Lap domain objects and SharedPreferences.
 *********************************************************************************/
#include <string>
#include <vector>

#include <stopwatch.h>

namespace cdroid {

class SharedPreferences;

namespace deskclock {
namespace data {

class StopwatchDAO {
public:
    /** @return the stopwatch from permanent storage or a reset stopwatch if none exists. */
    static Stopwatch getStopwatch(SharedPreferences& prefs);

    /** @param stopwatch the last state of the stopwatch. */
    static void setStopwatch(SharedPreferences& prefs, const Stopwatch& stopwatch);

    /** @return a list of recorded laps for the stopwatch (display order: newest first). */
    static std::vector<Lap> getLaps(SharedPreferences& prefs);

    /** @param newLapCount the number of laps including the new lap. */
    static void addLap(SharedPreferences& prefs, int newLapCount, int64_t accumulatedTime);

    /** Removes all laps from permanent storage. */
    static void clearLaps(SharedPreferences& prefs);
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_STOPWATCHDAO_H__
