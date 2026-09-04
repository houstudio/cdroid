#include <datamodel.h>

#include <content/dateformat.h>
#include <content/sharedpreferences.h>
#include <core/context.h>
#include <core/systemclock.h>

#include <utils.h>

namespace cdroid {
namespace deskclock {
namespace data {

DataModel& DataModel::getDataModel() {
    static DataModel sDataModel;
    return sDataModel;
}

void DataModel::init(Context& context, SharedPreferences& prefs) {
    if (mContext != &context) {
        mContext = &context;

        delete mNotificationModel;
        delete mStopwatchModel;
        delete mTimerModel;
        delete mCityModel;
        delete mSettingsModel;

        mSettingsModel = new SettingsModel(context, prefs);
        mNotificationModel = new NotificationModel();
        mCityModel = new CityModel(context, prefs, *mSettingsModel);
        mTimerModel = new TimerModel(context, prefs, *mSettingsModel, *mNotificationModel);
        mStopwatchModel = new StopwatchModel(context, prefs, *mNotificationModel);
        mHandler.reset(new Handler(Looper::getMainLooper()));
    }
}

void DataModel::run(const Runnable& runnable) {
    getHandler().post(runnable);
}

void DataModel::run(const Runnable& runnable, int64_t waitMillis) {
    getHandler().postDelayed(runnable, waitMillis);
}

Handler& DataModel::getHandler() {
    if (!mHandler) {
        mHandler.reset(new Handler(Looper::getMainLooper()));
    }
    return *mHandler;
}

void DataModel::updateAfterReboot() {
    mTimerModel->updateTimersAfterReboot();
    mStopwatchModel->setStopwatch(mStopwatchModel->getStopwatch().updateAfterReboot());
}

void DataModel::updateAfterTimeSet() {
    mTimerModel->updateTimersAfterTimeSet();
    mStopwatchModel->setStopwatch(mStopwatchModel->getStopwatch().updateAfterTimeSet());
}

void DataModel::updateAllNotifications() {
    // No notification surface on cdroid.
}

//
// Cities
//

const std::vector<City>& DataModel::getAllCities() { return mCityModel->getAllCities(); }
const City& DataModel::getHomeCity() { return mCityModel->getHomeCity(); }
const std::vector<City>& DataModel::getUnselectedCities() { return mCityModel->getUnselectedCities(); }
const std::vector<City>& DataModel::getSelectedCities() { return mCityModel->getSelectedCities(); }

void DataModel::setSelectedCities(const std::vector<City>& cities) {
    mCityModel->setSelectedCities(cities);
}

std::function<int(const City&, const City&)> DataModel::getCityIndexComparator() {
    return mCityModel->getCityIndexComparator();
}

CitySort DataModel::getCitySort() const { return mCityModel->getCitySort(); }

void DataModel::toggleCitySort() { mCityModel->toggleCitySort(); }

void DataModel::addCityListener(const CityListener& cityListener) {
    mCityModel->addCityListener(cityListener);
}

void DataModel::removeCityListener(const CityListener& cityListener) {
    mCityModel->removeCityListener(cityListener);
}

//
// Timers
//

void DataModel::addTimerListener(const TimerListener& timerListener) {
    mTimerModel->addTimerListener(timerListener);
}

void DataModel::removeTimerListener(const TimerListener& timerListener) {
    mTimerModel->removeTimerListener(timerListener);
}

const std::vector<Timer>& DataModel::getTimers() { return mTimerModel->getTimers(); }
const std::vector<Timer>& DataModel::getExpiredTimers() { return mTimerModel->getExpiredTimers(); }

bool DataModel::getTimer(int timerId, Timer& outTimer) {
    return mTimerModel->getTimer(timerId, outTimer);
}

bool DataModel::getMostRecentExpiredTimer(Timer& outTimer) {
    return mTimerModel->getMostRecentExpiredTimer(outTimer);
}

Timer DataModel::addTimer(int64_t length, const std::string& label, bool deleteAfterUse) {
    return mTimerModel->addTimer(length, label, deleteAfterUse);
}

void DataModel::removeTimer(const Timer& timer) { mTimerModel->removeTimer(timer); }

void DataModel::startTimer(const Timer& timer) { mTimerModel->updateTimer(timer.start()); }
void DataModel::pauseTimer(const Timer& timer) { mTimerModel->updateTimer(timer.pause()); }
void DataModel::expireTimer(const Timer& timer) { mTimerModel->expireTimer(timer); }

bool DataModel::resetTimer(const Timer& timer, Timer& outTimer) {
    return mTimerModel->resetTimer(timer, false /* allowDelete */, 0, outTimer);
}

bool DataModel::resetOrDeleteTimer(const Timer& timer, int eventLabelId, Timer& outTimer) {
    return mTimerModel->resetTimer(timer, true /* allowDelete */, eventLabelId, outTimer);
}

void DataModel::resetOrDeleteExpiredTimers(int eventLabelId) {
    mTimerModel->resetOrDeleteExpiredTimers(eventLabelId);
}

void DataModel::resetUnexpiredTimers(int eventLabelId) {
    mTimerModel->resetUnexpiredTimers(eventLabelId);
}

void DataModel::resetMissedTimers(int eventLabelId) {
    mTimerModel->resetMissedTimers(eventLabelId);
}

void DataModel::addTimerMinute(const Timer& timer) {
    mTimerModel->updateTimer(timer.addMinute());
}

void DataModel::setTimerLabel(const Timer& timer, const std::string& label) {
    mTimerModel->updateTimer(timer.setLabel(label));
}

void DataModel::setTimerLength(const Timer& timer, int64_t length) {
    mTimerModel->updateTimer(timer.setLength(length));
}

void DataModel::setRemainingTime(const Timer& timer, int64_t remainingTime) {
    mTimerModel->updateTimer(timer.setRemainingTime(remainingTime));
}

Uri* DataModel::getDefaultTimerRingtoneUri() const {
    return mTimerModel->getDefaultTimerRingtoneUri();
}

bool DataModel::isTimerRingtoneSilent() { return mTimerModel->isTimerRingtoneSilent(); }
Uri* DataModel::getTimerRingtoneUri() { return mTimerModel->getTimerRingtoneUri(); }
std::string DataModel::getTimerRingtoneTitle() { return mTimerModel->getTimerRingtoneTitle(); }
int64_t DataModel::getTimerCrescendoDuration() const { return mTimerModel->getTimerCrescendoDuration(); }
void DataModel::setTimerRingtoneUri(const Uri* uri) { mTimerModel->setTimerRingtoneUri(uri); }
bool DataModel::getTimerVibrate() const { return mTimerModel->getTimerVibrate(); }
void DataModel::setTimerVibrate(bool enabled) { mTimerModel->setTimerVibrate(enabled); }

//
// Stopwatch
//

void DataModel::addStopwatchListener(const StopwatchListener& stopwatchListener) {
    mStopwatchModel->addStopwatchListener(stopwatchListener);
}

void DataModel::removeStopwatchListener(const StopwatchListener& stopwatchListener) {
    mStopwatchModel->removeStopwatchListener(stopwatchListener);
}

const Stopwatch& DataModel::getStopwatch() { return mStopwatchModel->getStopwatch(); }

Stopwatch DataModel::startStopwatch() { return mStopwatchModel->setStopwatch(getStopwatch().start()); }
Stopwatch DataModel::pauseStopwatch() { return mStopwatchModel->setStopwatch(getStopwatch().pause()); }
Stopwatch DataModel::resetStopwatch() { return mStopwatchModel->setStopwatch(getStopwatch().reset()); }

const std::vector<Lap>& DataModel::getLaps() { return mStopwatchModel->getLaps(); }
bool DataModel::canAddMoreLaps() { return mStopwatchModel->canAddMoreLaps(); }
bool DataModel::addLap(Lap& outLap) { return mStopwatchModel->addLap(outLap); }
int64_t DataModel::getLongestLapTime() { return mStopwatchModel->getLongestLapTime(); }
int64_t DataModel::getCurrentLapTime(int64_t time) const {
    return mStopwatchModel->getCurrentLapTime(time);
}

//
// Time (TimeModel semantics)
//

int64_t DataModel::currentTimeMillis() const { return SystemClock::currentTimeMillis(); }
int64_t DataModel::elapsedRealtime() const { return SystemClock::elapsedRealtime(); }

bool DataModel::is24HourFormat() const {
    // DateFormat.is24HourFormat(mContext): the device 12/24h setting. CDROID has
    // no system setting; follow the locale convention.
    const Locale locale = Locale::getDefault();
    const std::string country = locale.getCountry();
    // Countries using 12-hour time by convention (subset of CLDR data).
    static const std::vector<std::string> k12Hour = {"US", "CA", "AU", "NZ", "IN", "JP",
                                                     "KR", "PH", "TH", "PK", "BD", "EG",
                                                     "SA", "MX", "CO", "MY", "NG"};
    for (const std::string& c : k12Hour) {
        if (country == c) return false;
    }
    return true;
}

std::unique_ptr<Calendar> DataModel::getCalendar() const {
    std::unique_ptr<Calendar> calendar = Calendar::getInstance();
    calendar->setTimeInMillis(currentTimeMillis());
    return calendar;
}

//
// Settings
//

ClockStyle DataModel::getClockStyle() const { return mSettingsModel->getClockStyle(); }
ClockStyle DataModel::getScreensaverClockStyle() const { return mSettingsModel->getScreensaverClockStyle(); }
bool DataModel::getDisplayClockSeconds() const { return mSettingsModel->getDisplayClockSeconds(); }
void DataModel::setDisplayClockSeconds(bool displaySeconds) {
    mSettingsModel->setDisplayClockSeconds(displaySeconds);
}
bool DataModel::getScreensaverNightModeOn() const {
    return mSettingsModel->getScreensaverNightModeOn();
}
TimeZone DataModel::getHomeTimeZone() const { return mSettingsModel->getHomeTimeZone(); }
bool DataModel::getShowHomeClock() const { return mSettingsModel->getShowHomeClock(); }
AlarmVolumeButtonBehavior DataModel::getAlarmVolumeButtonBehavior() const {
    return mSettingsModel->getAlarmVolumeButtonBehavior();
}
int DataModel::getAlarmTimeout() const { return mSettingsModel->getAlarmTimeout(); }
int DataModel::getSnoozeLength() const { return mSettingsModel->getSnoozeLength(); }
int64_t DataModel::getAlarmCrescendoDuration() const {
    return mSettingsModel->getAlarmCrescendoDuration();
}
Weekdays::Order::Value DataModel::getWeekdayOrder() const {
    return mSettingsModel->getWeekdayOrder();
}

//
// Notification flag
//

bool DataModel::isApplicationInForeground() const {
    return mNotificationModel->isApplicationInForeground();
}

void DataModel::setApplicationInForeground(bool inForeground) const {
    mNotificationModel->setApplicationInForeground(inForeground);
}

//
// Silent settings
//

void DataModel::addSilentSettingsListener(const OnSilentSettingsListener& listener) {
    mSilentSettingsListeners.push_back(listener);
}

void DataModel::removeSilentSettingsListener(const OnSilentSettingsListener& listener) {
    for (auto it = mSilentSettingsListeners.begin(); it != mSilentSettingsListeners.end(); ++it) {
        if (*it == listener) {
            mSilentSettingsListeners.erase(it);
            return;
        }
    }
}

} // namespace data
} // namespace deskclock
} // namespace cdroid
