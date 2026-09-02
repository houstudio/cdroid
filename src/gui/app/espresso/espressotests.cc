#include <app/espresso/espressotests.h>

#include <cstdio>
#include <exception>

namespace cdroid {
namespace espresso {

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
