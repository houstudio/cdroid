#ifndef __DESKCLOCK_LAPSADAPTER_H__
#define __DESKCLOCK_LAPSADAPTER_H__
/*********************************************************************************
 * Port of com.android.deskclock.stopwatch.LapsAdapter — the laps list (current
 * lap in slot 0, recorded laps newest-first) with format-stability refreshes.
 *********************************************************************************/
#include <string>
#include <vector>

#include <widget/textview.h>
#include <widgetEx/recyclerview/recyclerview.h>

#include <stopwatch.h>

namespace cdroid {

class Context;

namespace deskclock {
namespace stopwatch {

class LapsAdapter : public RecyclerView::Adapter {
private:
    LayoutInflater* mInflater;
    Context& mContext;

    int mLastFormattedLapTimeLength = 0;
    int mLastFormattedAccumulatedTimeLength = 0;

public:
    class LapItemHolder : public RecyclerView::ViewHolder {
    public:
        TextView* lapNumber;
        TextView* lapTime;
        TextView* accumulatedTime;

        explicit LapItemHolder(View* itemView);
    };

    explicit LapsAdapter(Context& context);

    int getItemCount() override;
    RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override;
    void onBindViewHolder(RecyclerView::ViewHolder& viewHolder, int position) override;
    long getItemId(int position) override;

    /** Updates the current-lap row in place with the given total time. */
    void updateCurrentLap(RecyclerView& rv, int64_t totalTime);

    /** @return the newly added lap, or false if no lap can be added. */
    bool addLap(data::Lap& outLap);

    void clearLaps();

    /** @return the text describing the stopwatch to share with other apps. */
    std::string getShareText();

    std::string formatLapNumber(int lapCount, int lapNumber);

    static std::string formatTime(int64_t maxTime, int64_t time, const std::string& separator);

private:
    std::string formatLapTime(int64_t lapTime, bool isBinding);
    std::string formatAccumulatedTime(int64_t accumulatedTime, bool isBinding);
};

} // namespace stopwatch
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_LAPSADAPTER_H__
