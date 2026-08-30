#ifndef __CDROID_AUTOTEST_H__
#define __CDROID_AUTOTEST_H__
#include <string>
#include <vector>
#include <map>
#include <random>
#include <fstream>

namespace cdroid {

class AccessibilityNodeInfo;
class Window;

/**
 * App-level semantic UI sweep (--auto-test): a generic driver on top of
 * UiAutomation that walks the active window's node tree, activates every
 * on-screen clickable in reading order and verifies each activation produced
 * its TYPE_VIEW_CLICKED event. No per-app test code — any CDROID app runs
 * `./app --auto-test` to get a semantic, coordinate-free smoke test that also
 * exercises the whole a11y pipeline.
 *
 * Two target-selection modes (Android has both): the default deterministic
 * per-page traversal, or `--auto-test=<seed>` for a Monkey-style seeded-random
 * walk (com.android.commands.monkey -s: the seed alone makes the run
 * reproducible; no cursor state at all).
 */
class UiAutoTest {
public:
    static UiAutoTest& getInstance();

    /** Starts the sweep: one activation step every stepIntervalMs. seed >= 0
     *  selects the Monkey-style seeded-random walk; seed < 0 (the default)
     *  the deterministic per-page traversal. */
    void start(long stepIntervalMs = 2500, long seed = -1);
    void stop();
    bool isRunning() const { return mRunning; }

    /** Parses and runs a line-based test script (see autotest.cc header
     *  comment); the process exits with the failure count when done. */
    bool runScript(const std::string& path);

    /** Records the sweep as a replayable --test-script (the Espresso Test
     *  Recorder idea over our own DSL): one `wait`+`click` pair per PASS
     *  step — `wait` carries the poll/timeout so replay survives timing.
     *  Targets without a text label (no text/contentDescription) become
     *  comments: the script grammar has no coordinate form. Set before
     *  start(); closed with a trailer on stop. */
    void setScriptRecorder(const std::string& path);

private:
    UiAutoTest() = default;
    ~UiAutoTest();
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
    bool mRandomWalk = false;             // Monkey mode (--auto-test=SEED)
    std::mt19937 mRng;                     // seeded in start(); drives idx picks
    /* Identity of a swept target within a page: class + position. Labels are
     * excluded — a toggle row's label flips "ON,"/"OFF," on every click, which
     * would break identity matching from one visit to the next. */
    struct TargetKey {
        std::string cls;
        int left = 0, top = 0;
    };
    /* Last-clicked target per PAGE, keyed by a signature of the page's clickable
     * set (classes + geometry), NOT by Window*: apps swap pages inside one window
     * (preferencedemo's detail screens), so a Window*-keyed cursor makes the main
     * page and its detail page share one slot — each overwrites the other's
     * target, neither ever matches, and the sweep ping-pongs on item 1 of both
     * pages forever. */
    std::map<std::string, TargetKey> mPageCursor;
    /* The identity touched on the previous step — the resume anchor when the
     * next snapshot is an unknown page (see step()): shared chrome (tab strips)
     * keeps its identity across page swaps, so the walk continues down the
     * strip in one pass instead of resetting to the first item. */
    TargetKey mLastClicked;
    bool mLastClickedValid = false;
    Window* mLastActiveWindow = nullptr;  // follow navigation: new window, new snapshot
    size_t mStepsSinceScroll = 0;         // one scroll per full click cycle
    int mScrollExhausted = 0;             // consecutive failed forward scrolls
    int mEmptySteps = 0;                  // consecutive steps with zero clickables
    int mEscapeRounds = 0;                // BACK rounds that changed nothing
    std::vector<AccessibilityNodeInfo*> mClickables;
    std::ofstream mRecord;               // sweep script-recorder sink

    // --- script state ---
    std::vector<Command> mScript;
    size_t mScriptIndex = 0;
    int mScriptFails = 0;
};

} /*endof namespace*/
#endif/*__CDROID_AUTOTEST_H__*/
