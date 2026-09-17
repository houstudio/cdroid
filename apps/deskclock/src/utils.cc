#include <utils.h>

#include <R.h>

using namespace ::deskclock; // generated R.h namespace

#include <algorithm>
#include <cstdint>   // INT64_MAX
#include <cstdio>

#include <drawable/colordrawable.h>
#include <widget/internal_R.h>

#include <core/calendar.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include <core/typeface.h>

#include <analogclock.h>
#include <datamodel.h>
#include <content/Locale.h>
#include <content/dateformat.h>
#include <content/numberformat.h>
#include <content/resources.h>
#include <content/simpledateformat.h>
#include <core/context.h>
#include <text/String.h>
#include <text/spannablestring.h>
#include <text/style/metricaffectingspan.h>
#include <text/textutils.h>
#include <view/view.h>
#include <widget/textclock.h>
#include <widget/textview.h>

namespace cdroid {
namespace deskclock {
namespace Utils {

namespace {
constexpr int64_t HOUR_IN_MILLIS = 60LL * 60 * 1000;
constexpr int64_t MINUTE_IN_MILLIS = 60LL * 1000;
} // namespace

void enforceMainLooper() {
    if (Looper::myLooper() != nullptr && Looper::myLooper() != Looper::getMainLooper()) {
        throw std::runtime_error("Must be called from the main looper thread");
    }
}

void enforceNotMainLooper() {
    if (Looper::myLooper() != nullptr && Looper::myLooper() == Looper::getMainLooper()) {
        throw std::runtime_error("Must be called from a non-main looper thread");
    }
}

int indexOf(const std::vector<void*>& array, const void* item) {
    const int size = (int) array.size();
    for (int i = 0; i < size; i++) {
        if (array[i] == item) return i;
    }
    return -1; // Arrays.asList(...).indexOf fallback
}

bool isScrolledToTop(View& view) {
    return !view.canScrollVertically(-1);
}

float calculateRadiusOffset(float strokeSize, float dotStrokeSize, float markerStrokeSize) {
    return std::max(strokeSize, std::max(dotStrokeSize, markerStrokeSize));
}

std::string getNextAlarm(Context& /*context*/) {
    // TODO(alarms): surface the next in-process alarm (AlarmStateManager port);
    // no system AlarmManager on cdroid.
    return std::string();
}

void refreshAlarm(Context& context, View* clock) {
    if (clock == nullptr) return;
    TextView* nextAlarmIconView = (TextView*) clock->findViewById(R::id::nextAlarmIcon);
    TextView* nextAlarmView = (TextView*) clock->findViewById(R::id::nextAlarm);
    if (nextAlarmView == nullptr) return;

    const std::string alarm = getNextAlarm(context);
    if (!alarm.empty()) {
        char description[256];
        snprintf(description, sizeof(description),
                 context.getString(R::string::next_alarm_description).c_str(), alarm.c_str());
        nextAlarmView->setText(alarm);
        nextAlarmView->setContentDescription(description);
        nextAlarmView->setVisibility(View::VISIBLE);
        if (nextAlarmIconView) {
            nextAlarmIconView->setVisibility(View::VISIBLE);
            nextAlarmIconView->setContentDescription(description);
        }
    } else {
        nextAlarmView->setVisibility(View::GONE);
        if (nextAlarmIconView) nextAlarmIconView->setVisibility(View::GONE);
    }
}

void updateDate(const std::string& dateSkeleton, const std::string& descriptionSkeleton, View* clock) {
    if (clock == nullptr) return;
    TextView* dateDisplay = (TextView*) clock->findViewById(R::id::date);
    if (dateDisplay == nullptr) return;

    const Locale l = Locale::getDefault();
    const std::string datePattern = DateFormat::getBestDateTimePattern(l, dateSkeleton);
    const std::string descriptionPattern = DateFormat::getBestDateTimePattern(l, descriptionSkeleton);

    const int64_t now = SystemClock::currentTimeMillis();
    SimpleDateFormat dateFmt(datePattern, l);
    dateDisplay->setText(dateFmt.format(now));
    dateDisplay->setVisibility(View::VISIBLE);
    SimpleDateFormat descFmt(descriptionPattern, l);
    dateDisplay->setContentDescription(descFmt.format(now));
}

namespace {

/** get12ModeFormat's pattern pipeline without the span application. */
std::string get12ModePattern(float amPmRatio, bool includeSeconds) {
    std::string pattern = DateFormat::getBestDateTimePattern(Locale::getDefault(),
            includeSeconds ? "hmsa" : "hma");
    if (amPmRatio <= 0) {
        // Remove the am/pm part.
        size_t pos;
        while ((pos = pattern.find('a')) != std::string::npos) pattern.erase(pos, 1);
    }

    // Replace spaces with "Hair Space" (U+200A, 3 UTF-8 bytes).
    for (size_t pos = pattern.find(' '); pos != std::string::npos; pos = pattern.find(' ', pos + 3)) {
        pattern.replace(pos, 1, "\xE2\x80\x8A");
    }
    return pattern;
}

} // namespace

void setTimeFormat(TextClock* clock, bool includeSeconds) {
    if (clock == nullptr) return;
    // Get the best format for 12 hours mode according to the locale.
    // cdroid's TextClock takes a plain string format; the am/pm spans built by
    // get12ModeFormat are dropped there (kept faithful for span-aware callers).
    clock->setFormat12Hour(get12ModePattern(0.4f, includeSeconds));
    // Get the best format for 24 hours mode according to the locale.
    clock->setFormat24Hour(get24ModeFormat(includeSeconds));
}


CharSequence* get12ModeFormat(float amPmRatio, bool includeSeconds) {
    const std::string pattern = get12ModePattern(amPmRatio, includeSeconds);

    // Build a spannable so that the am/pm will be formatted
    const size_t amPmPos = pattern.find('a');
    if (amPmPos == std::string::npos) {
        return new String(pattern);
    }

    SpannableString* sp = new SpannableString(TextUtils::utf8_utf16(pattern));
    sp->setSpan(new RelativeSizeSpan(amPmRatio), (int) amPmPos, (int) amPmPos + 1,
                Spannable::SPAN_EXCLUSIVE_EXCLUSIVE);
    sp->setSpan(new StyleSpan(Typeface::NORMAL), (int) amPmPos, (int) amPmPos + 1,
                Spannable::SPAN_EXCLUSIVE_EXCLUSIVE);
    sp->setSpan(new TypefaceSpan("sans-serif"), (int) amPmPos, (int) amPmPos + 1,
                Spannable::SPAN_EXCLUSIVE_EXCLUSIVE);
    return sp;
}

std::string get24ModeFormat(bool includeSeconds) {
    return DateFormat::getBestDateTimePattern(Locale::getDefault(),
            includeSeconds ? "Hms" : "Hm");
}

std::string getGMTHourOffset(int rawOffsetMillis, bool useShortForm) {
    const long hour = rawOffsetMillis / HOUR_IN_MILLIS;
    const long min = labs(rawOffsetMillis) % HOUR_IN_MILLIS / MINUTE_IN_MILLIS;

    char buf[32];
    if (useShortForm) {
        snprintf(buf, sizeof(buf), "%+ld", hour);
    } else {
        snprintf(buf, sizeof(buf), "GMT %+ld:%02ld", hour, min);
    }
    return buf;
}

int64_t getNextDay(int64_t timeMillis, const std::vector<int>& zoneRawOffsetMillis) {
    int64_t next = INT64_MAX;
    for (int rawOffset : zoneRawOffsetMillis) {
        std::unique_ptr<Calendar> c = Calendar::getInstance();
        c->setTimeZone(rawOffset / 1000);
        c->setTimeInMillis(timeMillis);

        // Advance to the next day.
        c->add(Calendar::DAY_OF_YEAR, 1);

        // Reset the time to midnight.
        c->set(Calendar::HOUR_OF_DAY, 0);
        c->set(Calendar::MINUTE, 0);
        c->set(Calendar::SECOND, 0);
        c->set(Calendar::MILLISECOND, 0);

        const int64_t t = c->getTimeInMillis();
        if (t < next) next = t;
    }
    return next;
}

std::string getNumberFormattedQuantityString(Context& context, int id, int quantity) {
    const std::string localizedQuantity = NumberFormat::getInstance()->format((int32_t) quantity);
    return context.getResources().getQuantityString(id, quantity, {localizedQuantity});
}

bool isPortrait(Context& context) {
    return context.getResources().getConfiguration().orientation
            == Configuration::ORIENTATION_PORTRAIT;
}

View* setClockStyle(View& digitalClock, View& analogClock) {
    if (data::DataModel::getDataModel().getClockStyle() == data::ClockStyle::ANALOG) {
        digitalClock.setVisibility(View::GONE);
        analogClock.setVisibility(View::VISIBLE);
        return &analogClock;
    }
    digitalClock.setVisibility(View::VISIBLE);
    analogClock.setVisibility(View::GONE);
    return &digitalClock;
}

View* setScreensaverClockStyle(View& digitalClock, View& analogClock) {
    if (data::DataModel::getDataModel().getScreensaverClockStyle() == data::ClockStyle::ANALOG) {
        digitalClock.setVisibility(View::GONE);
        analogClock.setVisibility(View::VISIBLE);
        return &analogClock;
    }
    digitalClock.setVisibility(View::VISIBLE);
    analogClock.setVisibility(View::GONE);
    return &digitalClock;
}

void setClockSecondsEnabled(TextClock& digitalClock, AnalogClock& analogClock) {
    const bool displaySeconds = data::DataModel::getDataModel().getDisplayClockSeconds();

    // The clock that is visible displays seconds. The clock that is not visible never
    // displays seconds to avoid it scheduling unnecessary ticking runnables.
    if (setClockStyle(digitalClock, analogClock) == &digitalClock) {
        analogClock.enableSeconds(false);
        setTimeFormat(&digitalClock, displaySeconds);
    } else {
        analogClock.enableSeconds(displaySeconds);
        setTimeFormat(&digitalClock, false);
    }
}

void setClockIconTypeface(View* clock) {
    if (clock == nullptr) return;
    if (TextView* icon = (TextView*) clock->findViewById(R::id::nextAlarmIcon)) {
        // UiDataModel.alarmIconTypeface (fonts/clock.ttf) is not shipped in the
        // app pak; the icon glyph falls back to the default typeface.
        icon->setTypeface(Typeface::create("sans-serif", Typeface::NORMAL));
    }
}

std::string createHoursDifferentString(Context& context, bool displayMinutes, bool isAhead,
                                       int hoursDifferent, int minutesDifferent) {
    std::string timeString;
    if (displayMinutes && hoursDifferent != 0) {
        // Both minutes and hours
        const std::string hoursShortQuantityString = getNumberFormattedQuantityString(
                context, R::plurals::hours_short, abs(hoursDifferent));
        const std::string minsShortQuantityString = getNumberFormattedQuantityString(
                context, R::plurals::minutes_short, abs(minutesDifferent));
        const int stringType = isAhead ? R::string::world_hours_minutes_ahead
                                       : R::string::world_hours_minutes_behind;
        char buf[128];
        const std::string fmt = context.getString(stringType);
        snprintf(buf, sizeof(buf), fmt.c_str(), hoursShortQuantityString.c_str(),
                 minsShortQuantityString.c_str());
        timeString = buf;
    } else {
        // Minutes alone or hours alone
        const std::string hoursQuantityString = getNumberFormattedQuantityString(
                context, R::plurals::hours, abs(hoursDifferent));
        const std::string minutesQuantityString = getNumberFormattedQuantityString(
                context, R::plurals::minutes, abs(minutesDifferent));
        const int stringType = isAhead ? R::string::world_time_ahead
                                       : R::string::world_time_behind;
        char buf[96];
        const std::string fmt = context.getString(stringType);
        if (displayMinutes) {
            snprintf(buf, sizeof(buf), fmt.c_str(), minutesQuantityString.c_str());
        } else {
            snprintf(buf, sizeof(buf), fmt.c_str(), hoursQuantityString.c_str());
        }
        timeString = buf;
    }
    return timeString;
}

std::string getTimeString(Context& context, int hours, int minutes, int seconds) {
    char buf[96];
    if (hours != 0) {
        const std::string fmt = context.getString(R::string::hours_minutes_seconds);
        snprintf(buf, sizeof(buf), fmt.c_str(), hours, minutes, seconds);
    } else if (minutes != 0) {
        const std::string fmt = context.getString(R::string::minutes_seconds);
        snprintf(buf, sizeof(buf), fmt.c_str(), minutes, seconds);
    } else {
        const std::string fmt = context.getString(R::string::seconds);
        snprintf(buf, sizeof(buf), fmt.c_str(), seconds);
    }
    return buf;
}

int64_t now() {
    // DataModel.dataModel.elapsedRealtime() delegates to SystemClock.elapsedRealtime()
    // (TimeModel is a pass-through); calling it directly avoids the model dependency cycle.
    return SystemClock::elapsedRealtime();
}

int64_t wallClock() {
    // DataModel.dataModel.currentTimeMillis() -> System.currentTimeMillis().
    return SystemClock::currentTimeMillis();
}

bool isLandscape(Context& context) {
    return context.getResources().getConfiguration().orientation
            == Configuration::ORIENTATION_LANDSCAPE;
}


void setDefaultBackground(View* root) {
    if (root == nullptr || root->getBackground() != nullptr) return;
    Context& context = *root->getContext();
    TypedValue value;
    if (context.getTheme().resolveAttribute(0x01010031 /* android:colorBackground */,
                &value, true)) {
        int color = value.data;
        if (value.resourceId != 0) color = context.getColor(value.resourceId);
        root->setBackground(new ColorDrawable(color));
    }
}

} // namespace Utils
} // namespace deskclock
} // namespace cdroid
