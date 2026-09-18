#ifndef __DESKCLOCK_UTILS_H__
#define __DESKCLOCK_UTILS_H__
/*********************************************************************************
 * Port of com.android.deskclock.Utils — formatting/pattern helpers shared by
 * the clock screens. The DataModel-backed clock-style helpers
 * (setClockStyle/setScreensaverClockStyle/setClockSecondsEnabled/now/wallClock)
 * live with the data package port; notification/widget helpers are cut.
 *********************************************************************************/
#include <string>
#include <vector>

#include <text/charsequence.h>

namespace cdroid {

class Context;
class TextClock;
class View;
class TextView;

namespace deskclock {
class AnalogClock;
}

namespace deskclock {
namespace Utils {

// Build.VERSION gates — cdroid targets a modern API level; all post-L gates true.
constexpr bool isPreL = false;
constexpr bool isLOrLMR1 = true;
constexpr bool isLOrLater = true;
constexpr bool isLMR1OrLater = true;
constexpr bool isMOrLater = true;
constexpr bool isNOrLater = true;
constexpr bool isNMR1OrLater = true;
constexpr bool isOOrLater = true;

void enforceMainLooper();
void enforceNotMainLooper();

int indexOf(const std::vector<void*>& array, const void* item);

bool isScrolledToTop(View& view);

/** @return the amount by which a CircleTimerView radius is offset by painted objects. */
float calculateRadiusOffset(float strokeSize, float dotStrokeSize, float markerStrokeSize);

// TODO(alarms): getNextAlarm reads the in-process alarm schedule once the
// AlarmStateManager port lands; returns "" until then.
std::string getNextAlarm(Context& context);

/** Updates the next alarm textView + icon in the given clock view. */
void refreshAlarm(Context& context, View* clock);
// setClockIconTypeface (UiDataModel.alarmIconTypeface) lands with the uidata port.

void updateDate(const std::string& dateSkeleton, const std::string& descriptionSkeleton, View* clock);

/**
 * Formats the time in the TextClock according to the Locale with a special
 * formatting treatment for the am/pm label.
 */
void setTimeFormat(TextClock* clock, bool includeSeconds);

/**
 * @return format CharSequence for 12 hours mode, with am/pm spans applied.
 * (The cdroid TextClock takes a plain string format, so the am/pm spans are
 * dropped when fed to it; kept faithful for span-aware consumers.)
 */
CharSequence* get12ModeFormat(float amPmRatio, bool includeSeconds);

std::string get24ModeFormat(bool includeSeconds);

/** Returns string denoting the timezone hour offset (e.g. GMT -8:00). */
std::string getGMTHourOffset(int rawOffsetMillis, bool useShortForm);

/**
 * Given a point in time, return the subsequent moment any of the zones changes
 * days (the nearest midnight across zones), in millis since the epoch.
 */
int64_t getNextDay(int64_t timeMillis, const std::vector<int>& zoneRawOffsetMillis);

std::string getNumberFormattedQuantityString(Context& context, int id, int quantity);

bool isPortrait(Context& context);
bool isLandscape(Context& context);

/** Configures the digital/analog clock pair; @return the visible view. */
View* setClockStyle(View& digitalClock, View& analogClock);

/** For screensavers to set whether the digital or analog clock should display. */
View* setScreensaverClockStyle(View& digitalClock, View& analogClock);

/** Configures the visible clock to display seconds; invisible one never ticks. */
void setClockSecondsEnabled(TextClock& digitalClock, AnalogClock& analogClock);

/** Set the icon font on the next-alarm icon TextView. */
void setClockIconTypeface(View* clock);

/** Formats "N hours ahead/behind" style difference strings. */
std::string createHoursDifferentString(Context& context, bool displayMinutes, bool isAhead,
                                       int hoursDifferent, int minutesDifferent);

/** Formats hours/minutes/seconds for the stopwatch main text. */
std::string getTimeString(Context& context, int hours, int minutes, int seconds);

/**
 * Paints the theme's colorBackground on a fragment root that has no background.
 * The pager keeps sibling pages attached, and without an opaque base layer the
 * window's default SRC_OVER compositing smears the overlapping pages together.
 */
void setDefaultBackground(View* root);

/** DataModel.elapsedRealtime() — milliseconds since boot, including sleep. */
int64_t now();

/** DataModel.currentTimeMillis() — the wall clock time in milliseconds. */
int64_t wallClock();

} // namespace Utils
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_UTILS_H__
