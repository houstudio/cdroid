#include <timepickerdialogfragment.h>

#include <app/alertdialog.h>
#include <core/bundle.h>
#include <core/context.h>
#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <widget/internal_R.h>

#include <datamodel.h>

namespace cdroid {
namespace deskclock {
namespace alarms {

namespace {
constexpr const char* TAG = "TimePickerDialogFragment";
constexpr const char* ARG_HOUR = "TimePickerDialogFragment_hour";
constexpr const char* ARG_MINUTE = "TimePickerDialogFragment_minute";
} // namespace

cdroid::Dialog* TimePickerDialogFragment::onCreateDialog(cdroid::Bundle* /*savedInstanceState*/) {
    OnTimeSetListener* listener = dynamic_cast<OnTimeSetListener*>(getParentFragment());

    auto now = Calendar::getInstance();
    const Bundle* args = getArguments();
    int hour = now->get(Calendar::HOUR_OF_DAY);
    int minute = now->get(Calendar::MINUTE);
    if (args != nullptr) {
        if (args->getInt(ARG_HOUR, -1) != -1) hour = args->getInt(ARG_HOUR, -1);
        if (args->getInt(ARG_MINUTE, -1) != -1) minute = args->getInt(ARG_MINUTE, -1);
    }

    Context* context = getContext();
    mTimePicker = new TimePicker(context);
    mTimePicker->setHour(hour);
    mTimePicker->setMinute(minute);
    mTimePicker->setIs24HourView(data::DataModel::getDataModel().is24HourFormat());

    AlertDialog::Builder builder(context);
    builder.setView(mTimePicker);
    builder.setPositiveButton(internal::R::string::ok,
            [this, listener](DialogInterface&, int) {
                if (listener != nullptr) {
                    listener->onTimeSet(this, mTimePicker->getHour(), mTimePicker->getMinute());
                }
            });
    builder.setNegativeButton(internal::R::string::cancel, nullptr);
    return builder.create();
}

void TimePickerDialogFragment::show(Fragment* parentFragment) {
    show(parentFragment, -1, -1);
}

void TimePickerDialogFragment::show(Fragment* parentFragment, int hourOfDay,
                                    int minute) {
    FragmentManager* manager = parentFragment->getChildFragmentManager();
    if (manager == nullptr) {
        return;
    }

    // Make sure the dialog isn't already added.
    removeTimeEditDialog(manager);

    TimePickerDialogFragment* fragment = new TimePickerDialogFragment();

    Bundle* args = new Bundle();
    if (hourOfDay >= 0 && hourOfDay <= 23) {
        args->putInt(ARG_HOUR, hourOfDay);
    }
    if (minute >= 0 && minute <= 59) {
        args->putInt(ARG_MINUTE, minute);
    }

    fragment->setArguments(args);
    fragment->DialogFragment::show(manager, TAG);
}

void TimePickerDialogFragment::removeTimeEditDialog(FragmentManager* manager) {
    if (manager == nullptr) return;
    Fragment* prev = manager->findFragmentByTag(TAG);
    if (prev != nullptr) {
        FragmentTransaction& tx = *manager->beginTransaction();
        tx.remove(prev);
        tx.commit();
    }
}

} // namespace alarms
} // namespace deskclock
} // namespace cdroid
