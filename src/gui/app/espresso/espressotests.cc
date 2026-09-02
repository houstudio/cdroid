#include <app/espresso/espressotests.h>

#include <app/espresso/espressoexception.h>

#include <cstdio>
#include <exception>

namespace cdroid {
namespace espresso {

/*Print the nested cause chain: Espresso wraps failures (PerformException
  around the action's own exception), and the outermost what() alone hides
  the actual breakage.*/
static void printCauseChain(const std::exception& e) {
    const std::exception* current = &e;
    for (int depth = 0; depth < 4; depth++) {
        const PerformException* pe = dynamic_cast<const PerformException*>(current);
        const AssertionFailedError* ae = dynamic_cast<const AssertionFailedError*>(current);
        std::exception_ptr cause = pe ? pe->getCause()
                                      : (ae ? ae->getCause() : nullptr);
        if (!cause) return;
        try {
            std::rethrow_exception(cause);
        } catch (const std::exception& inner) {
            printf("      caused by: %s\n", inner.what());
            current = &inner;
        } catch (...) {
            printf("      caused by: (unknown)\n");
            return;
        }
    }
}

EspressoTestRegistry& EspressoTestRegistry::getInstance() {
    static EspressoTestRegistry instance;
    return instance;
}

void EspressoTestRegistry::addTest(const std::string& name, std::function<void()> fn) {
    mTests.push_back(Entry{name, std::move(fn)});
}

int EspressoTestRegistry::runAll() {
    int passed = 0;
    int failed = 0;
    printf("==== Espresso: running %d test(s) ====\n", count());
    for (const Entry& entry : mTests) {
        printf("---- TEST %s\n", entry.name.c_str());
        try {
            entry.fn();
            passed++;
            printf("PASS: %s\n", entry.name.c_str());
        } catch (const std::exception& e) {
            failed++;
            printf("FAIL: %s\n      %s\n", entry.name.c_str(), e.what());
            printCauseChain(e);
        } catch (...) {
            failed++;
            printf("FAIL: %s\n      (unknown exception)\n", entry.name.c_str());
        }
    }
    printf("==== Espresso: %d passed, %d failed ====\n", passed, failed);
    return passed;
}

} /*endof namespace espresso*/
} /*endof namespace cdroid*/
