#ifndef __DESKCLOCK_ALARMFRAGMENT_H__
#define __DESKCLOCK_ALARMFRAGMENT_H__
/*********************************************************************************
 * Port of com.android.deskclock.AlarmClockFragment — the ALARMS tab: the
 * RecyclerView of collapsed/expanded alarm rows. Upstream loads via CursorLoader
 * + ALARMS_WITH_INSTANCES join; cdroid reads the prefs-backed AlarmDAO on
 * lifecycle callbacks and after mutations (notifications from the update handler
 * drive reloads through the DataModel-independent reloadAlarms()).
 *********************************************************************************/
#include <widget/button.h>
#include <widget/imageview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/recyclerview.h>

#include <alarm.h>
#include <alarmitemholder.h>
#include <alarmtimeclickhandler.h>
#include <alarmupdatehandler.h>
#include <deskclockfragment.h>
#include <itemadapter.h>
#include <timepickerdialogfragment.h>

namespace cdroid {
namespace deskclock {

class AlarmClockFragment : public DeskClockFragment,
                           public alarms::ScrollHandler,
                           public alarms::TimePickerDialogFragment::OnTimeSetListener {
private:
    // Updates "Today/Tomorrow" in the UI when midnight passes.
    Runnable mMidnightUpdater;

    // Views
    ViewGroup* mMainLayout = nullptr;
    RecyclerView* mRecyclerView = nullptr;
    TextView* mEmptyView = nullptr;

    // Data
    int64_t mScrollToAlarmId = data::Alarm::INVALID_ID;
    int64_t mExpandedAlarmId = data::Alarm::INVALID_ID;

    // Controllers
    ItemAdapter<alarms::AlarmItemHolder>* mItemAdapter = nullptr;
    alarms::AlarmUpdateHandler* mAlarmUpdateHandler = nullptr;
    alarms::AlarmTimeClickHandler* mAlarmTimeClickHandler = nullptr;
    LinearLayoutManager* mLayoutManager = nullptr;

    alarms::AlarmItemHolder* mRemovingHolder = nullptr;

public:
    AlarmClockFragment();
    ~AlarmClockFragment() override;

    View* onCreateView(LayoutInflater* inflater, ViewGroup* container,
                       Bundle* savedInstanceState) override;
    void onCreate(Bundle* savedInstanceState) override;
    void onStart() override;
    void onResume() override;
    void onPause() override;
    void onSaveInstanceState(Bundle* outState) override;

    void setLabel(const data::Alarm& alarm, const std::string& label);
    void removeItem(alarms::AlarmItemHolder* itemHolder);

    // ScrollHandler
    void smoothScrollTo(int position) override;
    void setSmoothScrollStableId(int64_t stableId) override;

    // TimePickerDialogFragment.OnTimeSetListener
    void onTimeSet(alarms::TimePickerDialogFragment* fragment, int hourOfDay,
                   int minute) override;

    // DeskClockFragment fab contract
    void onUpdateFab(ImageView& fab) override;
    void onFabClick(ImageView& fab) override;
    void onUpdateFabButtons(Button& left, Button& right) override;

    /** Re-reads the alarm list and refreshes the adapter (CursorLoader reload). */
    void reloadAlarms();

private:
    void setAdapterItems(std::vector<alarms::AlarmItemHolder*>* items);
    void scrollToAlarm(int64_t alarmId);
    void startCreatingAlarm();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ALARMFRAGMENT_H__
