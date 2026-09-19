#ifndef CDROID_ESPRESSO_VIEWACTIONS_H
#define CDROID_ESPRESSO_VIEWACTIONS_H

/*
 * android.support.test.espresso.ViewActions + the concrete action classes of
 * the android.support.test.espresso.action package (GeneralClickAction,
 * GeneralSwipeAction, TypeTextAction, ReplaceTextAction, ScrollToAction,
 * KeyEventAction, CloseKeyboardAction).
 *
 * CDROID omissions: pressImeActionButton (TextView has the listener surface
 * but no public onEditorAction(int) trigger), openLink* (no URLSpan
 * input-connection path).
 */

#include <string>
#include <vector>

#include <app/espresso/actioninterfaces.h>
#include <app/espresso/espressokey.h>
#include <app/espresso/viewaction.h>
#include <app/espresso/viewassertion.h>

namespace cdroid {
class View;

namespace espresso {

class UiController;

/* ---------------- GeneralClickAction ---------------- */

/** Enables clicking on views. */
class GeneralClickAction : public ViewAction {
public:
    GeneralClickAction(TapperPtr tapper, CoordinatesProviderPtr coordinatesProvider,
            PrecisionDescriberPtr precisionDescriber);
    GeneralClickAction(TapperPtr tapper, CoordinatesProviderPtr coordinatesProvider,
            PrecisionDescriberPtr precisionDescriber, ViewActionPtr rollbackAction);

    MatcherPtr<View> getConstraints() override;
    void perform(UiController& uiController, View& view) override;
    std::string getDescription() override;

private:
    CoordinatesProviderPtr mCoordinatesProvider;
    TapperPtr mTapper;
    PrecisionDescriberPtr mPrecisionDescriber;
    ViewActionPtr mRollbackAction;   // Java Optional — null when absent
};

/* ---------------- GeneralSwipeAction ---------------- */

/** Enables swiping across a view. */
class GeneralSwipeAction : public ViewAction {
public:
    GeneralSwipeAction(SwiperPtr swiper, CoordinatesProviderPtr startCoordinatesProvider,
            CoordinatesProviderPtr endCoordinatesProvider,
            PrecisionDescriberPtr precisionDescriber);

    MatcherPtr<View> getConstraints() override;
    void perform(UiController& uiController, View& view) override;
    std::string getDescription() override;

private:
    static constexpr int MAX_TRIES = 3;
    static constexpr int VIEW_DISPLAY_PERCENTAGE = 90;

    CoordinatesProviderPtr mStartCoordinatesProvider;
    CoordinatesProviderPtr mEndCoordinatesProvider;
    SwiperPtr mSwiper;
    PrecisionDescriberPtr mPrecisionDescriber;
};

/* ---------------- TypeTextAction ---------------- */

/** Enables typing text on views. */
class TypeTextAction : public ViewAction {
public:
    /** By default a tap is sent to the center of the view to attain focus. */
    explicit TypeTextAction(const std::string& stringToBeTyped);
    TypeTextAction(const std::string& stringToBeTyped, bool tapToFocus);

    MatcherPtr<View> getConstraints() override;
    void perform(UiController& uiController, View& view) override;
    std::string getDescription() override;

private:
    std::string mStringToBeTyped;
    bool mTapToFocus;
};

/* ---------------- ReplaceTextAction ---------------- */

/** Replaces view text by setting EditText's text property to given String. */
class ReplaceTextAction : public ViewAction {
public:
    explicit ReplaceTextAction(const std::string& stringToBeSet);

    MatcherPtr<View> getConstraints() override;
    void perform(UiController& uiController, View& view) override;
    std::string getDescription() override;

private:
    std::string mStringToBeSet;
};

/* ---------------- ScrollToAction ---------------- */

/** Enables scrolling to the given view (must be a descendant of a scrollable). */
class ScrollToAction : public ViewAction {
public:
    MatcherPtr<View> getConstraints() override;
    void perform(UiController& uiController, View& view) override;
    std::string getDescription() override;
};

/* ---------------- KeyEventAction ---------------- */

/** Enables pressing KeyEvents on views. */
class KeyEventAction : public ViewAction {
public:
    static constexpr int BACK_ACTIVITY_TRANSITION_MILLIS_DELAY = 150;

    explicit KeyEventAction(EspressoKey key);

    MatcherPtr<View> getConstraints() override;
    void perform(UiController& uiController, View& view) override;
    std::string getDescription() override;

private:
    bool sendKeyEvent(UiController& controller);
    EspressoKey mKey;
};

/* ---------------- CloseKeyboardAction ---------------- */

/** Closes soft keyboard. */
class CloseKeyboardAction : public ViewAction {
public:
    MatcherPtr<View> getConstraints() override;
    void perform(UiController& uiController, View& view) override;
    std::string getDescription() override;
};

/* ---------------- ViewActions (statics) ---------------- */

/** A collection of common actions. */
class ViewActions {
public:
    static ViewActionPtr click();
    static ViewActionPtr click(ViewActionPtr rollbackAction);
    static ViewActionPtr doubleClick();
    static ViewActionPtr longClick();
    static ViewActionPtr pressBack();
    static ViewActionPtr pressMenuKey();
    static ViewActionPtr pressKey(int keyCode);
    static ViewActionPtr pressKey(const EspressoKey& key);
    static ViewActionPtr closeSoftKeyboard();
    /** Scrolls to the view, if it is not already visible. */
    static ViewActionPtr scrollTo();
    static ViewActionPtr swipeLeft();
    static ViewActionPtr swipeRight();
    static ViewActionPtr swipeUp();
    static ViewActionPtr swipeDown();

    /**
     * Types the given string into the target view. If the string is empty it
     * results in no-op (nothing is typed). Default: taps to focus first.
     */
    static ViewActionPtr typeText(const std::string& stringToBeTyped);
    static ViewActionPtr typeTextIntoFocusedView(const std::string& stringToBeTyped);
    static ViewActionPtr replaceText(const std::string& stringToBeSet);
    /** Clears the text in the target view. */
    static ViewActionPtr clearText();

    /** AOSP EDGE_FUZZ_FACTOR for the swipe borders. */
    static constexpr float EDGE_FUZZ_FACTOR = 0.083f;

    /* Global assertions: applied by actionWithAssertions around every action. */
    static void addGlobalAssertion(ViewAssertionPtr viewAssertion);
    static void removeGlobalAssertion(ViewAssertionPtr viewAssertion);
    static void clearGlobalAssertions();
    /** Wraps the given action, running the global assertions first. */
    static ViewActionPtr actionWithAssertions(ViewActionPtr viewAction);
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_VIEWACTIONS_H*/
