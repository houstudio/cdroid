#ifndef CDROID_ESPRESSO_ESPRESSOEXCEPTION_H
#define CDROID_ESPRESSO_ESPRESSOEXCEPTION_H

/*
 * The android.support.test.espresso exception family, as thrown by the
 * framework (plus junit.framework.AssertionFailedError which DefaultFailureHandler
 * discriminates on). Java exceptions map onto std::runtime_error subclasses;
 * Throwable cause maps onto std::exception_ptr (rethrowable).
 */

#include <exception>
#include <stdexcept>
#include <string>

#include <app/espresso/hamcrest.h>

namespace cdroid {
class View;

namespace espresso {

/** android.support.test.espresso.EspressoException — marker interface → base class. */
class EspressoException : public std::runtime_error {
public:
    explicit EspressoException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * android.support.test.espresso.PerformException — indicates that a
 * ViewAction failed.
 */
class PerformException : public EspressoException {
public:
    class Builder;
    explicit PerformException(const std::string& message) : EspressoException(message) {}

    std::string getActionDescription() const { return mActionDescription; }
    std::string getViewDescription() const { return mViewDescription; }

private:
    friend class Builder;
    std::string mActionDescription;
    std::string mViewDescription;
    std::exception_ptr mCause;
};

class PerformException::Builder {
public:
    Builder& withActionDescription(const std::string& actionDescription) {
        mActionDescription = actionDescription;
        return *this;
    }
    Builder& withViewDescription(const std::string& viewDescription) {
        mViewDescription = viewDescription;
        return *this;
    }
    Builder& withCause(std::exception_ptr cause) {
        mCause = std::move(cause);
        return *this;
    }
    Builder& from(const PerformException& other) {
        mActionDescription = other.mActionDescription;
        mViewDescription = other.mViewDescription;
        mCause = other.mCause;
        return *this;
    }
    PerformException build();

private:
    std::string mActionDescription;
    std::string mViewDescription;
    std::exception_ptr mCause;
};

/**
 * android.support.test.espresso.InjectEventSecurityException — an unchecked
 * security exception thrown during a motion/key event injection.
 */
class InjectEventSecurityException : public std::runtime_error {
public:
    explicit InjectEventSecurityException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * android.support.test.espresso.NoMatchingViewException — indicates that the
 * given view matcher did not match any view in the view hierarchy.
 */
class NoMatchingViewException : public EspressoException {
public:
    class Builder;
    explicit NoMatchingViewException(const std::string& message) : EspressoException(message) {}

private:
    friend class Builder;
    MatcherPtr<View> mViewMatcher;   // matcher that failed to match
    std::string mViewHierarchy;
};

class NoMatchingViewException::Builder {
public:
    Builder& withViewMatcher(MatcherPtr<View> viewMatcher) {
        mViewMatcher = std::move(viewMatcher);
        return *this;
    }
    Builder& withViewHierarchy(const std::string& viewHierarchy) {
        mViewHierarchy = viewHierarchy;
        return *this;
    }
    NoMatchingViewException build();

private:
    MatcherPtr<View> mViewMatcher;
    std::string mViewHierarchy;
};

/**
 * android.support.test.espresso.AmbiguousViewMatcherException — indicates the
 * view matcher matched multiple views in the hierarchy.
 */
class AmbiguousViewMatcherException : public EspressoException {
public:
    class Builder;
    explicit AmbiguousViewMatcherException(const std::string& message) : EspressoException(message) {}

private:
    friend class Builder;
};

class AmbiguousViewMatcherException::Builder {
public:
    Builder& withViewMatcher(MatcherPtr<View> viewMatcher) {
        mViewMatcher = std::move(viewMatcher);
        return *this;
    }
    Builder& withView1(View* view1) { mView1 = view1; return *this; }
    Builder& withView2(View* view2) { mView2 = view2; return *this; }
    Builder& withRootView(View* rootView) { mRootView = rootView; return *this; }
    AmbiguousViewMatcherException build();

private:
    MatcherPtr<View> mViewMatcher;
    View* mView1 = nullptr;
    View* mView2 = nullptr;
    View* mRootView = nullptr;
};

/**
 * android.support.test.espresso.NoMatchingRootException — no root matched the
 * given root matcher.
 */
class NoMatchingRootException : public EspressoException {
public:
    explicit NoMatchingRootException(const std::string& message) : EspressoException(message) {}
};

/**
 * android.support.test.espresso.NoActivityResumedException — kept for API
 * parity; CDROID has no Activity lifecycle, KeyEventAction never throws it.
 */
class NoActivityResumedException : public EspressoException {
public:
    explicit NoActivityResumedException(const std::string& message) : EspressoException(message) {}
};

/**
 * android.support.test.espresso.AppNotIdleException — the master idling
 * policy timed out while loopUntil was pumping the main thread.
 */
class AppNotIdleException : public EspressoException {
public:
    explicit AppNotIdleException(const std::string& message) : EspressoException(message) {}
};

/**
 * android.support.test.espresso.IdlingResourceTimeoutException — dynamic
 * idling resources did not idle within their policy timeout.
 */
class IdlingResourceTimeoutException : public EspressoException {
public:
    explicit IdlingResourceTimeoutException(const std::string& message) : EspressoException(message) {}
};

/** junit.framework.AssertionFailedError. */
class AssertionFailedError : public std::runtime_error {
public:
    explicit AssertionFailedError(const std::string& message)
        : std::runtime_error(message), mCause(nullptr) {}
    AssertionFailedError(const std::string& message, std::exception_ptr cause)
        : std::runtime_error(message), mCause(std::move(cause)) {}

    /** Throwable.getCause() — null (empty) when absent. */
    std::exception_ptr getCause() const { return mCause; }

private:
    std::exception_ptr mCause;
};

/** Declared in hamcrest.h; junit Assert.assertThat throws AssertionFailedError. */
inline void throwAssertionFailedError(const std::string& message) {
    throw AssertionFailedError(message);
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ESPRESSOEXCEPTION_H*/
