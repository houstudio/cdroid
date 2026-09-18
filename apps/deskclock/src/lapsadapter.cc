#include <lapsadapter.h>

#include <R.h>

#include <algorithm>

#include <view/layoutinflater.h>
#include <widget/textview.h>

#include <datamodel.h>
#include <uidata.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace stopwatch {

using data::DataModel;
using uidata::UiDataModel;

namespace {
constexpr int64_t TEN_MINUTES = 10LL * 60 * 1000;
constexpr int64_t HOUR = 60LL * 60 * 1000;
constexpr int64_t TEN_HOURS = 10 * HOUR;
constexpr int64_t HUNDRED_HOURS = 100 * HOUR;
const std::string LRM_SPACE = "\xE2\x80\x8E "; // U+200E LRM + space
} // namespace

LapsAdapter::LapItemHolder::LapItemHolder(View* itemView) : RecyclerView::ViewHolder(itemView) {
    lapTime = (TextView*) itemView->findViewById(R::id::lap_time);
    lapNumber = (TextView*) itemView->findViewById(R::id::lap_number);
    accumulatedTime = (TextView*) itemView->findViewById(R::id::lap_total);
}

LapsAdapter::LapsAdapter(Context& context)
    : mContext(context), mInflater(LayoutInflater::from(&context)) {
    setHasStableIds(true);
}

int LapsAdapter::getItemCount() {
    const int lapCount = (int) DataModel::getDataModel().getLaps().size();
    const int currentLapCount = (lapCount == 0) ? 0 : 1;
    return currentLapCount + lapCount;
}

RecyclerView::ViewHolder* LapsAdapter::onCreateViewHolder(ViewGroup* parent, int /*viewType*/) {
    View* v = mInflater->inflate(R::layout::lap_view, parent, false /* attachToRoot */);
    return new LapItemHolder(v);
}

void LapsAdapter::onBindViewHolder(RecyclerView::ViewHolder& viewHolder, int position) {
    int64_t lapTime;
    int lapNumber;
    int64_t totalTime;

    // Lap will be null for the current lap.
    const std::vector<data::Lap>& laps = DataModel::getDataModel().getLaps();
    const bool isCurrentLap = (position == 0);
    if (!isCurrentLap) {
        // For a recorded lap, merely extract the values to format.
        const data::Lap& lap = laps[position - 1];
        lapTime = lap.lapTime;
        lapNumber = lap.lapNumber;
        totalTime = lap.accumulatedTime;
    } else {
        // For the current lap, compute times relative to the stopwatch.
        totalTime = DataModel::getDataModel().getStopwatch().getTotalTime();
        lapTime = DataModel::getDataModel().getCurrentLapTime(totalTime);
        lapNumber = (int) laps.size() + 1;
    }

    // Bind data into the child views.
    LapItemHolder& holder = static_cast<LapItemHolder&>(viewHolder);
    holder.lapTime->setText(formatLapTime(lapTime, true));
    holder.accumulatedTime->setText(formatAccumulatedTime(totalTime, true));
    holder.lapNumber->setText(formatLapNumber((int) laps.size() + 1, lapNumber));
}

long LapsAdapter::getItemId(int position) {
    const std::vector<data::Lap>& laps = DataModel::getDataModel().getLaps();
    if (position == 0) {
        return (long) laps.size() + 1;
    }
    return laps[position - 1].lapNumber;
}

void LapsAdapter::updateCurrentLap(RecyclerView& rv, int64_t totalTime) {
    // If no laps exist there is nothing to do.
    if (getItemCount() == 0) {
        return;
    }

    View* currentLapView = rv.getChildAt(0);
    if (currentLapView != nullptr) {
        // Compute the lap time using the total time.
        const int64_t lapTime = DataModel::getDataModel().getCurrentLapTime(totalTime);
        if (RecyclerView::ViewHolder* holder = rv.getChildViewHolder(currentLapView)) {
            LapItemHolder& lapHolder = static_cast<LapItemHolder&>(*holder);
            lapHolder.lapTime->setText(formatLapTime(lapTime, false));
            lapHolder.accumulatedTime->setText(formatAccumulatedTime(totalTime, false));
        }
    }
}

bool LapsAdapter::addLap(data::Lap& outLap) {
    bool added = DataModel::getDataModel().addLap(outLap);

    if (added && getItemCount() == 10) {
        // 10 total laps indicates all items switch from 1 to 2 digit lap numbers.
        notifyDataSetChanged();
    } else if (added) {
        // New current lap now exists.
        notifyItemInserted(0);

        // Prior current lap must be refreshed once with the true values in place.
        notifyItemChanged(1);
    }

    return added;
}

void LapsAdapter::clearLaps() {
    // Clear the computed time lengths related to the old recorded laps.
    mLastFormattedLapTimeLength = 0;
    mLastFormattedAccumulatedTimeLength = 0;

    notifyDataSetChanged();
}

std::string LapsAdapter::getShareText() {
    const data::Stopwatch& stopwatch = DataModel::getDataModel().getStopwatch();
    const int64_t totalTime = stopwatch.getTotalTime();
    const std::string stopwatchTime = formatTime(totalTime, totalTime, ":");

    std::string builder;

    // Add the total elapsed time of the stopwatch.
    char header[128];
    const std::string fmt = mContext.getString(R::string::sw_share_main);
    snprintf(header, sizeof(header), fmt.c_str(), stopwatchTime.c_str());
    builder.append(header);
    builder.append("\n");

    const std::vector<data::Lap>& laps = DataModel::getDataModel().getLaps();
    if (!laps.empty()) {
        // Add a header for lap times.
        builder.append(mContext.getString(R::string::sw_share_laps));
        builder.append("\n");

        // Loop through the laps in the order they were recorded; reverse of display order.
        const std::string separator = ". "; // DecimalFormatSymbols decimal separator
        for (int i = (int) laps.size() - 1; i >= 0; i--) {
            const data::Lap& lap = laps[i];
            builder.append(std::to_string(lap.lapNumber));
            builder.append(separator);
            builder.append(formatTime(lap.lapTime, lap.lapTime, " "));
            builder.append("\n");
        }

        // Append the final lap
        builder.append(std::to_string(laps.size() + 1));
        builder.append(separator);
        const int64_t lapTime = DataModel::getDataModel().getCurrentLapTime(totalTime);
        builder.append(formatTime(lapTime, lapTime, " "));
        builder.append("\n");
    }
    return builder;
}

std::string LapsAdapter::formatLapNumber(int lapCount, int lapNumber) {
    char buf[32];
    if (lapCount < 10) {
        const std::string fmt = mContext.getString(R::string::lap_number_single_digit);
        snprintf(buf, sizeof(buf), fmt.c_str(), lapNumber);
    } else {
        const std::string fmt = mContext.getString(R::string::lap_number_double_digit);
        snprintf(buf, sizeof(buf), fmt.c_str(), lapNumber);
    }
    return buf;
}

std::string LapsAdapter::formatLapTime(int64_t lapTime, bool isBinding) {
    // The longest lap dictates the way the given lapTime must be formatted.
    const int64_t longestLapTime = std::max(DataModel::getDataModel().getLongestLapTime(), lapTime);
    const std::string formattedTime = formatTime(longestLapTime, lapTime, LRM_SPACE);

    // If the newly formatted lap time has altered the format, refresh all laps.
    const int newLength = (int) formattedTime.size();
    if (!isBinding && mLastFormattedLapTimeLength != newLength) {
        mLastFormattedLapTimeLength = newLength;
        notifyDataSetChanged();
    }

    return formattedTime;
}

std::string LapsAdapter::formatAccumulatedTime(int64_t accumulatedTime, bool isBinding) {
    const int64_t totalTime = DataModel::getDataModel().getStopwatch().getTotalTime();
    const int64_t longestAccumulatedTime = std::max(totalTime, accumulatedTime);
    const std::string formattedTime = formatTime(longestAccumulatedTime, accumulatedTime, LRM_SPACE);

    // If the newly formatted accumulated time has altered the format, refresh all laps.
    const int newLength = (int) formattedTime.size();
    if (!isBinding && mLastFormattedAccumulatedTimeLength != newLength) {
        mLastFormattedAccumulatedTimeLength = newLength;
        notifyDataSetChanged();
    }

    return formattedTime;
}

std::string LapsAdapter::formatTime(int64_t maxTime, int64_t time, const std::string& separator) {
    int hours;
    int minutes;
    int seconds;
    int hundredths;
    if (time <= 0) {
        // A negative time should be impossible, but is tolerated to avoid crashing the app.
        hundredths = 0;
        seconds = hundredths;
        minutes = seconds;
        hours = minutes;
    } else {
        hours = (int) (time / HOUR);
        int64_t remainder = time % HOUR;
        minutes = (int) (remainder / (60 * 1000));
        remainder = remainder % (60 * 1000);
        seconds = (int) (remainder / 1000);
        remainder = remainder % 1000;
        hundredths = (int) (remainder / 10);
    }

    const char decimalSeparator = '.';
    std::string builder;

    // The display of hours and minutes varies based on maxTime.
    if (maxTime < TEN_MINUTES) {
        builder.append(UiDataModel::getUiDataModel().getFormattedNumber(minutes, 1));
    } else if (maxTime < HOUR) {
        builder.append(UiDataModel::getUiDataModel().getFormattedNumber(minutes, 2));
    } else if (maxTime < TEN_HOURS) {
        builder.append(UiDataModel::getUiDataModel().getFormattedNumber(hours, 1));
        builder.append(separator);
        builder.append(UiDataModel::getUiDataModel().getFormattedNumber(minutes, 2));
    } else if (maxTime < HUNDRED_HOURS) {
        builder.append(UiDataModel::getUiDataModel().getFormattedNumber(hours, 2));
        builder.append(separator);
        builder.append(UiDataModel::getUiDataModel().getFormattedNumber(minutes, 2));
    } else {
        builder.append(UiDataModel::getUiDataModel().getFormattedNumber(hours, 3));
        builder.append(separator);
        builder.append(UiDataModel::getUiDataModel().getFormattedNumber(minutes, 2));
    }

    // The display of seconds and hundredths-of-a-second is constant.
    builder.append(separator);
    builder.append(UiDataModel::getUiDataModel().getFormattedNumber(seconds, 2));
    builder.push_back(decimalSeparator);
    builder.append(UiDataModel::getUiDataModel().getFormattedNumber(hundredths, 2));

    return builder;
}

} // namespace stopwatch
} // namespace deskclock
} // namespace cdroid
