#include <app/espresso/viewmatchers.h>

#include <typeinfo>
#include <cxxabi.h>

#include <app/espresso/espressoexception.h>
#include <app/espresso/hamcrest.h>
#include <app/espresso/humanreadables.h>
#include <app/espresso/rootmatchers.h>
#include <app/espresso/treeiterables.h>

#include <core/app.h>
#include <core/rect.h>
#include <core/windowmanager.h>
#include <widget/checkable.h>
#include <widget/editorinfo.h>
#include <widget/textview.h>

namespace cdroid {
namespace espresso {

std::string getViewClassName(const View& view) {
    const char* mangled = typeid(view).name();
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    std::string name = (status == 0 && demangled != nullptr) ? demangled : mangled;
    free(demangled);
    return name;
}

namespace {

/** Common base for the anonymous AOSP matchers below. */
class ViewMatcher : public TypeSafeMatcher<View> {
public:
    explicit ViewMatcher(std::string description) : mDescription(std::move(description)) {}
    void describeTo(Description& description) const override {
        description.appendText(mDescription);
    }
private:
    std::string mDescription;
};

/** composes "(description + child)" descriptions for the parameterized ones. */
std::string describeWith(const std::string& prefix, const SelfDescribing& inner) {
    StringDescription description;
    description.appendText(prefix).appendDescriptionOf(inner);
    return description.str();
}

} // namespace

ViewMatchers::Ptr ViewMatchers::withClassName(MatcherPtr<std::string> classNameMatcher) {
    class WithClassName : public ViewMatcher {
    public:
        explicit WithClassName(MatcherPtr<std::string> matcher, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(matcher)) {}
        bool matchesSafely(const View& view) const override {
            return mMatcher->matches(getViewClassName(view));
        }
    private:
        MatcherPtr<std::string> mMatcher;
    };
    return std::make_shared<WithClassName>(std::move(classNameMatcher),
            describeWith("with class name ", *classNameMatcher));
}

ViewMatchers::Ptr ViewMatchers::isDisplayed() {
    class IsDisplayed : public ViewMatcher {
    public:
        IsDisplayed() : ViewMatcher("is displayed on the screen to the user") {}
        bool matchesSafely(const View& view) const override {
            Rect visibleRect;
            // AOSP: view.getGlobalVisibleRect(new Rect()) && withEffectiveVisibility(VISIBLE)
            // (CDROID View::getGlobalVisibleRect is non-const — const_cast bridges the port gap)
            View& v = const_cast<View&>(view);
            return v.getGlobalVisibleRect(visibleRect, nullptr)
                    && withEffectiveVisibility(Visibility::VISIBLE)->matches(v);
        }
    };
    return std::make_shared<IsDisplayed>();
}

ViewMatchers::Ptr ViewMatchers::isCompletelyDisplayed() {
    return isDisplayingAtLeast(100);
}

ViewMatchers::Ptr ViewMatchers::isDisplayingAtLeast(int areaPercentage) {
    class IsDisplayingAtLeast : public ViewMatcher {
    public:
        explicit IsDisplayingAtLeast(int areaPercentage)
            : ViewMatcher("at least " + std::to_string(areaPercentage)
                    + "% of the view's area is displayed to the user"),
              mAreaPercentage(areaPercentage) {}
        bool matchesSafely(const View& view) const override {
            Rect visibleRect;
            View& v = const_cast<View&>(view);
            const bool visibleAtLeast = v.getGlobalVisibleRect(visibleRect, nullptr);
            // AOSP: maxArea is the VIEW's own area (width*height), not the
            // display — the matcher asks how much of the view is on screen.
            const int64_t visibleViewArea =
                    (int64_t)visibleRect.width * (int64_t)visibleRect.height;
            const int64_t maxArea =
                    (int64_t)v.getWidth() * (int64_t)v.getHeight();
            return visibleAtLeast && (visibleViewArea * 100 >= maxArea * mAreaPercentage);
        }
    private:
        int mAreaPercentage;
    };
    return std::make_shared<IsDisplayingAtLeast>(areaPercentage);
}

ViewMatchers::Ptr ViewMatchers::isEnabled() {
    class IsEnabled : public ViewMatcher {
    public:
        IsEnabled() : ViewMatcher("view is enabled") {}
        bool matchesSafely(const View& view) const override { return view.isEnabled(); }
    };
    return std::make_shared<IsEnabled>();
}

ViewMatchers::Ptr ViewMatchers::isFocusable() {
    class IsFocusable : public ViewMatcher {
    public:
        IsFocusable() : ViewMatcher("view is focusable") {}
        bool matchesSafely(const View& view) const override { return view.isFocusable(); }
    };
    return std::make_shared<IsFocusable>();
}

ViewMatchers::Ptr ViewMatchers::hasFocus() {
    class HasFocus : public ViewMatcher {
    public:
        HasFocus() : ViewMatcher("view has focus") {}
        bool matchesSafely(const View& view) const override { return view.hasFocus(); }
    };
    return std::make_shared<HasFocus>();
}

ViewMatchers::Ptr ViewMatchers::hasWindowFocus() {
    class HasWindowFocus : public ViewMatcher {
    public:
        HasWindowFocus() : ViewMatcher("view has window focus") {}
        bool matchesSafely(const View& view) const override {
            return viewHasWindowFocus(const_cast<View*>(&view));
        }
    };
    return std::make_shared<HasWindowFocus>();
}

ViewMatchers::Ptr ViewMatchers::isSelected() {
    class IsSelected : public ViewMatcher {
    public:
        IsSelected() : ViewMatcher("view is selected") {}
        bool matchesSafely(const View& view) const override { return view.isSelected(); }
    };
    return std::make_shared<IsSelected>();
}

ViewMatchers::Ptr ViewMatchers::isClickable() {
    class IsClickable : public ViewMatcher {
    public:
        IsClickable() : ViewMatcher("view is clickable") {}
        bool matchesSafely(const View& view) const override { return view.isClickable(); }
    };
    return std::make_shared<IsClickable>();
}

ViewMatchers::Ptr ViewMatchers::isRoot() {
    class IsRoot : public ViewMatcher {
    public:
        IsRoot() : ViewMatcher("view is a root") {}
        bool matchesSafely(const View& view) const override {
            return view.getRootView() == &view;
        }
    };
    return std::make_shared<IsRoot>();
}

ViewMatchers::Ptr ViewMatchers::hasSibling(Ptr siblingMatcher) {
    class HasSibling : public ViewMatcher {
    public:
        explicit HasSibling(Ptr matcher, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(matcher)) {}
        bool matchesSafely(const View& view) const override {
            ViewGroup* parent = view.getParent();
            if (parent == nullptr) return false;
            for (int i = 0; i < parent->getChildCount(); i++) {
                View* child = parent->getChildAt(i);
                if (child == &view) continue;
                if (mMatcher->matches(*child)) return true;
            }
            return false;
        }
    private:
        Ptr mMatcher;
    };
    return std::make_shared<HasSibling>(std::move(siblingMatcher),
            describeWith("has sibling ", *siblingMatcher));
}

ViewMatchers::Ptr ViewMatchers::withContentDescription(const std::string& text) {
    return withContentDescription(is(text));
}

ViewMatchers::Ptr ViewMatchers::withContentDescription(MatcherPtr<std::string> charSequenceMatcher) {
    class WithContentDescription : public ViewMatcher {
    public:
        explicit WithContentDescription(MatcherPtr<std::string> matcher, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(matcher)) {}
        bool matchesSafely(const View& view) const override {
            return mMatcher->matches(view.getContentDescription());
        }
    private:
        MatcherPtr<std::string> mMatcher;
    };
    return std::make_shared<WithContentDescription>(std::move(charSequenceMatcher),
            describeWith("with content description ", *charSequenceMatcher));
}

ViewMatchers::Ptr ViewMatchers::withId(int id) {
    class WithId : public ViewMatcher {
    public:
        explicit WithId(int id) : ViewMatcher("with id: " + std::to_string(id)), mId(id) {}
        bool matchesSafely(const View& view) const override { return mId == view.getId(); }
    private:
        int mId;
    };
    return std::make_shared<WithId>(id);
}

ViewMatchers::Ptr ViewMatchers::withTagKey(int key) {
    class WithTagKey : public ViewMatcher {
    public:
        explicit WithTagKey(int key)
            : ViewMatcher("with key tagged " + std::to_string(key)), mKey(key) {}
        bool matchesSafely(const View& view) const override {
            // AOSP: view.getTag(key) != null via reflection; keyed tags can
            // legitimately store null values, CDROID keys on presence.
            return view.getTag(mKey) != nullptr;
        }
    private:
        int mKey;
    };
    return std::make_shared<WithTagKey>(key);
}

ViewMatchers::Ptr ViewMatchers::withTagValue(MatcherPtr<void*> tagValueMatcher) {
    class WithTagValue : public ViewMatcher {
    public:
        explicit WithTagValue(MatcherPtr<void*> matcher, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(matcher)) {}
        bool matchesSafely(const View& view) const override {
            void* tag = view.getTag();
            return mMatcher->matches(tag);
        }
    private:
        MatcherPtr<void*> mMatcher;
    };
    return std::make_shared<WithTagValue>(std::move(tagValueMatcher),
            describeWith("with tag value ", *tagValueMatcher));
}

ViewMatchers::Ptr ViewMatchers::withText(const std::string& text) {
    return withText(is(text));
}

ViewMatchers::Ptr ViewMatchers::withText(MatcherPtr<std::string> stringMatcher) {
    class WithText : public ViewMatcher {
    public:
        explicit WithText(MatcherPtr<std::string> matcher, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(matcher)) {}
        bool matchesSafely(const View& view) const override {
            TextView* textView = dynamic_cast<TextView*>(const_cast<View*>(&view));
            return textView != nullptr && mMatcher->matches(std::string(textView->getText()));
        }
    private:
        MatcherPtr<std::string> mMatcher;
    };
    return std::make_shared<WithText>(std::move(stringMatcher),
            describeWith("with text ", *stringMatcher));
}

ViewMatchers::Ptr ViewMatchers::withText(int resourceId) {
    return withText(App::getInstance().getString(resourceId));
}

ViewMatchers::Ptr ViewMatchers::withHint(const std::string& hintText) {
    return withHint(is(hintText));
}

ViewMatchers::Ptr ViewMatchers::withHint(MatcherPtr<std::string> stringMatcher) {
    class WithHint : public ViewMatcher {
    public:
        explicit WithHint(MatcherPtr<std::string> matcher, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(matcher)) {}
        bool matchesSafely(const View& view) const override {
            const TextView* textView = dynamic_cast<const TextView*>(&view);
            if (textView == nullptr) return false;
            CharSequence* hint = textView->getHint();
            return hint != nullptr && mMatcher->matches(std::string(*hint));
        }
    private:
        MatcherPtr<std::string> mMatcher;
    };
    return std::make_shared<WithHint>(std::move(stringMatcher),
            describeWith("with hint ", *stringMatcher));
}

ViewMatchers::Ptr ViewMatchers::isChecked() {
    class IsChecked : public ViewMatcher {
    public:
        IsChecked() : ViewMatcher("view is checked") {}
        bool matchesSafely(const View& view) const override {
            const Checkable* checkable = dynamic_cast<const Checkable*>(&view);
            return checkable != nullptr && checkable->isChecked();
        }
    };
    return std::make_shared<IsChecked>();
}

ViewMatchers::Ptr ViewMatchers::isNotChecked() {
    return not_(isChecked());
}

ViewMatchers::Ptr ViewMatchers::hasContentDescription() {
    class HasContentDescription : public ViewMatcher {
    public:
        HasContentDescription() : ViewMatcher("has content description") {}
        bool matchesSafely(const View& view) const override {
            return !view.getContentDescription().empty();
        }
    };
    return std::make_shared<HasContentDescription>();
}

ViewMatchers::Ptr ViewMatchers::hasDescendant(Ptr matcher) {
    class HasDescendant : public ViewMatcher {
    public:
        explicit HasDescendant(Ptr m, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(m)) {}
        bool matchesSafely(const View& view) const override {
            // AOSP: breadthFirstViewTraversal(view).skip(1) — every view but the root.
            const std::vector<View*> traversal = breadthFirstViewTraversal(
                    const_cast<View*>(&view));
            for (size_t i = 1; i < traversal.size(); i++) {
                if (mMatcher->matches(*traversal[i])) return true;
            }
            return false;
        }
    private:
        Ptr mMatcher;
    };
    return std::make_shared<HasDescendant>(std::move(matcher),
            describeWith("has descendant ", *matcher));
}

ViewMatchers::Ptr ViewMatchers::isDescendantOfA(Ptr matcher) {
    class IsDescendantOfA : public ViewMatcher {
    public:
        explicit IsDescendantOfA(Ptr m, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(m)) {}
        bool matchesSafely(const View& view) const override {
            return checkAncestors(view);
        }
    private:
        bool checkAncestors(const View& view) const {
            for (View* parent = view.getParent(); parent != nullptr; parent = parent->getParent()) {
                if (mMatcher->matches(*parent)) return true;
            }
            return false;
        }
        Ptr mMatcher;
    };
    return std::make_shared<IsDescendantOfA>(std::move(matcher),
            describeWith("is descendant of a ", *matcher));
}

ViewMatchers::Ptr ViewMatchers::withEffectiveVisibility(Visibility visibility) {
    class WithEffectiveVisibility : public ViewMatcher {
    public:
        explicit WithEffectiveVisibility(Visibility visibility)
            : ViewMatcher(visibilityName(visibility)), mVisibility(visibility) {}
        bool matchesSafely(const View& view) const override {
            if (mVisibility == Visibility::VISIBLE) {
                // every ancestor (and the view itself) must be VISIBLE
                if (view.getVisibility() != View::VISIBLE) return false;
                for (View* parent = view.getParent(); parent != nullptr; parent = parent->getParent()) {
                    if (parent->getVisibility() != View::VISIBLE) return false;
                }
                return true;
            }
            const int wanted = (mVisibility == Visibility::INVISIBLE)
                    ? View::INVISIBLE : View::GONE;
            if (view.getVisibility() == wanted) return true;
            for (View* parent = view.getParent(); parent != nullptr; parent = parent->getParent()) {
                if (parent->getVisibility() == wanted) return true;
            }
            return false;
        }
    private:
        static std::string visibilityName(Visibility visibility) {
            switch (visibility) {
                case Visibility::VISIBLE: return "view has effective visibility VISIBLE";
                case Visibility::INVISIBLE: return "view has effective visibility INVISIBLE";
                case Visibility::GONE: return "view has effective visibility GONE";
            }
            return "view has effective visibility";
        }
        Visibility mVisibility;
    };
    return std::make_shared<WithEffectiveVisibility>(visibility);
}

ViewMatchers::Ptr ViewMatchers::withParent(Ptr parentMatcher) {
    class WithParent : public ViewMatcher {
    public:
        explicit WithParent(Ptr matcher, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(matcher)) {}
        bool matchesSafely(const View& view) const override {
            ViewGroup* parent = view.getParent();
            return parent != nullptr && mMatcher->matches(*parent);
        }
    private:
        Ptr mMatcher;
    };
    return std::make_shared<WithParent>(std::move(parentMatcher),
            describeWith("with parent ", *parentMatcher));
}

ViewMatchers::Ptr ViewMatchers::withChild(Ptr childMatcher) {
    class WithChild : public ViewMatcher {
    public:
        explicit WithChild(Ptr matcher, std::string description)
            : ViewMatcher(std::move(description)), mMatcher(std::move(matcher)) {}
        bool matchesSafely(const View& view) const override {
            const ViewGroup* group = dynamic_cast<const ViewGroup*>(&view);
            if (group == nullptr) return false;
            for (int i = 0; i < group->getChildCount(); i++) {
                if (mMatcher->matches(*group->getChildAt(i))) return true;
            }
            return false;
        }
    private:
        Ptr mMatcher;
    };
    return std::make_shared<WithChild>(std::move(childMatcher),
            describeWith("with child ", *childMatcher));
}

ViewMatchers::Ptr ViewMatchers::supportsInputMethods() {
    class SupportsInputMethods : public ViewMatcher {
    public:
        SupportsInputMethods() : ViewMatcher("supports input methods") {}
        bool matchesSafely(const View& view) const override {
            // AOSP: view.onCreateInputConnection(new EditorInfo()) != null.
            // The InputConnection surface is unported; TextViews are the
            // views that take text input.
            return dynamic_cast<const TextView*>(&view) != nullptr;
        }
    };
    return std::make_shared<SupportsInputMethods>();
}

ViewMatchers::Ptr ViewMatchers::hasImeAction(int imeAction) {
    class HasImeAction : public ViewMatcher {
    public:
        explicit HasImeAction(int imeAction)
            : ViewMatcher("has ime action: " + std::to_string(imeAction)), mImeAction(imeAction) {}
        bool matchesSafely(const View& view) const override {
            const TextView* textView = dynamic_cast<const TextView*>(&view);
            if (textView == nullptr) return false;
            const int imeOptions = textView->getImeOptions();
            return (imeOptions & EditorInfo::IME_MASK_ACTION) == mImeAction;
        }
    private:
        int mImeAction;
    };
    return std::make_shared<HasImeAction>(imeAction);
}

ViewMatchers::Ptr ViewMatchers::withInputType(int inputType) {
    class WithInputType : public ViewMatcher {
    public:
        explicit WithInputType(int inputType)
            : ViewMatcher("with input type: " + std::to_string(inputType)),
              mInputType(inputType) {}
        bool matchesSafely(const View& view) const override {
            const TextView* textView = dynamic_cast<const TextView*>(&view);
            return textView != nullptr && textView->getInputType() == mInputType;
        }
    private:
        int mInputType;
    };
    return std::make_shared<WithInputType>(inputType);
}

void ViewMatchers::assertThat(View* actual, const Matcher<View>& matcher,
        const std::string& checkDescription) {
    if (!matcher.matches(*actual)) {
        StringDescription description;
        description.appendText("Check: ").appendText(checkDescription)
                .appendText("\nExpected: ").appendDescriptionOf(matcher)
                .appendText("\nActual view: ").appendText(HumanReadables::describe(actual));
        throw AssertionFailedError(description.str());
    }
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
