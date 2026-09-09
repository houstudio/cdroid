#include <stopwatchfragment.h>

#include <R.h>
#include <widget/internal_R.h>

#include <algorithm>
#include <cmath>
#include <memory>

#include <animation/animator.h>
#include <content/resources.h>
#include <drawable/colorstatelist.h>
#include <drawable/gradientdrawable.h>
#include <core/context.h>
#include <core/systemclock.h>
#include <transition/transitionmanager.h>
#include <view/view.h>
#include <widget/cdwindow.h>

#include <animatorutils.h>
#include <fragment/fragmentfactory.h>
#include <datamodel.h>
#include <uidata.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {
namespace stopwatch {

using data::DataModel;
using uidata::UiDataModel;

namespace {
constexpr int REDRAW_PERIOD_RUNNING = 25;
constexpr int REDRAW_PERIOD_PAUSED = 500;
} // namespace

/**
 * Draws a tinting gradient over the bottom of the stopwatch laps list. This reduces the
 * contrast between floating buttons and the laps list content.
 */
class StopwatchFragment::GradientItemDecoration : public RecyclerView::ItemDecoration {
private:
    /** Control point colors that define the gradient; based on the window background
     *  color and recomputed each time that color changes (upstream mGradientColors). */
    std::vector<int> mGradientColors;

    /** The drawable that produces the tinting gradient effect of this decoration. */
    std::unique_ptr<GradientDrawable> mGradient;

    /** The height of the gradient; sized relative to the fab height. */
    int mGradientHeight;

    static const int ALPHAS[21];

public:
    explicit GradientItemDecoration(Context& context) {
        mGradient.reset(new GradientDrawable(GradientDrawable::TOP_BOTTOM, {}));
        // Upstream seeds from android.R.attr.windowBackground — NOT colorAccent —
        // so the band blends the laps list back into the window background.
        // (Same obtainStyledAttributes rationale as the time text colors above.)
        const uint32_t attrs[] = {0x01010034 /* android:windowBackground */, 0};
        auto ta = context.obtainStyledAttributes(attrs);
        int windowBackground = ta ? (int) ta->getColor(0, 0xFF303030u) : 0xFF303030;
        updateGradientColors(windowBackground);

        Resources& res = context.getResources();
        const int fabHeight = res.getDimensionPixelSize(R::dimen::fab_height);
        mGradientHeight = (int) std::round(fabHeight * 1.2f);
    }

    /** Given a baseColor, compute a gradient of tinted colors that define the fade
     *  effect to apply to the bottom of the lap list. */
    void updateGradientColors(int baseColor) {
        mGradientColors.clear();
        for (int i = 0; i < 21; i++) {
            mGradientColors.push_back((baseColor & 0x00FFFFFF) | (ALPHAS[i] << 24));
        }
        mGradient->setColors(mGradientColors);
    }

    void onDrawOver(Canvas& canvas, RecyclerView& parent, RecyclerView::State&) override {
        const int w = parent.getWidth();
        const int h = parent.getHeight();
        // Rect::set/setBounds take (x, y, width, height) — the height is the
        // band height, not the bottom edge.
        mGradient->setBounds(0, h - mGradientHeight, w, mGradientHeight);
        mGradient->draw(canvas);
    }
};

const int StopwatchFragment::GradientItemDecoration::ALPHAS[21] = {
        0x00, // 0%
        0x1A, // 10%
        0x33, // 20%
        0x4D, // 30%
        0x66, // 40%
        0x80, // 50%
        0x89, // 53.8%
        0x93, // 57.6%
        0x9D, // 61.5%
        0xA7, // 65.3%
        0xB1, // 69.2%
        0xBA, // 73.0%
        0xC4, // 76.9%
        0xCE, // 80.7%
        0xD8, // 84.6%
        0xE2, // 88.4%
        0xEB, // 92.3%
        0xF5, // 96.1%
        0xFF, // 100%
        0xFF, // 100%
        0xFF, // 100%
};

StopwatchFragment::StopwatchFragment() : DeskClockFragment(uidata::Tab::STOPWATCH) {
    mTimeUpdateRunnable = [this]() {
        const int64_t startTime = Utils::now();
        updateTime();

        View& touchTarget = (mTime != nullptr) ? (View&) *mTime : *mStopwatchWrapper;
        const data::Stopwatch& stopwatch = DataModel::getDataModel().getStopwatch();
        const bool blink = stopwatch.isPaused() && startTime % 1000 < 500 && !touchTarget.isPressed();

        if (blink) {
            mMainTimeText->setAlpha(0.0f);
            mHundredthsTimeText->setAlpha(0.0f);
        } else {
            mMainTimeText->setAlpha(1.0f);
            mHundredthsTimeText->setAlpha(1.0f);
        }

        if (!stopwatch.isReset()) {
            const int64_t period = stopwatch.isPaused() ? REDRAW_PERIOD_PAUSED : REDRAW_PERIOD_RUNNING;
            const int64_t endTime = Utils::now();
            const int64_t delay = std::max((int64_t) 0, startTime + period - endTime);
            mMainTimeText->postDelayed(mTimeUpdateRunnable, delay);
        }
    };

    mTabWatcher = [this](int, int) {
        // TabWatcher.selectedTabChanged
        if (isTabSelected()) {
            updateUI(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);
        }
    };

    mStopwatchWatcher.stopwatchUpdated = [this](const data::Stopwatch&,
                                                const data::Stopwatch& after) {
        if (isTabSelected()) {
            updateUI(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);
        } else if (after.isReset()) {
            mStopwatchTextController->setTimeString(0);
        }
    };
    // AOSP StopwatchFragment.lapAdded is an empty override: LapsAdapter.addLap()
    // already emits notifyItemInserted(0) + notifyItemChanged(1). Notifying here
    // as well posts a second insert op for a single data insertion and corrupts
    // the AdapterHelper offsets ("Inconsistency detected. Invalid item position").
    mStopwatchWatcher.lapAdded = [](const data::Lap&) {
    };
}

StopwatchFragment::~StopwatchFragment() {
    delete mGradientItemDecoration;
    delete mLapsAdapter;
    delete mStopwatchTextController;
}

View* StopwatchFragment::onCreateView(LayoutInflater* inflater, ViewGroup* container,
                                      Bundle* savedInstanceState) {
    mLapsAdapter = new LapsAdapter(*getContext());
    mLapsLayoutManager = new LinearLayoutManager(getContext());
    mGradientItemDecoration = new GradientItemDecoration(*getContext());

    View* v = inflater->inflate(R::layout::stopwatch_fragment, container, false);
    Utils::setDefaultBackground(v);
    mTime = (StopwatchCircleView*) v->findViewById(R::id::stopwatch_circle);
    mLapsList = (RecyclerView*) v->findViewById(R::id::laps_list);
    // (SimpleItemAnimator.setSupportsChangeAnimations(false): the cdroid port does
    // not expose that toggle; change animations are disabled with the null animator.)
    mLapsList->setLayoutManager(mLapsLayoutManager);
    mLapsList->addItemDecoration(mGradientItemDecoration);

    if (Utils::isLandscape(*getContext())) {
        mScrollPositionWatcher.onScrolled = [this](RecyclerView&, int, int) {
            setTabScrolledToTop(Utils::isScrolledToTop(*mLapsList));
        };
        mLapsList->addOnScrollListener(mScrollPositionWatcher);
    } else {
        setTabScrolledToTop(true);
    }
    mLapsList->setAdapter(mLapsAdapter);

    mMainTimeText = (TextView*) v->findViewById(R::id::stopwatch_time_text);
    mHundredthsTimeText = (TextView*) v->findViewById(R::id::stopwatch_hundredths_text);
    mStopwatchTextController = new StopwatchTextController(*mMainTimeText, *mHundredthsTimeText);
    mStopwatchWrapper = v->findViewById(R::id::stopwatch_time_wrapper);

    DataModel::getDataModel().addStopwatchListener(mStopwatchWatcher);

    // Tap the time/circle to toggle; press suppresses the paused blink.
    mStopwatchWrapper->setOnClickListener([this](View&) {
        toggleStopwatchState();
    });
    if (mTime != nullptr) {
        mTime->setOnTouchListener([this](View&, MotionEvent&) -> bool {
            // CircleTouchListener: track press state only.
            return false;
        });
    }

    // Time text color: primary when inactive, accent when pressed/activated.
    Context& c = *getContext();
    int colorAccent = 0xFFDA4336;
    int textColorPrimary = 0xFFFFFFFF;
    {
        // Upstream reads these through ThemeUtils.resolveColor. cdroid's
        // Theme.resolveAttribute flattens a color-selector reference to a
        // pool index (TypedValue.data without a resourceId), so resolve the
        // attrs through obtainStyledAttributes instead — the same engine
        // path widgets use, which handles color-selector file resources.
        const uint32_t attrs[] = {0x01010435 /* android:colorAccent */,
                                  0x01010036 /* android:textColorPrimary */, 0};
        auto ta = c.obtainStyledAttributes(attrs);
        if (ta != nullptr) {
            colorAccent = (int) ta->getColor(0, (uint32_t) colorAccent);
            textColorPrimary = (int) ta->getColor(1, (uint32_t) textColorPrimary);
        }
    }
    auto timeTextColor = std::make_shared<ColorStateList>(
            std::vector<std::vector<int>>{
                    {-internal::R::attr::state_activated, -internal::R::attr::state_pressed}, {}},
            std::vector<int>{textColorPrimary, colorAccent});
    mMainTimeText->setTextColor(timeTextColor);
    mHundredthsTimeText->setTextColor(timeTextColor);

    return v;
}

void StopwatchFragment::onStart() {
    DeskClockFragment::onStart();

    mLapsAdapter->notifyDataSetChanged();

    updateUI(FabContainer::FAB_AND_BUTTONS_IMMEDIATE);

    UiDataModel::getUiDataModel().addTabListener(mTabWatcher);
}

void StopwatchFragment::onStop() {
    DeskClockFragment::onStop();

    stopUpdatingTime();

    UiDataModel::getUiDataModel().removeTabListener(mTabWatcher);

    // releaseWakeLock: no WindowManager flags on cdroid.
}

void StopwatchFragment::onDestroyView() {
    DeskClockFragment::onDestroyView();

    DataModel::getDataModel().removeStopwatchListener(mStopwatchWatcher);
}

void StopwatchFragment::onFabClick(ImageView& /*fab*/) {
    toggleStopwatchState();
}

void StopwatchFragment::onLeftButtonClick(Button& /*left*/) {
    doReset();
}

void StopwatchFragment::onRightButtonClick(Button& /*right*/) {
    switch (DataModel::getDataModel().getStopwatch().state) {
        case data::Stopwatch::State::RUNNING: doAddLap(); break;
        case data::Stopwatch::State::PAUSED: doShare(); break;
        case data::Stopwatch::State::RESET: break;
    }
}

void StopwatchFragment::updateFab(ImageView& fab, bool animate) {
    if (DataModel::getDataModel().getStopwatch().isRunning()) {
        fab.setImageResource(animate ? R::drawable::ic_play_pause_animation
                                     : R::drawable::ic_play_pause);
        fab.setContentDescription(fab.getContext()->getResources().getString(R::string::sw_pause_button));
    } else {
        fab.setImageResource(animate ? R::drawable::ic_pause_play_animation
                                     : R::drawable::ic_pause_play);
        fab.setContentDescription(fab.getContext()->getResources().getString(R::string::sw_start_button));
    }
    fab.setVisibility(View::VISIBLE);
}

void StopwatchFragment::onUpdateFab(ImageView& fab) {
    updateFab(fab, false);
}

void StopwatchFragment::onMorphFab(ImageView& fab) {
    updateFab(fab, true /* isNOrLater */);
    AnimatorUtils::startDrawableAnimation(fab);
}

void StopwatchFragment::onUpdateFabButtons(Button& left, Button& right) {
    Resources& resources = getContext()->getResources();
    left.setClickable(true);
    left.setText(R::string::sw_reset_button);
    left.setContentDescription(resources.getString(R::string::sw_reset_button));

    switch (DataModel::getDataModel().getStopwatch().state) {
        case data::Stopwatch::State::RESET:
            left.setVisibility(View::INVISIBLE);
            right.setClickable(true);
            right.setVisibility(View::INVISIBLE);
            break;
        case data::Stopwatch::State::RUNNING: {
            left.setVisibility(View::VISIBLE);
            const bool canRecordLaps = DataModel::getDataModel().canAddMoreLaps();
            right.setText(R::string::sw_lap_button);
            right.setContentDescription(resources.getString(R::string::sw_lap_button));
            right.setClickable(canRecordLaps);
            right.setVisibility(canRecordLaps ? View::VISIBLE : View::INVISIBLE);
            break;
        }
        case data::Stopwatch::State::PAUSED:
            left.setVisibility(View::VISIBLE);
            right.setClickable(true);
            right.setVisibility(View::VISIBLE);
            right.setText(R::string::sw_share_button);
            right.setContentDescription(resources.getString(R::string::sw_share_button));
            break;
    }
}

void StopwatchFragment::doStart() {
    DataModel::getDataModel().startStopwatch();
}

void StopwatchFragment::doPause() {
    DataModel::getDataModel().pauseStopwatch();
}

void StopwatchFragment::doReset() {
    const data::Stopwatch::State priorState = DataModel::getDataModel().getStopwatch().state;
    DataModel::getDataModel().resetStopwatch();
    mMainTimeText->setAlpha(1.0f);
    mHundredthsTimeText->setAlpha(1.0f);
    if (priorState == data::Stopwatch::State::RUNNING) {
        updateFab(FabContainer::FAB_MORPH);
    }
}

void StopwatchFragment::doShare() {
    // No system share targets on cdroid; the share text remains available via
    // LapsAdapter.getShareText().
    updateUI(FabContainer::BUTTONS_IMMEDIATE);
}

void StopwatchFragment::doAddLap() {
    data::Lap lap;
    if (!mLapsAdapter->addLap(lap)) {
        return;
    }

    updateUI(FabContainer::BUTTONS_IMMEDIATE);
    if (lap.lapNumber == 1) {
        mLapsList->removeAllViewsInLayout();
        if (mTime != nullptr) {
            mTime->update();
        }

        showOrHideLaps(false);
    }

    mLapsList->scrollToPosition(0);
}

void StopwatchFragment::showOrHideLaps(bool clearLaps) {
    ViewGroup* sceneRoot = getView() ? (ViewGroup*) getView() : nullptr;
    if (sceneRoot == nullptr) return;

    TransitionManager::beginDelayedTransition(sceneRoot);

    if (clearLaps) {
        mLapsAdapter->clearLaps();
    }

    const bool lapsVisible = mLapsAdapter->getItemCount() > 0;
    mLapsList->setVisibility(lapsVisible ? View::VISIBLE : View::GONE);

    if (Utils::isLandscape(*getContext())) {
        return; // (Upstream applies the padding shift only in portrait.)
    }
    Resources& res = getContext()->getResources();
    const int bottom = lapsVisible ? 0 : (int) res.getDimension(R::dimen::fab_height);
    const int top = sceneRoot->getPaddingTop();
    const int left = sceneRoot->getPaddingLeft();
    const int right = sceneRoot->getPaddingRight();
    sceneRoot->setPadding(left, top, right, bottom);
}

void StopwatchFragment::toggleStopwatchState() {
    if (DataModel::getDataModel().getStopwatch().isRunning()) {
        doPause();
    } else {
        doStart();
    }
}

void StopwatchFragment::startUpdatingTime() {
    stopUpdatingTime();
    mMainTimeText->post(mTimeUpdateRunnable);
}

void StopwatchFragment::stopUpdatingTime() {
    mMainTimeText->removeCallbacks(mTimeUpdateRunnable);
}

void StopwatchFragment::updateTime() {
    const data::Stopwatch& stopwatch = DataModel::getDataModel().getStopwatch();
    const int64_t totalTime = stopwatch.getTotalTime();
    mStopwatchTextController->setTimeString(totalTime);
    const bool currentLapIsVisible = mLapsLayoutManager->findFirstVisibleItemPosition() == 0;
    if (!stopwatch.isReset() && currentLapIsVisible) {
        mLapsAdapter->updateCurrentLap(*mLapsList, totalTime);
    }
}

void StopwatchFragment::updateUI(int updateTypes) {
    updateTime();
    if (mTime != nullptr) {
        mTime->update();
    }
    const data::Stopwatch& stopwatch = DataModel::getDataModel().getStopwatch();
    if (!stopwatch.isReset()) {
        startUpdatingTime();
    }

    showOrHideLaps(stopwatch.isReset());

    updateFab(updateTypes);
}

} // namespace stopwatch

// Registered under the bare name the UiDataModel STOPWATCH tab references.
static const int _cdroid_frag_reg_stopwatch_real =
    (::cdroid::FragmentFactory::registerFragment("StopwatchFragment",
        []() -> ::cdroid::Fragment* {
            return new ::cdroid::deskclock::stopwatch::StopwatchFragment();
        }), 0);

} // namespace deskclock
} // namespace cdroid
