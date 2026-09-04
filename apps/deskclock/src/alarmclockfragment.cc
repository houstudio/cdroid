#include <alarmclockfragment.h>

#include <R.h>

#include <core/systemclock.h>
#include <fragment/fragmentfactory.h>
#include <view/layoutinflater.h>
#include <widget/textview.h>

#include <alarminstance.h>
#include <alarmstatemanager.h>
#include <collapsedalarmviewholder.h>
#include <expandedalarmviewholder.h>
#include <itemanimator.h>
#include <timepickerdialogfragment.h>
#include <uidata.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

namespace {
constexpr const char* KEY_EXPANDED_ID = "expandedId";
} // namespace

AlarmClockFragment::AlarmClockFragment() : DeskClockFragment(uidata::Tab::ALARMS) {
    // MidnightRunnable: refresh the display of all alarms (Today/Tomorrow flip).
    mMidnightUpdater = [this]() {
        mItemAdapter->notifyDataSetChanged();
    };
}

AlarmClockFragment::~AlarmClockFragment() {
    delete mItemAdapter;
    delete mAlarmUpdateHandler;
    delete mAlarmTimeClickHandler;
}

void AlarmClockFragment::onCreate(Bundle* savedInstanceState) {
    DeskClockFragment::onCreate(savedInstanceState);
    if (savedInstanceState != nullptr) {
        mExpandedAlarmId = savedInstanceState->getLong(KEY_EXPANDED_ID, data::Alarm::INVALID_ID);
    }
}

View* AlarmClockFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
                                       Bundle* savedInstanceState) {
    View* v = inflater->inflate(R::layout::alarm_clock, container, false);
    // Opaque base layer: sibling pager pages smear under SRC_OVER without one.
    Utils::setDefaultBackground(v);

    mRecyclerView = (RecyclerView*) v->findViewById(R::id::alarms_recycler_view);
    mLayoutManager = new LinearLayoutManager(getContext());
    mRecyclerView->setLayoutManager(mLayoutManager);

    mMainLayout = (ViewGroup*) v->findViewById(R::id::main);
    mAlarmUpdateHandler = new alarms::AlarmUpdateHandler(getContext(),
            (alarms::ScrollHandler*) this, mMainLayout);
    mEmptyView = (TextView*) v->findViewById(R::id::alarms_empty_view);
    mAlarmTimeClickHandler = new alarms::AlarmTimeClickHandler((Fragment*) this,
            savedInstanceState, mAlarmUpdateHandler, (alarms::ScrollHandler*) this);

    mItemAdapter = new ItemAdapter<alarms::AlarmItemHolder>();
    mItemAdapter->setHasStableIds();
    LayoutInflater* layoutInflater = LayoutInflater::from(getContext());
    mItemAdapter->withViewTypes(
            [layoutInflater](ViewGroup& parent, int viewType) -> ItemViewHolder* {
                return alarms::CollapsedAlarmViewHolder::createViewHolder(layoutInflater,
                        &parent, viewType);
            }, nullptr, {alarms::alarmCollapsedViewType()});
    mItemAdapter->withViewTypes(
            [layoutInflater](ViewGroup& parent, int viewType) -> ItemViewHolder* {
                return alarms::ExpandedAlarmViewHolder::createViewHolder(layoutInflater,
                        &parent, viewType);
            }, nullptr, {alarms::alarmExpandedViewType()});

    OnItemChangedListener itemChangedListener;
    itemChangedListener.onItemChanged = [this](ItemHolder& itemHolder) {
        alarms::AlarmItemHolder* holder = dynamic_cast<alarms::AlarmItemHolder*>(&itemHolder);
        if (holder->isExpanded) {
            if (mExpandedAlarmId != holder->itemId) {
                // Collapse the prior expanded alarm.
                alarms::AlarmItemHolder* aih = mItemAdapter->findItemById(mExpandedAlarmId);
                if (aih != nullptr) aih->collapse();
                // Record the freshly expanded alarm.
                mExpandedAlarmId = holder->itemId;
                RecyclerView::ViewHolder* viewHolder =
                        mRecyclerView->findViewHolderForItemId(mExpandedAlarmId);
                if (viewHolder != nullptr) {
                    smoothScrollTo(viewHolder->getLayoutPosition());
                }
            }
        } else if (mExpandedAlarmId == holder->itemId) {
            // The expanded alarm is now collapsed so update the tracking id.
            mExpandedAlarmId = data::Alarm::INVALID_ID;
        }
    };
    mItemAdapter->setOnItemChangedListener(itemChangedListener);

    // ScrollPositionWatcher (scroll state into the UiDataModel).
    RecyclerView::OnScrollListener scrollPositionWatcher;
    scrollPositionWatcher.onScrolled = [this](RecyclerView&, int, int) {
        setTabScrolledToTop(Utils::isScrolledToTop(*mRecyclerView));
    };
    mRecyclerView->addOnScrollListener(scrollPositionWatcher);
    mRecyclerView->setAdapter(mItemAdapter);
    auto* itemAnimator = new ItemAnimator();
    itemAnimator->setChangeDuration(300);
    itemAnimator->setMoveDuration(300);
    mRecyclerView->setItemAnimator(itemAnimator);

    reloadAlarms();
    return v;
}

void AlarmClockFragment::onStart() {
    DeskClockFragment::onStart();

    if (!isTabSelected()) {
        alarms::TimePickerDialogFragment::removeTimeEditDialog(getParentFragmentManager());
    }
}

void AlarmClockFragment::onResume() {
    DeskClockFragment::onResume();

    // Schedule a runnable to update the "Today/Tomorrow" values when midnight passes.
    uidata::UiDataModel::getUiDataModel().addMidnightCallback(mMidnightUpdater);

    reloadAlarms();
}

void AlarmClockFragment::onPause() {
    DeskClockFragment::onPause();

    uidata::UiDataModel::getUiDataModel().removePeriodicCallback(mMidnightUpdater);
    mAlarmUpdateHandler->hideUndoBar();
}

void AlarmClockFragment::onSaveInstanceState(Bundle* outState) {
    DeskClockFragment::onSaveInstanceState(outState);
    mAlarmTimeClickHandler->saveInstance(*outState);
    outState->putLong(KEY_EXPANDED_ID, mExpandedAlarmId);
}

void AlarmClockFragment::reloadAlarms() {
    // onLoadFinished: build AlarmItemHolders from the alarms (+instance join).
    Context& context = *getContext();
    auto sp = context.getSharedPreferences("DeskClock", Context::MODE_PRIVATE);
    std::vector<data::Alarm> alarms = data::Alarm::getAlarms(*sp);

    auto* itemHolders = new std::vector<alarms::AlarmItemHolder*>();
    for (const data::Alarm& alarm : alarms) {
        data::Alarm withInstance = alarm;
        alarms::AlarmStateManager::applyInstanceData(withInstance);
        data::Alarminstance* instance = nullptr;
        data::Alarminstance joined;
        if (withInstance.canPreemptivelyDismiss()
                && data::Alarminstance::getInstance(*sp, withInstance.instanceId, joined)) {
            instance = &joined;
        }
        itemHolders->push_back(
                new alarms::AlarmItemHolder(withInstance, instance, mAlarmTimeClickHandler));
    }
    setAdapterItems(itemHolders);
}

void AlarmClockFragment::setAdapterItems(std::vector<alarms::AlarmItemHolder*>* items) {
    mItemAdapter->setItems(items);

    // Show or hide the empty view as appropriate (EmptyViewController inline).
    const bool noAlarms = items->empty();
    mEmptyView->setVisibility(noAlarms ? View::VISIBLE : View::GONE);
    mRecyclerView->setVisibility(noAlarms ? View::GONE : View::VISIBLE);
    if (noAlarms) {
        // Ensure the drop shadow is hidden when no alarms exist.
        setTabScrolledToTop(true);
    }

    // Expand the correct alarm.
    if (mExpandedAlarmId != data::Alarm::INVALID_ID) {
        alarms::AlarmItemHolder* aih = mItemAdapter->findItemById(mExpandedAlarmId);
        if (aih != nullptr) {
            mAlarmTimeClickHandler->setSelectedAlarm(&aih->item);
            aih->expand();
        } else {
            mAlarmTimeClickHandler->setSelectedAlarm(nullptr);
            mExpandedAlarmId = data::Alarm::INVALID_ID;
        }
    }

    // Scroll to the selected alarm.
    if (mScrollToAlarmId != data::Alarm::INVALID_ID) {
        scrollToAlarm(mScrollToAlarmId);
        setSmoothScrollStableId(data::Alarm::INVALID_ID);
    }
}

void AlarmClockFragment::scrollToAlarm(int64_t alarmId) {
    const int alarmCount = mItemAdapter->getItemCount();
    int alarmPosition = -1;
    for (int i = 0; i < alarmCount; i++) {
        if (mItemAdapter->getItemId(i) == alarmId) {
            alarmPosition = i;
            break;
        }
    }

    if (alarmPosition >= 0) {
        alarms::AlarmItemHolder* aih = mItemAdapter->findItemById(alarmId);
        if (aih != nullptr) aih->expand();
        smoothScrollTo(alarmPosition);
    }
    // (Deleted-alarm snackbar stub.)
}

void AlarmClockFragment::setLabel(const data::Alarm& alarm, const std::string& label) {
    data::Alarm mutableAlarm = alarm;
    mutableAlarm.label = label;
    mAlarmUpdateHandler->asyncUpdateAlarm(mutableAlarm, false, true);
}

void AlarmClockFragment::removeItem(alarms::AlarmItemHolder* itemHolder) {
    mItemAdapter->removeItem(itemHolder);
}

void AlarmClockFragment::smoothScrollTo(int position) {
    mLayoutManager->scrollToPositionWithOffset(position, 0);
}

void AlarmClockFragment::setSmoothScrollStableId(int64_t stableId) {
    mScrollToAlarmId = stableId;
}

void AlarmClockFragment::onTimeSet(alarms::TimePickerDialogFragment* /*fragment*/,
                                   int hourOfDay, int minute) {
    mAlarmTimeClickHandler->onTimeSet(hourOfDay, minute);
}

void AlarmClockFragment::onFabClick(ImageView& /*fab*/) {
    mAlarmUpdateHandler->hideUndoBar();
    startCreatingAlarm();
}

void AlarmClockFragment::onUpdateFab(ImageView& fab) {
    fab.setVisibility(View::VISIBLE);
    fab.setImageResource(R::drawable::ic_add_white_24dp);
    fab.setContentDescription(fab.getContext()->getResources().getString(R::string::button_alarms));
}

void AlarmClockFragment::onUpdateFabButtons(Button& left, Button& right) {
    left.setVisibility(View::INVISIBLE);
    right.setVisibility(View::INVISIBLE);
}

void AlarmClockFragment::startCreatingAlarm() {
    // Clear the currently selected alarm.
    mAlarmTimeClickHandler->setSelectedAlarm(nullptr);
    alarms::TimePickerDialogFragment::show(this);
}

} // namespace deskclock

// Registered under the bare class name the UiDataModel ALARMS tab references
// (replaces the tabfragments.cc placeholder).
static const int _cdroid_frag_reg_alarm_real =
    (::cdroid::FragmentFactory::registerFragment("AlarmClockFragment",
        []() -> ::cdroid::Fragment* {
            return new ::cdroid::deskclock::AlarmClockFragment();
        }), 0);

} // namespace cdroid
