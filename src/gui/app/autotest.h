#ifndef __CDROID_AUTOTEST_H__
#define __CDROID_AUTOTEST_H__
#include <string>
#include <vector>
#include <fstream>

namespace cdroid {

class AccessibilityNodeInfo;
class Window;

/**
 * App-level semantic UI sweep (--auto-test): a generic driver on top of
 * UiAutomation that walks the active window's node tree, activates every
 * on-screen clickable in reading order and verifies each activation produced
 * its TYPE_VIEW_CLICKED event. No per-app test code — any CDROID app runs
 * `./app --auto-test` (or AUTOTEST=1) to get a semantic, coordinate-free
 * smoke test that also exercises the whole a11y pipeline.
 */
class UiAutoTest {
public:
    static UiAutoTest& getInstance();

    /** Starts the sweep: one activation step every stepIntervalMs. */
    void start(long stepIntervalMs = 2500);
    void stop();
    bool isRunning() const { return mRunning; }

    /** Parses and runs a line-based test script (see autotest.cc header
     *  comment); the process exits with the failure count when done. */
    bool runScript(const std::string& path);

private:
    UiAutoTest() = default;
    void step();
    void collectClickable(AccessibilityNodeInfo* node, int depth);
    /** One ACTION_SCROLL_FORWARD on the first scrollable node (ping-pong
     *  backward at the bottom). Returns true when a scroll happened. */
    bool scrollOnce(AccessibilityNodeInfo* root);

    // --- script mode ---
    struct Command {
        std::string verb;        // wait/click/assert/assert-absent/dump/sleep
        bool byText = true;      // selector kind: text= / id=
        std::string selector;
        long timeoutMs = 3000;   // wait poll budget
        int line = 0;             // source line (error reporting)
    };
    void scriptNext();
    void scriptDone();
    bool parseScript(const std::string& path);
    static AccessibilityNodeInfo* findOne(const Command& c);

    bool mRunning = false;
    long mStepIntervalMs = 2500;
    int mStepCount = 0;
    size_t mScanIndex = (size_t)-1;
    bool mKeepIndexOnRebuild = false;  // stale-rebuild resumes, wrap restarts
    Window* mLastActiveWindow = nullptr;  // follow navigation: new window, new snapshot
    size_t mStepsSinceScroll = 0;         // one scroll per full click cycle
    int mScrollExhausted = 0;             // consecutive failed forward scrolls
    int mEmptySteps = 0;                  // consecutive steps with zero clickables
    int mEscapeRounds = 0;                // BACK rounds that changed nothing
    std::vector<AccessibilityNodeInfo*> mClickables;

    // --- script state ---
    std::vector<Command> mScript;
    size_t mScriptIndex = 0;
    int mScriptFails = 0;
};

} /*endof namespace*/
#endif/*__CDROID_AUTOTEST_H__*/
