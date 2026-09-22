#include <cstdint>
#include <app/autotest.h>
#include <app/uiautomation.h>
#include <core/looper.h>
#include <core/app.h>
#include <core/handler.h>
#include <core/systemclock.h>
#include <core/windowmanager.h>
#include <core/tokenizer.h>
#include <widget/cdwindow.h>   // Window (createAccessibilityNodeInfo, TYPE_SYSTEM_WINDOW)
#include <widget/internal_R.h>   // R::id::accessibilityAction* (frozen framework ids)
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <view/accessibility/accessibilitymanager.h>   // getActiveApplicationWindow
#include <accessibilityservice/accessibilityservice.h>   // GLOBAL_ACTION_BACK
#include <view/motionevent.h>
#include <view/keyevent.h>
#include <view/viewconfiguration.h>
#include <core/inputdevice.h>   // SOURCE_TOUCHSCREEN
#include <core/bundle.h>
#include <porting/cdlog.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <strings.h>   // strcasecmp (argument-key tail matching)
#include <sstream>
#include <functional>

namespace cdroid {

// static constexpr members are odr-used (std::min passes them by reference)
constexpr int32_t UiAutoTest::FLAG_INJECTED_BY_TEST;
constexpr long UiAutoTest::LONGPRESS_TIMEOUT_MS;
constexpr long UiAutoTest::GAP_THRESHOLD_MS;
constexpr long UiAutoTest::GAP_SLEEP_CAP_MS;

namespace {
constexpr int kMaxDepth = 16;
// InteractionController.REGULAR_CLICK_LENGTH: the press duration between an
// injected DOWN and UP (legacy uiautomator sleeps it on its own thread; the
// driver runs on the UI thread, so the UP is posted instead).
constexpr long REGULAR_CLICK_LENGTH = 100;
// A standing handler: the posted steps must outlive each call frame (the
// cdwindow teardown-post idiom — a temporary Handler can drop them).
Handler& stepHandler() {
    static Handler sHandler(Looper::getMainLooper());
    return sHandler;
}

/** Whether the node advertises the action id in its action list. */
bool advertises(AccessibilityNodeInfo* node, int actionId) {
    for (AccessibilityNodeInfo::AccessibilityAction* a : node->getActionList()) {
        if (a->getId() == actionId) return true;
    }
    return false;
}

/** Nearest self-or-ancestor node that advertises actionId — a selector match
 *  is often a label TextView while the action lives on the widget (the same
 *  ancestor-walk the click verb does on isClickable). Returns nullptr when
 *  nobody up the chain advertises it. `node` itself is never recycled;
 *  intermediate nodes are; the caller owns the result (and `node`). */
AccessibilityNodeInfo* resolveByAction(AccessibilityNodeInfo* node, int actionId) {
    AccessibilityNodeInfo* cur = node;
    while (cur != nullptr) {
        if (advertises(cur, actionId)) return cur;
        AccessibilityNodeInfo* parent = cur->getParent();
        if (cur != node) cur->recycle();
        cur = parent;
    }
    return nullptr;
}

/** First depth-first node advertising actionId below (and including) `node` —
 *  the downward counterpart of resolveByAction: a dialog's scrollable ListView
 *  lives BELOW the window root, so the ancestor walk can never reach it from a
 *  root. `node` itself is never recycled (the resolveByAction contract —
 *  recycling it here double-frees it against the caller's own recycle);
 *  walked intermediates are; the caller owns the result (and `node`). */
AccessibilityNodeInfo* findActionDescendant(AccessibilityNodeInfo* node, int actionId) {
    if (node == nullptr) return nullptr;
    if (advertises(node, actionId)) return node;
    AccessibilityNodeInfo* found = nullptr;
    for (int i = 0; found == nullptr && i < node->getChildCount(); i++) {
        AccessibilityNodeInfo* child = node->getChild(i);
        found = findActionDescendant(child, actionId);
        if (child != found) child->recycle();
    }
    return found;
}

// The standard accessibility actions by name — every AccessibilityNodeInfo
// legacy constant and R::id singleton of the android-36 surface (no
// CDROID-invented actions). The perform verb resolves names through this
// table: the passthrough equivalent of UiObject2.performAction(action, bundle).
const std::map<std::string, int>& standardActionTable() {
    typedef AccessibilityNodeInfo ANI;
    namespace R = cdroid::internal::R;
    static const std::map<std::string, int> table = {
        // (int) casts: the legacy constexpr action ids are ODR-used by the
        // map's forwarding-reference initializers otherwise (no definition).
        {"focus", (int)ANI::ACTION_FOCUS},
        {"clear-focus", (int)ANI::ACTION_CLEAR_FOCUS},
        {"select", (int)ANI::ACTION_SELECT},
        {"clear-selection", (int)ANI::ACTION_CLEAR_SELECTION},
        {"click", (int)ANI::ACTION_CLICK},
        {"long-click", (int)ANI::ACTION_LONG_CLICK},
        {"accessibility-focus", (int)ANI::ACTION_ACCESSIBILITY_FOCUS},
        {"clear-accessibility-focus", (int)ANI::ACTION_CLEAR_ACCESSIBILITY_FOCUS},
        {"next-at-movement-granularity", (int)ANI::ACTION_NEXT_AT_MOVEMENT_GRANULARITY},
        {"previous-at-movement-granularity", (int)ANI::ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY},
        {"next-html-element", (int)ANI::ACTION_NEXT_HTML_ELEMENT},
        {"previous-html-element", (int)ANI::ACTION_PREVIOUS_HTML_ELEMENT},
        {"scroll-forward", (int)ANI::ACTION_SCROLL_FORWARD},
        {"scroll-backward", (int)ANI::ACTION_SCROLL_BACKWARD},
        {"copy", (int)ANI::ACTION_COPY},
        {"paste", (int)ANI::ACTION_PASTE},
        {"cut", (int)ANI::ACTION_CUT},
        {"set-selection", (int)ANI::ACTION_SET_SELECTION},
        {"expand", (int)ANI::ACTION_EXPAND},
        {"collapse", (int)ANI::ACTION_COLLAPSE},
        {"dismiss", (int)ANI::ACTION_DISMISS},
        {"set-text", (int)ANI::ACTION_SET_TEXT},
        {"show-on-screen", R::id::accessibilityActionShowOnScreen},
        {"scroll-to-position", R::id::accessibilityActionScrollToPosition},
        {"scroll-up", R::id::accessibilityActionScrollUp},
        {"scroll-left", R::id::accessibilityActionScrollLeft},
        {"scroll-down", R::id::accessibilityActionScrollDown},
        {"scroll-right", R::id::accessibilityActionScrollRight},
        {"context-click", R::id::accessibilityActionContextClick},
        {"set-progress", R::id::accessibilityActionSetProgress},
        {"move-window", R::id::accessibilityActionMoveWindow},
        {"page-up", R::id::accessibilityActionPageUp},
        {"page-down", R::id::accessibilityActionPageDown},
        {"page-left", R::id::accessibilityActionPageLeft},
        {"page-right", R::id::accessibilityActionPageRight},
        {"show-tooltip", R::id::accessibilityActionShowTooltip},
        {"hide-tooltip", R::id::accessibilityActionHideTooltip},
        {"press-and-hold", R::id::accessibilityActionPressAndHold},
        {"ime-enter", R::id::accessibilityActionImeEnter},
        {"drag-start", R::id::accessibilityActionDragStart},
        {"drag-drop", R::id::accessibilityActionDragDrop},
        {"drag-cancel", R::id::accessibilityActionDragCancel},
        {"show-text-suggestions", R::id::accessibilityActionShowTextSuggestions},
        {"scroll-in-direction", R::id::accessibilityActionScrollInDirection},
    };
    return table;
}

// The standard ACTION_ARGUMENT_* keys with their canonical types (the
// perform verb types its key=value payload from this table — android-36).
struct ArgSpec {
    const char* key;
    enum Type { Int, Float, Boolean, String } type;
};
const std::vector<ArgSpec>& standardArgumentTable() {
    typedef AccessibilityNodeInfo ANI;
    static const std::vector<ArgSpec> table = {
        {ANI::ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT, ArgSpec::Int},
        {ANI::ACTION_ARGUMENT_HTML_ELEMENT_STRING, ArgSpec::String},
        {ANI::ACTION_ARGUMENT_EXTEND_SELECTION_BOOLEAN, ArgSpec::Boolean},
        {ANI::ACTION_ARGUMENT_SELECTION_START_INT, ArgSpec::Int},
        {ANI::ACTION_ARGUMENT_SELECTION_END_INT, ArgSpec::Int},
        {ANI::ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE, ArgSpec::String},
        {ANI::ACTION_ARGUMENT_ROW_INT, ArgSpec::Int},
        {ANI::ACTION_ARGUMENT_COLUMN_INT, ArgSpec::Int},
        {ANI::ACTION_ARGUMENT_PROGRESS_VALUE, ArgSpec::Float},
        {ANI::ACTION_ARGUMENT_MOVE_WINDOW_X, ArgSpec::Int},
        {ANI::ACTION_ARGUMENT_MOVE_WINDOW_Y, ArgSpec::Int},
        {ANI::ACTION_ARGUMENT_SCROLL_AMOUNT_FLOAT, ArgSpec::Float},
    };
    return table;
}

/** Builds the perform-verb arguments Bundle from "key=value" tokens. Keys
 *  accept the full AOSP constant or its distinctive tail ("progress_value",
 *  "selection_start_int" — matched case-insensitively against the constant's
 *  end, on a '_'/'.' word boundary). Returns false (with reason) on an
 *  unknown key or a malformed token — silently guessing a type would
 *  misdeliver the action. */
bool buildArguments(const std::vector<std::string>& tokens, Bundle& out, std::string& error) {
    for (const std::string& token : tokens) {
        const size_t eq = token.find('=');
        if (eq == std::string::npos) {
            error = "expected key=value, got '" + token + "'";
            return false;
        }
        const std::string key = token.substr(0, eq);
        const std::string value = token.substr(eq + 1);
        const ArgSpec* spec = nullptr;
        for (const ArgSpec& s : standardArgumentTable()) {
            const size_t klen = strlen(s.key);
            if (key.size() > klen) continue;
            if (strcasecmp(s.key + klen - key.size(), key.c_str()) != 0) continue;
            // Word boundary before the matched tail (a mid-word hit like
            // "ress_value" must not count).
            if (key.size() < klen) {
                const char boundary = s.key[klen - key.size() - 1];
                if (boundary != '_' && boundary != '.' && boundary != '/') continue;
            }
            spec = &s;
            break;
        }
        if (spec == nullptr) {
            error = "unknown argument key '" + key + "'";
            return false;
        }
        switch (spec->type) {
        case ArgSpec::Int:      out.putInt(spec->key, atoi(value.c_str())); break;
        case ArgSpec::Float:    out.putFloat(spec->key, strtof(value.c_str(), nullptr)); break;
        case ArgSpec::Boolean:  out.putBoolean(spec->key, value == "true" || value == "1"); break;
        case ArgSpec::String:   out.putString(spec->key, value); break;
        }
    }
    return true;
}

/** Espresso check(matches(...)) over the standard node properties — the
 *  getters uiautomator's UiObject2 exposes, one "key=value" token per
 *  property: checked/selected/enabled/clickable/long-clickable/focusable/
 *  focused/visible/scrollable/editable/password/multi-line/dismissable
 *  (true/false), text/class (exact string), progress (RangeInfo current).
 *  Unknown keys fail loudly — a typo'd assertion must never read as a pass.
 *  Tokens without '=' (the wait budget) are skipped. */
bool checkNodeProperties(AccessibilityNodeInfo* n, const std::vector<std::string>& tokens,
                         std::string& why) {
    for (const std::string& token : tokens) {
        const size_t eq = token.find('=');
        if (eq == std::string::npos) continue;   // the numeric budget token
        const std::string key = token.substr(0, eq);
        const std::string value = token.substr(eq + 1);
        const bool wantBool = (value == "true" || value == "1");
        bool ok = false;
        if (key == "checked")              ok = n->isChecked() == wantBool;
        else if (key == "selected")        ok = n->isSelected() == wantBool;
        else if (key == "enabled")         ok = n->isEnabled() == wantBool;
        else if (key == "clickable")       ok = n->isClickable() == wantBool;
        else if (key == "long-clickable")  ok = n->isLongClickable() == wantBool;
        else if (key == "focusable")       ok = n->isFocusable() == wantBool;
        else if (key == "focused")         ok = n->isFocused() == wantBool;
        else if (key == "visible")         ok = n->isVisibleToUser() == wantBool;
        else if (key == "scrollable")      ok = n->isScrollable() == wantBool;
        else if (key == "editable")        ok = n->isEditable() == wantBool;
        else if (key == "password")        ok = n->isPassword() == wantBool;
        else if (key == "multi-line")      ok = n->isMultiLine() == wantBool;
        else if (key == "dismissable")     ok = n->isDismissable() == wantBool;
        else if (key == "text")            ok = n->getText() == value;
        else if (key == "class")           ok = n->getClassName() == value;
        // AccessibilityNodeInfo.getContentDescription() / getStateDescription()
        // (API 30+): the string faces TalkBack reads aloud.
        else if (key == "content-desc")    ok = n->getContentDescription() == value;
        else if (key == "state-desc")      ok = n->getStateDescription() == value;
        else if (key == "progress") {
            // The UiObject2.getRangeInfo() check — closes the set-progress
            // loop without needing a readout TextView on the page.
            const AccessibilityNodeInfo::RangeInfo* range = n->getRangeInfo();
            const float want = strtof(value.c_str(), nullptr);
            ok = range != nullptr && fabsf(range->getCurrent() - want) < 0.5f;
        } else {
            why = "unknown predicate '" + key + "'";
            return false;
        }
        if (!ok) {
            why = key + "=" + value + " does not hold";
            return false;
        }
    }
    return true;
}
} // namespace

UiAutoTest& UiAutoTest::getInstance() {
    static UiAutoTest sInstance;
    return sInstance;
}

void UiAutoTest::start(long stepIntervalMs, long seed) {
    mStepIntervalMs = stepIntervalMs;
    if (mRunning) return;
    UiAutomation::getInstance().connect();  // the backing service (idempotent)
    mRunning = true;
    mStepCount = 0;
    mPageCursor.clear();
    mLastClickedValid = false;
    mRandomWalk = seed >= 0;
    if (mRandomWalk) mRng.seed((uint32_t)seed);
    LOGI("AUTOTEST sweep start (interval %ldms, %s)", mStepIntervalMs,
         mRandomWalk ? "monkey-style seeded random" : "deterministic traversal");
    stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
}

UiAutoTest::~UiAutoTest() {
    for (auto* stale : mClickables) stale->recycle();
    mClickables.clear();
}

void UiAutoTest::stop() {
    mRunning = false;
    LOGI("AUTOTEST sweep stop after %d steps", mStepCount);
    if (mRecord.is_open()) {
        mRecord << "# sweep stopped after " << mStepCount << " steps\n";
        mRecord.close();
    }
    // The kept clickables are pool objects — the app may quit right after the
    // sweep stops (or the sweep dies when its window closes), and un-recycled
    // pooled nodes then read as definite leaks (9 blocks / ~4.7K on pd).
    for (auto* stale : mClickables) stale->recycle();
    mClickables.clear();
}

void UiAutoTest::collectClickable(AccessibilityNodeInfo* node, int depth) {
    if (node == nullptr) return;
    if (depth > kMaxDepth) { node->recycle(); return; }   // cut node is still owned
    // Progress ranges (SeekBar & friends) are not clickable — AOSP drives
    // them with ACTION_SET_PROGRESS / the SCROLL_* actions AbsSeekBar
    // advertises, so collect nodes advertising SET_PROGRESS as targets too
    // (a click-only sweep could never move them).
    if (node->isVisibleToUser() && node->isEnabled()
            && (node->isClickable()
                || advertises(node, cdroid::internal::R::id::accessibilityActionSetProgress))) {
        mClickables.push_back(node);
        // Keep descending anyway: a clickable CONTAINER (fragment roots often
        // carry clickable=true) still holds independent child targets —
        // stopping here is why the sweep only ever reached the tab row.
    }
    for (int i = 0; i < node->getChildCount(); i++) {
        collectClickable(node->getChild(i), depth + 1);
    }
    if (std::find(mClickables.begin(), mClickables.end(), node) == mClickables.end()) {
        node->recycle();  // walk scaffolding (kept clickables stay alive)
    }
}

namespace {
// Identity label for a swept target: its own text/content description, else the
// first non-empty descendant text (list rows carry their title in a child).
std::string targetLabel(AccessibilityNodeInfo* node, int depth = 0) {
    if (node == nullptr || depth > 4) return "";
    std::string own = node->getText();
    if (own.empty()) own = node->getContentDescription();
    if (!own.empty()) return own;
    for (int i = 0; i < node->getChildCount(); i++) {
        AccessibilityNodeInfo* child = node->getChild(i);
        const std::string label = targetLabel(child, depth + 1);
        child->recycle();
        if (!label.empty()) return label;
    }
    return "";
}

// "540,1200" — the manual recorder's coordinate form (tap/drag selectors).
bool parseCoordPair(const std::string& s, float* x, float* y) {
    const size_t comma = s.find(',');
    if (comma == std::string::npos) return false;
    *x = strtof(s.c_str(), nullptr);
    *y = strtof(s.c_str() + comma + 1, nullptr);
    return true;
}
} // namespace

// static
size_t UiAutoTest::advancePastIdentity(const std::vector<AccessibilityNodeInfo*>& nodes,
        const TargetKey& key) {
    size_t seen = 0;
    size_t fallback = SIZE_MAX;
    for (size_t i = 0; i < nodes.size(); i++) {
        Rect b; nodes[i]->getBoundsInScreen(b);
        if (nodes[i]->getClassName() == key.cls
                && b.left == key.left && b.top == key.top) {
            if (fallback == SIZE_MAX) fallback = i;
            if ((int)seen == key.rank) {
                return (i + 1) % nodes.size();
            }
            seen++;
        }
    }
    if (fallback != SIZE_MAX) {
        return (fallback + 1) % nodes.size();
    }
    return SIZE_MAX;
}

// static
bool UiAutoTest::windowSwipeArmed(Window* w) {
    /*The discovery probe: AOSP WearGestureInterceptionDetector.isEnabled
      (Detector.java:60-75) minus FEATURE_WATCH (CDROID models no watch
      feature) — resolve windowSwipeToDismiss against the window's own
      context theme, the same read Window::loadThemeSwipeToDismiss made when
      it armed the detector. The a11y tree never carries this (AOSP: the
      interception goes DecorView -> ViewRootImpl -> SystemUI, bypassing
      accessibility), so the driver asks the theme directly.*/
    if (w == nullptr) return false;
    Context* ctx = w->getContext();
    if (ctx == nullptr) return false;
    namespace R = cdroid::internal::R;
    static const uint32_t attrs[] = {R::attr::windowSwipeToDismiss, 0};
    auto ta = ctx->getTheme().obtainStyledAttributes(attrs);
    if (!ta) return false;
    return ta->getBoolean(0, false);
}

bool UiAutoTest::isBottomApplicationWindow(Window* w) const {
    /*mWindows is bottom-up (WindowManager): the FIRST application window in
      stack order is the app's root surface. Leaving it (BACK on the root
      activity) ends the app — the endurance model keeps cycling the root
      window instead (printerdemo's 9h runs). getWindows, not
      getVisibleWindows, on purpose: a covered parent stays a valid root even
      when it reports INVISIBLE. Same application-type predicate as
      getActiveApplicationWindow (type < TYPE_SYSTEM_WINDOW).*/
    std::vector<Window*> all;
    WindowManager::getInstance().getWindows(all);
    for (Window* c : all) {
        if (c->getAttributes().type < Window::TYPE_SYSTEM_WINDOW) return c == w;
    }
    return false;
}

void UiAutoTest::exitCurrentPage() {
    /*The exit ACTION inventory — all AOSP primitives, picked by capability
      (no configuration): a theme-armed window (windowSwipeArmed probe) exits
      with the wear gesture itself — a right-drag covering ~65% of the window
      in 10 interpolated MOVEs, past the 33% distance ratio and fast enough
      to fling (monkey's MOTION family injects drags the same way) — every
      other window, and any later round (e.g. a drag that latched canScroll
      on horizontally scrollable content), with GLOBAL_ACTION_BACK, the
      TalkBack/uiautomator/monkey-SYSOPS exit. Window frame comes from the
      a11y root: screen coordinates the injected stream needs.*/
    Window* active = mLastActiveWindow;
    const bool trySwipe = mPageEscapeRounds == 0 && windowSwipeArmed(active);
    Rect bounds;
    if (trySwipe) {
        AccessibilityNodeInfo* root = UiAutomation::getInstance().getRootInActiveWindow();
        if (root != nullptr) {
            root->getBoundsInScreen(bounds);
            root->recycle();
        }
    }
    if (trySwipe && bounds.width > 0 && bounds.height > 0) {
        LOGI("AUTOTEST page exit: swipe-dismiss drag on window %p", (void*)active);
        const float y = (float)bounds.centerY();
        const float x0 = bounds.left + bounds.width * 0.10f;
        const float x1 = bounds.left + bounds.width * 0.75f;
        const int64_t downTime = SystemClock::uptimeMillis();
        injectMarkedMotion(MotionEvent::ACTION_DOWN, x0, y, downTime, downTime);
        for (int i = 1; i <= 10; i++) {
            const float t = (float)i / 10;
            stepHandler().postDelayed([this, x0, x1, y, t, downTime]() {
                injectMarkedMotion(MotionEvent::ACTION_MOVE,
                        x0 + (x1 - x0) * t, y, downTime, SystemClock::uptimeMillis());
            }, i * 16);
        }
        stepHandler().postDelayed([this, x1, y, downTime]() {
            injectMarkedMotion(MotionEvent::ACTION_UP, x1, y,
                    downTime, SystemClock::uptimeMillis());
        }, 11 * 16);
        if (mRecord.is_open()) {
            char line[64];
            snprintf(line, sizeof(line), "drag %d,%d %d,%d 10",
                     (int)x0, (int)y, (int)x1, (int)y);
            recordLine(line);
        }
    } else {
        LOGI("AUTOTEST page exit: BACK on window %p", (void*)active);
        // The escape is a global BACK (UiAutomation.performGlobalAction ->
        // the service synthesizes the key through the input pipeline).
        UiAutomation::getInstance().performGlobalAction(
                AccessibilityService::GLOBAL_ACTION_BACK);
        if (mRecord.is_open()) recordLine("back");
    }
    mPageEscapeFrom = active;
    mPageEscapeRounds++;
    stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
}

void UiAutoTest::step() {
    if (!mRunning) return;
    UiAutomation& automation = UiAutomation::getInstance();
    // Follow window navigation (dialogs/sub-activities): stale snapshot would
    // click a now-background window invisibly. The a11y active window (top
    // application window — the IME never becomes it), same rule the root
    // query uses.
    if (Window* active = WindowManager::getInstance().getActiveApplicationWindow()) {
        if (active != mLastActiveWindow) {
            LOGI("AUTOTEST active window %p -> %p", (void*)mLastActiveWindow, (void*)active);
            mLastActiveWindow = active;
            // Any window change proves the last page-exit worked (or the last
            // click navigated) — restart the escape ladder.
            mPageEscapeRounds = 0;
            mPageEscapeFrom = nullptr;
        }
    }
    AccessibilityNodeInfo* root = automation.getRootInActiveWindow();
    if (root == nullptr) {  // no window yet (or between windows) — retry
        stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
        return;
    }
    // Rebuild the snapshot EVERY step. Tab clicks flip ViewPager pages whose
    // outgoing fragment views stay alive-but-offscreen — refresh() cannot
    // detect that, and a kept snapshot ends up clicking invisible pages. A
    // fresh position-sorted walk each step makes the sweep follow the UI;
    // mStepCount % size still cycles through a stable page systematically.
    for (auto* stale : mClickables) stale->recycle();
    mClickables.clear();
    collectClickable(root, 0);
    // Sweep the whole visible surface (uiautomator searches across windows):
    // after the active application window, walk the VISIBLE system-layer
    // windows too — a shown IME's keys are targets (B's virtual-key views),
    // a hidden one is not visible and stays out, so the sweep neither stalls
    // on it nor skips the keyboard. The geometry sort below places the keys
    // below the app content, reading order intact.
    std::vector<Window*> visible;
    WindowManager::getInstance().getVisibleWindows(visible);
    for (Window* w : visible) {
        if (w->getAttributes().type < Window::TYPE_SYSTEM_WINDOW) continue;
        AccessibilityNodeInfo* sysRoot = w->createAccessibilityNodeInfo();
        if (sysRoot != nullptr) {
            // Seal at the boundary exactly like the service's root query —
            // getChild() enforces the sealed state (AOSP) and throws otherwise.
            sysRoot->setSealed(true);
            collectClickable(sysRoot, 0);
        }
    }
    std::sort(mClickables.begin(), mClickables.end(),
              [](AccessibilityNodeInfo* a, AccessibilityNodeInfo* b) {
                  Rect ra, rb;
                  a->getBoundsInScreen(ra);
                  b->getBoundsInScreen(rb);
                  if (ra.top != rb.top) return ra.top < rb.top;
                  return ra.left < rb.left;
              });
    if (mClickables.empty()) {
        // A window with no a11y targets would loop here silently forever —
        // escape it the way a screen-reader user would: synthesize BACK
        // after a few empty passes (dismisses a target-less popup; on a bare
        // main window BACK ends the app, which also terminates a sweep that
        // has nothing left to test). Menu popups are NOT such a window:
        // AbsListView's ListItemAccessibilityDelegate exposes their items
        // as clickable (verified on printerdemo's language PopupMenu).
        if (++mEmptySteps >= 3) {
            mEmptySteps = 0;
            if (++mEscapeRounds > 3) {
                // BACK did not change the situation — this window is a dead
                // end the driver cannot interact with or dismiss. End the
                // sweep instead of looping forever.
                LOGW("AUTOTEST escape failed %d rounds on window %p — stopping sweep",
                     mEscapeRounds - 1, (void*)mLastActiveWindow);
                stop();
                return;
            }
            LOGW("AUTOTEST no clickables in window %p for 3 steps — sending BACK (round %d)",
                 (void*)mLastActiveWindow, mEscapeRounds);
            // AOSP: the escape is a global BACK (UiAutomation.performGlobalAction
            // → the service synthesizes the key through the input pipeline).
            UiAutomation::getInstance().performGlobalAction(
                    AccessibilityService::GLOBAL_ACTION_BACK);
            mEmptySteps = 0;
        }
        stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
        return;
    }
    mEmptySteps = 0;
    mEscapeRounds = 0;  // a productive step also proves the last escape worked
    // ScrollView pages fold their lower content out of isVisibleToUser —
    // after each full click cycle, spend one step scrolling the first
    // scrollable forward (backward ping-pong at the bottom).
    if (++mStepsSinceScroll > mClickables.size()) {
        mStepsSinceScroll = 0;
        // collectClickable already recycled the walk root — take a FRESH
        // root for the scroll walk (scrollOnce owns this one).
        scrollOnce(automation.getRootInActiveWindow());
        stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
        return;
    }
    mStepCount++;
    size_t idx = 0;
    std::string pageSig;  // deterministic mode: identity of this page for the cursor
    if (mRandomWalk) {
        // Monkey SYSOPS (MonkeySourceRandom:135, default 2.0): system keys
        // HOME/BACK/CALL/ENDCALL/VOLUME*; the global-action surface here
        // implements BACK only, so the factor collapses to a BACK — the
        // random walk's page-escape, exactly monkey's.
        if (mRng() % 100 < 2) {
            LOGI("AUTOTEST [%d] SYSOPS -> BACK", mStepCount);
            UiAutomation::getInstance().performGlobalAction(
                    AccessibilityService::GLOBAL_ACTION_BACK);
            if (mRecord.is_open()) recordLine("back");
            stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
            return;
        }
        // Monkey -s semantics: pick uniformly from the fresh snapshot — the seed
        // alone makes the walk reproducible, so there is no cursor state to keep.
        idx = mRng() % mClickables.size();
    } else {
        // Identity-following cursor: the snapshot is rebuilt every step, so "next" means
        // "the item AFTER the one we clicked last on this page", found by identity in the
        // fresh vector. An index into a resized/re-sorted vector would skip or repeat items
        // (dialog opens, toggles re-layout, scrolling exposes rows). Unknown page → top.
        // Page identity = the clickable set's classes + geometry (see mPageCursor); labels
        // are excluded so a toggled row doesn't fork the page.
        for (auto* n : mClickables) {
            Rect b; n->getBoundsInScreen(b);
            pageSig += n->getClassName();
            pageSig += '@';
            pageSig += std::to_string(b.left);
            pageSig += ',';
            pageSig += std::to_string(b.top);
            pageSig += ';';
        }
        bool cursorHit = false;
        auto cursorIt = mPageCursor.find(pageSig);
        if (cursorIt != mPageCursor.end()) {
            idx = advancePastIdentity(mClickables, cursorIt->second);
            cursorHit = (idx != SIZE_MAX);
        }
        // Page fully swept (known page, cursor wrapped to the top) — leave it
        // (swipe/BACK, see exitCurrentPage) unless it is the app's root
        // window: there the sweep keeps cycling (BACK on the root activity
        // would end the app — the endurance model). The cursor is dropped so
        // a later re-entry sweeps the page from the top instead of wrapping
        // straight back out; mLastClicked too, or the resume-after anchor
        // would shortcut to the same wrap.
        if (cursorHit && idx == 0 && !isBottomApplicationWindow(mLastActiveWindow)) {
            if (mPageEscapeRounds >= 3) {
                LOGW("AUTOTEST page exit failed %d rounds on window %p — stopping sweep",
                     mPageEscapeRounds, (void*)mLastActiveWindow);
                stop();
                return;
            }
            LOGI("AUTOTEST page cycle complete on window %p — exiting (round %d)",
                 (void*)mLastActiveWindow, mPageEscapeRounds);
            mPageCursor.erase(pageSig);
            mLastClickedValid = false;
            exitCurrentPage();
            return;
        }
        if (!cursorHit && mLastClickedValid) {
            // Unknown page (first visit, or the cursor identity left the
            // viewport): resume AFTER the identity touched on the previous
            // step when this snapshot still contains it. Shared chrome — a
            // TabLayout strip keeps the same tabs on every page — then
            // advances one tab per step in a single pass, instead of every
            // navigation resetting the walk to the first tab (widgetsDemo
            // spent whole rounds up in the strip otherwise).
            const size_t resumed = advancePastIdentity(mClickables, mLastClicked);
            if (resumed != SIZE_MAX) idx = resumed;
        }
        // Pin-breaker: the cursor advance NEVER re-picks the item it stored
        // last step unless identities collide (stacked twins) or the control
        // re-creates itself at the same place. A few repeats are tolerated;
        // a longer streak means the sweep is nailed to one node — hop over
        // it instead of testing it forever (hauswirt main page pinned 40+).
        if (mPinStreak >= 4) {
            LOGW("AUTOTEST target pinned %d steps — skipping one ahead", mPinStreak);
            idx = (idx + 1) % mClickables.size();
            mPinStreak = 0;
        }
    }
    AccessibilityNodeInfo* target = mClickables.at(idx);
    if (!mRandomWalk) {
        // Remember the target we are ABOUT to click, so the next visit to this
        // page resumes after it — and an unknown next page resumes after the
        // same identity (mLastClicked). rank = how many earlier nodes in this
        // sorted snapshot share the identity (stacked-twin disambiguation).
        Rect tb; target->getBoundsInScreen(tb);
        TargetKey key{target->getClassName(), tb.left, tb.top, 0};
        for (size_t i = 0; i < idx; i++) {
            Rect b; mClickables[i]->getBoundsInScreen(b);
            if (mClickables[i]->getClassName() == key.cls
                    && b.left == key.left && b.top == key.top) {
                key.rank++;
            }
        }
        mPinStreak = (mLastClickedValid && key.cls == mLastClicked.cls
                && key.left == mLastClicked.left && key.top == mLastClicked.top
                && key.rank == mLastClicked.rank) ? mPinStreak + 1 : 0;
        mPageCursor[pageSig] = key;
        mLastClicked = key;
        mLastClickedValid = true;
    }
    const std::string label = target->getText().empty()
            ? targetLabel(target) : target->getText();

    // Non-clickable targets are progress ranges (the only other kind the
    // sweep collects). Drive them with the SCROLL_* action the node itself
    // advertises — AbsSeekBar adds SCROLL_FORWARD while progress < max and
    // SCROLL_BACKWARD while progress > min, so the advertised set picks the
    // direction and the walk ping-pongs at the ends — and verify the
    // deferred TYPE_VIEW_SELECTED ProgressBar schedules on user progress
    // changes (AOSP uiautomator drives seek bars exactly this way when it
    // is not injecting a tap gesture).
    if (!target->isClickable()) {
        const int seekAction = advertises(target, AccessibilityNodeInfo::ACTION_SCROLL_FORWARD)
                ? AccessibilityNodeInfo::ACTION_SCROLL_FORWARD
                : AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD;
        target->performAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);
        AccessibilityEvent* hit = automation.executeAndWaitForEvent(
            [target, seekAction]() { target->performAction(seekAction); },
            [](AccessibilityEvent& e) {
                return e.getEventType() == AccessibilityEvent::TYPE_VIEW_SELECTED; },
            1500);
        LOGI("AUTOTEST [%d] %2zu/%zu [%s] -> %s (seek)", mStepCount,
             idx + 1, mClickables.size(), label.c_str(), hit ? "PASS" : "no-event");
        if (hit) {
            hit->recycle();
            if (mRecord.is_open()) {
                // Same shape as a click step: the wait carries the poll
                // budget, and the scroll verb mirrors the action the sweep
                // actually drove (AbsSeekBar advertises FORWARD below max,
                // BACKWARD above min, so the direction is what was advertised).
                // findAccessibilityNodeInfosByText matches content
                // descriptions too, so labeled seek bars replay cleanly.
                const std::string sel = selectorFor(target);
                if (!sel.empty()) {
                    mRecord << "wait " << sel << " 5000\n"
                            << "scroll " << sel << " "
                            << (seekAction == AccessibilityNodeInfo::ACTION_SCROLL_FORWARD
                                ? "forward" : "backward") << "\n";
                } else {
                    mRecord << "# step " << mStepCount << ": " << target->getClassName()
                            << " — seek target, no selector\n";
                }
                mRecord.flush();
            }
        }
        stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
        return;
    }

    // AOSP ACTION_FOCUS (= requestFocus): a semantic ACTION_CLICK never
    // focuses (performClick doesn't) — without this an editable target could
    // never raise the IME, and the keyboard would stay unswept. Focus the
    // editor first; showSoftInput rides the focus change, and the keyboard's
    // keys then join the sweep through the visible-system-window walk.
    if (target->isEditable()) {
        target->performAction(AccessibilityNodeInfo::ACTION_FOCUS);
    }

    // Visual feedback: a semantic click fires no pressed-state animation —
    // park the accessibility focus highlight on the target so the sweep is
    // observable on screen (the scanner's own highlight path).
    target->performAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);
    AccessibilityEvent* hit = automation.executeAndWaitForEvent(
        [target]() { target->performAction(AccessibilityNodeInfo::ACTION_CLICK); },
        [](AccessibilityEvent& e) {
            return e.getEventType() == AccessibilityEvent::TYPE_VIEW_CLICKED; },
        1500);
    LOGI("AUTOTEST [%d] %2zu/%zu [%s] -> %s", mStepCount,
         idx + 1, mClickables.size(),
         label.c_str(), hit ? "PASS" : "no-event");
    if (hit) {
        hit->recycle();
        if (mRecord.is_open()) {
            // selectorFor is the SAME basis the manual recorder uses; the
            // wait carries the poll budget (click alone fails fast on a
            // not-yet-arrived page), so replay survives timing.
            const std::string sel = selectorFor(target);
            if (!sel.empty()) {
                mRecord << "wait " << sel << " 5000\n"
                        << "click " << sel << "\n";
            } else {
                mRecord << "# step " << mStepCount << ": " << target->getClassName()
                        << " — no selector\n";
            }
            mRecord.flush();   // keep the script usable if the sweep dies mid-run
        }
    }

    stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
}

bool UiAutoTest::scrollOnce(AccessibilityNodeInfo* root) {
    // Takes ownership of root. Find the largest visible scrollable, spend one
    // scroll action on it; ping-pong backward once the forward end is hit.
    AccessibilityNodeInfo* best = nullptr;
    Rect bestB;
    std::function<void(AccessibilityNodeInfo*, int)> visit = [&](AccessibilityNodeInfo* n, int d) {
        if (!n) return;
        if (d > kMaxDepth) { n->recycle(); return; }   // cut node is still owned
        const bool candidate = n->isScrollable() && n->isVisibleToUser();
        Rect b;
        if (candidate) n->getBoundsInScreen(b);
        for (int i = 0; i < n->getChildCount(); i++) visit(n->getChild(i), d + 1);
        if (candidate && (!best || (long)b.width * b.height > (long)bestB.width * bestB.height)) {
            if (best) best->recycle();
            best = n; bestB = b;        // kept
        } else if (n != root) {
            n->recycle();               // walk scaffolding (root handled below)
        }
    };
    visit(root, 0);
    if (best == nullptr) {
        root->recycle();
        return false;
    }
    // The walk's else-branch skips the root itself ("handled below"), so the
    // best==nullptr case above is NOT the only one that must return it: when
    // the scrollable is a descendant, root is neither kept nor recycled — it
    // was a definite-lost 958B block on printerdemo's valgrind sweep (the
    // record's :362 allocation stack is the pool handing out this block's
    // first life; the drop happens here).
    if (best != root) root->recycle();
    const int action = (mScrollExhausted >= 1)
            ? AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD
            : AccessibilityNodeInfo::ACTION_SCROLL_FORWARD;
    const bool scrolled = best->performAction(action);
    best->recycle();
    if (!scrolled) mScrollExhausted++;
    else if (action == AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD) mScrollExhausted = 0;
    LOGI("AUTOTEST scroll %s -> %s",
         action == AccessibilityNodeInfo::ACTION_SCROLL_FORWARD ? "forward" : "backward",
         scrolled ? "ok" : "end");
    return scrolled;
}

// --- script mode ------------------------------------------------------------
// Line-based DSL ('#' comments; whitespace-separated):
//   wait   text=登录 3000     poll until a matching node exists (default 3000ms)
//   wait   text=x checked=true 5000
//                             Until.hasObject with criteria: the match must
//                             also satisfy the property predicates
//   wait-absent text=弹窗 3000 Until.gone: poll until NO match remains
//   click  text=登录          highlight + semantic click, wait VIEW_CLICKED
//   long-click text=行        ACTION_LONG_CLICK, wait TYPE_VIEW_LONG_CLICKED
//   scroll text=列表 forward  ACTION_SCROLL_FORWARD/BACKWARD on the matched
//                             node (backward at the list end)
//   set-progress text=音量 75 UiObject2.setProgress: ACTION_SET_PROGRESS with
//                             ARGUMENT_PROGRESS_VALUE — the standard way to
//                             drive a SeekBar/ProgressBar
//   set-text text=框 hello    ACTION_SET_TEXT with ARGUMENT_SET_TEXT
//   tap    text=滑条          the gesture wheel: an injected coordinate tap
//                             (DOWN + UP after 100ms) through the input
//                             pipeline, uiautomator's actual click; a tap on
//                             a SeekBar seeks to that position (tap-to-seek)
//   perform text=x <action> [key=value ...]
//                             UiObject2.performAction(action, bundle): any
//                             standard a11y action by name (focus, copy,
//                             expand, page-up, scroll-up, press-and-hold …),
//                             typed arguments by the standard key table
//   assert text=欢迎 checked=true
//                             Espresso check(matches(...)): fail when absent
//                             or a predicate fails. Predicates (the standard
//                             node properties UiObject2 exposes):
//                             checked/selected/enabled/clickable/
//                             long-clickable/focusable/focused/visible/
//                             scrollable/editable/password/multi-line/
//                             dismissable = true|false, text/class = exact
//                             string, progress = RangeInfo current value
//   assert-absent text=错误   fail if a match exists
//   dump   [file.log]         append the on-screen tree (stdout when empty)
//   sleep  500                ms
// Selectors: text=<substring> (findAccessibilityNodeInfosByText) or
// id=<pkg:id/name> (findAccessibilityNodeInfosByViewId). The process exits
// with the failure count when the script ends (CI exit code).
AccessibilityNodeInfo* UiAutoTest::findOne(const Command& c) {
    UiAutomation& automation = UiAutomation::getInstance();
    // Search the whole visible surface (the sweep's rule): the active
    // application window first, then any VISIBLE system-layer windows — a
    // shown IME's keys are only findable there (the app root never covers
    // them; replayed keyboard clicks would all miss otherwise).
    std::vector<AccessibilityNodeInfo*> roots;
    AccessibilityNodeInfo* root = automation.getRootInActiveWindow();
    if (root != nullptr) roots.push_back(root);
    std::vector<Window*> visible;
    WindowManager::getInstance().getVisibleWindows(visible);
    for (Window* w : visible) {
        if (w->getAttributes().type < Window::TYPE_SYSTEM_WINDOW) continue;
        AccessibilityNodeInfo* sysRoot = w->createAccessibilityNodeInfo();
        if (sysRoot != nullptr) {
            sysRoot->setSealed(true);   // boundary rule; getChild enforces it
            roots.push_back(sysRoot);
        }
    }
    for (AccessibilityNodeInfo* r : roots) {
        std::vector<AccessibilityNodeInfo*> hits = c.byText
                ? r->findAccessibilityNodeInfosByText(c.selector)
                : r->findAccessibilityNodeInfosByViewId(c.selector);
        if (!hits.empty()) {
            // "Visible surface" must mean the hits too: the app root keeps every
            // offscreen ViewPager page attached, and without this filter the
            // first tree-order match can be an offscreen twin (its performClick
            // still fires VIEW_CLICKED, so a script click "passes" while the UI
            // never moves). Prefer the first visible hit; keep the first overall
            // as fallback so text on a scrolled-away row stays addressable.
            AccessibilityNodeInfo* first = nullptr;
            AccessibilityNodeInfo* fallback = nullptr;
            for (AccessibilityNodeInfo* hit : hits) {
                // Quoted selector: exact label. findAccessibilityNodeInfosByText
                // is containment ("25 minutes" contains "5 minutes") — without
                // this filter a scrolled choice list matches its "2N minutes"
                // sibling first and the script clicks the wrong row.
                if (c.exact && c.byText
                        && hit->getText() != c.selector
                        && hit->getContentDescription() != c.selector) {
                    hit->recycle();
                    continue;
                }
                if (first == nullptr && hit->isVisibleToUser()) {
                    first = hit;
                } else if (fallback == nullptr) {
                    fallback = hit;
                } else {
                    hit->recycle();
                }
            }
            if (first == nullptr) {
                first = fallback;   // no visible hit: fall back to the first overall
            } else if (fallback != nullptr) {
                fallback->recycle(); // visible hit won — the fallback is unconsumed
            }
            for (AccessibilityNodeInfo* other : roots) other->recycle();
            return first;  // caller recycles
        }
    }
    for (AccessibilityNodeInfo* r : roots) r->recycle();
    return nullptr;
}

void UiAutoTest::performScriptClick(AccessibilityNodeInfo* node, size_t lineNo,
                                    const std::string& label) {
    UiAutomation& automation = UiAutomation::getInstance();
    // A text selector matches the TextView carrying the text, not the
    // clickable row that owns it (uiautomator hides this behind a
    // coordinate tap on the node bounds); semantic ACTION_CLICK needs the
    // nearest clickable ancestor.
    AccessibilityNodeInfo* clickTarget = node;
    while (!clickTarget->isClickable()) {
        AccessibilityNodeInfo* parent = clickTarget->getParent();
        if (parent == nullptr) break;
        if (clickTarget != node) clickTarget->recycle();
        clickTarget = parent;
    }
    // Editors get focus first (same AOSP ACTION_FOCUS as the sweep): a
    // plain ACTION_CLICK never focuses, so a replaying script could never
    // raise the IME — every recorded keyboard click would miss.
    if (clickTarget->isEditable()) {
        clickTarget->performAction(AccessibilityNodeInfo::ACTION_FOCUS);
    }
    clickTarget->performAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);  // visible target
    AccessibilityEvent* hit = automation.executeAndWaitForEvent(
        [clickTarget]() { clickTarget->performAction(AccessibilityNodeInfo::ACTION_CLICK); },
        [](AccessibilityEvent& e) {
            return e.getEventType() == AccessibilityEvent::TYPE_VIEW_CLICKED; },
        1500);
    LOGI("SCRIPT %zu click %s -> %s", lineNo, label.c_str(),
         hit ? "OK" : "no-event (FAIL)");
    if (!hit) mScriptFails++;
    if (hit) hit->recycle();
    if (clickTarget != node) clickTarget->recycle();
    node->recycle();
    mScriptIndex++;
    stepHandler().post([this]() { scriptNext(); });
}

bool UiAutoTest::parseScript(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        LOGE("AUTOTEST script not found: %s", path.c_str());
        return false;
    }
    // The AOSP native Tokenizer (the .kl/.kcm lexer): line tracking gives
    // script errors a file:line location for free.
    Tokenizer* t = nullptr;
    if (Tokenizer::fromStream(path, in, &t) != 0 || t == nullptr) {
        LOGE("AUTOTEST cannot tokenize %s", path.c_str());
        return false;
    }
    while (!t->isEof()) {
        t->skipDelimiters(" \t\r");
        if (t->isEol() || t->peekChar() == '#') { t->nextLine(); continue; }
        Command cmd;
        cmd.line = t->getLineNumber();
        cmd.verb = t->nextToken(" \t\r");
        t->skipDelimiters(" \t\r");
        std::string sel;
        if (t->peekChar() == '"') {          // quoted selector may span words
            t->skipDelimiters("\"");
            sel = t->nextToken("\"");
            t->skipDelimiters("\"");
        } else if (!t->isEol() && t->peekChar() != '#') {
            sel = t->nextToken(" \t\r");
            // A value quote opened mid-token (text="a b) spans words — keep
            // consuming tokens until its closing quote.
            if (!sel.empty() && sel.find('"') != std::string::npos && sel.back() != '"') {
                while (!t->isEol()) {
                    t->skipDelimiters(" \t\r");
                    if (t->isEol() || t->peekChar() == '#') break;
                    sel += ' ';
                    sel += t->nextToken(" \t\r");
                    if (!sel.empty() && sel.back() == '"') break;
                }
            }
        }
        // Quotes may wrap the whole selector ("a b") or the value (text="a b").
        // A quoted selector also means EXACT label matching (the AOSP escape
        // from findByText's containment: By.text(Pattern "^...$")).
        auto stripQuotes = [](std::string& s) -> bool {
            if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
                s = s.substr(1, s.size() - 2);
                return true;
            }
            return false;
        };
        if (sel.rfind("text=", 0) == 0) { cmd.byText = true; cmd.selector = sel.substr(5); cmd.exact = stripQuotes(cmd.selector); }
        else if (sel.rfind("id=", 0) == 0) { cmd.byText = false; cmd.selector = sel.substr(3); cmd.exact = stripQuotes(cmd.selector); }
        else { cmd.byText = true; cmd.selector = sel; cmd.exact = stripQuotes(cmd.selector); }  // bare == text=
        t->skipDelimiters(" \t\r");
        while (!t->isEol() && t->peekChar() != '#') {  // verb payload tokens
            cmd.args.push_back(t->nextToken(" \t\r"));
            t->skipDelimiters(" \t\r");
        }
        // wait/sleep keep the milliseconds meaning — a bare number anywhere
        // in the payload is the budget, because predicate tokens (key=value)
        // may precede it ("wait text=x checked=true 5000").
        if (cmd.verb == "wait" || cmd.verb == "sleep" || cmd.verb == "wait-absent") {
            for (const std::string& a : cmd.args) {
                if (a.find('=') == std::string::npos) {
                    cmd.timeoutMs = atol(a.c_str());
                    break;
                }
            }
        }
        t->nextLine();
        mScript.push_back(cmd);
    }
    const std::string loc = t->getLocation();
    delete t;
    LOGI("AUTOTEST script %s: %zu commands (through %s)", path.c_str(), mScript.size(), loc.c_str());
    return !mScript.empty();
}

bool UiAutoTest::runScript(const std::string& path) {
    if (mRunning) return false;
    if (!parseScript(path)) return false;
    UiAutomation::getInstance().connect();
    mRunning = true;
    mScriptIndex = 0;
    mScriptFails = 0;
    stepHandler().postDelayed([this]() { scriptNext(); }, 1500);
    return true;
}

void UiAutoTest::setScriptRecorder(const std::string& path) {
    if (mRecord.is_open()) mRecord.close();
    mRecord.open(path);
    if (!mRecord.is_open()) {
        LOGW("AUTOTEST cannot open script record file %s", path.c_str());
        return;
    }
    mRecord << "# AUTOTEST recording — replay with --test-script\n";
    mRecord.flush();
}

// ============================================================================
//  Recording: manual operations + sweep steps, one shared sink/DSL basis
//  (Android's three manual-recording approaches collapsed into one: capture
//  at the WindowManager dispatch seam — the in-process equivalent of
//  Espresso Test Recorder's framework breakpoints; MonkeyRecorder's faithful
//  coordinate+wait lines for targets without a semantic identity; replay
//  re-injects through the same pipeline, getevent-style.)
// ============================================================================

void UiAutoTest::recordLine(const std::string& line) {
    if (!mRecord.is_open()) return;
    mRecord << line << "\n";
    mRecord.flush();   // keep the script usable if the session dies mid-record
}

void UiAutoTest::recordGapSleep() {
    // MonkeyRecorder's WAIT semantics: preserve the human's pacing (capped)
    // between recorded ops so replay timing tracks the recording.
    const int64_t now = SystemClock::uptimeMillis();
    if (mLastRecordedOpAtMs != 0) {
        const long gap = (long)(now - mLastRecordedOpAtMs);
        if (gap > GAP_THRESHOLD_MS) {
            recordLine("sleep " + std::to_string(std::min(gap, GAP_SLEEP_CAP_MS)));
        }
    }
    mLastRecordedOpAtMs = now;
}

// static
std::string UiAutoTest::selectorFor(AccessibilityNodeInfo* node) {
    /*The shared selector basis for BOTH recording producers: the sweep's
      identity label (text -> contentDescription -> first descendant text),
      else the resource id. Returns the whole selector token — "text=..."
      (quoted) or id=... — or empty when the target has neither; callers
      fall back to a comment (sweep) or the coordinate form (manual).*/
    std::string label = targetLabel(node);
    for (auto& ch : label) if (ch == '"' || ch == '\n' || ch == '\r') ch = ' ';
    if (!label.empty()) return "\"text=" + label + "\"";
    const std::string resName = node->getViewIdResourceName();
    if (!resName.empty()) return "id=" + resName;
    return std::string();
}

// static
AccessibilityNodeInfo* UiAutoTest::findDeepestAt(AccessibilityNodeInfo* node, int x, int y) {
    /*Deepest visible node whose screen bounds contain the point — the
      point->node inverse of the sweep's node->click direction. Children win
      over parents; a hit recycles every intermediate node it walked. The
      caller owns (recycles) the returned node. Nodes the a11y layer cannot
      reach (IME window trees) simply never match — the caller falls back to
      the coordinate form, which is exactly the truthful recording.*/
    if (node == nullptr) return nullptr;
    Rect b;
    node->getBoundsInScreen(b);
    if (!b.contains(x, y)) { node->recycle(); return nullptr; }
    const int n = node->getChildCount();
    for (int i = 0; i < n; i++) {
        AccessibilityNodeInfo* child = node->getChild(i);
        if (child == nullptr) continue;
        AccessibilityNodeInfo* hit = findDeepestAt(child, x, y);
        if (hit != nullptr) { node->recycle(); return hit; }
        // child (and its subtree) did not contain the point — already recycled
    }
    return node;
}

void UiAutoTest::recordManualClick(const char* verb, float fx, float fy) {
    /*Resolve the point to an a11y node for a stable selector — the same
      wait+verb pair shape the sweep records. Anything without a reachable
      node (IME keys, candidates, plain background) records as a coordinate
      tap, which replays through the identical input pipeline.*/
    const int x = (int)fx, y = (int)fy;
    std::string line;
    AccessibilityNodeInfo* root = UiAutomation::getInstance().getRootInActiveWindow();
    AccessibilityNodeInfo* hit = findDeepestAt(root, x, y);
    if (hit != nullptr) {
        const std::string sel = selectorFor(hit);
        hit->recycle();
        if (!sel.empty()) {
            recordLine("wait " + sel + " 5000");
            line = std::string(verb) + " " + sel;
        }
    }
    if (line.empty()) {
        char coords[32];
        snprintf(coords, sizeof(coords), "%d,%d", x, y);
        line = std::string("tap ") + coords;
    }
    recordLine(line);
}

void UiAutoTest::observeInput(const InputEvent& e) {
    if (!mRecord.is_open()) return;
    /*Skip our own injections (replay / sweep tap verb): the producer flags
      them FLAG_INJECTED_BY_TEST — the stand-in for AOSP's dispatcher-side
      POLICY_FLAG_INJECTED. InputEvent's flags live on the subclasses.*/
    const int32_t flags = (e.getType() == InputEvent::INPUT_EVENT_TYPE_KEY)
            ? ((const KeyEvent&)e).getFlags() : ((const MotionEvent&)e).getFlags();
    if (flags & FLAG_INJECTED_BY_TEST) return;

    if (e.getType() == InputEvent::INPUT_EVENT_TYPE_MOTION) {
        const MotionEvent& me = (const MotionEvent&)e;
        if (!me.isFromSource(InputDevice::SOURCE_CLASS_POINTER)) return;
        const int action = me.getActionMasked();
        const float x = me.getX(), y = me.getY();
        switch (action) {
        case MotionEvent::ACTION_DOWN:
            mGesture = Gesture{};
            mGesture.down = true;
            mGesture.x0 = mGesture.x = x;
            mGesture.y0 = mGesture.y = y;
            mGesture.downAtMs = SystemClock::uptimeMillis();
            break;
        case MotionEvent::ACTION_POINTER_DOWN:   // pinch: not expressible in the DSL yet
            mGesture.multiPointer = true;
            break;
        case MotionEvent::ACTION_MOVE:
            if (mGesture.down) {
                const float dx = x - mGesture.x0, dy = y - mGesture.y0;
                mGesture.maxDist = std::max(mGesture.maxDist,
                        std::sqrt(dx * dx + dy * dy));
                mGesture.x = x;
                mGesture.y = y;
            }
            break;
        case MotionEvent::ACTION_UP: {
            if (!mGesture.down) break;
            const bool multi = mGesture.multiPointer;
            const float x0 = mGesture.x0, y0 = mGesture.y0;
            const float maxDist = mGesture.maxDist;
            const long durMs = SystemClock::uptimeMillis() - mGesture.downAtMs;
            mGesture.down = false;
            recordGapSleep();
            if (multi) { recordLine("# multi-pointer gesture elided"); break; }
            const float dx = x - x0, dy = y - y0;
            const float dist = std::sqrt(dx * dx + dy * dy);
            if (mTouchSlop < 0) {
                mTouchSlop = ViewConfiguration::get(&App::getInstance()).getScaledTouchSlop();
            }
            if (dist <= mTouchSlop && maxDist <= mTouchSlop) {
                // AOSP ViewConfiguration.getLongPressTimeout() == 400ms
                recordManualClick(durMs >= LONGPRESS_TIMEOUT_MS ? "long-click" : "click", x, y);
            } else {
                char coords[64];
                snprintf(coords, sizeof(coords), "drag %d,%d %d,%d 10",
                         (int)x0, (int)y0, (int)x, (int)y);
                recordLine(coords);
            }
            break;
        }
        case MotionEvent::ACTION_CANCEL:
            mGesture.down = false;   // system aborted the gesture — nothing happened
            break;
        default:
            break;
        }
        return;
    }

    // Key: record on ACTION_DOWN only (UP/repeats would double every press).
    const KeyEvent& ke = (const KeyEvent&)e;
    if (ke.getAction() != KeyEvent::ACTION_DOWN || ke.getRepeatCount() > 0) return;
    recordGapSleep();
    if (ke.getKeyCode() == KeyEvent::KEYCODE_BACK) {
        recordLine("back");   // the DSL's global-BACK verb
    } else {
        recordLine("key " + std::to_string(ke.getKeyCode()));
    }
}

// static
void UiAutoTest::injectMarkedMotion(int action, float x, float y,
        int64_t downTimeMs, int64_t eventTimeMs) {
    MotionEvent* e = MotionEvent::obtain(downTimeMs, eventTimeMs, action, x, y, 0);
    e->setSource(InputDevice::SOURCE_TOUCHSCREEN);
    e->setFlags(e->getFlags() | FLAG_INJECTED_BY_TEST);
    UiAutomation::getInstance().injectInputEvent(*e, true);
    e->recycle();
}

void UiAutoTest::scriptNext() {
    if (!mRunning || mScriptIndex >= mScript.size()) { scriptDone(); return; }
    const Command& cmd = mScript[mScriptIndex];
    const int lineNo = mScript[mScriptIndex].line;
    UiAutomation& automation = UiAutomation::getInstance();

    if (cmd.verb == "back") {
        // Global BACK (no selector) — the same navigation escape the sweep
        // uses; lets recorded scripts express "return" without hunting for a
        // back-arrow label.
        const bool ok = automation.performGlobalAction(
                AccessibilityService::GLOBAL_ACTION_BACK);
        LOGI("SCRIPT %zu back -> %s", lineNo, ok ? "OK" : "unsupported (FAIL)");
        if (!ok) mScriptFails++;
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "sleep") {
        mScriptIndex++;
        stepHandler().postDelayed([this]() { scriptNext(); }, cmd.timeoutMs);
        return;
    }
    if (cmd.verb == "dump") {
        // Reuse the tree dump format: depth-indented class/text/bounds lines.
        AccessibilityNodeInfo* root = automation.getRootInActiveWindow();
        LOGI("SCRIPT %zu dump:", lineNo);
        std::function<void(AccessibilityNodeInfo*, int)> dump = [&](AccessibilityNodeInfo* n, int d) {
            if (!n) return;
            if (d > 20) { n->recycle(); return; }   // cut node is still owned
            Rect b; n->getBoundsInScreen(b);
            // TalkBack reading order: content description stands in when the
            // node has no text (uiautomator dump shows both attributes).
            std::string label = n->getText();
            if (label.empty()) label = n->getContentDescription();
            // State description (API 30+): ProgressBar %, Switch on/off, …
            std::string state = n->getStateDescription();
            if (!state.empty()) state = " state='" + state + "'";
            LOGI("  %*s%s [%s]%s (%d,%d %dx%d) clk=%d vis=%d en=%d", d * 2, "", n->getClassName().c_str(),
                 label.c_str(), state.c_str(), b.left, b.top, b.width, b.height,
                 n->isClickable(), n->isVisibleToUser(), n->isEnabled());
            for (int i = 0; i < n->getChildCount(); i++) dump(n->getChild(i), d + 1);
            n->recycle();   // every walked node is pool-owned, root included
        };
        dump(root, 0);
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }

    if (cmd.verb == "key") {
        /*Raw key injection (the manual recorder writes one line per key
          press — the replay walks the REAL input path, IME/KeyListener and
          all). Stack KeyEvent + initialize: injectInputEvent copies the
          event, so nothing leaks. FLAG_INJECTED_BY_TEST keeps a concurrent
          recorder from echoing the replay into its script.*/
        const int keyCode = atoi(cmd.selector.c_str());
        KeyEvent down, up;
        const int64_t now = SystemClock::uptimeMillis();
        down.initialize(0, InputDevice::SOURCE_KEYBOARD, 0, KeyEvent::ACTION_DOWN,
                FLAG_INJECTED_BY_TEST, keyCode, 0, 0, 0, now, now);
        up.initialize(0, InputDevice::SOURCE_KEYBOARD, 0, KeyEvent::ACTION_UP,
                FLAG_INJECTED_BY_TEST, keyCode, 0, 0, 0, now, now + 50);
        LOGI("SCRIPT %zu key %d", lineNo, keyCode);
        automation.injectInputEvent(down, true);
        automation.injectInputEvent(up, true);
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "drag") {
        /*Coordinate drag (manual recorder): DOWN at the start, interpolated
          MOVEs (~16ms apart, eventTime rising so velocity math sees a real
          gesture), UP last. The step completes only after the UP lands, so
          the next verb observes the scrolled layout.*/
        float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        int steps = 10;
        parseCoordPair(cmd.selector, &x0, &y0);
        if (!cmd.args.empty()) parseCoordPair(cmd.args[0], &x1, &y1);
        if (cmd.args.size() > 1) steps = std::max(1, atoi(cmd.args[1].c_str()));
        const int64_t downTime = SystemClock::uptimeMillis();
        LOGI("SCRIPT %zu drag %g,%g -> %g,%g (%d steps)", lineNo, x0, y0, x1, y1, steps);
        injectMarkedMotion(MotionEvent::ACTION_DOWN, x0, y0, downTime, downTime);
        for (int i = 1; i <= steps; i++) {
            const float t = (float)i / steps;
            stepHandler().postDelayed([this, x0, y0, x1, y1, t, downTime, i]() {
                injectMarkedMotion(MotionEvent::ACTION_MOVE,
                        x0 + (x1 - x0) * t, y0 + (y1 - y0) * t,
                        downTime, SystemClock::uptimeMillis());
            }, i * 16);
        }
        stepHandler().postDelayed([this, x1, y1, downTime, steps]() {
            injectMarkedMotion(MotionEvent::ACTION_UP, x1, y1,
                    downTime, SystemClock::uptimeMillis());
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
        }, (steps + 1) * 16);
        return;
    }
    float tapX = 0, tapY = 0;
    if (cmd.verb == "tap" && cmd.selector.find(',') != std::string::npos
            && parseCoordPair(cmd.selector, &tapX, &tapY)) {
        /*Coordinate form (manual recorder): fire-and-forget DOWN/UP at the
          raw point. The recorded target may have no a11y node (IME keys,
          candidates) — no TYPE_VIEW_CLICKED arbitration and no FAIL on
          silence; the touch pipeline itself is the verification.*/
        LOGI("SCRIPT %zu tap %g,%g (coords)", lineNo, tapX, tapY);
        const int64_t downTime = SystemClock::uptimeMillis();
        injectMarkedMotion(MotionEvent::ACTION_DOWN, tapX, tapY, downTime, downTime);
        stepHandler().postDelayed([this, x = tapX, y = tapY, downTime]() {
            injectMarkedMotion(MotionEvent::ACTION_UP, x, y,
                    downTime, SystemClock::uptimeMillis());
        }, REGULAR_CLICK_LENGTH);
        mScriptIndex++;
        stepHandler().postDelayed([this]() { scriptNext(); }, REGULAR_CLICK_LENGTH + 50);
        return;
    }

    AccessibilityNodeInfo* node = findOne(cmd);
    if (cmd.verb == "wait") {
        // Until.hasObject with criteria: a node that exists but fails the
        // predicates keeps polling like a not-yet-arrived one.
        std::string why;
        const bool satisfied = node != nullptr && checkNodeProperties(node, cmd.args, why);
        if (node != nullptr) node->recycle();
        if (satisfied) {
            LOGI("SCRIPT %zu wait %s -> FOUND", lineNo, cmd.selector.c_str());
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
        } else if (cmd.timeoutMs > 0) {
            // Poll cadence; cmd is const so decrement the stored budget.
            mScript[mScriptIndex].timeoutMs -= 250;
            stepHandler().postDelayed([this]() { scriptNext(); }, 250);
        } else {
            LOGE("SCRIPT %zu wait %s -> TIMEOUT%s (FAIL)", lineNo, cmd.selector.c_str(),
                 why.empty() ? "" : (" (" + why + ")").c_str());
            mScriptFails++;
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
        }
        return;
    }
    if (cmd.verb == "wait-absent") {
        // Until.gone: poll until no match remains (a menu that dismisses on
        // selection, a dialog that closes) — the inverse budget of wait.
        if (node == nullptr) {
            LOGI("SCRIPT %zu wait-absent %s -> GONE", lineNo, cmd.selector.c_str());
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
        } else {
            node->recycle();
            if (cmd.timeoutMs > 0) {
                mScript[mScriptIndex].timeoutMs -= 250;
                stepHandler().postDelayed([this]() { scriptNext(); }, 250);
            } else {
                LOGE("SCRIPT %zu wait-absent %s -> STILL PRESENT (FAIL)",
                     lineNo, cmd.selector.c_str());
                mScriptFails++;
                mScriptIndex++;
                stepHandler().post([this]() { scriptNext(); });
            }
        }
        return;
    }
    if (cmd.verb == "assert") {
        // check(matches(...)): absent fails, and so does any predicate that
        // does not hold — the reason lands in the log for free.
        std::string why = "not found";
        bool ok = false;
        if (node != nullptr) {
            ok = checkNodeProperties(node, cmd.args, why);
            node->recycle();
        }
        LOGI("SCRIPT %zu assert %s -> %s%s", lineNo, cmd.selector.c_str(),
             ok ? "OK" : "FAIL", ok ? "" : (" (" + why + ")").c_str());
        if (!ok) mScriptFails++;
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "assert-absent") {
        const bool ok = node == nullptr;
        LOGI("SCRIPT %zu assert-absent %s -> %s", lineNo, cmd.selector.c_str(), ok ? "OK" : "FAIL");
        if (!ok) { mScriptFails++; node->recycle(); }
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "sclick") {
        // uiautomator UiScrollable.getChildByText(): page the window's
        // scrollable until the selector's row is attached and visible, then
        // click it. ListView materializes only the visible rows — offscreen
        // items have no a11y nodes at all, so a plain "click text=" can never
        // address them (a choice list opened with setSelectionFromTop starts
        // scrolled to the current value). Pages backward first, then forward;
        // the timeout budget bounds a scrollable that never reaches its edge.
        if (mSclickLine != (size_t)lineNo) {
            mSclickLine = lineNo;
            mSclickPhase = 0;
            mSclickHasPending = false;
            mScript[mScriptIndex].timeoutMs = 8000;
        }
        if (node != nullptr && node->isVisibleToUser()) {
            // uiautomator clicks by coordinates (UiObject.getVisibleBounds +
            // InteractionController.click): a semantic ACTION_CLICK resolves
            // the row's adapter position through the paged list's internal
            // mapping, which can be stale right after a scroll (observed:
            // clicking "5 minutes" selecting "25 minutes" — an exact one-page
            // offset). The touch pipeline hit-tests the on-screen row, and the
            // DOWN anchors the stream so the posted UP still lands on it.
            Rect b; node->getBoundsInScreen(b);
            node->recycle();
            // Cross-frame stability gate: the swipe's UP arms a fling, and a
            // coasting list shows identical rects to two same-frame lookups.
            // Record the rect, re-read 200ms later, tap only on a match.
            if (!mSclickHasPending) {
                mSclickHasPending = true;
                mSclickPending = b;
                stepHandler().postDelayed([this]() { scriptNext(); }, 200);
                return;
            }
            const bool stable = mSclickPending.left == b.left && mSclickPending.top == b.top
                    && mSclickPending.width == b.width && mSclickPending.height == b.height;
            mSclickHasPending = false;
            if (!stable) {
                if ((mScript[mScriptIndex].timeoutMs -= 200) <= 0) {
                    LOGE("SCRIPT %zu sclick %s -> NOT FOUND (FAIL)", lineNo, cmd.selector.c_str());
                    mScriptFails++;
                    mScriptIndex++;
                    stepHandler().post([this]() { scriptNext(); });
                    return;
                }
                stepHandler().postDelayed([this]() { scriptNext(); }, 200);
                return;
            }
            const float x = b.left + b.width / 2.0f;
            const float y = b.top + b.height / 2.0f;
            const int64_t downTime = SystemClock::uptimeMillis();
            LOGI("SCRIPT %zu sclick %s tap %g,%g", lineNo, cmd.selector.c_str(), x, y);
            // The coordinate-tap idiom (the tap verb): DOWN inline, UP posted
            // +REGULAR_CLICK_LENGTH, no TYPE_VIEW_CLICKED arbitration — the
            // touch pipeline is the verification. (Routing the DOWN through
            // executeAndWaitForEvent's command handler defers it past the
            // wait; the back-to-back late dispatch reads as a long press and
            // the item never clicks.)
            injectMarkedMotion(MotionEvent::ACTION_DOWN, x, y, downTime, downTime);
            stepHandler().postDelayed([this, x, y, downTime]() {
                injectMarkedMotion(MotionEvent::ACTION_UP, x, y,
                        downTime, SystemClock::uptimeMillis());
            }, REGULAR_CLICK_LENGTH);
            mScriptIndex++;
            stepHandler().postDelayed([this]() { scriptNext(); }, REGULAR_CLICK_LENGTH + 50);
            return;
        }
        if (node != nullptr) node->recycle();
        // UiScrollable pages with swipe gestures. ACTION_SCROLL_* (a verbatim
        // android-36 smoothScrollBy) animates through the frame driver, which
        // never advances on an idle dialog window — the swipe drives the touch
        // pipeline directly and sticks. Phase 0 swipes down (reveal items
        // above), phase 1 swipes up.
        const int action = (mSclickPhase == 0)
                ? AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD
                : AccessibilityNodeInfo::ACTION_SCROLL_FORWARD;
        AccessibilityNodeInfo* root = automation.getRootInActiveWindow();
        AccessibilityNodeInfo* scroller = (root != nullptr)
                ? findActionDescendant(root, action) : nullptr;
        bool paged = false;
        if (scroller != nullptr) {
            Rect b; scroller->getBoundsInScreen(b);
            if (b.width > 0 && b.height > 0) {
                const float cx = b.left + b.width / 2.0f;
                const float y0 = b.top + b.height * (mSclickPhase == 0 ? 0.30f : 0.70f);
                const float y1 = b.top + b.height * (mSclickPhase == 0 ? 0.70f : 0.30f);
                const int64_t downTime = SystemClock::uptimeMillis();
                injectMarkedMotion(MotionEvent::ACTION_DOWN, cx, y0, downTime, downTime);
                // Slow cadence: a fast swipe arms a long fling; ~900px/s
                // keeps the coast short enough for the settle + stability gate.
                constexpr int steps = 20;
                constexpr int cadenceMs = 25;
                for (int i = 1; i <= steps; i++) {
                    const float t = (float)i / steps;
                    stepHandler().postDelayed([this, cx, y0, y1, t, downTime, i, cadenceMs]() {
                        injectMarkedMotion(MotionEvent::ACTION_MOVE,
                                cx, y0 + (y1 - y0) * t, downTime, downTime + i * cadenceMs);
                    }, i * cadenceMs);
                }
                stepHandler().postDelayed([this, cx, y1, downTime]() {
                    injectMarkedMotion(MotionEvent::ACTION_UP, cx, y1, downTime,
                            SystemClock::uptimeMillis());
                }, (steps + 1) * cadenceMs);
                paged = true;
            }
        }
        if (scroller != nullptr) scroller->recycle();
        if (root != nullptr && root != scroller) root->recycle();
        if (paged) {
            // Re-enter only AFTER the swipe's own UP has dispatched (the settle
            // must exceed the injection cadence) plus a coast margin; the
            // stability gate covers the fling tail.
            constexpr int settleMs = (20 + 1) * 25 + 400;
            mScript[mScriptIndex].timeoutMs -= settleMs;
            stepHandler().postDelayed([this]() { scriptNext(); }, settleMs);
            return;
        }
        if (!paged && mSclickPhase == 0) {   // no scrollable for this direction — try the other
            mSclickPhase = 1;
            stepHandler().post([this]() { scriptNext(); });
            return;
        }
        LOGE("SCRIPT %zu sclick %s -> NOT FOUND (FAIL)", lineNo, cmd.selector.c_str());
        mScriptFails++;
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "click") {
        if (node == nullptr) {
            LOGE("SCRIPT %zu click %s -> NOT FOUND (FAIL)", lineNo, cmd.selector.c_str());
            mScriptFails++;
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
            return;
        }
        performScriptClick(node, lineNo, cmd.selector);
        return;
    }
    if (cmd.verb == "long-click") {
        if (node == nullptr) {
            LOGE("SCRIPT %zu long-click %s -> NOT FOUND (FAIL)", lineNo, cmd.selector.c_str());
            mScriptFails++;
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
            return;
        }
        // Same ancestor-walk as click, plus long-clickable itself (AOSP's
        // ACTION_LONG_CLICK handler gates on isLongClickable).
        AccessibilityNodeInfo* clickTarget = node;
        while (!clickTarget->isClickable() && !clickTarget->isLongClickable()) {
            AccessibilityNodeInfo* parent = clickTarget->getParent();
            if (parent == nullptr) break;
            if (clickTarget != node) clickTarget->recycle();
            clickTarget = parent;
        }
        clickTarget->performAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);
        AccessibilityEvent* hit = automation.executeAndWaitForEvent(
            [clickTarget]() { clickTarget->performAction(AccessibilityNodeInfo::ACTION_LONG_CLICK); },
            [](AccessibilityEvent& e) {
                return e.getEventType() == AccessibilityEvent::TYPE_VIEW_LONG_CLICKED; },
            1500);
        LOGI("SCRIPT %zu long-click %s -> %s", lineNo, cmd.selector.c_str(),
             hit ? "OK" : "no-event (FAIL)");
        if (!hit) mScriptFails++;
        if (hit) hit->recycle();
        if (clickTarget != node) clickTarget->recycle();
        node->recycle();
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "scroll") {
        // ACTION_SCROLL_FORWARD/BACKWARD performed on the node that
        // advertises it (selector matches are often the row label).
        const bool forward = cmd.args.empty() || cmd.args[0] != "backward";
        const int action = forward ? AccessibilityNodeInfo::ACTION_SCROLL_FORWARD
                                   : AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD;
        AccessibilityNodeInfo* target = (node == nullptr) ? nullptr
                : resolveByAction(node, action);
        const bool ok = target != nullptr && target->performAction(action);
        LOGI("SCRIPT %zu scroll %s %s -> %s", lineNo, cmd.selector.c_str(),
             forward ? "forward" : "backward", ok ? "OK" : "FAIL");
        if (!ok) mScriptFails++;
        if (target != nullptr && target != node) target->recycle();
        if (node != nullptr) node->recycle();
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "set-progress") {
        // UiObject2.setProgress(float): ACTION_SET_PROGRESS with
        // ARGUMENT_PROGRESS_VALUE — the standard automation way to drive a
        // SeekBar (AbsSeekBar clamps and notifies with fromUser=true).
        const float value = cmd.args.empty() ? 0.0f : strtof(cmd.args[0].c_str(), nullptr);
        AccessibilityNodeInfo* target = (node == nullptr) ? nullptr
                : resolveByAction(node, cdroid::internal::R::id::accessibilityActionSetProgress);
        bool ok = false;
        if (target != nullptr && !cmd.args.empty()) {
            Bundle arguments;
            arguments.putFloat(AccessibilityNodeInfo::ACTION_ARGUMENT_PROGRESS_VALUE, value);
            ok = target->performAction(
                    cdroid::internal::R::id::accessibilityActionSetProgress, &arguments);
        }
        LOGI("SCRIPT %zu set-progress %s -> %g : %s", lineNo, cmd.selector.c_str(),
             value, ok ? "OK" : "FAIL");
        if (!ok) mScriptFails++;
        if (target != nullptr && target != node) target->recycle();
        if (node != nullptr) node->recycle();
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "set-text") {
        // ACTION_SET_TEXT with ARGUMENT_SET_TEXT (editors advertise it; the
        // standard action is performed regardless of CDROID's handler state).
        std::string value;
        for (size_t i = 0; i < cmd.args.size(); i++) {
            if (i > 0) value += ' ';
            value += cmd.args[i];
        }
        AccessibilityNodeInfo* target = (node == nullptr) ? nullptr
                : resolveByAction(node, AccessibilityNodeInfo::ACTION_SET_TEXT);
        bool ok = false;
        if (target != nullptr) {
            Bundle arguments;
            arguments.putString(AccessibilityNodeInfo::ACTION_ARGUMENT_SET_TEXT_CHARSEQUENCE, value);
            ok = target->performAction(AccessibilityNodeInfo::ACTION_SET_TEXT, &arguments);
        }
        LOGI("SCRIPT %zu set-text %s -> %s", lineNo, cmd.selector.c_str(), ok ? "OK" : "FAIL");
        if (!ok) mScriptFails++;
        if (target != nullptr && target != node) target->recycle();
        if (node != nullptr) node->recycle();
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "tap") {
        // The gesture wheel: an injected coordinate tap (DOWN, UP after
        // REGULAR_CLICK_LENGTH) through the input pipeline — what legacy
        // uiautomator's click actually is. A tap on a SeekBar seeks to that
        // position (tap-to-seek); such targets produce no VIEW_CLICKED, so
        // the filter accepts VIEW_SELECTED/CONTENT_CHANGED too — the exact
        // set legacy clickAndSync waits on.
        bool ok = false;
        if (node != nullptr) {
            Rect b;
            node->getBoundsInScreen(b);
            if (b.width > 0 && b.height > 0) {
                const float x = b.left + b.width / 2.0f;
                const float y = b.top + b.height / 2.0f;
                const nsecs_t downTime = SystemClock::uptimeMillis();
                MotionEvent* down = MotionEvent::obtain(downTime, downTime,
                        MotionEvent::ACTION_DOWN, x, y, 0);
                down->setSource(InputDevice::SOURCE_TOUCHSCREEN);
                down->setFlags(down->getFlags() | FLAG_INJECTED_BY_TEST);
                automation.injectInputEvent(*down, true);
                down->recycle();
                AccessibilityEvent* hit = automation.executeAndWaitForEvent(
                    [&automation, x, y, downTime]() {
                        stepHandler().postDelayed([&automation, x, y, downTime]() {
                            MotionEvent* up = MotionEvent::obtain(downTime,
                                    SystemClock::uptimeMillis(), MotionEvent::ACTION_UP, x, y, 0);
                            up->setSource(InputDevice::SOURCE_TOUCHSCREEN);
                            up->setFlags(up->getFlags() | FLAG_INJECTED_BY_TEST);
                            automation.injectInputEvent(*up, true);
                            up->recycle();
                        }, REGULAR_CLICK_LENGTH);
                    },
                    [](AccessibilityEvent& e) {
                        return e.getEventType() == AccessibilityEvent::TYPE_VIEW_CLICKED
                                || e.getEventType() == AccessibilityEvent::TYPE_VIEW_SELECTED
                                || e.getEventType() == AccessibilityEvent::TYPE_WINDOW_CONTENT_CHANGED; },
                    1500);
                ok = hit != nullptr;
                if (hit) hit->recycle();
            }
        }
        LOGI("SCRIPT %zu tap %s -> %s", lineNo, cmd.selector.c_str(),
             ok ? "OK" : "no-event (FAIL)");
        if (!ok) mScriptFails++;
        if (node != nullptr) node->recycle();
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    if (cmd.verb == "perform") {
        // UiObject2.performAction(action, bundle): any standard action by
        // name, typed arguments from the standard key table.
        if (cmd.args.empty()) {
            LOGE("SCRIPT %zu perform: missing action name (FAIL)", lineNo);
            mScriptFails++;
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
            return;
        }
        const auto entry = standardActionTable().find(cmd.args[0]);
        if (entry == standardActionTable().end()) {
            LOGE("SCRIPT %zu perform: unknown action '%s' (FAIL)", lineNo, cmd.args[0].c_str());
            mScriptFails++;
            if (node != nullptr) node->recycle();
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
            return;
        }
        Bundle arguments;
        std::string argError;
        if (!buildArguments(std::vector<std::string>(cmd.args.begin() + 1, cmd.args.end()),
                            arguments, argError)) {
            LOGE("SCRIPT %zu perform: %s (FAIL)", lineNo, argError.c_str());
            mScriptFails++;
            if (node != nullptr) node->recycle();
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
            return;
        }
        const int actionId = entry->second;
        // Resolve to the node advertising the action when the selector match
        // is a label child; otherwise perform on the matched node itself
        // (the raw passthrough — UiObject2 performs on the found object).
        AccessibilityNodeInfo* target = (node == nullptr) ? nullptr
                : resolveByAction(node, actionId);
        if (target == nullptr) target = node;
        const bool ok = target != nullptr && target->performAction(actionId, &arguments);
        LOGI("SCRIPT %zu perform %s %s -> %s", lineNo, cmd.args[0].c_str(),
             cmd.selector.c_str(), ok ? "OK" : "FAIL");
        if (!ok) mScriptFails++;
        if (target != node && target != nullptr) target->recycle();
        if (node != nullptr) node->recycle();
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }
    LOGE("SCRIPT %zu unknown verb '%s'", lineNo, cmd.verb.c_str());
    mScriptFails++;
    mScriptIndex++;
    stepHandler().post([this]() { scriptNext(); });
}

void UiAutoTest::scriptDone() {
    mRunning = false;
    LOGI("AUTOTEST script done: %zu commands, %d FAIL(S) — exit",
         mScript.size(), mScriptFails);
    App::getInstance().exit(mScriptFails);
}

} /*endof namespace*/
