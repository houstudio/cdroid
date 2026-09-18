#ifndef __DESKCLOCK_COLLAPSEDALARMVIEWHOLDER_H__
#define __DESKCLOCK_COLLAPSEDALARMVIEWHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.alarms.dataadapter.CollapsedAlarmViewHolder —
 * the collapsed alarm row. The collapse change from the expanded editor runs
 * the upstream staggered cross-fade (old editor fades out while the collapsed
 * row fades in); same-type rebinds stay unanimated.
 *********************************************************************************/
#include <alarmitemviewholder.h>
#include <itemanimator.h>

namespace cdroid {
namespace deskclock {
namespace alarms {

class CollapsedAlarmViewHolder : public AlarmItemViewHolder, public OnAnimateChangeListener {
private:
    TextView* alarmLabel;
    TextView* daysOfWeekView;
    TextView* upcomingInstanceLabel;
    View* hairLine;

public:
    explicit CollapsedAlarmViewHolder(View* itemView);

    // OnAnimateChangeListener: animate the collapse from the expanded editor.
    Animator* onAnimateChange(RecyclerView::ViewHolder& oldHolder,
                              RecyclerView::ViewHolder& newHolder, int64_t duration) override;
    Animator* onAnimateChange(std::vector<Object*>* payloads, int fromLeft, int fromTop,
                              int fromRight, int fromBottom, int64_t duration) override;

protected:
    void onBindItemView(ItemHolder& itemHolder) override;

private:
    void bindReadOnlyLabel(Context& context, const data::Alarm& alarm);
    void bindRepeatText(Context& context, const data::Alarm& alarm);
    void bindUpcomingInstance(Context& context, const data::Alarm& alarm);

    Animator* createExpandingAnimator(AlarmItemViewHolder& newHolder, int64_t duration);
    Animator* createCollapsingAnimator(AlarmItemViewHolder& oldHolder, int64_t duration);
    void setChangingViewsAlpha(float alpha);

public:
    /** ItemViewHolder.Factory for the collapsed rows. */
    static ItemViewHolder* createViewHolder(LayoutInflater* inflater, ViewGroup* parent,
                                            int viewType);
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_COLLAPSEDALARMVIEWHOLDER_H__
