#include <labeldialogfragment.h>

#include <app/alertdialog.h>
#include <core/bundle.h>
#include <core/context.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <text/String.h>
#include <widget/internal_R.h>
#include <R.h>

#include <datamodel.h>

namespace cdroid {
namespace deskclock {

namespace {
constexpr const char* TAG = "label_dialog";
constexpr const char* ARG_LABEL = "arg_label";
constexpr const char* ARG_TIMER_ID = "arg_timer_id";
} // namespace

LabelDialogFragment* LabelDialogFragment::newInstance(const data::Alarm& alarm,
                                                       const std::string& label,
                                                       const OnAlarmLabelSet& onAlarmLabelSet) {
    LabelDialogFragment* frag = new LabelDialogFragment();
    frag->mAlarm = alarm;
    frag->mHasAlarm = true;
    frag->mLabel = label;
    frag->mOnAlarmLabelSet = onAlarmLabelSet;
    return frag;
}

LabelDialogFragment* LabelDialogFragment::newInstance(const data::Timer& timer) {
    Bundle* args = new Bundle();
    args->putString(ARG_LABEL, timer.label);
    args->putInt(ARG_TIMER_ID, timer.id);

    LabelDialogFragment* frag = new LabelDialogFragment();
    frag->setArguments(args);
    return frag;
}

cdroid::Dialog* LabelDialogFragment::onCreateDialog(cdroid::Bundle* savedInstanceState) {
    const Bundle* args = getArguments();
    if (args != nullptr) {
        mTimerId = args->getInt(ARG_TIMER_ID, -1);
        mLabel = args->getString(ARG_LABEL);
    }
    if (savedInstanceState != nullptr) {
        mLabel = savedInstanceState->getString(ARG_LABEL);
    }

    Context* context = getContext();
    AlertDialog::Builder builder(context);
    builder.setMessage(::deskclock::R::string::label);
    builder.setPositiveButton(internal::R::string::ok, [this](DialogInterface&, int) {
        setLabel();
        dismiss();
    });
    builder.setNegativeButton(internal::R::string::cancel, nullptr);

    mLabelBox = new EditText(context);
    mLabelBox->setSingleLine(true);
    // TYPE_CLASS_TEXT | TYPE_TEXT_FLAG_CAP_SENTENCES
    mLabelBox->setInputType(0x00000001 | 0x00004000);
    mLabelBox->setText(mLabel);
    mLabelBox->setSelectAllOnFocus(true);
    mLabelBox->selectAllText();
    builder.setView(mLabelBox);

    AlertDialog* dialog = builder.create();

    // The line at the bottom of EditText is part of its background therefore the padding
    // must be added to its container (upstream dialog.setView(box, padding, 0, padding, 0)).
    const int padding = context->getResources()
            .getDimensionPixelSize(::deskclock::R::dimen::label_edittext_padding);
    dialog->setView(mLabelBox, padding, 0, padding, 0);

    // Upstream also shows the soft keyboard (SOFT_INPUT_STATE_VISIBLE); cdroid apps
    // are hardware-key driven, IME is out of scope.
    return dialog;
}

void LabelDialogFragment::onDestroyView() {
    DialogFragment::onDestroyView();
    // Stop callbacks from the IME since there is no view to process them.
    // (The label box is owned by the dialog window being torn down.)
    mLabelBox = nullptr;
}

void LabelDialogFragment::setLabel() {
    std::string label = mLabelBox ? mLabelBox->getText().toString()->str() : std::string();
    // Trim: don't allow a label with only whitespace.
    size_t b = label.find_first_not_of(" \t\r\n");
    size_t e = label.find_last_not_of(" \t\r\n");
    label = (b == std::string::npos) ? std::string() : label.substr(b, e - b + 1);

    if (mHasAlarm) {
        // Upstream delegates to the activity's AlarmLabelDialogHandler.
        if (mOnAlarmLabelSet) mOnAlarmLabelSet(mAlarm, label);
    } else if (mTimerId >= 0) {
        data::Timer timer;
        if (data::DataModel::getDataModel().getTimer(mTimerId, timer)) {
            data::DataModel::getDataModel().setTimerLabel(timer, label);
        }
    }
}

void LabelDialogFragment::showDialog(FragmentManager* manager,
                                      LabelDialogFragment* fragment) {
    // Upstream bails when the manager is destroyed; cdroid FragmentManager does not
    // expose that state (see FragmentTabPagerAdapter::finishUpdate).
    if (manager == nullptr) {
        delete fragment;
        return;
    }

    // Finish any outstanding fragment work.
    manager->executePendingTransactions();

    FragmentTransaction* tx = manager->beginTransaction();

    // Remove existing instance of LabelDialogFragment if necessary.
    Fragment* existing = manager->findFragmentByTag(TAG);
    if (existing != nullptr) {
        tx->remove(existing);
    }
    tx->addToBackStack(nullptr);

    fragment->show(tx, TAG);
}

} // namespace deskclock
} // namespace cdroid
