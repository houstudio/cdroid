#ifndef __DESKCLOCK_EXPANDEDALARMVIEWHOLDER_H__
#define __DESKCLOCK_EXPANDEDALARMVIEWHOLDER_H__
/*********************************************************************************
 * Port of com.android.deskclock.alarms.dataadapter.ExpandedAlarmViewHolder —
 * the expanded alarm editor row (repeat days grid, label, ringtone, vibrate,
 * delete). The expand change from a collapsed row runs the upstream staggered
 * cross-fade (collapsed row fades out while the editor fades in).
 *********************************************************************************/
#include <alarmitemviewholder.h>
#include <itemanimator.h>

#include <widget/checkbox.h>
#include <widget/linearlayout.h>

namespace cdroid {
namespace deskclock {
namespace alarms {

class ExpandedAlarmViewHolder : public AlarmItemViewHolder, public OnAnimateChangeListener {
private:
    CheckBox* repeat;
    TextView* editLabel;
    LinearLayout* repeatDays;
    CompoundButton* dayButtons[7] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                     nullptr};
    CheckBox* vibrateCheckBox;
    TextView* ringtone;
    TextView* deleteView;
    View* hairLine;

public:
    explicit ExpandedAlarmViewHolder(View* itemView);

    // OnAnimateChangeListener: animate the expand from a collapsed row.
    Animator* onAnimateChange(RecyclerView::ViewHolder& oldHolder,
                              RecyclerView::ViewHolder& newHolder, int64_t duration) override;
    Animator* onAnimateChange(std::vector<Object*>* payloads, int fromLeft, int fromTop,
                              int fromRight, int fromBottom, int64_t duration) override;

protected:
    void onBindItemView(ItemHolder& itemHolder) override;

private:
    void bindRingtone(Context& context, const data::Alarm& alarm);
    void bindDaysOfWeekButtons(const data::Alarm& alarm, Context& context);

    Animator* createExpandingAnimator(AlarmItemViewHolder& oldHolder, int64_t duration);
    Animator* createCollapsingAnimator(AlarmItemViewHolder& newHolder, int64_t duration);
    void setTranslationY(float repeatDaysTranslationY, float translationY);
    void setChangingViewsAlpha(float alpha);
    int countNumberOfItems() const;
    void bindEditLabel(Context& context, const data::Alarm& alarm);
    void bindVibrator(const data::Alarm& alarm);

public:
    static ItemViewHolder* createViewHolder(LayoutInflater* inflater, ViewGroup* parent,
                                            int viewType);
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_EXPANDEDALARMVIEWHOLDER_H__
