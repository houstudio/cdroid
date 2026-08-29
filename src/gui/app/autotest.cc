#include <app/autotest.h>
#include <app/uiautomation.h>
#include <core/looper.h>
#include <core/app.h>
#include <core/handler.h>
#include <core/systemclock.h>
#include <core/windowmanager.h>
#include <core/tokenizer.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitynodeinfo.h>
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

void UiAutoTest::start(long stepIntervalMs) {
    mStepIntervalMs = stepIntervalMs;
    if (mRunning) return;
    UiAutomation::getInstance().connect();  // the backing service (idempotent)
    mRunning = true;
    mStepCount = 0;
    mScanIndex = (size_t)-1;
    LOGI("AUTOTEST sweep start (interval %ldms)", mStepIntervalMs);
    stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
}

void UiAutoTest::stop() {
    mRunning = false;
    LOGI("AUTOTEST sweep stop after %d steps", mStepCount);
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

void UiAutoTest::step() {
    if (!mRunning) return;
    UiAutomation& automation = UiAutomation::getInstance();
    // Follow window navigation (dialogs/sub-activities): stale snapshot would
    // click a now-background window invisibly.
    if (Window* active = WindowManager::getInstance().getActiveWindow()) {
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
    std::sort(mClickables.begin(), mClickables.end(),
              [](AccessibilityNodeInfo* a, AccessibilityNodeInfo* b) {
                  Rect ra, rb;
                  a->getBoundsInScreen(ra);
                  b->getBoundsInScreen(rb);
                  if (ra.top != rb.top) return ra.top < rb.top;
                  return ra.left < rb.left;
              });
    if (mClickables.empty()) {
        stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
        return;
    }
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
    // collectClickable recycled the walk scaffolding (root included).
    mStepCount++;
    AccessibilityNodeInfo* target = mClickables.at(mStepCount % mClickables.size());
    const std::string label = target->getText().empty()
            ? target->getContentDescription() : target->getText();

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
         mStepCount % mClickables.size() + 1, mClickables.size(),
         label.c_str(), hit ? "PASS" : "no-event");
    if (hit) hit->recycle();

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
    AccessibilityNodeInfo* root = automation.getRootInActiveWindow();
    if (root == nullptr) return nullptr;
    std::vector<AccessibilityNodeInfo*> hits = c.byText
            ? root->findAccessibilityNodeInfosByText(c.selector)
            : root->findAccessibilityNodeInfosByViewId(c.selector);
    AccessibilityNodeInfo* first = hits.empty() ? nullptr : hits.front();
    for (size_t i = 1; i < hits.size(); i++) hits[i]->recycle();
    root->recycle();
    return first;  // caller recycles
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

void UiAutoTest::scriptNext() {
    if (!mRunning || mScriptIndex >= mScript.size()) { scriptDone(); return; }
    const Command& cmd = mScript[mScriptIndex];
    const int lineNo = mScript[mScriptIndex].line;
    UiAutomation& automation = UiAutomation::getInstance();

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
            LOGI("  %*s%s [%s] (%d,%d %dx%d)", d * 2, "", n->getClassName().c_str(),
                 n->getText().c_str(), b.left, b.top, b.width, b.height);
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
        node->performAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);  // visible target
        AccessibilityEvent* hit = automation.executeAndWaitForEvent(
            [node]() { node->performAction(AccessibilityNodeInfo::ACTION_CLICK); },
            [](AccessibilityEvent& e) {
                return e.getEventType() == AccessibilityEvent::TYPE_VIEW_CLICKED; },
            1500);
        LOGI("SCRIPT %zu click %s -> %s", lineNo, cmd.selector.c_str(),
             hit ? "OK" : "no-event (FAIL)");
        if (!hit) mScriptFails++;
        if (hit) hit->recycle();
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
