#include <ringtonepickeractivity.h>

#include <R.h>
#include <porting/cdlog.h>

#include <core/activityfactory.h>
#include <core/calendar.h>
#include <core/uri.h>
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <core/bundle.h>
#include <fragment/dialogfragment.h>
#include <fragment/fragmentmanager.h>
#include <menu/menu.h>
#include <widget/internal_R.h>
#include <view/layoutinflater.h>
#include <widget/actionbar.h>
#include <widget/textview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>

#include <alarm.h>
#include <alarmupdatehandler.h>
#include <addcustomringtoneviewholder.h>
#include <datamodel.h>
#include <headerviewholder.h>
#include <ringtoneholder.h>
#include <ringtonepreviewklaxon.h>
#include <ringtoneviewholder.h>
#include <ringtoneloader.h>
#include <searchmenuitemcontroller.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace ringtone {

// The DeskClock prefs backing the alarm DAOs (alarmstatemanager.cc's helper).
static std::shared_ptr<SharedPreferences> prefs(Context& context) {
    return context.getSharedPreferences("DeskClock", Context::MODE_PRIVATE);
}

//
// ConfirmRemoveCustomRingtoneDialogFragment — informs the user of the
// side-effects of removing a custom ringtone while it is in use by alarms
// and/or timers and prompts them to confirm the removal.
//

namespace {

class ConfirmRemoveCustomRingtoneDialogFragment : public DialogFragment {
private:
    static constexpr const char* ARG_RINGTONE_URI_TO_REMOVE = "arg_ringtone_uri_to_remove";
    static constexpr const char* ARG_RINGTONE_HAS_PERMISSIONS = "arg_ringtone_has_permissions";

public:
    cdroid::Dialog* onCreateDialog(cdroid::Bundle* /*savedInstanceState*/) override {
        Bundle* arguments = getArguments();
        const std::string toRemove = arguments->getString(ARG_RINGTONE_URI_TO_REMOVE);
        Context* context = getContext();

        // Upstream: (activity as RingtonePickerActivity).removeCustomRingtone(uri).
        DialogInterface::OnClickListener okListener =
                [activity = getActivity(), toRemove](DialogInterface&, int) {
            RingtonePickerActivity* picker =
                    dynamic_cast<RingtonePickerActivity*>(activity);
            if (picker != nullptr) picker->removeCustomRingtone(toRemove);
        };

        if (arguments->getBoolean(ARG_RINGTONE_HAS_PERMISSIONS, false)) {
            return AlertDialog::Builder(context)
                    .setPositiveButton(R::string::remove_sound, okListener)
                    .setNegativeButton(internal::R::string::cancel, nullptr /* listener */)
                    .setMessage(R::string::confirm_remove_custom_ringtone)
                    .create();
        }
        return AlertDialog::Builder(context)
                .setPositiveButton(R::string::remove_sound, okListener)
                .setMessage(R::string::custom_ringtone_lost_permissions)
                .create();
    }

    static void show(FragmentManager* manager, const std::string& toRemove, bool hasPermissions) {
        // Upstream: if (manager.isDestroyed) return; FragmentManager exposes no
        // isDestroyed on cdroid (LabelDialogFragment precedent).

        Bundle args;
        args.putString(ARG_RINGTONE_URI_TO_REMOVE, toRemove);
        args.putBoolean(ARG_RINGTONE_HAS_PERMISSIONS, hasPermissions);

        auto* fragment = new ConfirmRemoveCustomRingtoneDialogFragment();
        fragment->setArguments(&args);
        fragment->setCancelable(hasPermissions);
        fragment->DialogFragment::show(manager, "confirm_ringtone_remove");
    }
};

} // namespace

//
// RingtonePickerActivity
//

RingtonePickerActivity::RingtonePickerActivity() : FragmentActivity(0, 0, -1, -1) {
}

RingtonePickerActivity::~RingtonePickerActivity() {
    delete mRingtoneAdapter;
    delete mDropShadowController;
}

void RingtonePickerActivity::onCreate(Bundle* savedInstanceState) {
    FragmentActivity::onCreate(savedInstanceState);

    View* content = LayoutInflater::from(getContext())
            ->inflate(R::layout::ringtone_picker, nullptr, false);
    // Opaque base layer (same reason as the other standalone windows).
    Utils::setDefaultBackground(content);
    addView(content);

    mOptionsMenuManager.addMenuItemController(
            {new actionbarmenu::NavUpMenuItemController(this)});
    {
        std::vector<actionbarmenu::MenuItemController*> controllers =
                actionbarmenu::MenuItemControllerFactory::buildMenuItemControllers(getContext());
        mOptionsMenuManager.addMenuItemController(controllers);
    }

    Intent intent = getIntent();

    // savedInstanceState is never non-null on cdroid (no Window state save);
    // the playing/selection extras below restore from the launch intent only.

    mHasSelectedRingtoneUri = !intent.getStringExtra(EXTRA_RINGTONE_URI).empty();
    mSelectedRingtoneUri = intent.getStringExtra(EXTRA_RINGTONE_URI);

    mAlarmId = intent.getLongExtra(EXTRA_ALARM_ID, -1);
    mDefaultRingtoneUri = intent.getStringExtra(EXTRA_DEFAULT_RINGTONE_URI);
    const int defaultRingtoneTitleId = intent.getIntExtra(EXTRA_DEFAULT_RINGTONE_NAME, 0);
    mDefaultRingtoneTitle = getContext()->getString(defaultRingtoneTitleId);

    LayoutInflater* inflater = LayoutInflater::from(getContext());
    OnItemClickedListener listener = [this](RecyclerView::ViewHolder& viewHolder, int id) {
        // ItemClickWatcher.onItemClicked
        if (id == AddCustomRingtoneViewHolder::CLICK_ADD_NEW) {
            stopPlayingRingtone(getSelectedRingtoneHolder(), false);
            // Upstream: startActivityForResult(ACTION_OPEN_DOCUMENT / CATEGORY_OPENABLE,
            // "audio/*"). No SAF file picker on cdroid — DEFERRED.
            LOGW("Add new sound: no system file picker (SAF) on cdroid — deferred");
        } else if (id == RingtoneViewHolder::CLICK_NORMAL) {
            RingtoneHolder* oldSelection = getSelectedRingtoneHolder();
            ItemViewHolder* itemViewHolder = dynamic_cast<ItemViewHolder*>(&viewHolder);
            if (itemViewHolder == nullptr) return;
            RingtoneHolder* newSelection =
                    dynamic_cast<RingtoneHolder*>(itemViewHolder->itemHolder);
            if (newSelection == nullptr) return;

            // Tapping the existing selection toggles playback of the ringtone.
            if (oldSelection == newSelection) {
                if (newSelection->isPlaying) {
                    stopPlayingRingtone(newSelection, false);
                } else {
                    startPlayingRingtone(*newSelection);
                }
            } else {
                // Tapping a new selection changes the selection and playback.
                stopPlayingRingtone(oldSelection, true);
                startPlayingRingtone(*newSelection);
            }
        } else if (id == RingtoneViewHolder::CLICK_LONG_PRESS) {
            mIndexOfRingtoneToRemove = viewHolder.getBindingAdapterPosition();
        } else if (id == RingtoneViewHolder::CLICK_NO_PERMISSIONS) {
            ItemViewHolder* itemViewHolder = dynamic_cast<ItemViewHolder*>(&viewHolder);
            RingtoneHolder* holder = itemViewHolder == nullptr ? nullptr
                    : dynamic_cast<RingtoneHolder*>(itemViewHolder->itemHolder);
            if (holder != nullptr) {
                ConfirmRemoveCustomRingtoneDialogFragment::show(getSupportFragmentManager(),
                        holder->uri, false);
            }
        }
    };

    ItemViewHolderFactory ringtoneFactory = [inflater](ViewGroup& parent, int) {
        return (ItemViewHolder*) new RingtoneViewHolder(
                inflater->inflate(R::layout::ringtone_item_sound, &parent, false));
    };
    ItemViewHolderFactory headerFactory = [inflater](ViewGroup& parent, int viewType) {
        return (ItemViewHolder*) new HeaderViewHolder(inflater->inflate(viewType, &parent, false));
    };
    ItemViewHolderFactory addNewFactory = [inflater](ViewGroup& parent, int) {
        return (ItemViewHolder*) new AddCustomRingtoneViewHolder(
                inflater->inflate(R::layout::ringtone_item_sound, &parent, false));
    };

    mRingtoneAdapter = new ItemAdapter<ItemHolder>();
    mRingtoneAdapter
            ->withViewTypes(headerFactory, nullptr,
                            {HeaderViewHolder::VIEW_TYPE_ITEM_HEADER})
            .withViewTypes(addNewFactory, listener,
                           {AddCustomRingtoneViewHolder::VIEW_TYPE_ADD_NEW})
            .withViewTypes(ringtoneFactory, listener,
                           {RingtoneViewHolder::VIEW_TYPE_SYSTEM_SOUND,
                            RingtoneViewHolder::VIEW_TYPE_CUSTOM_SOUND});

    mRecyclerView = (RecyclerView*) findViewById(R::id::ringtone_content);
    mRecyclerView->setLayoutManager(std::unique_ptr<RecyclerView::LayoutManager>(
            new LinearLayoutManager(getContext())));
    mRecyclerView->setAdapter(mRingtoneAdapter);
    mRecyclerView->setItemAnimator(nullptr);

    mCloseContextMenuOnScroll.onScrollStateChanged =
            [this](RecyclerView&, int /*newState*/) {
        if (mIndexOfRingtoneToRemove != RecyclerView::NO_POSITION) {
            closeContextMenu();
        }
    };
    mRecyclerView->addOnScrollListener(mCloseContextMenuOnScroll);

    const int titleResourceId = intent.getIntExtra(EXTRA_TITLE, 0);
    if (ActionBar* actionBar = getActionBar()) {
        actionBar->setTitle(getContext()->getString(titleResourceId));
    }

    // LoaderManager.initLoader stand-in (synchronous load; see header note).
    loadRingtoneData();

    registerForContextMenu(mRecyclerView);
}

void RingtonePickerActivity::onResume() {
    FragmentActivity::onResume();

    mDropShadowController = new DropShadowController(*findViewById(R::id::drop_shadow),
            *mRecyclerView);
}

void RingtonePickerActivity::onPause() {
    mDropShadowController->stop();
    delete mDropShadowController;
    mDropShadowController = nullptr;

    if (mHasSelectedRingtoneUri) {
        if (mAlarmId != -1) {
            // Fetch the alarm whose ringtone must be updated, update the
            // default ringtone for future new alarms, then persist the alarm
            // (upstream chains two AsyncTasks; synchronized here).
            data::Alarm alarm;
            if (data::Alarm::getAlarm(*prefs(*getContext()), mAlarmId, alarm)) {
                alarm.alert = mSelectedRingtoneUri;
                std::unique_ptr<Uri> alertUri(Uri::parse(alarm.alert));
                data::DataModel::getDataModel().setDefaultAlarmRingtoneUri(alertUri.get());
                alarms::AlarmUpdateHandler(getContext(), nullptr, nullptr)
                        .asyncUpdateAlarm(alarm, false /* popToast */, true /* minorUpdate */);
            }
        } else {
            std::unique_ptr<Uri> uri(Uri::parse(mSelectedRingtoneUri));
            data::DataModel::getDataModel().setTimerRingtoneUri(uri.get());
        }
    }

    FragmentActivity::onPause();
}

void RingtonePickerActivity::onStop() {
    // No isChangingConfigurations() on cdroid (no configuration restarts).
    stopPlayingRingtone(getSelectedRingtoneHolder(), false);
    FragmentActivity::onStop();
}

bool RingtonePickerActivity::onCreateOptionsMenu(Menu& menu) {
    mOptionsMenuManager.onCreateOptionsMenu(menu);
    return true;
}

bool RingtonePickerActivity::onPrepareOptionsMenu(Menu& menu) {
    mOptionsMenuManager.onPrepareOptionsMenu(menu);
    return true;
}

bool RingtonePickerActivity::onOptionsItemSelected(MenuItem& item) {
    return mOptionsMenuManager.onOptionsItemSelected(item) ||
           FragmentActivity::onOptionsItemSelected(item);
}

bool RingtonePickerActivity::onContextItemSelected(MenuItem& /*item*/) {
    // Find the ringtone to be removed.
    std::vector<ItemHolder*>* items = mRingtoneAdapter->items;
    if (items == nullptr || mIndexOfRingtoneToRemove < 0
            || mIndexOfRingtoneToRemove >= (int) items->size()) {
        mIndexOfRingtoneToRemove = RecyclerView::NO_POSITION;
        return true;
    }
    RingtoneHolder* toRemove = dynamic_cast<RingtoneHolder*>((*items)[mIndexOfRingtoneToRemove]);
    mIndexOfRingtoneToRemove = RecyclerView::NO_POSITION;
    if (toRemove == nullptr) return true;

    // Launch the confirmation dialog.
    ConfirmRemoveCustomRingtoneDialogFragment::show(getSupportFragmentManager(), toRemove->uri,
            toRemove->hasPermissions());
    return true;
}

void RingtonePickerActivity::loadRingtoneData() {
    RingtoneLoader loader(*getContext(), mDefaultRingtoneUri, mDefaultRingtoneTitle);
    onLoadFinished(loader.loadInBackground());
}

void RingtonePickerActivity::onLoadFinished(std::vector<ItemHolder*>* itemHolders) {
    // Update the adapter with fresh data.
    mRingtoneAdapter->setItems(itemHolders);

    // Attempt to select the requested ringtone.
    RingtoneHolder* toSelect =
            mHasSelectedRingtoneUri ? getRingtoneHolder(mSelectedRingtoneUri) : nullptr;
    if (toSelect != nullptr) {
        toSelect->isSelected = true;
        mSelectedRingtoneUri = toSelect->uri;
        mHasSelectedRingtoneUri = true;
        toSelect->notifyItemChanged();

        // Start playing the ringtone if indicated.
        if (mIsPlaying) {
            startPlayingRingtone(*toSelect);
        }
    } else {
        // Clear the selection since it does not exist in the data.
        RingtonePreviewKlaxon::stop(*getContext());
        mHasSelectedRingtoneUri = false;
        mSelectedRingtoneUri.clear();
        mIsPlaying = false;
    }
}

RingtoneHolder* RingtonePickerActivity::getRingtoneHolder(const std::string& uri) const {
    std::vector<ItemHolder*>* items = mRingtoneAdapter->items;
    if (items == nullptr) return nullptr;
    for (ItemHolder* itemHolder : *items) {
        RingtoneHolder* ringtoneHolder = dynamic_cast<RingtoneHolder*>(itemHolder);
        if (ringtoneHolder != nullptr && ringtoneHolder->uri == uri) {
            return ringtoneHolder;
        }
    }
    return nullptr;
}

RingtoneHolder* RingtonePickerActivity::getSelectedRingtoneHolder() const {
    return mHasSelectedRingtoneUri ? getRingtoneHolder(mSelectedRingtoneUri) : nullptr;
}

void RingtonePickerActivity::startPlayingRingtone(RingtoneHolder& ringtone) {
    if (!ringtone.isPlaying && !ringtone.isSilent()) {
        std::unique_ptr<Uri> uri(Uri::parse(ringtone.uri));
        RingtonePreviewKlaxon::start(*getContext(), *uri);
        ringtone.isPlaying = true;
        mIsPlaying = true;
    }
    if (!ringtone.isSelected) {
        ringtone.isSelected = true;
        mSelectedRingtoneUri = ringtone.uri;
        mHasSelectedRingtoneUri = true;
    }
    ringtone.notifyItemChanged();
}

void RingtonePickerActivity::stopPlayingRingtone(RingtoneHolder* ringtone, bool deselect) {
    if (ringtone == nullptr) {
        return;
    }

    if (ringtone->isPlaying) {
        RingtonePreviewKlaxon::stop(*getContext());
        ringtone->isPlaying = false;
        mIsPlaying = false;
    }
    if (deselect && ringtone->isSelected) {
        ringtone->isSelected = false;
        mHasSelectedRingtoneUri = false;
        mSelectedRingtoneUri.clear();
    }
    ringtone->notifyItemChanged();
}

void RingtonePickerActivity::removeCustomRingtone(const std::string& toRemove) {
    // RemoveCustomRingtoneTask, synchronized: reassign alarms using the
    // removed sound to the system default, reset defaults that pointed at it,
    // then drop it from the model and the adapter.
    std::unique_ptr<Uri> mSystemDefaultRingtoneUri(
            data::DataModel::getDataModel().getDefaultAlarmRingtoneUri());
    const std::string systemDefault = mSystemDefaultRingtoneUri->toString();

    // Update all alarms that use the custom ringtone to use the system default.
    for (const data::Alarm& alarm : data::Alarm::getAlarms(*prefs(*getContext()))) {
        if (toRemove == alarm.alert) {
            data::Alarm reassigned = alarm;
            reassigned.alert = systemDefault;
            // Persist the updated alarm (upstream second background task).
            alarms::AlarmUpdateHandler(getContext(), nullptr, nullptr)
                    .asyncUpdateAlarm(reassigned, false /* popToast */, true /* minorUpdate */);
        }
    }
    // Upstream releases the SAF persistable-uri permission here; no SAF on cdroid.

    // Reset the default alarm ringtone if it was just removed.
    std::unique_ptr<Uri> currentDefault(
            data::DataModel::getDataModel().getDefaultAlarmRingtoneUri());
    if (toRemove == currentDefault->toString()) {
        data::DataModel::getDataModel().setDefaultAlarmRingtoneUri(mSystemDefaultRingtoneUri.get());
    }

    // Reset the timer ringtone if it was just removed.
    std::unique_ptr<Uri> timerRingtone(data::DataModel::getDataModel().getTimerRingtoneUri());
    if (toRemove == timerRingtone->toString()) {
        std::unique_ptr<Uri> defaultTimerRingtone(
                data::DataModel::getDataModel().getDefaultTimerRingtoneUri());
        data::DataModel::getDataModel().setTimerRingtoneUri(defaultTimerRingtone.get());
    }

    // Remove the corresponding custom ringtone.
    data::DataModel::getDataModel().removeCustomRingtone(toRemove);

    // Find the ringtone to be removed from the adapter.
    RingtoneHolder* holderToRemove = getRingtoneHolder(toRemove);
    if (holderToRemove == nullptr) {
        return;
    }

    // If the ringtone to remove is also the selected ringtone, adjust the selection.
    if (holderToRemove->isSelected) {
        stopPlayingRingtone(holderToRemove, false);
        RingtoneHolder* defaultRingtone = getRingtoneHolder(mDefaultRingtoneUri);
        if (defaultRingtone != nullptr) {
            defaultRingtone->isSelected = true;
            mSelectedRingtoneUri = defaultRingtone->uri;
            mHasSelectedRingtoneUri = true;
            defaultRingtone->notifyItemChanged();
        }
    }

    // Remove the ringtone from the adapter.
    mRingtoneAdapter->removeItem(holderToRemove);
}

Intent RingtonePickerActivity::createAlarmRingtonePickerIntent(Context& context,
        const data::Alarm& alarm) {
    (void) context;
    std::unique_ptr<Uri> defaultUri(data::DataModel::getDataModel().getDefaultAlarmRingtoneUri());
    return Intent()
            .setClassName("cdroid.deskclock", "RingtonePickerActivity")
            .putExtra(EXTRA_TITLE, (int) R::string::alarm_sound)
            .putExtra(EXTRA_ALARM_ID, (int64_t) alarm.id)
            .putExtra(EXTRA_RINGTONE_URI, alarm.alert)
            .putExtra(EXTRA_DEFAULT_RINGTONE_URI, defaultUri->toString())
            .putExtra(EXTRA_DEFAULT_RINGTONE_NAME, (int) R::string::default_alarm_ringtone_title);
}

Intent RingtonePickerActivity::createTimerRingtonePickerIntent(Context& context) {
    (void) context;
    data::DataModel& dataModel = data::DataModel::getDataModel();
    std::unique_ptr<Uri> timerUri(dataModel.getTimerRingtoneUri());
    std::unique_ptr<Uri> defaultUri(dataModel.getDefaultTimerRingtoneUri());
    return Intent()
            .setClassName("cdroid.deskclock", "RingtonePickerActivity")
            .putExtra(EXTRA_TITLE, (int) R::string::timer_sound)
            .putExtra(EXTRA_RINGTONE_URI, timerUri->toString())
            .putExtra(EXTRA_DEFAULT_RINGTONE_URI, defaultUri->toString())
            .putExtra(EXTRA_DEFAULT_RINGTONE_NAME, (int) R::string::default_timer_ringtone_title);
}

} // namespace ringtone
} // namespace deskclock
} // namespace cdroid

// Registered under the bare class name the click handlers' Intents carry
// (REGISTER_ACTIVITY would stringify the qualified name).
static const int _cdroid_act_reg_ringtonepicker =
    (::cdroid::ActivityFactory::registerActivity("RingtonePickerActivity",
        []() -> ::cdroid::Window* {
            ::cdroid::Window* w = new ::cdroid::deskclock::ringtone::RingtonePickerActivity();
            w->setActivityName("RingtonePickerActivity");
            return w;
        }), 0);
