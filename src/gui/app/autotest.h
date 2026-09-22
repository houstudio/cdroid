#ifndef __CDROID_AUTOTEST_H__
#define __CDROID_AUTOTEST_H__
#include <string>
#include <vector>
#include <map>
#include <random>
#include <fstream>
#include <cstdint>
#include <core/rect.h>

namespace cdroid {

class AccessibilityNodeInfo;
class InputEvent;
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

    /** Opens the shared recording sink (replayable via --test-script). Both
     *  producers write the same DSL through it:
     *  - the sweep (--auto-test --auto-test-record=x): one `wait`+`click`
     *    pair per PASS step;
     *  - MANUAL operations (--auto-test-record=x alone): the WindowManager
     *    input observer classifies every real touch/key (click / long-click /
     *    drag / tap-by-coordinate / key / back) and writes it with the same
     *    selector basis (text=/id= when the point resolves to an a11y node,
     *    coordinates otherwise — IME keys/candidates have no reachable node
     *    tree, they record as `tap x,y`). Recording starts immediately.
     *  See observeInput(); closed with a trailer on stop. */
    void setScriptRecorder(const std::string& path);

    /** The manual-operation capture path: install as WindowManager's input
     *  observer (setInputEventObserver). Classifies gestures and keys into
     *  script lines. Events flagged FLAG_INJECTED_BY_TEST (our own replay
     *  injections) are ignored. No-op when no recorder sink is open. */
    void observeInput(const InputEvent& e);

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
        std::string verb;        // wait/wait-absent/click/long-click/scroll/set-progress/set-text/tap/perform/assert/assert-absent/dump/sleep/back/sclick
        bool byText = true;      // selector kind: text= / id=
        bool exact = false;      // quoted selector: exact label match (AOSP By.text(Pattern "^...$"));
                                 // bare word keeps findAccessibilityNodeInfosByText's containment semantics
        std::string selector;
        std::vector<std::string> args;  // verb payload: value / direction / key=value pairs
        long timeoutMs = 3000;   // wait poll budget
        int line = 0;             // source line (error reporting)
    };
    void scriptNext();
    void scriptDone();
    bool parseScript(const std::string& path);
    static AccessibilityNodeInfo* findOne(const Command& c);
    // Shared tail of the click/sclick verbs: walk to the nearest clickable
    // ancestor, focus editors, a11y-focus, ACTION_CLICK with VIEW_CLICKED
    // arbitration, log, advance the script. Consumes (recycles) node.
    void performScriptClick(AccessibilityNodeInfo* node, size_t lineNo, const std::string& label);
    // sclick scan state: which command owns the scan (line number), its
    // phase — 0 paging backward, 1 paging forward (uiautomator
    // UiScrollable.getChildByText order) — and the cross-frame stability gate
    // for the tap: a rect is only tapped when a re-read 200ms later matches
    // (a coasting fling shows identical rects within one frame).
    size_t mSclickLine = 0;
    int mSclickPhase = 0;
    bool mSclickHasPending = false;
    Rect mSclickPending;

    bool mRunning = false;
    long mStepIntervalMs = 2500;
    int mStepCount = 0;
    bool mRandomWalk = false;             // Monkey mode (--auto-test=SEED)
    std::mt19937 mRng;                     // seeded in start(); drives idx picks
    /* Identity of a swept target within a page: class + position + the
     * occurrence ordinal among same-position twins. Labels are excluded — a
     * toggle row's label flips "ON,"/"OFF," on every click, which would break
     * identity matching from one visit to the next. The rank disambiguates
     * STACKED targets (dial-style UIs layer clickables at identical bounds):
     * without it "find cursor, take next" always hits the first twin and the
     * sweep pins on one node forever (hauswirt's "2/11" 40-click loop). */
    struct TargetKey {
        std::string cls;
        int left = 0, top = 0;
        int rank = 0;  // Nth node with this cls+left+top in the sorted snapshot
    };
    /* Last-clicked target per PAGE, keyed by a signature of the page's clickable
     * set (classes + geometry), NOT by Window*: apps swap pages inside one window
     * (preferencedemo's detail screens), so a Window*-keyed cursor makes the main
     * page and its detail page share one slot — each overwrites the other's
     * target, neither ever matches, and the sweep ping-pongs on item 1 of both
     * pages forever. */
    /** Index AFTER the node matching key (cls+left+top+rank) in the sorted
     *  snapshot, wrapping to 0 past the end; SIZE_MAX when the identity is
     *  not present. Falls back to the first occurrence when the stored rank
     *  no longer exists (twins left the snapshot between steps). */
    static size_t advancePastIdentity(const std::vector<AccessibilityNodeInfo*>& nodes,
            const TargetKey& key);

    std::map<std::string, TargetKey> mPageCursor;
    /* The identity touched on the previous step — the resume anchor when the
     * next snapshot is an unknown page (see step()): shared chrome (tab strips)
     * keeps its identity across page swaps, so the walk continues down the
     * strip in one pass instead of resetting to the first item. */
    TargetKey mLastClicked;
    bool mLastClickedValid = false;
    /* Pin-breaker: consecutive steps that chose the SAME target identity.
     * A correct cursor advance never repeats (it takes the item AFTER the
     * cursor); repeats mean identity collision or a self-reverting control. */
    int mPinStreak = 0;
    Window* mLastActiveWindow = nullptr;  // follow navigation: new window, new snapshot
    size_t mStepsSinceScroll = 0;         // one scroll per full click cycle
    int mScrollExhausted = 0;             // consecutive failed forward scrolls
    int mEmptySteps = 0;                  // consecutive steps with zero clickables
    int mEscapeRounds = 0;                // BACK rounds that changed nothing

    // --- deterministic page-exit (full-cycle navigation-up) ---
    /* Policy is our own — AOSP drivers never auto-leave a swept page (monkey
     * relies on its 2% SYSOPS BACK, uiautomator scripts press back
     * explicitly); the exit ACTION is AOSP primitives only (see
     * exitCurrentPage: swipe-to-dismiss drag or GLOBAL_ACTION_BACK). */
    int mPageEscapeRounds = 0;          // exits fired at one window without leaving it
    Window* mPageEscapeFrom = nullptr;  // the window the last exit tried to leave
    static bool windowSwipeArmed(Window* w);
    bool isBottomApplicationWindow(Window* w) const;
    void exitCurrentPage();
    std::vector<AccessibilityNodeInfo*> mClickables;
    std::ofstream mRecord;               // shared script-recorder sink (sweep + manual)

    // --- manual recording (WindowManager input observer) ---
    /* Producer-side stand-in for AOSP POLICY_FLAG_INJECTED: every event THIS
     * class injects (replay, sweep seek) carries this private bit so the
     * recording observer can tell its own injections from the user's hand. */
    static constexpr int32_t FLAG_INJECTED_BY_TEST = 0x20000000;
    /* Gesture classification thresholds. AOSP ViewConfiguration:
     * getLongPressTimeout() == 400ms; the gap/sleep caps keep replay pacing
     * sane without distorting short pauses. */
    static constexpr long LONGPRESS_TIMEOUT_MS = 400;
    static constexpr long GAP_THRESHOLD_MS = 200;
    static constexpr long GAP_SLEEP_CAP_MS = 5000;
    struct Gesture {
        bool down = false;
        float x0 = 0, y0 = 0;     // ACTION_DOWN position
        float x = 0, y = 0;       // latest position
        int64_t downAtMs = 0;     // SystemClock::uptimeMillis at DOWN
        float maxDist = 0;        // max distance from the DOWN point (slop test)
        bool multiPointer = false;
    } mGesture;
    int64_t mLastRecordedOpAtMs = 0;
    int mTouchSlop = -1;                 // resolved lazily via ViewConfiguration
    void recordLine(const std::string& line);
    void recordGapSleep();
    void recordManualClick(const char* verb, float x, float y);
    static std::string selectorFor(AccessibilityNodeInfo* node);
    static AccessibilityNodeInfo* findDeepestAt(AccessibilityNodeInfo* node, int x, int y);
    /* Inject a touchscreen motion event carrying FLAG_INJECTED_BY_TEST (used
     * by the replay side of the manual-recording verbs: coordinate tap/drag). */
    static void injectMarkedMotion(int action, float x, float y,
            int64_t downTimeMs, int64_t eventTimeMs);

    // --- script state ---
    std::vector<Command> mScript;
    size_t mScriptIndex = 0;
    int mScriptFails = 0;
};

} /*endof namespace*/
#endif/*__CDROID_AUTOTEST_H__*/
