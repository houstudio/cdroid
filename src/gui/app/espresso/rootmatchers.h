#ifndef CDROID_ESPRESSO_ROOTMATCHERS_H
#define CDROID_ESPRESSO_ROOTMATCHERS_H

/*
 * android.support.test.espresso.RootMatchers — a collection of matchers for
 * Root objects.
 *
 * CDROID adaptations (no binder tokens / no Activity stack):
 *  - isSubwindowOfCurrentActivity(): in-process there is exactly one
 *    application, so every root IS a subwindow of "the current activity" —
 *    the matcher matches unconditionally (kept so the DEFAULT composition
    *    stays AOSP-shaped);
 *  - isFocusable(): AOSP reads LayoutParams.FLAG_NOT_FOCUSABLE — kept
 *    verbatim; CDROID windows do not set the flag today, so it reduces to
 *    true unless an app opts in;
 *  - isDialog(): AOSP compares window/app tokens; the port recognizes an
 *    application-layer window (Window::window_type mirrored into
 *    LayoutParams.type) that is not the active application window.
 */

#include <app/espresso/hamcrest.h>
#include <app/espresso/root.h>

namespace cdroid {
namespace espresso {

/** Matcher<Root> — the AOSP generic is Matcher<? super Root>. */
using RootMatcherPtr = MatcherPtr<Root>;

/**
 * CDROID stand-in for the protected View::hasWindowFocus: a view's window
 * has focus iff its root window is the WindowManager's active window.
 */
bool viewHasWindowFocus(View* view);

class RootMatchers {
public:
    /**
     * Matches roots (usually windows) which can receive input and can be
     * touched. In almost all cases this will be what you want to use.
     */
    static RootMatcherPtr DEFAULT();

    static RootMatcherPtr isFocusable();

    static RootMatcherPtr hasWindowLayoutParams();

    static RootMatcherPtr isTouchable();

    static RootMatcherPtr isDialog();

    static RootMatcherPtr isPlatformPopup();

    /**
     * Matches Roots that hold the DecorView which the given view matcher
     * matches.
     */
    static RootMatcherPtr withDecorView(MatcherPtr<View> decorViewMatcher);

    /**
     * Matches Roots that are sub-windows of the current Activity —
     * unconditional in the single-process CDROID model (see file header).
     */
    static RootMatcherPtr isSubwindowOfCurrentActivity();
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ROOTMATCHERS_H*/
