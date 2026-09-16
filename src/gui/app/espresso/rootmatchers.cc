#include <app/espresso/rootmatchers.h>

#include <core/windowmanager.h>
#include <widget/cdwindow.h>

#include <app/espresso/viewmatchers.h>

namespace cdroid {
namespace espresso {

// AOSP WindowManager.LayoutParams flag bits — the canonical LayoutParams
// constants (the local FLAG_NOT_FOCUSABLE here used to carry the wrong value
// 0x2; AOSP's is 0x8, which popup computeFlags now stamps for !mFocusable).
static constexpr int FLAG_NOT_FOCUSABLE = WindowManager::LayoutParams::FLAG_NOT_FOCUSABLE;
static constexpr int FLAG_NOT_TOUCHABLE = WindowManager::LayoutParams::FLAG_NOT_TOUCHABLE;

// AOSP window-layer values (Window::window_type mirrors them into
// LayoutParams.type — see cdwindow.cc initWindow).
static constexpr int TYPE_BASE_APPLICATION = 1;
static constexpr int FIRST_APPLICATION_WINDOW = 1;
static constexpr int LAST_APPLICATION_WINDOW = 99;
static constexpr int FIRST_SUB_WINDOW = 1000;
static constexpr int LAST_SUB_WINDOW = 1999;

bool viewHasWindowFocus(View* view) {
    // AOSP View.hasWindowFocus (public there; protected in the CDROID port).
    // AOSP window focus stays on the application window while an IME is up
    // (input-method panels never take it), but the raw getActiveWindow flips
    // to the last-added window — an open keyboard made every app root report
    // unfocused and the RootViewPicker spun. Compare against the active
    // APPLICATION window instead, which system windows (IME among them)
    // cannot steal.
    if (view == nullptr || view->getRootView() == nullptr) return false;
    Window* window = dynamic_cast<Window*>(view->getRootView());
    return window != nullptr
            && window == WindowManager::getInstance().getActiveApplicationWindow();
}

namespace {

/** Local TypeSafeMatcher<Root> base for the anonymous AOSP classes below. */
class RootMatcher : public TypeSafeMatcher<Root> {
public:
    explicit RootMatcher(std::string description) : mDescription(std::move(description)) {}
    void describeTo(Description& description) const override {
        description.appendText(mDescription);
    }
private:
    std::string mDescription;
};

} // namespace

RootMatcherPtr RootMatchers::hasWindowLayoutParams() {
    class HasWindowLayoutParams : public RootMatcher {
    public:
        HasWindowLayoutParams() : RootMatcher("has window layout params") {}
        bool matchesSafely(const Root& root) const override {
            return root.getWindowLayoutParams() != nullptr;
        }
    };
    return std::make_shared<HasWindowLayoutParams>();
}

RootMatcherPtr RootMatchers::isFocusable() {
    class IsFocusable : public RootMatcher {
    public:
        IsFocusable() : RootMatcher("is focusable") {}
        bool matchesSafely(const Root& root) const override {
            if (root.getWindowLayoutParams() != nullptr) {
                return 0 == (root.getWindowLayoutParams()->flags & FLAG_NOT_FOCUSABLE);
            }
            return false;
        }
    };
    return std::make_shared<IsFocusable>();
}

RootMatcherPtr RootMatchers::isTouchable() {
    class IsTouchable : public RootMatcher {
    public:
        IsTouchable() : RootMatcher("is touchable") {}
        bool matchesSafely(const Root& root) const override {
            if (root.getWindowLayoutParams() != nullptr) {
                return 0 == (root.getWindowLayoutParams()->flags & FLAG_NOT_TOUCHABLE);
            }
            return false;
        }
    };
    return std::make_shared<IsTouchable>();
}

RootMatcherPtr RootMatchers::isDialog() {
    class IsDialog : public RootMatcher {
    public:
        IsDialog() : RootMatcher("is dialog") {}
        bool matchesSafely(const Root& root) const override {
            if (root.getWindowLayoutParams() == nullptr) return false;
            const int type = root.getWindowLayoutParams()->type;
            if (!(type != TYPE_BASE_APPLICATION && type <= LAST_APPLICATION_WINDOW)) {
                return false;
            }
            // AOSP distinguishes the activity's base window (token identity)
            // from dialogs; CDROID windows share TYPE_APPLICATION, so the
            // base window is recognized as the active application window.
            return root.getWindow() == nullptr
                    || root.getWindow() != WindowManager::getInstance().getActiveApplicationWindow();
        }
    };
    return std::make_shared<IsDialog>();
}

RootMatcherPtr RootMatchers::isPlatformPopup() {
    class IsPlatformPopup : public RootMatcher {
    public:
        IsPlatformPopup() : RootMatcher("is platform popup") {}
        bool matchesSafely(const Root& root) const override {
            // AOSP matches PopupWindow$PopupViewContainer by class name;
            // CDROID mirrors sub-window layering into LayoutParams.type
            // (no window occupies the range today — matcher kept dormant).
            return root.getWindowLayoutParams() != nullptr
                    && root.getWindowLayoutParams()->type >= FIRST_SUB_WINDOW
                    && root.getWindowLayoutParams()->type <= LAST_SUB_WINDOW;
        }
    };
    return std::make_shared<IsPlatformPopup>();
}

RootMatcherPtr RootMatchers::withDecorView(MatcherPtr<View> decorViewMatcher) {
    class WithDecorView : public RootMatcher {
    public:
        explicit WithDecorView(MatcherPtr<View> matcher, std::string description)
            : RootMatcher(std::move(description)), mMatcher(std::move(matcher)) {}
        bool matchesSafely(const Root& root) const override {
            return root.getDecorView() != nullptr && mMatcher->matches(*root.getDecorView());
        }
    private:
        MatcherPtr<View> mMatcher;
    };
    // AOSP: describeTo().appendText("with decor view ").appendDescriptionOf(matcher)
    // — composed once here since RootMatcher's describeTo is fixed-string.
    StringDescription described;
    described.appendText("with decor view ").appendDescriptionOf(*decorViewMatcher);
    return std::make_shared<WithDecorView>(std::move(decorViewMatcher), described.str());
}

RootMatcherPtr RootMatchers::isSubwindowOfCurrentActivity() {
    class IsSubwindow : public RootMatcher {
    public:
        IsSubwindow() : RootMatcher("is subwindow of current activity") {}
        bool matchesSafely(const Root& root) const override {
            // AOSP: the window's type falls in the application range or the
            // sub-window range of the resumed activity's token. The port was
            // unconditionally true ("single-process"), which let DEFAULT also
            // match system windows — an IME keyboard popping over a focused
            // EditText hijacked the root selection. Type ranges are the
            // CDROID equivalent of the token check.
            if (root.getWindowLayoutParams() == nullptr) return false;
            const int type = root.getWindowLayoutParams()->type;
            if (type >= FIRST_APPLICATION_WINDOW && type <= LAST_APPLICATION_WINDOW) {
                return true;
            }
            return type >= FIRST_SUB_WINDOW && type <= LAST_SUB_WINDOW;
        }
    };
    return std::make_shared<IsSubwindow>();
}

RootMatcherPtr RootMatchers::DEFAULT() {
    // AOSP DEFAULT:
    // allOf(hasWindowLayoutParams(), allOf(anyOf(
    //     allOf(isDialog(), withDecorView(hasWindowFocus())),
    //     isSubwindowOfCurrentActivity()), isFocusable()));
    return allOf(hasWindowLayoutParams(),
            allOf(anyOf(allOf(isDialog(), withDecorView(ViewMatchers::hasWindowFocus())),
                    isSubwindowOfCurrentActivity()),
            isFocusable()));
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
