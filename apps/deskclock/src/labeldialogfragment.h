#ifndef __DESKCLOCK_LABELDIALOGFRAGMENT_H__
#define __DESKCLOCK_LABELDIALOGFRAGMENT_H__
/*********************************************************************************
 * Port of com.android.deskclock.LabelDialogFragment — DialogFragment that edits
 * the label of a Timer (the Alarm overload lands with the alarms module #10).
 *********************************************************************************/
#include <fragment/dialogfragment.h>
#include <widget/edittext.h>

#include <alarm.h>
#include <timer.h>

namespace cdroid {
namespace deskclock {

class LabelDialogFragment : public DialogFragment {
public:
    /** Upstream AlarmLabelDialogHandler.onDialogLabelSet, delivered as a callback. */
    typedef std::function<void(const data::Alarm&, const std::string&)> OnAlarmLabelSet;

private:
    EditText* mLabelBox = nullptr;

    int mTimerId = 0;

    /** The alarm being relabeled, when in alarm mode. */
    data::Alarm mAlarm;
    bool mHasAlarm = false;
    OnAlarmLabelSet mOnAlarmLabelSet;

    std::string mLabel;

public:
    cdroid::Dialog* onCreateDialog(cdroid::Bundle* savedInstanceState) override;
    void onDestroyView() override;

    static LabelDialogFragment* newInstance(const data::Timer& timer);
    static LabelDialogFragment* newInstance(const data::Alarm& alarm, const std::string& label,
                                            const OnAlarmLabelSet& onAlarmLabelSet);

    /** Replaces any existing LabelDialogFragment with the given fragment.
     *  (Named showDialog: a static show() here would hide DialogFragment::show.) */
    static void showDialog(FragmentManager* manager, LabelDialogFragment* fragment);

private:
    /** Sets the new label into the timer or alarm. */
    void setLabel();
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_LABELDIALOGFRAGMENT_H__
