#ifndef CDROID_ESPRESSO_VIEWMATCHERS_H
#define CDROID_ESPRESSO_VIEWMATCHERS_H

/*
 * android.support.test.espresso.matcher.ViewMatchers — a collection of
 * hamcrest matchers that match Views.
 *
 * CDROID deviations (APIs not ported — see the .cc):
 *  - withSpinnerText / hasErrorText / hasLinks / isJavascriptEnabled /
 *    hasBackground / hasTextColor: dropped (no WebView, no getError/getUrls,
 *    adapter items are void* with no toString contract);
 *  - supportsInputMethods: AOSP probes onCreateInputConnection; CDROID has
 *    no InputConnection surface, so the matcher recognizes TextViews (the
 *    only text editors);
 *  - withText(int resId) resolves through App::getString.
 */

#include <string>

#include <view/view.h>

#include <app/espresso/hamcrest.h>

namespace cdroid {
namespace espresso {

/** Demangled class name of the view's dynamic type (Java getClass().getName()). */
std::string getViewClassName(const View& view);

class ViewMatchers {
public:
    /** Java Matcher<View>. */
    using Ptr = MatcherPtr<View>;

    /** Matches views whose dynamic type is (a subclass of) S. */
    template<typename S>
    static Ptr isAssignableFrom();

    static Ptr withClassName(MatcherPtr<std::string> classNameMatcher);

    /**
     * Matches a View if it is displayed on the screen — visible rect
     * intersects the screen AND every ancestor is VISIBLE.
     */
    static Ptr isDisplayed();

    /** Matches a View that is fully displayed on the screen (100%). */
    static Ptr isCompletelyDisplayed();

    /**
     * Matches a View if it is displayed with at least the given percentage
     * of its view's area visible to the user (screen-relative).
     */
    static Ptr isDisplayingAtLeast(int areaPercentage);

    static Ptr isEnabled();
    static Ptr isFocusable();
    static Ptr hasFocus();

    /** CDROID: protected View::hasWindowFocus probed via the window stack. */
    static Ptr hasWindowFocus();

    static Ptr isSelected();
    static Ptr isClickable();
    static Ptr isRoot();

    /** Matches a view whose sibling matches the given matcher. */
    static Ptr hasSibling(Ptr siblingMatcher);

    /**
     * Matches a view with the given content description — string form
     * (CDROID View::getContentDescription returns std::string).
     */
    static Ptr withContentDescription(const std::string& text);
    static Ptr withContentDescription(MatcherPtr<std::string> charSequenceMatcher);

    static Ptr withId(int id);

    /** Matches a view with the given key tagged on it (View.setTag(int,Object)). */
    static Ptr withTagKey(int key);

    /** Matches a view with the given tag value (View.setTag(Object)). */
    static Ptr withTagValue(MatcherPtr<void*> tagValueMatcher);

    static Ptr withText(const std::string& text);
    static Ptr withText(MatcherPtr<std::string> stringMatcher);
    static Ptr withText(int resourceId);

    static Ptr withHint(const std::string& hintText);
    static Ptr withHint(MatcherPtr<std::string> stringMatcher);

    /** AOSP uses BoundedMatcher<View, Checkable>; runtime narrowing via dynamic_cast. */
    static Ptr isChecked();
    static Ptr isNotChecked();

    /** Matches a view whose content description is non-empty. */
    static Ptr hasContentDescription();

    /** Matches a view whose descendants match the given matcher. */
    static Ptr hasDescendant(Ptr matcher);

    /** Matches a view which is a descendant of a view matching the given matcher. */
    static Ptr isDescendantOfA(Ptr matcher);

    /** AOSP ViewMatchers.Visibility. */
    enum class Visibility { VISIBLE, INVISIBLE, GONE };

    /**
     * Matches a view whose effective visibility is the given value — VISIBLE
     * requires the view AND all its ancestors visible, INVISIBLE/GONE match
     * when any view on the ancestor chain carries that visibility.
     */
    static Ptr withEffectiveVisibility(Visibility visibility);

    static Ptr withParent(Ptr parentMatcher);
    static Ptr withChild(Ptr childMatcher);

    /**
     * Matches views that support input methods — in CDROID, TextViews
     * (AOSP probes a non-null onCreateInputConnection).
     */
    static Ptr supportsInputMethods();

    /** Matches a TextView whose imeOptions action matches. */
    static Ptr hasImeAction(int imeAction);

    /** Matches a TextView with the given input type. */
    static Ptr withInputType(int inputType);

    /**
     * AOSP ViewMatchers.assertThat(T, Matcher<T>, String) — assertion with a
     * HumanReadables description of the failing view.
     */
    static void assertThat(View* actual, const Matcher<View>& matcher,
            const std::string& checkDescription);
};

template<typename S>
ViewMatchers::Ptr ViewMatchers::isAssignableFrom() {
    class IsAssignableFrom : public TypeSafeMatcher<View> {
    public:
        bool matchesSafely(const View& view) const override {
            return dynamic_cast<const S*>(&view) != nullptr;
        }
        void describeTo(Description& description) const override {
            description.appendText("is assignable from class ").appendText(typeid(S).name());
        }
    };
    return std::make_shared<IsAssignableFrom>();
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_VIEWMATCHERS_H*/
