#include <app/espresso/viewactions.h>

#include <cctype>
#include <mutex>

#include <app/espresso/espressoexception.h>
#include <app/espresso/generallocation.h>
#include <app/espresso/humanreadables.h>
#include <app/espresso/motionevents.h>
#include <app/espresso/press.h>
#include <app/espresso/swipe.h>
#include <app/espresso/tap.h>
#include <app/espresso/uicontroller.h>
#include <app/espresso/viewassertion.h>
#include <app/espresso/viewmatchers.h>

#include <core/inputmethodmanager.h>
#include <core/rect.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <view/keyevent.h>
#include <view/view.h>
#include <view/viewconfiguration.h>
#include <widget/edittext.h>
#include <widget/horizontalscrollview.h>
#include <widget/listview.h>
#include <widget/scrollview.h>
#include <widget/textview.h>

namespace cdroid {
namespace espresso {

static std::string toLower(const std::string& s) {
    std::string out = s;
    for (char& c : out) c = (char)tolower((unsigned char)c);
    return out;
}

/* ---------------- GeneralClickAction ---------------- */

GeneralClickAction::GeneralClickAction(TapperPtr tapper,
        CoordinatesProviderPtr coordinatesProvider, PrecisionDescriberPtr precisionDescriber)
    : GeneralClickAction(std::move(tapper), std::move(coordinatesProvider),
            std::move(precisionDescriber), nullptr) {}

GeneralClickAction::GeneralClickAction(TapperPtr tapper,
        CoordinatesProviderPtr coordinatesProvider, PrecisionDescriberPtr precisionDescriber,
        ViewActionPtr rollbackAction)
    : mCoordinatesProvider(std::move(coordinatesProvider)),
      mTapper(std::move(tapper)),
      mPrecisionDescriber(std::move(precisionDescriber)),
      mRollbackAction(std::move(rollbackAction)) {}

MatcherPtr<View> GeneralClickAction::getConstraints() {
    MatcherPtr<View> standardConstraint = ViewMatchers::isDisplayingAtLeast(90);
    if (mRollbackAction) {
        return allOf(standardConstraint, mRollbackAction->getConstraints());
    }
    return standardConstraint;
}

void GeneralClickAction::perform(UiController& uiController, View& view) {
    FloatCoord coordinates = mCoordinatesProvider->calculateCoordinates(view);
    FloatCoord precision = mPrecisionDescriber->describePrecision();

    Tapper::Status status = Tapper::Status::FAILURE;
    int loopCount = 0;
    // Native event injection is quite a tricky process. A tap is actually 2
    // separate motion events which need to get injected into the system...
    // (see GeneralClickAction.java for the full AOSP comment)
    // If you have a separate long press behaviour from your short press, you
    // can pass in a 'RollBack' ViewAction which when executed will undo the
    // effects of long press.
    while (status != Tapper::Status::SUCCESS && loopCount < 3) {
        try {
            status = mTapper->sendTap(uiController, coordinates, precision);
        } catch (std::runtime_error& re) {
            throw PerformException::Builder()
                    .withActionDescription(getDescription())
                    .withViewDescription(HumanReadables::describe(&view))
                    .withCause(std::current_exception())
                    .build();
        }

        const int duration = ViewConfiguration::getPressedStateDuration();
        // ensures that all work enqueued to process the tap has been run.
        if (duration > 0) {
            uiController.loopMainThreadForAtLeast(duration);
        }
        if (status == Tapper::Status::WARNING) {
            if (mRollbackAction) {
                mRollbackAction->perform(uiController, view);
            } else {
                break;
            }
        }
        loopCount++;
    }
    if (status == Tapper::Status::FAILURE) {
        char detail[512];
        snprintf(detail, sizeof(detail),
                "Couldn't click at: %f,%f precision: %f, %f . Tapper: %s coordinate provider: "
                "%p precision describer: %p. Tried %d times. With Rollback? %s",
                coordinates[0], coordinates[1], precision[0], precision[1],
                mTapper->toString().c_str(), (void*)mCoordinatesProvider.get(),
                (void*)mPrecisionDescriber.get(), loopCount, mRollbackAction ? "yes" : "no");
        throw PerformException::Builder()
                .withActionDescription(getDescription())
                .withViewDescription(HumanReadables::describe(&view))
                .withCause(std::make_exception_ptr(std::runtime_error(detail)))
                .build();
    }
    // AOSP waits the double-tap timeout for WebView targets; CDROID has no
    // WebView port.
}

std::string GeneralClickAction::getDescription() {
    return toLower(mTapper->toString()) + " click";
}

/* ---------------- GeneralSwipeAction ---------------- */

GeneralSwipeAction::GeneralSwipeAction(SwiperPtr swiper,
        CoordinatesProviderPtr startCoordinatesProvider,
        CoordinatesProviderPtr endCoordinatesProvider,
        PrecisionDescriberPtr precisionDescriber)
    : mStartCoordinatesProvider(std::move(startCoordinatesProvider)),
      mEndCoordinatesProvider(std::move(endCoordinatesProvider)),
      mSwiper(std::move(swiper)),
      mPrecisionDescriber(std::move(precisionDescriber)) {}

MatcherPtr<View> GeneralSwipeAction::getConstraints() {
    return ViewMatchers::isDisplayingAtLeast(VIEW_DISPLAY_PERCENTAGE);
}

void GeneralSwipeAction::perform(UiController& uiController, View& view) {
    FloatCoord startCoordinates = mStartCoordinatesProvider->calculateCoordinates(view);
    FloatCoord endCoordinates = mEndCoordinatesProvider->calculateCoordinates(view);
    FloatCoord precision = mPrecisionDescriber->describePrecision();

    Swiper::Status status = Swiper::Status::FAILURE;
    for (int tries = 0; tries < MAX_TRIES && status != Swiper::Status::SUCCESS; tries++) {
        try {
            status = mSwiper->sendSwipe(uiController, startCoordinates, endCoordinates, precision);
        } catch (std::runtime_error& re) {
            throw PerformException::Builder()
                    .withActionDescription(getDescription())
                    .withViewDescription(HumanReadables::describe(&view))
                    .withCause(std::current_exception())
                    .build();
        }

        const int duration = ViewConfiguration::getPressedStateDuration();
        // ensures that all work enqueued to process the swipe has been run.
        if (duration > 0) {
            uiController.loopMainThreadForAtLeast(duration);
        }
    }

    if (status == Swiper::Status::FAILURE) {
        char detail[512];
        snprintf(detail, sizeof(detail),
                "Couldn't swipe from: %f,%f to: %f,%f precision: %f, %f . Swiper: %s "
                "start coordinate provider: %p precision describer: %p. Tried %d times",
                startCoordinates[0], startCoordinates[1], endCoordinates[0], endCoordinates[1],
                precision[0], precision[1], mSwiper->toString().c_str(),
                (void*)mStartCoordinatesProvider.get(), (void*)mPrecisionDescriber.get(),
                MAX_TRIES);
        throw PerformException::Builder()
                .withActionDescription(getDescription())
                .withViewDescription(HumanReadables::describe(&view))
                .withCause(std::make_exception_ptr(std::runtime_error(detail)))
                .build();
    }
}

std::string GeneralSwipeAction::getDescription() {
    return toLower(mSwiper->toString()) + " swipe";
}

/* ---------------- TypeTextAction ---------------- */

TypeTextAction::TypeTextAction(const std::string& stringToBeTyped)
    : TypeTextAction(stringToBeTyped, true) {}

TypeTextAction::TypeTextAction(const std::string& stringToBeTyped, bool tapToFocus)
    : mStringToBeTyped(stringToBeTyped), mTapToFocus(tapToFocus) {}

MatcherPtr<View> TypeTextAction::getConstraints() {
    // AOSP adds anyOf(supportsInputMethods(), isAssignableFrom(SearchView))
    // — SearchView is not ported to CDROID, so the input-methods check stands.
    if (!mTapToFocus) {
        return allOf(ViewMatchers::isDisplayed(), ViewMatchers::hasFocus(),
                ViewMatchers::supportsInputMethods());
    }
    return allOf(ViewMatchers::isDisplayed(), ViewMatchers::supportsInputMethods());
}

void TypeTextAction::perform(UiController& uiController, View& view) {
    // No-op if string is empty.
    if (mStringToBeTyped.empty()) {
        LOGW("Supplied string is empty resulting in no-op (nothing is typed).");
        return;
    }

    if (mTapToFocus) {
        // Perform a click.
        GeneralClickAction(Tap::SINGLE(), GeneralLocation::CENTER(), Press::FINGER())
                .perform(uiController, view);
        uiController.loopMainThreadUntilIdle();
    }

    // InjectEventSecurityException would wrap here; the in-process seam
    // cannot throw it.
    if (!uiController.injectString(mStringToBeTyped)) {
        LOGE("Failed to type text: %s", mStringToBeTyped.c_str());
        throw PerformException::Builder()
                .withActionDescription(getDescription())
                .withViewDescription(HumanReadables::describe(&view))
                .withCause(std::make_exception_ptr(
                        std::runtime_error("Failed to type text: " + mStringToBeTyped)))
                .build();
    }
}

std::string TypeTextAction::getDescription() {
    return "type text(" + mStringToBeTyped + ")";
}

/* ---------------- ReplaceTextAction ---------------- */

ReplaceTextAction::ReplaceTextAction(const std::string& stringToBeSet)
    : mStringToBeSet(stringToBeSet) {}

MatcherPtr<View> ReplaceTextAction::getConstraints() {
    return allOf(ViewMatchers::isDisplayed(), ViewMatchers::isAssignableFrom<EditText>());
}

void ReplaceTextAction::perform(UiController& /*uiController*/, View& view) {
    static_cast<EditText&>(view).setText(mStringToBeSet);
}

std::string ReplaceTextAction::getDescription() {
    return "replace text";
}

/* ---------------- ScrollToAction ---------------- */

MatcherPtr<View> ScrollToAction::getConstraints() {
    return allOf(ViewMatchers::withEffectiveVisibility(ViewMatchers::Visibility::VISIBLE),
            ViewMatchers::isDescendantOfA(anyOf(ViewMatchers::isAssignableFrom<ScrollView>(),
                    ViewMatchers::isAssignableFrom<HorizontalScrollView>(),
                    ViewMatchers::isAssignableFrom<ListView>())));
}

void ScrollToAction::perform(UiController& uiController, View& view) {
    if (ViewMatchers::isDisplayingAtLeast(90)->matches(view)) {
        LOGI("View is already displayed. Returning.");
        return;
    }
    Rect rect;
    view.getDrawingRect(rect);
    if (!view.requestRectangleOnScreen(rect, true /* immediate */)) {
        LOGW("Scrolling to view was requested, but none of the parents scrolled.");
    }
    uiController.loopMainThreadUntilIdle();
    if (!ViewMatchers::isDisplayingAtLeast(90)->matches(view)) {
        throw PerformException::Builder()
                .withActionDescription(getDescription())
                .withViewDescription(HumanReadables::describe(&view))
                .withCause(std::make_exception_ptr(std::runtime_error(
                        "Scrolling to view was attempted, but the view is not displayed")))
                .build();
    }
}

std::string ScrollToAction::getDescription() {
    return "scroll to";
}

/* ---------------- KeyEventAction ---------------- */

KeyEventAction::KeyEventAction(EspressoKey key) : mKey(std::move(key)) {}

MatcherPtr<View> KeyEventAction::getConstraints() {
    return ViewMatchers::isDisplayed();
}

void KeyEventAction::perform(UiController& uiController, View& view) {
    try {
        if (!sendKeyEvent(uiController)) {
            LOGE("Failed to inject key event: %s", mKey.toString().c_str());
            throw PerformException::Builder()
                    .withActionDescription(getDescription())
                    .withViewDescription(HumanReadables::describe(&view))
                    .withCause(std::make_exception_ptr(
                            std::runtime_error("Failed to inject key event " + mKey.toString())))
                    .build();
        }
    } catch (InjectEventSecurityException& e) {
        LOGE("Failed to inject key event: %s", mKey.toString().c_str());
        throw PerformException::Builder()
                .withActionDescription(getDescription())
                .withViewDescription(HumanReadables::describe(&view))
                .withCause(std::current_exception())
                .build();
    }
}

bool KeyEventAction::sendKeyEvent(UiController& controller) {
    // AOSP snapshots the current Activity and consults the
    // ActivityLifecycleMonitor after BACK (stage transitions, pending
    // foreground activities, NoActivityResumedException). CDROID has no
    // Activity stack; the settle delay below keeps the transition window
    // open for the window stack to pop.
    bool injected = false;
    int64_t eventTime = SystemClock::uptimeMillis();
    for (int attempts = 0; !injected && attempts < 4; attempts++) {
        KeyEvent* event = KeyEvent::obtain(eventTime, eventTime, KeyEvent::ACTION_DOWN,
                mKey.getKeyCode(), 0 /* repeat */, mKey.getMetaState(), 0 /* deviceId */,
                0 /* scancode */, 0 /* flags */, 0 /* source */);
        injected = controller.injectKeyEvent(*event);
        event->recycle();
    }

    if (!injected) {
        // it is not a transient failure... :(
        return false;
    }

    injected = false;
    eventTime = SystemClock::uptimeMillis();
    for (int attempts = 0; !injected && attempts < 4; attempts++) {
        KeyEvent* event = KeyEvent::obtain(eventTime, eventTime, KeyEvent::ACTION_UP,
                mKey.getKeyCode(), 0 /* repeat */, 0 /* metaState */, 0 /* deviceId */,
                0 /* scancode */, 0 /* flags */, 0 /* source */);
        injected = controller.injectKeyEvent(*event);
        event->recycle();
    }

    if (mKey.getKeyCode() == KeyEvent::KEYCODE_BACK) {
        // Wait for the back navigation to settle (AOSP waits for the
        // Activity stage change; the window stack settles here instead).
        controller.loopMainThreadForAtLeast(BACK_ACTIVITY_TRANSITION_MILLIS_DELAY);
        controller.loopMainThreadUntilIdle();
    }

    return injected;
}

std::string KeyEventAction::getDescription() {
    return "send " + mKey.toString() + " key event";
}

/* ---------------- CloseKeyboardAction ---------------- */

MatcherPtr<View> CloseKeyboardAction::getConstraints() {
    return allOf(ViewMatchers::isDisplayed(), ViewMatchers::supportsInputMethods());
}

void CloseKeyboardAction::perform(UiController& uiController, View& view) {
    // AOSP: imm.hideSoftInputFromWindow(view.getWindowToken(), 0); CDROID's
    // IMM keys the request on the view (no window tokens).
    InputMethodManager::getInstance().hideSoftInputFromWindow(&view, 0);
    uiController.loopMainThreadUntilIdle();
}

std::string CloseKeyboardAction::getDescription() {
    return "close soft keyboard";
}

/* ---------------- ViewActions ---------------- */

ViewActionPtr ViewActions::click() {
    return std::make_shared<GeneralClickAction>(Tap::SINGLE(),
            GeneralLocation::VISIBLE_CENTER(), Press::FINGER());
}

ViewActionPtr ViewActions::click(ViewActionPtr rollbackAction) {
    return std::make_shared<GeneralClickAction>(Tap::SINGLE(),
            GeneralLocation::CENTER(), Press::FINGER(), std::move(rollbackAction));
}

ViewActionPtr ViewActions::doubleClick() {
    return std::make_shared<GeneralClickAction>(Tap::DOUBLE(),
            GeneralLocation::VISIBLE_CENTER(), Press::FINGER());
}

ViewActionPtr ViewActions::longClick() {
    return std::make_shared<GeneralClickAction>(Tap::LONG(),
            GeneralLocation::VISIBLE_CENTER(), Press::FINGER());
}

ViewActionPtr ViewActions::pressBack() {
    return pressKey(KeyEvent::KEYCODE_BACK);
}

ViewActionPtr ViewActions::pressMenuKey() {
    return pressKey(KeyEvent::KEYCODE_MENU);
}

ViewActionPtr ViewActions::pressKey(int keyCode) {
    return pressKey(EspressoKey::Builder().withKeyCode(keyCode).build());
}

ViewActionPtr ViewActions::pressKey(const EspressoKey& key) {
    return std::make_shared<KeyEventAction>(key);
}

ViewActionPtr ViewActions::closeSoftKeyboard() {
    return std::make_shared<CloseKeyboardAction>();
}

ViewActionPtr ViewActions::scrollTo() {
    return std::make_shared<ScrollToAction>();
}

ViewActionPtr ViewActions::swipeLeft() {
    return std::make_shared<GeneralSwipeAction>(Swipe::FAST(),
            GeneralLocation::translate(GeneralLocation::CENTER_RIGHT(), -EDGE_FUZZ_FACTOR, 0),
            GeneralLocation::translate(GeneralLocation::CENTER_LEFT(), EDGE_FUZZ_FACTOR, 0),
            Press::FINGER());
}

ViewActionPtr ViewActions::swipeRight() {
    return std::make_shared<GeneralSwipeAction>(Swipe::FAST(),
            GeneralLocation::translate(GeneralLocation::CENTER_LEFT(), EDGE_FUZZ_FACTOR, 0),
            GeneralLocation::translate(GeneralLocation::CENTER_RIGHT(), -EDGE_FUZZ_FACTOR, 0),
            Press::FINGER());
}

ViewActionPtr ViewActions::swipeUp() {
    return std::make_shared<GeneralSwipeAction>(Swipe::FAST(),
            GeneralLocation::translate(GeneralLocation::BOTTOM_CENTER(), 0, -EDGE_FUZZ_FACTOR),
            GeneralLocation::translate(GeneralLocation::TOP_CENTER(), 0, EDGE_FUZZ_FACTOR),
            Press::FINGER());
}

ViewActionPtr ViewActions::swipeDown() {
    return std::make_shared<GeneralSwipeAction>(Swipe::FAST(),
            GeneralLocation::translate(GeneralLocation::TOP_CENTER(), 0, EDGE_FUZZ_FACTOR),
            GeneralLocation::translate(GeneralLocation::BOTTOM_CENTER(), 0, -EDGE_FUZZ_FACTOR),
            Press::FINGER());
}

ViewActionPtr ViewActions::typeText(const std::string& stringToBeTyped) {
    return std::make_shared<TypeTextAction>(stringToBeTyped);
}

ViewActionPtr ViewActions::typeTextIntoFocusedView(const std::string& stringToBeTyped) {
    return std::make_shared<TypeTextAction>(stringToBeTyped, false);
}

ViewActionPtr ViewActions::replaceText(const std::string& stringToBeSet) {
    return std::make_shared<ReplaceTextAction>(stringToBeSet);
}

ViewActionPtr ViewActions::clearText() {
    return std::make_shared<ReplaceTextAction>("");
}

/* Global assertions (AOSP ViewActions static Set<ViewAssertion>). */
static std::vector<ViewAssertionPtr>& globalAssertions() {
    static std::vector<ViewAssertionPtr> assertions;
    return assertions;
}
static std::mutex& globalAssertionsLock() {
    static std::mutex mutex;
    return mutex;
}

void ViewActions::addGlobalAssertion(ViewAssertionPtr viewAssertion) {
    std::lock_guard<std::mutex> lock(globalAssertionsLock());
    globalAssertions().push_back(std::move(viewAssertion));
}

void ViewActions::removeGlobalAssertion(ViewAssertionPtr viewAssertion) {
    std::lock_guard<std::mutex> lock(globalAssertionsLock());
    auto& assertions = globalAssertions();
    for (size_t i = 0; i < assertions.size(); i++) {
        if (assertions[i] == viewAssertion) {
            assertions.erase(assertions.begin() + i);
            return;
        }
    }
}

void ViewActions::clearGlobalAssertions() {
    std::lock_guard<std::mutex> lock(globalAssertionsLock());
    globalAssertions().clear();
}

ViewActionPtr ViewActions::actionWithAssertions(ViewActionPtr viewAction) {
    class ActionWithAssertions : public ViewAction {
    public:
        explicit ActionWithAssertions(ViewActionPtr action) : mAction(std::move(action)) {}
        MatcherPtr<View> getConstraints() override { return mAction->getConstraints(); }
        void perform(UiController& uiController, View& view) override {
            std::vector<ViewAssertionPtr> assertions;
            {
                std::lock_guard<std::mutex> lock(globalAssertionsLock());
                assertions = globalAssertions();
            }
            for (const ViewAssertionPtr& assertion : assertions) {
                assertion->check(&view, nullptr);
            }
            mAction->perform(uiController, view);
        }
        std::string getDescription() override { return mAction->getDescription(); }
    private:
        ViewActionPtr mAction;
    };
    return std::make_shared<ActionWithAssertions>(std::move(viewAction));
}

/* EspressoKey — definition out of line for the Builder friendship. */

EspressoKey::EspressoKey(const Builder& builder)
    : mKeyCode(builder.mBuilderKeyCode), mMetaState(builder.getMetaState()) {}

EspressoKey EspressoKey::Builder::build() {
    // AOSP also checks against KeyEvent.getMaxKeyCode() — not ported; the
    // positive-code lower bound is kept.
    if (mBuilderKeyCode <= 0) {
        throw std::runtime_error("Invalid key code: " + std::to_string(mBuilderKeyCode));
    }
    return EspressoKey(*this);
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
