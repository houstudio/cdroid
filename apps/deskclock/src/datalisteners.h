#ifndef __DESKCLOCK_DATALISTENERS_H__
#define __DESKCLOCK_DATALISTENERS_H__
/*********************************************************************************
 * Ports of the data-package listener interfaces (TimerListener, StopwatchListener,
 * CityListener, OnSilentSettingsListener) — multi-callback EventSet classes per
 * the cdroid listener convention.
 *********************************************************************************/
#include <core/callbackbase.h>

#include <city.h>
#include <stopwatch.h>
#include <timer.h>

namespace cdroid {
namespace deskclock {
namespace data {

/** The interface through which interested parties are notified of timer changes. */
class TimerListener : public EventSet {
public:
    CallbackBase<void, const Timer&> timerAdded;
    CallbackBase<void, const Timer&> timerRemoved;
    CallbackBase<void, const Timer& /*before*/, const Timer& /*after*/> timerUpdated;
};

/** The interface through which interested parties are notified of stopwatch changes. */
class StopwatchListener : public EventSet {
public:
    CallbackBase<void, const Stopwatch& /*before*/, const Stopwatch& /*after*/> stopwatchUpdated;
    CallbackBase<void, const Lap&> lapAdded;
};

/** The interface through which interested parties are notified of city changes. */
class CityListener : public EventSet {
public:
    CallbackBase<void, const std::vector<City>& /*old*/, const std::vector<City>& /*new_*/> citiesChanged;
    CallbackBase<void, const City&> homeCityChanged;
};

/** The interface through which interested parties are notified of silencing settings. */
class OnSilentSettingsListener : public EventSet {
public:
    CallbackBase<void, void* /*before (unused)*/, void* /*after (unused)*/> onSilentSettingsChange;
};

} // namespace data
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_DATALISTENERS_H__
