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
} // namespace

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
            for (size_t i = 0; i < mClickables.size(); i++) {
                Rect b; mClickables[i]->getBoundsInScreen(b);
                if (mClickables[i]->getClassName() == cursorIt->second.cls
                        && b.left == cursorIt->second.left && b.top == cursorIt->second.top) {
                    idx = (i + 1) % mClickables.size();
                    cursorHit = true;
                    break;
                }
            }
        }
        if (!cursorHit && mLastClickedValid) {
            // Unknown page (first visit, or the cursor identity left the
            // viewport): resume AFTER the identity touched on the previous
            // step when this snapshot still contains it. Shared chrome — a
            // TabLayout strip keeps the same tabs on every page — then
            // advances one tab per step in a single pass, instead of every
            // navigation resetting the walk to the first tab (widgetsDemo
            // spent whole rounds up in the strip otherwise).
            for (size_t i = 0; i < mClickables.size(); i++) {
                Rect b; mClickables[i]->getBoundsInScreen(b);
                if (mClickables[i]->getClassName() == mLastClicked.cls
                        && b.left == mLastClicked.left && b.top == mLastClicked.top) {
                    idx = (i + 1) % mClickables.size();
                    break;
                }
            }
        }
    }
    AccessibilityNodeInfo* target = mClickables.at(idx);
    if (!mRandomWalk) {
        // Remember the target we are ABOUT to click, so the next visit to this
        // page resumes after it — and an unknown next page resumes after the
        // same identity (mLastClicked).
        Rect tb; target->getBoundsInScreen(tb);
        mPageCursor[pageSig] = { target->getClassName(), tb.left, tb.top };
        mLastClicked = { target->getClassName(), tb.left, tb.top };
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
                std::string sel = label;
                for (auto& ch : sel) if (ch == '"' || ch == '\n' || ch == '\r') ch = ' ';
                const std::string resName = target->getViewIdResourceName();
                if (!sel.empty()) {
                    mRecord << "wait \"text=" << sel << "\" 5000\n"
                            << "scroll \"text=" << sel << "\" "
                            << (seekAction == AccessibilityNodeInfo::ACTION_SCROLL_FORWARD
                                ? "forward" : "backward") << "\n";
                } else if (!resName.empty()) {
                    // uiautomator records by resource-id when text is absent
                    // (FLAG_REPORT_VIEW_IDS makes nodes carry it).
                    mRecord << "wait id=" << resName << " 5000\n"
                            << "scroll id=" << resName << " "
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
            std::string sel = label;
            for (auto& ch : sel) if (ch == '"' || ch == '\n' || ch == '\r') ch = ' ';
            const std::string resName = target->getViewIdResourceName();
            if (!sel.empty()) {
                // wait carries the poll budget (click alone fails fast on a
                // not-yet-arrived page), so replay survives timing.
                mRecord << "wait \"text=" << sel << "\" 5000\n"
                        << "click \"text=" << sel << "\"\n";
            } else if (!resName.empty()) {
                // uiautomator records by resource-id when text is absent
                // (FLAG_REPORT_VIEW_IDS makes nodes carry it).
                mRecord << "wait id=" << resName << " 5000\n"
                        << "click id=" << resName << "\n";
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
            AccessibilityNodeInfo* first = hits.front();
            for (size_t i = 1; i < hits.size(); i++) hits[i]->recycle();
            for (AccessibilityNodeInfo* other : roots) other->recycle();
            return first;  // caller recycles
        }
    }
    for (AccessibilityNodeInfo* r : roots) r->recycle();
    return nullptr;
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
        }
        // Quotes may wrap the whole selector ("a b") or the value (text="a b").
        auto stripQuotes = [](std::string& s) {
            if (s.size() >= 2 && s.front() == '"' && s.back() == '"') s = s.substr(1, s.size() - 2);
        };
        if (sel.rfind("text=", 0) == 0) { cmd.byText = true; cmd.selector = sel.substr(5); stripQuotes(cmd.selector); }
        else if (sel.rfind("id=", 0) == 0) { cmd.byText = false; cmd.selector = sel.substr(3); stripQuotes(cmd.selector); }
        else { cmd.byText = true; cmd.selector = sel; stripQuotes(cmd.selector); }  // bare == text=
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
    mRecord << "# AUTOTEST sweep recording — replay with --test-script\n";
    mRecord.flush();
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
    if (cmd.verb == "click") {
        if (node == nullptr) {
            LOGE("SCRIPT %zu click %s -> NOT FOUND (FAIL)", lineNo, cmd.selector.c_str());
            mScriptFails++;
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
            return;
        }
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
        LOGI("SCRIPT %zu click %s -> %s", lineNo, cmd.selector.c_str(),
             hit ? "OK" : "no-event (FAIL)");
        if (!hit) mScriptFails++;
        if (hit) hit->recycle();
        if (clickTarget != node) clickTarget->recycle();
        node->recycle();
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
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
                automation.injectInputEvent(*down, true);
                down->recycle();
                AccessibilityEvent* hit = automation.executeAndWaitForEvent(
                    [&automation, x, y, downTime]() {
                        stepHandler().postDelayed([&automation, x, y, downTime]() {
                            MotionEvent* up = MotionEvent::obtain(downTime,
                                    SystemClock::uptimeMillis(), MotionEvent::ACTION_UP, x, y, 0);
                            up->setSource(InputDevice::SOURCE_TOUCHSCREEN);
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
