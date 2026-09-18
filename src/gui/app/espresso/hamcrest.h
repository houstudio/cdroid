#ifndef CDROID_ESPRESSO_HAMCREST_H
#define CDROID_ESPRESSO_HAMCREST_H

/*
 * Minimal port of the org.hamcrest subset used by Espresso (hamcrest-core
 * 1.3, as vendored by android-support-test): SelfDescribing, Description,
 * Matcher, BaseMatcher, TypeSafeMatcher, BoundedMatcher, StringDescription
 * and the Matchers combinators the framework calls into (allOf/anyOf/not/is/
 * instanceOf/containsString/startsWith/endsWith/nullValue/notNullValue/any).
 *
 * Deviation from Java (forced by C++): there is no reflection and no universal
 * toString(), so Description.appendValue overloads known scalar/string types
 * and falls back to the type name for anything else; Java generics erasure is
 * replaced by the static template parameter T (runtime narrowing survives only
 * where Android itself narrows at runtime — BoundedMatcher<T,S>).
 *
 * Java GC ownership maps to std::shared_ptr: composed/stored matchers are held
 * by value through the MatcherPtr<T> typedef.
 */

#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

namespace cdroid {
namespace espresso {

class Description;
class StringDescription;

/** org.hamcrest.SelfDescribing. */
class SelfDescribing {
public:
    virtual ~SelfDescribing() = default;
    /** Generates a description of the object. */
    virtual void describeTo(Description& description) const = 0;
};

/**
 * org.hamcrest.Description — a description item in a mismatch report.
 * Fluent methods return *this (Java returns Description).
 */
class Description {
public:
    virtual ~Description() = default;

    /** Appends some plain text to the description. */
    virtual Description& appendText(const std::string& text) = 0;

    /** Appends description of a SelfDescribing value to the description. */
    virtual Description& appendDescriptionOf(const SelfDescribing& value) = 0;

    /** Appends an arbitrary value to the description (Java: appendValue(Object)). */
    virtual Description& appendValue(bool value) = 0;
    virtual Description& appendValue(int value) = 0;
    virtual Description& appendValue(int64_t value) = 0;
    virtual Description& appendValue(float value) = 0;
    virtual Description& appendValue(double value) = 0;
    virtual Description& appendValue(const char* value) = 0;
    virtual Description& appendValue(const std::string& value) = 0;

    /** Fallback for types without an overload: Java would call toString(). */
    template<typename T>
    Description& appendValue(const T& value);

    /** Java appendValueList(start, separator, end, T... values). */
    virtual Description& appendValueList(const std::string& start, const std::string& separator,
            const std::string& end, const std::vector<std::string>& values) = 0;
};

template<typename T>
Description& Description::appendValue(const T& value) {
    appendText("<");
    appendText(typeid(value).name());
    appendText(">");
    return *this;
}

// Overload resolution must treat std::string specially (template above would
// otherwise be an exact match for the const char*/std::string overloads is
// fine — declarations above are non-template, so they always win).

/** org.hamcrest.Matcher<T>. */
template<typename T>
class Matcher : public SelfDescribing {
public:
    /** Evaluates the matcher for argument item. */
    virtual bool matches(const T& item) const = 0;

    /**
     * Generates a description of why the matcher has not accepted the item.
     * Describes the item, not the matcher, e.g. "was <a dab hand at krilling>".
     */
    virtual void describeMismatch(const T& item, Description& mismatchDescription) const = 0;
};

template<typename T> using MatcherPtr = std::shared_ptr<Matcher<T>>;

/** org.hamcrest.BaseMatcher<T> — the recommended base class. */
template<typename T>
class BaseMatcher : public Matcher<T> {
public:
    // Java BaseMatcher: mismatchDescription.appendText("was ").appendValue(item);
    void describeMismatch(const T& item, Description& mismatchDescription) const override {
        mismatchDescription.appendText("was ").appendValue(item);
    }

    /** Java toString(): StringDescription.toString(this). */
    std::string toString() const;
};

/**
 * org.hamcrest.TypeSafeMatcher<T> — matchesSafely receives the statically
 * typed item. Java's reflective runtime-type check exists because of generics
 * erasure; with C++ templates the parameter type is already exact, so
 * matches() simply forwards (documented deviation, see file header).
 */
template<typename T>
class TypeSafeMatcher : public BaseMatcher<T> {
public:
    bool matches(const T& item) const final { return matchesSafely(item); }

protected:
    /** Subclasses should implement this to match the item. */
    virtual bool matchesSafely(const T& item) const = 0;

    /** Override to provide mismatch description; default defers to BaseMatcher. */
    virtual void describeMismatchSafely(const T& item, Description& mismatchDescription) const {
        BaseMatcher<T>::describeMismatch(item, mismatchDescription);
    }

public:
    void describeMismatch(const T& item, Description& mismatchDescription) const override {
        describeMismatchSafely(item, mismatchDescription);
    }
};

/**
 * org.hamcrest.BoundedMatcher<T, S extends T> — a matcher that only matches
 * items the runtime narrows to S (dynamic_cast replaces Java's Class.isInstance).
 * S must be a polymorphic type derived from T.
 */
template<typename T, typename S>
class BoundedMatcher : public BaseMatcher<T> {
    static_assert(std::is_base_of<T, S>::value, "BoundedMatcher requires S derived from T");
public:
    bool matches(const T& item) const final {
        const S* typed = dynamic_cast<const S*>(&item);
        return typed && matchesSafely(*typed);
    }

    void describeMismatch(const T& item, Description& mismatchDescription) const override {
        const S* typed = dynamic_cast<const S*>(&item);
        if (typed) {
            describeMismatchSafely(*typed, mismatchDescription);
        } else {
            mismatchDescription.appendText("was of wrong class ").appendText(typeid(item).name());
        }
    }

protected:
    virtual bool matchesSafely(const S& item) const = 0;
    virtual void describeMismatchSafely(const S& item, Description& mismatchDescription) const {
        BaseMatcher<T>::describeMismatch(item, mismatchDescription);
    }
};

/** org.hamcrest.StringDescription — a Description that logs to a std::string. */
class StringDescription : public Description {
public:
    /** Java StringDescription.toString(SelfDescribing). */
    static std::string toString(const SelfDescribing& selfDescribing);

    /** Java StringDescription.toString(SelfDescribing, String prefix). */
    static std::string toString(const std::string& prefix, const SelfDescribing& selfDescribing);

    Description& appendText(const std::string& text) override;
    Description& appendDescriptionOf(const SelfDescribing& value) override;
    Description& appendValue(bool value) override;
    Description& appendValue(int value) override;
    Description& appendValue(int64_t value) override;
    Description& appendValue(float value) override;
    Description& appendValue(double value) override;
    Description& appendValue(const char* value) override;
    Description& appendValue(const std::string& value) override;
    Description& appendValueList(const std::string& start, const std::string& separator,
            const std::string& end, const std::vector<std::string>& values) override;

    std::string str() const { return mOut; }

private:
    std::string mOut;
};

template<typename T>
std::string BaseMatcher<T>::toString() const {
    return StringDescription::toString(*this);
}

/* ------------------------------------------------------------------ */
/* org.hamcrest.Matchers — the combinators Espresso calls into.        */
/* ------------------------------------------------------------------ */

/** Matchers.allOf(Iterable<Matcher>) — evaluates to true only if ALL matchers agree. */
template<typename T>
class AllOf : public BaseMatcher<T> {
public:
    explicit AllOf(std::vector<MatcherPtr<T>> matchers) : mMatchers(std::move(matchers)) {}

    bool matches(const T& item) const override {
        for (const auto& m : mMatchers) {
            if (!m->matches(item)) return false;
        }
        return true;
    }

    void describeTo(Description& description) const override {
        description.appendText("(");
        bool first = true;
        for (const auto& m : mMatchers) {
            if (!first) description.appendText(" and ");
            first = false;
            description.appendDescriptionOf(*m);
        }
        description.appendText(")");
    }

    void describeMismatch(const T& item, Description& mismatchDescription) const override {
        // Java AllOf describes the first non-matching child.
        for (const auto& m : mMatchers) {
            if (!m->matches(item)) {
                m->describeMismatch(item, mismatchDescription);
                return;
            }
        }
        BaseMatcher<T>::describeMismatch(item, mismatchDescription);
    }

private:
    std::vector<MatcherPtr<T>> mMatchers;
};

/** Matchers.anyOf(Iterable<Matcher>) — true if ANY matcher agrees. */
template<typename T>
class AnyOf : public BaseMatcher<T> {
public:
    explicit AnyOf(std::vector<MatcherPtr<T>> matchers) : mMatchers(std::move(matchers)) {}

    bool matches(const T& item) const override {
        for (const auto& m : mMatchers) {
            if (m->matches(item)) return true;
        }
        return false;
    }

    void describeTo(Description& description) const override {
        description.appendText("(");
        bool first = true;
        for (const auto& m : mMatchers) {
            if (!first) description.appendText(" or ");
            first = false;
            description.appendDescriptionOf(*m);
        }
        description.appendText(")");
    }

    void describeMismatch(const T& item, Description& mismatchDescription) const override {
        for (const auto& m : mMatchers) {
            if (m->matches(item)) return;  // should not happen; defensive
        }
        mMatchers.empty() ? (void)0 : mMatchers.front()->describeMismatch(item, mismatchDescription);
    }

private:
    std::vector<MatcherPtr<T>> mMatchers;
};

/** Matchers.not(Matcher) — inverts the rule. */
template<typename T>
class IsNot : public BaseMatcher<T> {
public:
    explicit IsNot(MatcherPtr<T> matcher) : mMatcher(std::move(matcher)) {}

    bool matches(const T& item) const override { return !mMatcher->matches(item); }

    void describeTo(Description& description) const override {
        description.appendText("not ");
        description.appendDescriptionOf(*mMatcher);
    }

    void describeMismatch(const T& item, Description& mismatchDescription) const override {
        mMatcher->describeTo(mismatchDescription);  // Java IsNot describes the positive rule
    }

private:
    MatcherPtr<T> mMatcher;
};

/** Matchers.is(T value) via IsEqual — equality with operator==. */
template<typename T>
class IsEqual : public BaseMatcher<T> {
public:
    explicit IsEqual(T expected) : mExpected(std::move(expected)) {}

    bool matches(const T& item) const override { return item == mExpected; }

    void describeTo(Description& description) const override {
        description.appendValue(mExpected);
    }

    void describeMismatch(const T& item, Description& mismatchDescription) const override {
        mismatchDescription.appendText("was ").appendValue(item);
    }

private:
    T mExpected;
};

/** Matchers.instanceOf(Class) — runtime class narrowing (dynamic_cast). */
template<typename T, typename S>
class IsInstanceOf : public BaseMatcher<T> {
    static_assert(std::is_base_of<T, S>::value, "IsInstanceOf requires S derived from T");
public:
    bool matches(const T& item) const override {
        return dynamic_cast<const S*>(&item) != nullptr;
    }
    void describeTo(Description& description) const override {
        description.appendText("instanceOf ").appendText(typeid(S).name());
    }
};

/** Matchers.containsString(String). */
class StringContains : public BaseMatcher<std::string> {
public:
    explicit StringContains(std::string substring) : mSubstring(std::move(substring)) {}
    bool matches(const std::string& item) const override {
        return item.find(mSubstring) != std::string::npos;
    }
    void describeTo(Description& description) const override {
        description.appendText("a string containing ").appendValue(mSubstring);
    }
private:
    std::string mSubstring;
};

/** Matchers.startsWith(String). */
class StringStartsWith : public BaseMatcher<std::string> {
public:
    explicit StringStartsWith(std::string prefix) : mPrefix(std::move(prefix)) {}
    bool matches(const std::string& item) const override {
        return item.size() >= mPrefix.size() && item.compare(0, mPrefix.size(), mPrefix) == 0;
    }
    void describeTo(Description& description) const override {
        description.appendText("a string starting with ").appendValue(mPrefix);
    }
private:
    std::string mPrefix;
};

/** Matchers.endsWith(String). */
class StringEndsWith : public BaseMatcher<std::string> {
public:
    explicit StringEndsWith(std::string suffix) : mSuffix(std::move(suffix)) {}
    bool matches(const std::string& item) const override {
        return item.size() >= mSuffix.size()
            && item.compare(item.size() - mSuffix.size(), mSuffix.size(), mSuffix) == 0;
    }
    void describeTo(Description& description) const override {
        description.appendText("a string ending with ").appendValue(mSuffix);
    }
private:
    std::string mSuffix;
};

/* Factory functions — the org.hamcrest.Matchers static surface. */

template<typename T, typename... Rest>
MatcherPtr<T> allOf(MatcherPtr<T> first, Rest... rest) {
    std::vector<MatcherPtr<T>> matchers;
    collectMatchers(matchers, first, rest...);
    return std::make_shared<AllOf<T>>(std::move(matchers));
}

template<typename T, typename... Rest>
MatcherPtr<T> anyOf(MatcherPtr<T> first, Rest... rest) {
    std::vector<MatcherPtr<T>> matchers;
    collectMatchers(matchers, first, rest...);
    return std::make_shared<AnyOf<T>>(std::move(matchers));
}

template<typename T>
MatcherPtr<T> not_(MatcherPtr<T> matcher) {
    return std::make_shared<IsNot<T>>(std::move(matcher));
}

template<typename T>
MatcherPtr<T> is(T value) {
    return std::make_shared<IsEqual<T>>(std::move(value));
}

template<typename T>
MatcherPtr<T> is(MatcherPtr<T> matcher) {  // Java is(Matcher<T>) — decorates
    return std::move(matcher);
}

template<typename T, typename S>
MatcherPtr<T> instanceOf() {
    return std::make_shared<IsInstanceOf<T, S>>();
}

inline MatcherPtr<std::string> containsString(const std::string& substring) {
    return std::make_shared<StringContains>(substring);
}
inline MatcherPtr<std::string> startsWith(const std::string& prefix) {
    return std::make_shared<StringStartsWith>(prefix);
}
inline MatcherPtr<std::string> endsWith(const std::string& suffix) {
    return std::make_shared<StringEndsWith>(suffix);
}

/** Matchers.any — matches anything (Java: Matchers.anything()). */
template<typename T>
class AnyThing : public BaseMatcher<T> {
public:
    bool matches(const T& /*item*/) const override { return true; }
    void describeTo(Description& description) const override {
        description.appendText("ANYTHING");
    }
};

template<typename T>
MatcherPtr<T> anything() {
    return std::make_shared<AnyThing<T>>();
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

namespace cdroid {
namespace espresso {

namespace internal {
// Variadic collection helper (private detail of allOf/anyOf).
template<typename T>
void collectMatchersImpl(std::vector<MatcherPtr<T>>& out, MatcherPtr<T> m) {
    out.push_back(std::move(m));
}
template<typename T, typename... Rest>
void collectMatchersImpl(std::vector<MatcherPtr<T>>& out, MatcherPtr<T> first, Rest... rest) {
    out.push_back(std::move(first));
    collectMatchersImpl(out, rest...);
}
} // namespace internal

template<typename T, typename... Rest>
void collectMatchers(std::vector<MatcherPtr<T>>& out, MatcherPtr<T> first, Rest... rest) {
    internal::collectMatchersImpl(out, std::move(first), rest...);
}

/** junit.framework.Assert.assertThat(String, T, Matcher<T>). */
class AssertionFailedError;  // espressoexception.h

void throwAssertionFailedError(const std::string& message);  // throws AssertionFailedError

template<typename T>
void assertThat(const std::string& reason, const T& actual, const Matcher<T>& matcher) {
    if (!matcher.matches(actual)) {
        StringDescription description;
        description.appendText(reason)
                .appendText("\nExpected: ")
                .appendDescriptionOf(matcher)
                .appendText("\n     but: ");
        matcher.describeMismatch(actual, description);
        // AssertionFailedError is defined in espressoexception.h; the throw
        // lives there to keep this header free of junit dependencies.
        throwAssertionFailedError(description.str());
    }
}

template<typename T>
void assertThat(const T& actual, const Matcher<T>& matcher) {
    assertThat("", actual, matcher);
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_HAMCREST_H*/
