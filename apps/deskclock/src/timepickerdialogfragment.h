#ifndef __DESKCLOCK_TIMEPICKERDIALOGFRAGMENT_H__
#define __DESKCLOCK_TIMEPICKERDIALOGFRAGMENT_H__
/*********************************************************************************
 * Port of com.android.deskclock.alarms.TimePickerDialogFragment — DialogFragment
 * hosting a TimePicker (the pre-L builder branch: AlertDialog + TimePicker view;
 * cdroid has no framework TimePickerDialog face).
 *********************************************************************************/
#include <fragment/dialogfragment.h>
#include <widget/timepicker.h>

namespace cdroid {
namespace deskclock {
namespace alarms {

class TimePickerDialogFragment : public DialogFragment {
public:
    /** The callback interface used to indicate the user is done filling in the time. */
    class OnTimeSetListener {
    public:
        virtual ~OnTimeSetListener() = default;
        virtual void onTimeSet(TimePickerDialogFragment* fragment, int hourOfDay, int minute) = 0;
    };

    cdroid::Dialog* onCreateDialog(cdroid::Bundle* savedInstanceState) override;

    static void show(Fragment* parentFragment);
    static void show(Fragment* parentFragment, int hourOfDay, int minute);
    static void removeTimeEditDialog(FragmentManager* manager);

private:
    TimePicker* mTimePicker = nullptr;
    int mHour = -1;
    int mMinute = -1;
};

} // namespace alarms
} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_TIMEPICKERDIALOGFRAGMENT_H__
