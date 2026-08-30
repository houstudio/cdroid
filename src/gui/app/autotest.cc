#include <app/autotest.h>
#include <app/uiautomation.h>
#include <core/looper.h>
#include <core/app.h>
#include <core/handler.h>
#include <core/systemclock.h>
#include <core/windowmanager.h>
#include <core/tokenizer.h>
#include <widget/cdwindow.h>   // Window (createAccessibilityNodeInfo, TYPE_SYSTEM_WINDOW)
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <view/accessibility/accessibilitymanager.h>   // getActiveApplicationWindow
#include <accessibilityservice/accessibilityservice.h>   // GLOBAL_ACTION_BACK
#include <porting/cdlog.h>
#include <algorithm>
#include <sstream>
#include <functional>

namespace cdroid {

namespace {
constexpr int kMaxDepth = 16;
// A standing handler: the posted steps must outlive each call frame (the
// cdwindow teardown-post idiom — a temporary Handler can drop them).
Handler& stepHandler() {
    static Handler sHandler(Looper::getMainLooper());
    return sHandler;
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
    if (node == nullptr || depth > kMaxDepth) return;
    if (node->isVisibleToUser() && node->isClickable() && node->isEnabled()) {
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
        // A window with no a11y targets (e.g. a menu PopupWindow whose items
        // are not exposed as clickable) would loop here silently forever —
        // escape it the way a screen-reader user would: synthesize BACK
        // after a few empty passes (dismisses the popup; on a bare main
        // window BACK ends the app, which also terminates a sweep that has
        // nothing left to test).
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
            if (!sel.empty()) {
                // wait carries the poll budget (click alone fails fast on a
                // not-yet-arrived page), so replay survives timing.
                mRecord << "wait \"text=" << sel << "\" 5000\n"
                        << "click \"text=" << sel << "\"\n";
            } else {
                mRecord << "# step " << mStepCount << ": " << target->getClassName()
                        << " — no text selector\n";
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
        if (!n || d > kMaxDepth) return;
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
//   click  text=登录          highlight + semantic click, wait VIEW_CLICKED
//   assert text=欢迎          fail if no match RIGHT NOW
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
        if (!t->isEol() && t->peekChar() != '#') {  // optional ms argument
            cmd.timeoutMs = atol(t->nextToken(" \t\r").c_str());
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
            if (!n || d > 20) return;
            Rect b; n->getBoundsInScreen(b);
            LOGI("  %*s%s [%s] (%d,%d %dx%d) clk=%d vis=%d en=%d", d * 2, "", n->getClassName().c_str(),
                 n->getText().c_str(), b.left, b.top, b.width, b.height,
                 n->isClickable(), n->isVisibleToUser(), n->isEnabled());
            for (int i = 0; i < n->getChildCount(); i++) dump(n->getChild(i), d + 1);
        };
        dump(root, 0);
        if (root) root->recycle();
        mScriptIndex++;
        stepHandler().post([this]() { scriptNext(); });
        return;
    }

    AccessibilityNodeInfo* node = findOne(cmd);
    if (cmd.verb == "wait") {
        if (node != nullptr) {
            LOGI("SCRIPT %zu wait %s -> FOUND", lineNo, cmd.selector.c_str());
            node->recycle();
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
        } else if (cmd.timeoutMs > 0) {
            // Poll cadence; cmd is const so decrement the stored budget.
            mScript[mScriptIndex].timeoutMs -= 250;
            stepHandler().postDelayed([this]() { scriptNext(); }, 250);
        } else {
            LOGE("SCRIPT %zu wait %s -> TIMEOUT (FAIL)", lineNo, cmd.selector.c_str());
            mScriptFails++;
            mScriptIndex++;
            stepHandler().post([this]() { scriptNext(); });
        }
        return;
    }
    if (cmd.verb == "assert") {
        const bool ok = node != nullptr;
        LOGI("SCRIPT %zu assert %s -> %s", lineNo, cmd.selector.c_str(), ok ? "OK" : "FAIL");
        if (!ok) mScriptFails++;
        if (node) node->recycle();
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
