#ifndef CDROID_ESPRESSO_ESPRESSOTESTS_H
#define CDROID_ESPRESSO_ESPRESSOTESTS_H

/*
 * CDROID test loader — the in-process replacement for the JUnit/Instrumentation
 * runner that AOSP Espresso rides on (am instrument loads the test APK's
 * classes into the app process and drives JUnit on them). Here tests are
 * plain functions registered at static-init time — the same trick as the
 * DECLARE_WIDGET factory registry — and the runner is posted on the main
 * looper by the host app (see main.cc).
 *
 * A test body uses the Espresso API exactly as a Java test would:
 *   onView(withText("OK")).perform(click()).check(matches(isEnabled()));
 */

#include <functional>
#include <string>
#include <vector>

#include <porting/cdlog.h>

namespace cdroid {
namespace espresso {

class EspressoTestRegistry {
public:
    struct Entry {
        std::string name;
        std::function<void()> fn;
    };

    static EspressoTestRegistry& getInstance();

    void addTest(const std::string& name, std::function<void()> fn);

    int count() const { return (int)mTests.size(); }

    /**
     * Runs every registered test in order, on the calling (main) thread.
     * Each failure is caught and reported; one test cannot abort the run.
     * @return the number of passed tests.
     */
    int runAll();

private:
    EspressoTestRegistry() = default;
    std::vector<Entry> mTests;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

/**
 * Registers a test function with the in-process Espresso runner.
 *   static void testFoo() { onView(...).perform(...); }
 *   REGISTER_ESPRESSO_TEST("foo", testFoo)
 */
#define REGISTER_ESPRESSO_TEST(testName, testFn)                              \
    static const bool espresso_test_registered_##testFn =                     \
        (cdroid::espresso::EspressoTestRegistry::getInstance()                 \
                 .addTest(testName, testFn),                                  \
         true)

#endif /*CDROID_ESPRESSO_ESPRESSOTESTS_H*/
