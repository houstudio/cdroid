#include <app/autotest.h>
#include <app/uiautomation.h>
#include <core/looper.h>
#include <core/app.h>
#include <core/handler.h>
#include <core/systemclock.h>
#include <core/tokenizer.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <porting/cdlog.h>
#include <algorithm>
#include <sstream>
#include <functional>

namespace cdroid {

namespace {
// Screen clip for the visibility filter (the active window's frame).
constexpr int kScreenWidth = 1280;
constexpr int kScreenHeight = 720;
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
    Rect b; node->getBoundsInScreen(b);
    if (b.left < kScreenWidth && b.top < kScreenHeight
            && b.left + b.width > 0 && b.top + b.height > 0
            && node->isClickable() && node->isEnabled()) {
        mClickables.push_back(node);
        return;  // clickable subtrees are activation units; don't descend
    }
    for (int i = 0; i < node->getChildCount(); i++) {
        collectClickable(node->getChild(i), depth + 1);
    }
    // Non-branching intermediate nodes were only walk scaffolding.
    if (depth > 0) node->recycle();
}

void UiAutoTest::step() {
    if (!mRunning) return;
    UiAutomation& automation = UiAutomation::getInstance();
    AccessibilityNodeInfo* root = automation.getRootInActiveWindow();
    if (root == nullptr) {  // no window yet (or between windows) — retry
        stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
        return;
    }
    // Refresh the snapshot on wrap (position-stable otherwise), like the
    // switchaccess scanner.
    if (mClickables.empty() || mScanIndex + 1 >= mClickables.size()) {
        for (auto* stale : mClickables) stale->recycle();
        mClickables.clear();
        mScanIndex = (size_t)-1;
        collectClickable(root, 0);
        std::sort(mClickables.begin(), mClickables.end(),
                  [](AccessibilityNodeInfo* a, AccessibilityNodeInfo* b) {
                      Rect ra, rb;
                      a->getBoundsInScreen(ra);
                      b->getBoundsInScreen(rb);
                      if (ra.top != rb.top) return ra.top < rb.top;
                      return ra.left < rb.left;
                  });
        root->recycle();  // walk scaffolding; clickables are separate objects
    } else {
        root->recycle();
    }
    if (mClickables.empty()) {  // root already recycled above on both paths
        stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
        return;
    }
    mScanIndex = (mScanIndex + 1) % mClickables.size();
    AccessibilityNodeInfo* target = mClickables.at(mScanIndex);
    const std::string label = target->getText().empty()
            ? target->getContentDescription() : target->getText();

    mStepCount++;
    // Visual feedback: a semantic click fires no pressed-state animation —
    // park the accessibility focus highlight on the target so the sweep is
    // observable on screen (the scanner's own highlight path).
    target->performAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);
    AccessibilityEvent* hit = automation.executeAndWaitForEvent(
        [target]() { target->performAction(AccessibilityNodeInfo::ACTION_CLICK); },
        [](AccessibilityEvent& e) {
            return e.getEventType() == AccessibilityEvent::TYPE_VIEW_CLICKED; },
        1500);
    LOGI("AUTOTEST [%d] %2zu/%zu [%s] -> %s", mStepCount, mScanIndex + 1,
         mClickables.size(), label.c_str(), hit ? "PASS" : "no-event");
    if (hit) hit->recycle();

    stepHandler().postDelayed([this]() { step(); }, mStepIntervalMs);
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
