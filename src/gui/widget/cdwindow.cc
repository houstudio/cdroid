/*
 * Copyright (C) 2015 UI project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//#include <cdroid.h>
#include <core/app.h>
#include <content/contextthemewrapper.h>
#include <core/intent.h>
#include <core/componentname.h>
#include <widget/cdwindow.h>
#include <widget/actionbar.h>
#include <widget/activitytransitioncoordinator.h>
#include <widget/internal_R.h>
#include <menu/menuinflater.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/focusfinder.h>
#include <core/systemclock.h>
#include <content/typedvalue.h>
#include <core/windowmanager.h>
#include <animation/animator.h>
#include <porting/cdlog.h>
#include <porting/cdgraph.h>

using namespace Cairo;
namespace cdroid {
using namespace cdroid::internal;
constexpr int FORWARD = 0;
constexpr int FINISH_HANDLED = 1;
constexpr int FINISH_NOT_HANDLED = 2;

Window::Window(Context*ctx,const AttributeSet*atts)
  :FrameLayout(ctx,atts){
    initWindow();
    Point pt;
    WindowManager::getInstance().getDefaultDisplay().getSize(pt);
    // Full-screen window: attributes mirror the frame laid out below (a later
    // relayoutWindow on MATCH_PARENT keeps it display-sized at (0,0)).
    mWindowAttributes.width  = WindowManager::LayoutParams::MATCH_PARENT;
    mWindowAttributes.height = WindowManager::LayoutParams::MATCH_PARENT;
    setFrame(0,0,pt.x,pt.y);
    WindowManager::getInstance().addWindow(this);
    mAttachInfo->mPlaySoundEffect = std::bind(&Window::playSoundImpl,this,std::placeholders::_1);
    loadThemeWindowAnimations();
    loadThemeWindowBackground();
    loadThemeCloseOnTouchOutside();
}

void Window::loadThemeCloseOnTouchOutside() {
    // AOSP PhoneWindow.generateLayout (PhoneWindow.java:2737-2745): a themed
    // windowCloseOnTouchOutside=true opts this window into outside-close.
    // (PopupDecorView paths skip the theme read — popups drive flags through
    // PopupWindow.computeFlags instead.)
    if (mContext == nullptr) return;
    static const uint32_t attrs[] = {R::attr::windowCloseOnTouchOutside, 0};
    auto ta = mContext->getTheme().obtainStyledAttributes(attrs);
    if (!ta) return;
    setCloseOnTouchOutsideIfNotSet(ta->getBoolean(0, false));
}

Window::Window(int x,int y,int width,int height,int type)
  // Delegate to the THEMED ctor: every window constructor installs the theme
  // dressing (windowBackground, window animations, closeOnTouchOutside) by
  // default — AOSP windows are themed windows; the bare-window flavor (popups,
  // toast) opts out through the explicit themeWindowAnimations=false argument.
  // This ctor used to build the context as a plain &App (no themed wrapper), so
  // Activity-family windows here had a fully transparent surface until the
  // subclass hand-called setTheme (widgetsDemo's transparent window) — and
  // setTheme itself could not apply (Window::setTheme casts mContext to
  // ContextThemeWrapper, which the bare assignment never was).
  // Delegation also keeps the first-frame pending region this ctor used to seed.
  : Window(&App::getInstance(), x, y, width, height, type){
    LOGD("Window::Window(%p)",this);
}

// AOSP PhoneWindow(context): same window, but the caller's (possibly themed —
// ContextThemeWrapper) context drives inflation instead of the global App.
// AOSP windows belong to an Activity, which IS a ContextThemeWrapper — CDROID
// windows are the Activity, so a plain context is wrapped in an empty
// ContextThemeWrapper overlay (inherits the app theme via lazy setTo(base)),
// giving every window its own theme for Window::setTheme()/recreate().
// This ctor owns the geometric setup too (the (x,y,w,h) ctor delegates here):
// display-size resolution, LayoutParams, frame, first-frame pending region,
// compositor registration. themeWindowAnimations=false is the bare-window
// flavor (popup decors, toast) that skips the theme dressing.
Window::Window(Context*ctx,int x,int y,int width,int height,int type, bool themeWindowAnimations)
  : FrameLayout(ctx ? ctx : &App::getInstance()),window_type(type){
    initWindow();
    LOGD("Window::Window(%p)",this);
    // AOSP performLaunchActivity applies the manifest theme (activity's, else
    // the application's) before the activity class instantiates; App routes it
    // through a pending slot so the themed overlay exists before the subclass
    // ctor inflates content.
    const int themeResId = App::getInstance().mPendingActivityTheme;
    if (dynamic_cast<ContextThemeWrapper*>(ctx) == nullptr) {
        mContext = new ContextThemeWrapper(ctx ? ctx : &App::getInstance(), themeResId);
        mOwnsContext = true;
    } else {
        mContext = ctx;
    }
    Point size;
    WindowManager::getInstance().getDefaultDisplay().getSize(size);
    if(width<0)  width = size.x;
    if(height<0) height= size.y;
    mWindowAttributes.x = x;
    mWindowAttributes.y = y;
    // initWindow() already mirrors window_type into LayoutParams.type for
    // every ctor (the compositor layers on it) — no second write needed here.
    mWindowAttributes.width  = width;
    mWindowAttributes.height = height;
    setFrame(x, y, width, height);
    mPendingRgn->do_union({0,0,width,height});
    WindowManager::getInstance().addWindow(this);
    mAttachInfo->mPlaySoundEffect = std::bind(&Window::playSoundImpl,this,std::placeholders::_1);
    // Theme dressing (windowAnimationStyle pair AND windowBackground/fallback) —
    // AOSP's generateLayout belongs to app/activity windows; popup decors and
    // the toast opt out (they carry their own animation style / draw their own
    // backdrop, and popups align to the anchor AFTER construction, so a
    // ctor-time enter snap would capture a stale resting position).
    if (themeWindowAnimations) {
        loadThemeWindowAnimations();
        loadThemeWindowBackground();
    }
}

void Window::initWindow(){
    mInLayout= false;
    mAccessibilityManager =&AccessibilityManager::getInstance(mContext);
    mSendWindowContentChangedAccessibilityEvent = nullptr;
    mPendingRgn = Cairo::Region::create();
    // window_type is member-initialized before initWindow() runs; mirror it into
    // the window attributes (AOSP keeps type only on LayoutParams — CDROID's
    // compositor layering reads Window::window_type, so the two stay in sync).
    mWindowAttributes.type = window_type;
    mActionBar = nullptr;
    mActionMode = nullptr;
    mMenuInflater = nullptr;
    setBackground(nullptr);
    setLayoutDirection(View::LAYOUT_DIRECTION_LTR);
    setTextDirection(View::TEXT_DIRECTION_LTR);
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    setFocusable(true);
    setKeyboardNavigationCluster(true);
    mA11yListenerAlive = std::make_shared<bool>(true);
    // Shared-element liveness token: other windows' return flights weak_ptr-watch it to see
    // this window die without dereferencing it (see ActivityTransitionCoordinator).
    mSceneLiveness = std::make_shared<bool>(true);
    // Stored in the member so ~Window can remove the exact entry: CallbackBase
    // copies alias, so the vector entry and mA11yStateListener compare equal.
    mA11yStateListener = AccessibilityManager::AccessibilityStateChangeListener(
            [this, alive = mA11yListenerAlive](bool enabled) {
        if (!*alive) return;  // the window is gone (exit-time unbind order)
        LOGD("%d",enabled);
        // both gates below were constant-true (||1) in the original port —
        // flattened; the mAttachInfo dereference they forced is also gone
        // (the listener can fire between initWindow and addWindow).
        sendAccessibilityEvent(AccessibilityEvent::TYPE_WINDOW_STATE_CHANGED);
        View* focusedView = findFocus();
        if ((focusedView != nullptr) && (focusedView != this)) {
            focusedView->sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_FOCUSED);
        }
        LOGD("focusedView=%d",focusedView);
    });
    mAccessibilityManager->addAccessibilityStateChangeListener(mA11yStateListener);
}

Window::~Window(){
    *mA11yListenerAlive = false;  // detach the manager's state listener
    // Unregister the exact functor (the member copy aliases the stored entry) —
    // previously only the alive-flag dropped, leaking one dead closure per
    // window in the manager's vector (every popup show creates a window).
    mAccessibilityManager->removeAccessibilityStateChangeListener(mA11yStateListener);
    if (mActionMode != nullptr) {
        ActionMode* mode = mActionMode;
        mActionMode = nullptr;
        mode->finish();
    }
    delete mActionBar;
    delete mMenuInflater;
    delete mBackgroundFallbackDrawable;
    if (mSendWindowContentChangedAccessibilityEvent != nullptr) {
        // Unpost the run() bound to this callback object before freeing it
        // (quit path deletes the window with the post still queued).
        mSendWindowContentChangedAccessibilityEvent->removeCallbacks();
    }
    delete mSendWindowContentChangedAccessibilityEvent;
    if (mAccessibilityFocusedVirtualView != nullptr) {
        // The host View dies with the tree; the node is ours — return it to
        // the a11y node pool (recycle, not a bare delete: pool bookkeeping).
        mAccessibilityFocusedVirtualView->recycle();
    }
    mDestroyed = true;  // the transition end-callback skips finishClose during teardown
    cancelTransitionAnimator();
    // Shared-element coordinator: its dtor cancels its animator safely (own mTornDown guard)
    // and returns any ghosts still parked in the caller's overlay. Runs while this window's
    // view tree is still intact — before the base ~ViewGroup frees the overlay.
    delete mSceneTransition;
    mSceneTransition = nullptr;
    delete mEnterTransition;
    delete mExitTransition;
    delete mReturnTransition;
    delete mReenterTransition;
    // The auto-wrapped ContextThemeWrapper goes LAST: the teardown above
    // (ActionMode::finish, ActionBar, MenuInflater, scene/enter/exit
    // transitions) still reads themes/resources through mContext.
    if (mOwnsContext) delete mContext;
    // The AttachInfo was stashed by finishClose()'s post, which frees it — ~Window
    // must not touch mAttachInfo (removeWindow has already detached it).
    LOGD("%p:%d destroied!",this,mID);
}

// =====================================================================================
//  ActionBar / Options menu
// =====================================================================================
// AOSP Activity.setTheme(resid): super (ContextThemeWrapper.setTheme) applies the
// style to the live Theme + Window.setTheme stores it. CDROID's Activity IS the
// Window and the themed-context overlay carries the theme: applyStyle lands
// immediately, so subsequent inflation and lazy ?attr resolution see it.
void Window::setTheme(int resid){
    ContextThemeWrapper* themed = dynamic_cast<ContextThemeWrapper*>(mContext);
    if (themed) themed->setTheme(resid);
    // Re-apply the theme-derived window dressing. AOSP reads these at decor
    // INSTALL time (PhoneWindow.generateLayout runs on setContentView, i.e.
    // AFTER Activity.setTheme); CDROID's Window IS the view tree and is built
    // eagerly in the ctor, so the ctor-time pass resolves against whatever
    // theme was live then (for app activities: none yet). Apps call setTheme()
    // in their ctor body / onCreate — the theme swap must refresh the window
    // background, window animations and closeOnTouchOutside, or the window
    // keeps the stale (empty) resolution forever (the "black page" seen when
    // an app relies on Theme.Light's windowBackground).
    loadThemeWindowAnimations();
    loadThemeWindowBackground();
    loadThemeCloseOnTouchOutside();
}

// AOSP Activity.recreate(): the system relaunches the activity with a NEW
// instance, which inflates under the theme selected before recreation
// (already-inflated views are never re-themed in place — AOSP does the same).
// CDROID: close this window and run the REGISTER_ACTIVITY factory again; the
// new instance's constructor re-runs its content setup. The theme selection
// itself is the app's contract across recreation (re-read it in the ctor from
// wherever it persists, or set it app-wide before recreating) — exactly the
// AOSP recreate + onCreate(re-read persisted choice) shape.
// AOSP ComponentCallbacks.onConfigurationChanged: default is a no-op
// (subclasses override).
void Window::onConfigurationChanged(Configuration& newConfig){
    (void)newConfig;
}

// AOSP Activity.dispatchConfigurationChanged → onConfigurationChanged; the
// content tree walk mirrors AOSP ViewRootImpl.dispatchConfigurationChanged.
void Window::dispatchConfigurationChanged(Configuration& newConfig){
    onConfigurationChanged(newConfig);
    FrameLayout::dispatchConfigurationChanged(newConfig);
}

void Window::recreate(){
    if (mActivityName.empty()) {
        LOGW("Window::recreate: no activity name (not REGISTER_ACTIVITY'd); cannot relaunch");
        return;
    }
    const std::string name = mActivityName;
    close();   // posts removeWindow + onDestroy + delete (async, transition-aware)
    Intent intent("");
    intent.setComponent(ComponentName("", name));
    // Dispatch through the window's own context (AOSP View/Window route
    // startActivity via getContext(); App's override resolves the activity).
    mContext->startActivity(intent);
}

// Menu/panel/context-menu dispatch and the ActionBar/ActionMode plumbing live in
// cdwindowmenus.cc (AOSP: Activity delegation + Window.Callback panels).

View* Window::getCommonPredecessor(View* first, View* second){
    // AOSP ViewRootImpl.getCommonPredecessor: mark first's ancestor chain,
    // then walk second's until the first marked view.
    std::set<View*> seen;
    for (View* v = first; v != nullptr; v = v->mParent)
        seen.insert(v);
    for (View* v = second; v != nullptr; v = v->mParent)
        if (seen.count(v)) return v;
    return nullptr;
}

void Window::postSendWindowContentChangedCallback(View*source,int changeType){
    if (mSendWindowContentChangedAccessibilityEvent == nullptr) {
         mSendWindowContentChangedAccessibilityEvent = new SendWindowContentChangedAccessibilityEvent(this);
     }
     mSendWindowContentChangedAccessibilityEvent->runOrPost(source, changeType);
}

void Window::removeSendWindowContentChangedCallback(){
    if (mSendWindowContentChangedAccessibilityEvent != nullptr) {
         mSendWindowContentChangedAccessibilityEvent->removeCallbacks();
    }
}

void Window::notifySubtreeAccessibilityStateChanged(View* child, View* source, int changeType){
    // The tree is on its way down: a posted a11y run (the interval branch of
    // runOrPost) would outlive the window — removeWindow purges the UI queue
    // only up to the purge moment, and notifications fired DURING the detach
    // dispatch enqueue after it, landing on freed views/window. Drop instead.
    if (mClosePending) return;
    postSendWindowContentChangedCallback(source, changeType);
}

void Window::dispatchDetachedFromWindow(){
    // AOSP ViewRootImpl.dispatchDetachedFromWindow removes the pending
    // SendWindowContentChangedAccessibilityEvent callbacks here: the posted
    // runnable holds a raw source-view pointer, and once the tree is detached
    // (or torn down by the posted close) running it would reach through freed
    // memory. removeCallbacks() lets an already-queued run() no-op (mSource
    // nulls only in run — removeCallbacks drops the post instead).
    removeSendWindowContentChangedCallback();
    ViewGroup::dispatchDetachedFromWindow();
    // AOSP DecorView.onDetachedFromWindow -> mWindow.getCallback()
    // .onDetachedFromWindow(). After the cascade, like the traversal purge
    // below — the owner may do teardown work of its own.
    if (mCallback != nullptr && mCallback != this) {
        mCallback->onDetachedFromWindow();
    }
    // Drop the scheduled traversal AFTER the child detach cascade above: a
    // child's onDetachedFromWindow can requestLayout/invalidate its way back
    // up to this root and RE-POST a traversal (the coalescing flag was reset),
    // so purging before the cascade leaves a token=this callback queued past
    // the delete — the next doFrame then wrote mTraversalScheduled on freed
    // memory (valgrind: invalid write, DIALOG.ListPopupWindowBorrowedAdapter
    // Dismiss). finishClose's posted delete purges AGAIN after onDestroy()
    // (app teardown code may re-post there too); both spots are load-bearing.
    Choreographer::getInstance().removeCallbacks(
        Choreographer::CALLBACK_TRAVERSAL, nullptr, this);
    mTraversalScheduled = false;
}

void Window::requestTransitionStart(LayoutTransition* /*transition*/){
    // ViewGroup::requestTransitionStart climbs to the root; the Window IS the
    // root, so the walk ends here (viewgroup.cc drives the transitions itself
    // — the old pending-vector accumulator this override fed was write-only).
}

void Window::setText(const std::string&txt){
    mText=txt;
}

const std::string Window::getText()const{
    return mText;
}

void Window::bringToFront(){
    WindowManager::getInstance().bringToFront(this);
}

void Window::handleWindowContentChangedEvent(AccessibilityEvent& event){
    View* focusedHost = mAccessibilityFocusedHost;
    if ((focusedHost == nullptr) || (mAccessibilityFocusedVirtualView == nullptr)) {
        // No virtual view focused, nothing to do here.
        return;
    }

    AccessibilityNodeProvider* provider = focusedHost->getAccessibilityNodeProvider();
    if (provider == nullptr) {
        // Error state: virtual view with no provider. Clear focus.
        AccessibilityNodeInfo* stale = mAccessibilityFocusedVirtualView;
        mAccessibilityFocusedHost = nullptr;
        mAccessibilityFocusedVirtualView = nullptr;
        // AOSP nulls the reference and lets the GC collect the node; the pool
        // node is ours to return — recycle(), not a bare delete (a delete on
        // pooled memory poisons the pool; see setAccessibilityFocus below).
        stale->recycle();
        focusedHost->clearAccessibilityFocusNoCallbacks(0);
        return;
    }

    // We only care about change types that may affect the bounds of the
    // focused virtual view.
    const int changes = event.getContentChangeTypes();
    if ((changes & AccessibilityEvent::CONTENT_CHANGE_TYPE_SUBTREE) == 0
            && changes != AccessibilityEvent::CONTENT_CHANGE_TYPE_UNDEFINED) {
        return;
    }

    const long eventSourceNodeId = event.getSourceNodeId();
    const int changedViewId = AccessibilityNodeInfo::getAccessibilityViewId(eventSourceNodeId);

    // Search up the tree for subtree containment.
    bool hostInSubtree = false;
    View* root = mAccessibilityFocusedHost;
    while (root != nullptr && !hostInSubtree) {
        if (changedViewId == root->getAccessibilityViewId()) {
            hostInSubtree = true;
        } else {
            ViewGroup* parent = root->getParent();
            root = parent;
        }
    }

    // We care only about changes in subtrees containing the host view.
    if (!hostInSubtree) {
        return;
    }

    const long focusedSourceNodeId = mAccessibilityFocusedVirtualView->getSourceNodeId();
    int focusedChildId = AccessibilityNodeInfo::getVirtualDescendantId(focusedSourceNodeId);

    // Refresh the node for the focused virtual view.
    Rect oldBounds;
    mAccessibilityFocusedVirtualView->getBoundsInScreen(oldBounds);
    mAccessibilityFocusedVirtualView->recycle();  // AOSP recycles the replaced node
    mAccessibilityFocusedVirtualView = provider->createAccessibilityNodeInfo(focusedChildId);
    if (mAccessibilityFocusedVirtualView == nullptr) {
        // Error state: The node no longer exists. Clear focus.
        mAccessibilityFocusedHost = nullptr;
        focusedHost->clearAccessibilityFocusNoCallbacks(0);

        // This will probably fail, but try to keep the provider's internal
        // state consistent by clearing focus.
        provider->performAction(focusedChildId,
                AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS,nullptr);
    } else {
        // The node was refreshed, invalidate bounds if necessary.
        Rect newBounds = mAccessibilityFocusedVirtualView->getBoundsInScreen();
        if (oldBounds!=newBounds) {
            oldBounds.Union(newBounds);
        }
    }
}

bool Window::requestSendAccessibilityEvent(View* child, AccessibilityEvent& event) {
    if (child== nullptr/* || mStopped || mPausedForTransition*/) {
        return false;
    }

    // Intercept accessibility focus events fired by virtual nodes to keep
    // track of accessibility focus position in such nodes.
    const int eventType = event.getEventType();
    long sourceNodeId =-1;
    int accessibilityViewId =-1;
    View*source = nullptr;
    switch (eventType) {
    case AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUSED:
    case AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED: {
        sourceNodeId = event.getSourceNodeId();
        accessibilityViewId = AccessibilityNodeInfo::getAccessibilityViewId(sourceNodeId);
        source = findViewByAccessibilityId(accessibilityViewId);
        AccessibilityNodeProvider* provider =
                (source != nullptr) ? source->getAccessibilityNodeProvider() : nullptr;
        if (provider != nullptr) {
            if (eventType == AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUSED) {
                AccessibilityNodeInfo* node = provider->createAccessibilityNodeInfo(
                        AccessibilityNodeInfo::getVirtualDescendantId(sourceNodeId));
                setAccessibilityFocus(source, node);
            } else {
                setAccessibilityFocus(nullptr, nullptr);
            }
        }
        break;
    }

    case AccessibilityEvent::TYPE_WINDOW_CONTENT_CHANGED:
        handleWindowContentChangedEvent(event);
        break;
    }
    AccessibilityManager::getInstance(mContext).sendAccessibilityEvent(event);
    return true;
}

// AOSP ViewRootImpl.setAccessibilityFocus: track where accessibility focus sits
// (a host View plus, for virtual trees, the focused virtual node). Wiping the
// outgoing state BEFORE calling into the provider matters — the provider's
// CLEAR_FOCUS action fires an event that re-enters this method, and it must
// see clean state (the same reason handleWindowContentChangedEvent's
// early-guard reads both members).
void Window::setAccessibilityFocus(View* view, AccessibilityNodeInfo* node){
    // If we have a virtual view with accessibility focus we need
    // to clear the focus and invalidate the virtual view bounds.
    if (mAccessibilityFocusedVirtualView != nullptr) {
        AccessibilityNodeInfo* focusNode = mAccessibilityFocusedVirtualView;
        View* focusHost = mAccessibilityFocusedHost;

        // Wipe the state of the current accessibility focus since
        // the call into the provider to clear accessibility focus
        // will fire an accessibility event which will end up calling
        // this method and we want to have clean state when this
        // invocation happens.
        mAccessibilityFocusedHost = nullptr;
        mAccessibilityFocusedVirtualView = nullptr;

        // Clear accessibility focus on the host after clearing state since
        // this method may be reentrant.
        focusHost->clearAccessibilityFocusNoCallbacks(
                AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);

        AccessibilityNodeProvider* provider = focusHost->getAccessibilityNodeProvider();
        if (provider != nullptr) {
            // Invalidate the area of the cleared accessibility focus.
            Rect focusBounds;
            focusNode->getBoundsInParent(focusBounds);
            focusHost->invalidate(focusBounds);
            // Clear accessibility focus in the virtual node.
            const int virtualNodeId = AccessibilityNodeInfo::getVirtualDescendantId(
                    focusNode->getSourceNodeId());
            provider->performAction(virtualNodeId,
                    AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS, nullptr);
        }
        // AOSP: focusNode.recycle() (ViewRootImpl:6449) — a bare delete
        // poisons the node pool: the pointer stays in sPool and is handed
        // out again after the free.
        focusNode->recycle();
    }
    if ((mAccessibilityFocusedHost != nullptr) && (mAccessibilityFocusedHost != view))  {
        // Clear accessibility focus in the view.
        mAccessibilityFocusedHost->clearAccessibilityFocusNoCallbacks(
                AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);
    }

    // Set the new focus host and node.
    mAccessibilityFocusedHost = view;
    mAccessibilityFocusedVirtualView = node;
    // AOSP tail: requestInvalidateRootRenderNode() + scheduleTraversals().
    // The focus drawable is painted by Window::draw, so without invalidating
    // the window here the OLD box pixels stay composited on screen until
    // something else repaints the region — stale green boxes over pages that
    // changed, and the box lagging animated hosts.
    invalidate();
    scheduleTraversals();
}

bool Window::ensureTouchMode(bool inTouchMode) {
    LOGD("ensureTouchMode( %d), current touch mode is ",inTouchMode, mAttachInfo->mInTouchMode);
    if (mAttachInfo->mInTouchMode == inTouchMode) return false;
    // tell the window manager
    /*try {
        IWindowManager windowManager = WindowManagerGlobal.getWindowManagerService();
        windowManager.setInTouchMode(inTouchMode, getDisplayId());
    } catch (RemoteException e) {
        throw new RuntimeException(e);
    }*/
    // handle the change
    return ensureTouchModeLocally(inTouchMode);
}

bool Window::ensureTouchModeLocally(bool inTouchMode) {
    LOGD("ensureTouchModeLocally(%d), current touch mode is ",inTouchMode, mAttachInfo->mInTouchMode);

    if (mAttachInfo->mInTouchMode == inTouchMode) return false;

    mAttachInfo->mInTouchMode = inTouchMode;
    mAttachInfo->mTreeObserver->dispatchOnTouchModeChanged(inTouchMode);

    return (inTouchMode) ? enterTouchMode() : leaveTouchMode();
}

ViewGroup*Window::findAncestorToTakeFocusInTouchMode(View* focused) {
    ViewGroup* parent = focused->getParent();
    while (parent){
        ViewGroup* vgParent = (ViewGroup*) parent;
        if (vgParent->getDescendantFocusability() == ViewGroup::FOCUS_AFTER_DESCENDANTS
                && vgParent->isFocusableInTouchMode()) {
            return vgParent;
        }
        /*if (vgParent->isRootNamespace()) {
            return nullptr;
        } else */{
            parent = vgParent->getParent();
        }
    }
    return nullptr;
}

bool Window::enterTouchMode() {
    if (hasFocus()) {
        // note: not relying on mFocusedView here because this could
        // be when the window is first being added, and mFocused isn't
        // set yet.
        View* focused = findFocus();
        if (focused && !focused->isFocusableInTouchMode()) {
            ViewGroup* ancestorToTakeFocus = findAncestorToTakeFocusInTouchMode(focused);
            if (ancestorToTakeFocus != nullptr) {
                // there is an ancestor that wants focus after its
                // descendants that is focusable in touch mode.. give it
                // focus
                return ancestorToTakeFocus->requestFocus();
            } else {
                // There's nothing to focus. Clear and propagate through the
                // hierarchy, but don't attempt to place new focus.
                focused->clearFocusInternal(nullptr, true, false);
                return true;
            }
        }
    }
    return false;
}

bool Window::leaveTouchMode() {
    if (mChildren.size()) {
        if (hasFocus()) {
            View* focusedView = findFocus();
            if (dynamic_cast<ViewGroup*>(focusedView)==nullptr) {
                // some view has focus, let it keep it
                return false;
            } else if (((ViewGroup*) focusedView)->getDescendantFocusability() !=
                    ViewGroup::FOCUS_AFTER_DESCENDANTS) {
                // some view group has focus, and doesn't prefer its children
                // over itself for focus, so let them keep it.
                return false;
            }
        }

        // find the best view to give focus to in this brave new non-touch-mode
        // world
        return restoreDefaultFocus();
    }
    return false;
}

void Window::draw(){
    if( mVisibleRgn && (mVisibleRgn->get_num_rectangles()==0) ){
        return;
    }
    RefPtr<Canvas>canvas = getCanvas();
    mAttachInfo->mDrawingTime = SystemClock::uptimeMillis();

    mAttachInfo->mTreeObserver->dispatchOnPreDraw();
    FrameLayout::draw(*canvas);
    drawAccessibilityFocusedDrawableIfNeeded(*canvas);
    mAttachInfo->mTreeObserver->dispatchOnDraw();

    if (mAttachInfo->mViewScrollChanged) {
         mAttachInfo->mViewScrollChanged = false;
         mAttachInfo->mTreeObserver->dispatchOnScrollChanged();
    }
    if(View::VIEW_DEBUG){drawInvalidateRegion(*canvas);
        const int duration = int(SystemClock::uptimeMillis() - mAttachInfo->mDrawingTime);
        LOGD_IF(duration>10,"%p:%d used %dms",this,mID,duration);
    }
    GraphDevice::getInstance().flip();
}

void Window::setPos(int x,int y){
    const bool changed =(x!=mLeft)||(mTop!=y);
    if( changed && isAttachedToWindow()){
        // Keep LayoutParams in sync (AOSP: the window frame lives in
        // WindowManager.LayoutParams; relayout writes it back).
        mWindowAttributes.x = x;
        mWindowAttributes.y = y;
        WindowManager::getInstance().moveWindow(this,x,y);
        FrameLayout::layout(x,y,getWidth(),getHeight());
        mAttachInfo->mWindowLeft= x;
        mAttachInfo->mWindowTop = y;
    }
    GraphDevice::getInstance().flip();
}

void Window::resize(int width,int height){
    if(width<=0||height<=0) return;
    if(width==getWidth()&&height==getHeight()) return;
    LOGD("Window::resize %dx%d -> %dx%d pos=(%d,%d)",getWidth(),getHeight(),width,height,mLeft,mTop);
    // Keep LayoutParams in sync (see setPos: the frame lives in the attributes;
    // AOSP relayout writes the new size back into WindowManager.LayoutParams).
    mWindowAttributes.width = width;
    mWindowAttributes.height = height;
    if(isAttachedToWindow()){
        // moveWindow applies the new frame (setFrame invalidates this window
        // and damages the vacated band into the windows below), exactly like a move.
        WindowManager::getInstance().moveWindow(this,mLeft,mTop,width,height);
        if(mAttachInfo && getVisibility()==View::VISIBLE){
            // Realloc the content canvas and repaint into it SYNCHRONOUSLY, before
            // any compose can run: compose reads mAttachInfo->mCanvas per pass and
            // computes the other windows' visible regions from the pass's window
            // list, so a null (or stale-sized) canvas here would drop this window
            // out of an interleaved compose while the windows below still carry
            // damage against its old coverage — a one-pass hole in the screen.
            // Atomic frame+canvas+content, the way setPos moves atomically.
            // (The canvas is allocated once in getCanvas() at the then-current
            // size, so a window that GROWS must swap it or draw clipped.)
            mAttachInfo->mCanvas = nullptr;
            invalidate();      // full new bounds queued (setFrame already did too)
            draw();            // getCanvas() reallocates at the new size and paints NOW
        }
        requestLayout();       // re-measure the subtree at the new size (next
                               // traversal refines the layout the draw above used)
        GraphDevice::getInstance().flip();
    } else {
        FrameLayout::layout(mLeft,mTop,width,height);
    }
}

void Window::setSurfaceTranslation(int dx,int dy){
    if (dx == mSurfaceDx && dy == mSurfaceDy) return;
    // Damage model for a moving surface (AOSP's SurfaceFlinger recomposites everything; CDROID's
    // damage-region compositor must be told): the area the surface VACATES at the old offset is
    // repainted from the windows below, and this window's full extent re-blits at the new offset.
    const Rect vacated = Rect::Make(getLeft() + mSurfaceDx, getTop() + mSurfaceDy, getWidth(), getHeight());
    mSurfaceDx = dx;
    mSurfaceDy = dy;
    if (isAttachedToWindow())
        WindowManager::getInstance().exposeRegionBelow(this, vacated);
    const Rect selfLocal = Rect::Make(0, 0, getWidth(), getHeight());
    mPendingRgn->do_union((Cairo::RectangleInt&)selfLocal);
    GraphDevice::getInstance().flip();
}

WindowManager::LayoutParams& Window::getAttributes(){
    return mWindowAttributes;
}

const WindowManager::LayoutParams& Window::getAttributes()const{
    return mWindowAttributes;
}

void Window::setSoftInputMode(int mode){
    // AOSP Window.setSoftInputMode writes mWindowAttributes.softInputMode.
    mWindowAttributes.softInputMode = mode;
}

int Window::getSoftInputMode()const{
    return mWindowAttributes.softInputMode;
}

void Window::setFlags(int flags, int mask){
    // AOSP Window.setFlags (Window.java:1089-1113).
    mWindowAttributes.flags = (mWindowAttributes.flags & ~mask) | (flags & mask);
}

void Window::addFlags(int flags){
    setFlags(flags, flags);
}

void Window::clearFlags(int flags){
    setFlags(0, flags);
}

void Window::setCloseOnTouchOutside(bool close){
    // AOSP Window.setCloseOnTouchOutside (Window.java:1618-1621).
    mCloseOnTouchOutside = close;
    mSetCloseOnTouchOutside = true;
}

void Window::setCloseOnTouchOutsideIfNotSet(bool close){
    // AOSP Window.setCloseOnTouchOutsideIfNotSet (Window.java:1625-1631).
    if (mSetCloseOnTouchOutside) return;
    setCloseOnTouchOutside(close);
}

bool Window::shouldCloseOnTouchOutside() const{
    // AOSP Window.shouldCloseOnTouchOutside (Window.java:1633-1635).
    return mCloseOnTouchOutside;
}

bool Window::shouldCloseOnTouch(Context* context, MotionEvent& event){
    // AOSP Window.shouldCloseOnTouch (Window.java:1644-1652): the ACTION_OUTSIDE
    // clause serves WATCH_OUTSIDE_TOUCH windows; the UP-out-of-bounds clause
    // serves touch-modal windows that receive the real gesture (kept for
    // parity). The Window IS the decor here, so the attached check is trivial.
    const bool isOutside = (event.getAction() == MotionEvent::ACTION_UP
                                && isOutOfBounds(context, event))
                          || event.getAction() == MotionEvent::ACTION_OUTSIDE;
    return mCloseOnTouchOutside && isAttachedToWindow() && isOutside;
}

bool Window::isOutOfBounds(Context* context, const MotionEvent& event){
    // AOSP Window.isOutOfBounds (Window.java:1663-1669): outside = beyond the
    // decor frame with windowTouchSlop slack.
    const int slop = ViewConfiguration::get(context).getScaledWindowTouchSlop();
    return event.getX() < -slop || event.getY() < -slop
        || event.getX() > getWidth() + slop || event.getY() > getHeight() + slop;
}

void Window::setCallback(WindowCallback* callback){
    // AOSP Window.setCallback: replace wholesale (null clears).
    mCallback = callback;
}

WindowCallback* Window::getCallback(){
    return mCallback;
}

bool Window::superDispatchKeyEvent(KeyEvent& event){
    // AOSP Window.superDispatchKeyEvent -> mDecor.superDispatchKeyEvent: the
    // decor tree's own dispatch, bypassing the callback seam.
    return FrameLayout::dispatchKeyEvent(event);
}

bool Window::superDispatchKeyShortcutEvent(KeyEvent& event){
    return FrameLayout::dispatchKeyShortcutEvent(event);
}

bool Window::superDispatchTouchEvent(MotionEvent& event){
    return FrameLayout::dispatchTouchEvent(event);
}

bool Window::superDispatchTrackballEvent(MotionEvent& event){
    return FrameLayout::dispatchTrackballEvent(event);
}

bool Window::superDispatchGenericMotionEvent(MotionEvent& event){
    return FrameLayout::dispatchGenericMotionEvent(event);
}

bool Window::dispatchKeyShortcutEvent(KeyEvent& event){
    // Same DecorView seam as dispatchKeyEvent.
    if (mCallback != nullptr && mCallback != this) {
        return mCallback->dispatchKeyShortcutEvent(event);
    }
    return FrameLayout::dispatchKeyShortcutEvent(event);
}

bool Window::dispatchGenericMotionEvent(MotionEvent& event){
    if (mCallback != nullptr && mCallback != this) {
        return mCallback->dispatchGenericMotionEvent(event);
    }
    return FrameLayout::dispatchGenericMotionEvent(event);
}

void Window::dispatchAttachedToWindow(AttachInfo* info, int visibility){
    // AOSP DecorView.onAttachedToWindow -> mWindow.getCallback().onAttachedToWindow.
    FrameLayout::dispatchAttachedToWindow(info, visibility);
    if (mCallback != nullptr && mCallback != this) {
        mCallback->onAttachedToWindow();
    }
}

void Window::setAttributes(const WindowManager::LayoutParams& a){
    mWindowAttributes = a;
}

View& Window::setAlpha(float alpha){
    if (alpha == getAlpha()) return *this;
    // Window alpha is PURELY COMPOSITIONAL: composeSurfaces reads getAlpha()
    // and paints this window's blit with paint_with_alpha. The window's pixels
    // are frame-invariant during a fade, so View::setAlpha's invalidate /
    // full-tree re-render pipeline is wasted work every animation frame — use
    // the no-invalidation path (AOSP setAlphaNoInvalidation) and just damage
    // our own extent + flip: the next compose re-blits with the new alpha.
    setAlphaNoInvalidation(alpha);
    if (isAttachedToWindow() && mAttachInfo != nullptr && mAttachInfo->mCanvas != nullptr) {
        LOGV("setAlpha(%p,%d)",this,(int)(alpha*255));
        GFXSurfaceSetOpacity(mAttachInfo->mCanvas->mHandle, (alpha*255));
        const RectangleInt full = {0, 0, getWidth(), getHeight()};
        mPendingRgn->do_union(full);
        GraphDevice::getInstance().flip();
    }
    return *this;
}

void Window::onSizeChanged(int w,int h,int oldw,int oldh){
    /* empty by design: the window IS the root — no visible-region reset here. */
}

void Window::onVisibilityChanged(View& changedView,int visibility){
    // When this window becomes hidden, the screen area it covered must be
    // repainted from the windows below — hideWindow propagates that damage
    // (and flips). It does NOT change visibility; the setVisibility that
    // triggered us already did. On show we just schedule a compose.
    if(visibility != View::VISIBLE){
        WindowManager::getInstance().hideWindow(this);
    }else{
        GraphDevice::getInstance().flip();
    }
}

ViewGroup*Window::invalidateChildInParent(int* location,Rect& dirty){
    FrameLayout::invalidateChildInParent(location,dirty);
    invalidate(dirty);
    Looper::getMainLooper()->wake();
    return nullptr;
}

void Window::onFinishInflate(){
    requestLayout();
    startLayoutAnimation();
}

RefPtr<Canvas>Window::getCanvas(){
    RefPtr<Canvas> canvas;
    //for children's canvas is allcated by it slef and delete by drawing thread(UIEventSource)
    if(mAttachInfo == nullptr)
        return nullptr;
    canvas = mAttachInfo->mCanvas;
    if((canvas==nullptr)&&(getVisibility()==VISIBLE)){	
        const int rotation  = WindowManager::getInstance().getDefaultDisplay().getRotation();
        const int swapeWH = (rotation == Display::ROTATION_90)||(rotation == Display::ROTATION_270);
        const int canvasWidth = swapeWH?getHeight():getWidth();
        const int canvasHeight= swapeWH?getWidth():getHeight();

        canvas = make_refptr_for_instance<Canvas>(new Canvas(canvasWidth,canvasHeight));
        mAttachInfo->mCanvas = canvas;
        Cairo::Matrix matrix = Cairo::identity_matrix();
        Cairo::FontOptions options;
        canvas->get_font_options(options);
        options.set_hint_style(Cairo::FontOptions::HintStyle::MEDIUM);
        options.set_hint_metrics(Cairo::FontOptions::HintMetrics::OFF);
        canvas->set_font_options(options);

        LOGV("rotation=%d window.size=%dx%d canvas.size=%dx%d antialias=%",rotation*90,getWidth(),getHeight(),
             canvasWidth,canvasHeight,canvas->get_antialias());
        switch(rotation){
        case Display::ROTATION_0:break;
        case Display::ROTATION_90:
            matrix.rotate(-M_PI/2);
            matrix.translate(-canvasHeight,0);
            canvas->transform(matrix);
            break;
        case Display::ROTATION_180:
            matrix.translate(canvasWidth,canvasHeight);
            matrix.scale(-1,-1);
            canvas->transform(matrix);
            break;
        case Display::ROTATION_270:
            matrix.translate(canvasWidth,0);
            matrix.rotate(M_PI/2);
            canvas->transform(matrix);
            break;
        }
    }
    Cairo::RefPtr<Cairo::Region>transRgn = Cairo::Region::create();//{0,0,getWidth(),getHeight()});
    int num = gatherTransparentRegion(transRgn);
    Cairo::RectangleInt rec = transRgn->get_extents();
    LOGV_IF(num,"transRgn.rects=%d extents=(%d,%d,%d,%d)",num,rec.x,rec.y,rec.width,rec.height);
    mInvalidRgn->subtract(transRgn);
    num = mInvalidRgn->get_num_rectangles();
    canvas->reset_clip();
    for(int i = 0;i < num; i ++){
        RectangleInt r = mInvalidRgn->get_rectangle(i);
        canvas->rectangle(r.x,r.y,r.width,r.height);
    }
    mPendingRgn->do_union(mInvalidRgn);
    mInvalidRgn->subtract(mInvalidRgn);
    if(num > 0)canvas->clip();
    return canvas;
}

void Window::onCreate(){
    LOGV("%p[%s]:%d",this,getText().c_str(),mID);
}

void Window::onCreate(Bundle* /*savedInstanceState*/){
    // Activity-aligned onCreate(Bundle); forward to the legacy no-arg hook so
    // existing subclasses overriding onCreate() keep firing.
    onCreate();
}

void Window::onStart(){
}

void Window::onResume(){
    // Backward compatibility: route through the legacy activation hook so that
    // existing Window subclasses overriding onActive() still receive the event.
    onActive();
}

void Window::onPause(){
    // Backward compatibility: route through the legacy deactivation hook.
    onDeactive();
}

void Window::onStop(){
}

void Window::onDestroy(){
    LOGD("%p[%s]:%d destroyed",this,getText().c_str(),mID);
}

void Window::onActive(){
    LOGD("%p[%s]:%d",this,getText().c_str(),mID);
}

void Window::onDeactive(){
    LOGV("%p[%s]:%d",this,getText().c_str(),mID);
}

/* AOSP ViewRootImpl.processKeyEvent's keyboard-group-navigation probe
 * (ViewRootImpl.java:7729-7741 — single copy there; CDROID also probes it in
 * dispatchKeyEvent). NOTE: AOSP gates on META_CTRL_ON; CDROID uses META_META_ON
 * — an intentional divergence, kept verbatim so a future re-sync notices it. */
static int keyboardGroupNavigationDirection(const KeyEvent& event){
    if (event.getAction() == KeyEvent::ACTION_DOWN && event.getKeyCode() == KeyEvent::KEYCODE_TAB) {
        if (KeyEvent::metaStateHasModifiers(event.getMetaState(), KeyEvent::META_META_ON))
            return View::FOCUS_FORWARD;
        if (KeyEvent::metaStateHasModifiers(event.getMetaState(),
                KeyEvent::META_META_ON | KeyEvent::META_SHIFT_ON))
            return View::FOCUS_BACKWARD;
    }
    return 0;
}

int Window::processKeyEvent(KeyEvent&event){
    const int action = event.getAction();
    LOGV_IF(action==KeyEvent::ACTION_DOWN,"%s:0x%x %s %x",event.actionToString(action).c_str(),
            event.getKeyCode(),KeyEvent::keyCodeToString(event.getKeyCode()).c_str(),KeyEvent::KEYCODE_DPAD_DOWN);
    if(dispatchKeyEvent(event))
        return FINISH_HANDLED;
    const int groupNavigationDirection = keyboardGroupNavigationDirection(event);
    if (event.getAction() == KeyEvent::ACTION_DOWN
            && !KeyEvent::metaStateHasNoModifiers(event.getMetaState())
            && event.getRepeatCount() == 0
            && !KeyEvent::isModifierKey(event.getKeyCode())
            && groupNavigationDirection == 0) {
        if (dispatchKeyShortcutEvent(event)) {
            return FINISH_HANDLED;
        }
    }
    if(action == KeyEvent::ACTION_DOWN){
        if(groupNavigationDirection != 0){
            if(performKeyboardGroupNavigation(groupNavigationDirection))
                return FINISH_HANDLED;
        }else {
            if(performFocusNavigation(event)){
                return FINISH_HANDLED;
            }
        }
    }
    return FORWARD;
}

bool Window::dispatchKeyEvent(KeyEvent&event){
    // AOSP DecorView.dispatchKeyEvent (PhoneWindow.java):
    //   cb != null ? cb.dispatchKeyEvent(event) : super.dispatchKeyEvent(event)
    // — full delegation when a callback owner is installed (Dialog); the cb's
    // own dispatch chain re-enters the tree through superDispatchKeyEvent, so
    // there is deliberately NO local fallback in this branch.
    if (mCallback != nullptr && mCallback != this) {
        return mCallback->dispatchKeyEvent(event);
    }
    View* focused = getFocusedChild();
    bool handled  = false;
    const int action = event.getAction();
    if(focused && focused->dispatchKeyEvent(event))
        return true;
    const int groupNavigationDirection = keyboardGroupNavigationDirection(event);
    if(action == KeyEvent::ACTION_DOWN){
        if(groupNavigationDirection!=0)
            return performKeyboardGroupNavigation(groupNavigationDirection);
        handled = performFocusNavigation(event);
    }
    if(!handled){
        // Focus view didn't consume it; route to the WINDOW's own onKeyDown/onKeyUp — not
        // ViewGroup::dispatchKeyEvent (FrameLayout::), which re-walks children and never reaches
        // the window's callback when the window itself isn't PFLAG_FOCUSED (root windows rarely are;
        // focus lives in a child like EditText). Mirrors androidx: an unhandled key falls back to
        // the window/Activity callback. View::dispatchKeyEvent -> event.dispatch -> onKeyDown/onKeyUp
        // (Window::onKeyDown BACK startTracking; Window::onKeyUp BACK isTracking -> onBackPressed).
        handled = View::dispatchKeyEvent(event);
    }
    return handled;
}

View* Window::focusSearch(View* focused, int direction){
    /* Alternative (i), kept for the record: mark the window itself as root
       namespace, letting ViewGroup::focusSearch's isRootNamespace() branch
       make the same FocusFinder call below. Works, but the root-namespace
       flag also carries "top of a LocalActivityManager activity tower"
       semantics (see popupwindow.cc's decor view, its only user), so (ii) —
       an explicit override mirroring ViewRootImpl.focusSearch — is preferred.
    //setIsRootNamespace(true);
    */
    // AOSP ViewRootImpl.focusSearch(ViewRootImpl.java:8093): the chain's
    // terminal resolver runs FocusFinder on the window's view tree. CDROID's
    // Window is its own root view (no ViewRootImpl), so the parent chain that
    // starts at View::focusSearch(int) ends at THIS override.
    return FocusFinder::getInstance().findNextFocus((ViewGroup*)this, focused, direction);
}

bool Window::performFocusNavigation(KeyEvent& event){
    int direction = -1;
    switch (event.getKeyCode()) {
    case KeyEvent::KEYCODE_DPAD_LEFT:
        direction = View::FOCUS_LEFT;  break;
    case KeyEvent::KEYCODE_DPAD_RIGHT:
        direction = View::FOCUS_RIGHT; break;
    case KeyEvent::KEYCODE_DPAD_UP:
        direction = View::FOCUS_UP;    break;
    case KeyEvent::KEYCODE_DPAD_DOWN:
        direction = View::FOCUS_DOWN;  break;
    case KeyEvent::KEYCODE_TAB:
        if (event.hasNoModifiers()) {
            direction = View::FOCUS_FORWARD;
        } else if (event.hasModifiers(KeyEvent::META_SHIFT_ON)) {
            direction = View::FOCUS_BACKWARD;
        }
        break;
    }

    if (direction != -1){
        ViewGroup*mView= (ViewGroup*)this;
        View* focused  = mView->findFocus();
        Rect& mTempRect= mRectOfFocusedView;
        if (focused != nullptr) {
            View* v = mView->focusSearch(focused,direction);
            LOGV("mView=%p focused=%p:%d v=%p",mView,focused,focused->getId(),v);
            if (v != nullptr && v != focused) {
                focused->getFocusedRect(mTempRect);
                if (dynamic_cast<ViewGroup*>(mView)) {
                    mView->offsetDescendantRectToMyCoords(focused, mTempRect);
                    mView->offsetRectIntoDescendantCoords(v, mTempRect);
                }
                LOGV("request focus at rect(%d,%d-%d,%d)",mTempRect.left,mTempRect.top,mTempRect.width,mTempRect.height);
                if (v->requestFocus(direction, &mTempRect)) {
                    return true;
                }
            }

            if (mView->dispatchUnhandledMove(focused, direction)) {
                return true;
            }
        } else {
            if (mView->restoreDefaultFocus()) {
                return true;
            }
        }
    }
    return false;
}

bool Window::onKeyDown(int keyCode,KeyEvent& evt){
    switch(keyCode){
    case KeyEvent::KEYCODE_ESCAPE:
    case KeyEvent::KEYCODE_BACK:
        // AOSP DecorView/View: BACK tracks on DOWN; the UP side fires
        // onBackPressed (the overridden chain: FragmentActivity pops its
        // back stack, the Window default finishes). ESC additionally stands
        // in for BACK on keyboards without a BACK key (x64 qwerty.kl maps Esc
        // to KEYCODE_ESCAPE and no key produces KEYCODE_BACK) — AOSP TV and
        // embedded builds do the same remap in their keylayout; accepting
        // both here keeps desktop dismiss working without resurrecting the
        // dead-BACK problem the swap fixed.
        evt.startTracking();
        LOGD("recv %d %s flags=%x",keyCode,KeyEvent::keyCodeToString(keyCode).c_str(),evt.getFlags());
        return true;
    default:
        LOGV("recv %d %s",keyCode,KeyEvent::keyCodeToString(keyCode).c_str());
        return FrameLayout::onKeyDown(keyCode,evt);
    }
    return false;
}

bool Window::onKeyUp(int keyCode,KeyEvent& evt){
    LOGV("recv %d %s flags=%x track=%d cance=%d",keyCode,KeyEvent::keyCodeToString(keyCode).c_str(),
            evt.getFlags(),evt.isTracking(),evt.isCanceled());
    switch(keyCode){
    case KeyEvent::KEYCODE_ESCAPE:
    case KeyEvent::KEYCODE_BACK:
        if(evt.isTracking()&&!evt.isCanceled()){
            onBackPressed();
            return true;
        }//pass throught return false;
    default:return false;
    }
}

void Window::onBackPressed(){
    LOGD("recv BackPressed");
    close();
}

bool Window::isInLayout()const{
    return mInLayout;
}

void Window::doLayout(){
    LOGV("doLayout(%dx%d) child.count=%d HAS_BOUNDS=%x",getWidth(),getHeight(),
                getChildCount(),(mPrivateFlags&PFLAG_HAS_BOUNDS));
    // Measure the whole subtree and lay out every child — matches Android's
    // ViewRootImpl.performTraversals (measure the root, then lay it out). The
    // previous code only did this when mChildren.size()==1 and only positioned
    // mChildren[0], which left any window whose direct child count wasn't exactly
    // 1 (and that child's descendants) permanently unmeasured.
    const int widthSpec  = MeasureSpec::makeMeasureSpec(getWidth(),MeasureSpec::EXACTLY);
    const int heightSpec = MeasureSpec::makeMeasureSpec(getHeight(),MeasureSpec::EXACTLY);
    FrameLayout::measure(widthSpec,heightSpec);
    FrameLayout::layout(getLeft(),getTop(),getMeasuredWidth(),getMeasuredHeight());
    getViewTreeObserver()->dispatchOnGlobalLayout();
    mPrivateFlags&=~PFLAG_FORCE_LAYOUT;
    mInLayout = false;
}


// AOSP Activity.startActivityForResult(Intent, int, Bundle options) — the 2-arg call
// form is the default-parameter path (options == nullptr).
void Window::startActivityForResult(const Intent& intent, int requestCode, ActivityOptions* options){
    App::getInstance().startActivityForResultInternal(this, intent, requestCode, options);
}

// =====================================================================================
//  DecorView dressing (PhoneWindow.generateLayout's background half + DecorView's
//  fallback draw). The transition half of generateLayout (window animations) lives in
//  cdwindowtransitions.cc with the driver it feeds.
// =====================================================================================

// AOSP PhoneWindow.generateLayout, getContainer()==null branch:
//     if (mBackgroundDrawable == null && a.hasValue(R.styleable.Window_windowBackground))
//         mBackgroundDrawable = a.getDrawable(R.styleable.Window_windowBackground);
//     if (a.hasValue(R.styleable.Window_windowBackgroundFallback))
//         mBackgroundFallbackDrawable = a.getDrawable(R.styleable.Window_windowBackgroundFallback);
//     ...
//     mDecor.setWindowBackground(mBackgroundDrawable);          // -> DecorView.setBackground
//     if (mDecor.getBackground() == null && mBackgroundFallbackDrawable != null)
//         mDecor.setBackgroundFallback(mBackgroundFallbackDrawable);
// The Window IS the fused decor, so the resolved background goes straight to
// setBackground (View ownership; PhoneWindow's mBackgroundDrawable alias is dropped —
// a later app setBackground would free what it points at). The hand-built attr array
// stands in for the generated Window styleable (the trailing 0 is the sentinel
// obtainStyledAttributes scans to); TypedArray::getDrawable is the full AOSP decode
// (references, inline colors, file paths).
void Window::loadThemeWindowBackground() {
    if (mContext == nullptr) return;
    static const uint32_t attrs[] = {R::attr::windowBackground, R::attr::windowBackgroundFallback, 0};
    auto ta = mContext->getTheme().obtainStyledAttributes(attrs);
    if (!ta) return;
    if (Drawable* background = ta->getDrawable(0)) {  // null = unset or unresolvable
        // Swap freely between theme installs, but never clobber a background
        // the APP installed (setBackgroundDrawable after construction wins,
        // AOSP DecorView.setWindowBackground semantics).
        if (mBackgroundFromTheme || getBackground() == nullptr) {
            LOGD("theme windowBackground=%p (fromTheme=%d)", background, (int)mBackgroundFromTheme);
            setBackground(background);  // DecorView.setWindowBackground -> setBackground
            mBackgroundFromTheme = true;
        } else {
            delete background;   // obtained but not installed — ours to free
        }
        return;  // the fallback only applies when no window background is set
    }
    setBackgroundFallback(ta->getDrawable(1));
}

// AOSP DecorView.setBackgroundFallback (its BackgroundFallback member folded into
// the fused Window; the drawable is owned here — Java's GC becomes a delete).
void Window::setBackgroundFallback(Drawable* fallbackDrawable) {
    if (mBackgroundFallbackDrawable != fallbackDrawable) {
        delete mBackgroundFallbackDrawable;
        mBackgroundFallbackDrawable = fallbackDrawable;
    }
    setWillNotDraw(getBackground() == nullptr && mBackgroundFallbackDrawable == nullptr);
}

// AOSP DecorView.onDraw: super, then the background fallback.
void Window::onDraw(Canvas& canvas) {
    FrameLayout::onDraw(canvas);
    drawBackgroundFallback(canvas);
}

// AOSP com.android.internal.widget.BackgroundFallback.draw(boundsView, root, c, content,
// coveringView1, coveringView2) with boundsView/root == this Window and null covering
// views: track the union of the opaque visible children and fill the uncovered strips
// with the fallback drawable.
void Window::drawBackgroundFallback(Canvas& canvas) {
    if (mBackgroundFallbackDrawable == nullptr) return;  // !hasFallback()

    // Draw the fallback in the padding.
    const int width = getWidth();
    const int height = getHeight();

    int left = width;
    int top = height;
    int right = 0;
    int bottom = 0;

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        Drawable* childBg = child->getBackground();
        // Potentially translucent or invisible children don't count, and we assume the
        // content view will cover the whole area if we're in a background fallback
        // situation.
        if (child->getVisibility() != View::VISIBLE
                || childBg == nullptr || childBg->getOpacity() != PixelFormat::OPAQUE) {
            continue;
        }
        left = std::min(left, child->getLeft());
        top = std::min(top, child->getTop());
        right = std::max(right, child->getRight());
        bottom = std::max(bottom, child->getBottom());
    }

    if (left >= right || top >= bottom) {
        // No valid area to draw in.
        return;
    }

    // CDROID Drawable::setBounds takes (x, y, w, h) — AOSP's (l, t, r, b) strips below.
    if (top > 0) {
        mBackgroundFallbackDrawable->setBounds(0, 0, width, top);
        mBackgroundFallbackDrawable->draw(canvas);
    }
    if (left > 0) {
        mBackgroundFallbackDrawable->setBounds(0, top, left, height - top);
        mBackgroundFallbackDrawable->draw(canvas);
    }
    if (right < width) {
        mBackgroundFallbackDrawable->setBounds(right, top, width - right, height - top);
        mBackgroundFallbackDrawable->draw(canvas);
    }
    if (bottom < height) {
        mBackgroundFallbackDrawable->setBounds(left, bottom, right - left, height - bottom);
        mBackgroundFallbackDrawable->draw(canvas);
    }
}

void Window::close(){
    close(nullptr);
}

void Window::close(const std::function<void()>& onTeardown){
    // AOSP removal semantics: the view tree and every callback tear down
    // SYNCHRONOUSLY (ViewRootImpl.die(true) -> doDie -> dispatchDetachedFromWindow;
    // PopupWindow.dismiss/Dialog.dismiss ride removeViewImmediate). The exit
    // transition stays purely VISUAL — WMS keeps animating the REMOVED window's
    // surface (WindowStateAnimator); the CDROID analog is a compositor ghost
    // snapshot that outlives the tree. Nothing is deferred: no re-show window,
    // no mid-flight state, and mTeardownCb runs now (finishClose consumes it).
    // Idempotent: a re-entered close (dismiss listener, second close) must not
    // post a second delete — mirrors Dialog::dismiss's mShowing guard.
    if (mClosePending) return;
    mClosePending = true;
    mTeardownCb = onTeardown;
    App::getInstance().dispatchPendingResult(this);
    // AOSP PopupWindow.dismiss cancels the decor's in-flight transitions BEFORE
    // the exit branch; the enter animator here would keep ticking (and its end
    // listener re-touch this window) while the teardown below proceeds. Cancel
    // it now — its end listener lands the surface at rest (identity), which is
    // also what the ghost snapshot wants to capture.
    cancelTransitionAnimator();
    // Shared-element return flight (B route): ghosts fly in the caller's overlay
    // while this window hides at once; finishClose runs on landing. No valid pair
    // (caller gone / no view matches) falls through to the window-level path below.
    if (mSceneTransition && mSceneTransition->startReturn([this](){ finishClose(); })) {
        return;
    }
    ActivityTransition* t = mReturnTransition ? mReturnTransition : mExitTransition;
    if (t && t->getType() != ActivityTransition::Type::NONE
            && isAttachedToWindow() && getVisibility() == VISIBLE) {
        startGhostExit(t);   // pure visual; an in-flight ENTER no longer blocks close
    }
    finishClose();
}

void Window::finishClose(){
    // Teardown sequence and its ordering constraints:
    //  1. mTeardownCb runs first — the caller's last point with the hierarchy intact
    //     (detachOwner() clears it when the owner dies mid-animation).
    //  2. removeWindow runs immediately — the window leaves the compositor at once and
    //     the tree is detached (mAttachInfo nulled), so the AttachInfo is stashed first.
    //  3. The deletes are posted (not run inline) so the current call stack can still
    //     touch the window; on a standalone heap Handler, NOT View::post — removeWindow
    //     purges mUIEventHandler's queue, which would drop this very post.
    //  4. Inside the post: purge the Choreographer traversal callbacks (separate queue,
    //     re-postable during dispatchDetachedFromWindow), then `delete self` BEFORE
    //     `delete info` — the tree's destructor belts resolve their pinned observer
    //     while it is still alive.
    if (mTeardownCb) {
        std::function<void()> cb = mTeardownCb;
        mTeardownCb = nullptr;
        cb();
    }
    // Drop any a11y run posted before the close (its mSource/mRunnable capture
    // this window and tree views); the deletes below free both.
    removeSendWindowContentChangedCallback();
    auto* info = mAttachInfo;
    Window* self = this;
    Handler* h = new Handler();
    h->post([h, self, info](){
        self->onDestroy();
        // Last safe point before the delete: purge by token again — onDestroy()
        // (app teardown code) may have re-posted a traversal on this still-alive
        // window; the queue is separate from the UIEventHandler's.
        Choreographer::getInstance().removeCallbacks(
            Choreographer::CALLBACK_TRAVERSAL, nullptr, self);
        self->mTraversalScheduled = false;
        // Posts made mid-detach-dispatch (e.g. a11y scrolled/content-changed runs
        // queued through AttachInfo.mHandler after View::dispatchDetachedFromWindow
        // cancelled its own callbacks) would dispatch on freed views — drain the
        // handler's pending messages before the tree and the AttachInfo die.
        if (info != nullptr && info->mHandler != nullptr) {
            info->mHandler->removeCallbacksAndMessages(nullptr);
        }
        delete self;   // before info: the tree's belts need the live observer
        delete info;
        delete h;
    });
    WindowManager::getInstance().removeWindow(this);
}

// Activity/window transitions (theme window animations, the A-route FADE/SLIDE driver,
// and the B-route shared-element stamp) live in cdwindowtransitions.cc.

void Window::scheduleTraversals(){
    if(mTraversalScheduled) return;
    mTraversalScheduled = true;
    Choreographer::getInstance().postCallback(
        Choreographer::CALLBACK_TRAVERSAL,
        [this](){ doTraversal(); },
        this);  // token=this so close() can purge the pending callback by window identity
}

void Window::doTraversal(){
    // Guard FIRST, member writes after: a stale traversal record must not even
    // touch this object (detached-but-alive windows — retireFromCompositor —
    // have nothing to traverse).
    if (!isAttachedToWindow()) {
        return;
    }
    mTraversalScheduled = false;
    GraphDevice::getInstance().lock();
    if(isAttachedToWindow()){
        if(isLayoutRequested()) doLayout();
        // Shared-element enter (B route): resolve targets and park the snapshot ghosts between
        // layout and the first draw, so frame 1 never flashes the targets un-ghosted. No pair
        // resolves -> drop the coordinator; the window-level enter animation (never suppressed
        // in that case) takes over via the post-draw hook below (AOSP's app-transition fallback).
        if (mSceneTransition && !mSceneTransition->prepareEnter()) {
            delete mSceneTransition;
            mSceneTransition = nullptr;
        }
        if(isDirty() && getVisibility() == View::VISIBLE){
            draw();
            GraphDevice::getInstance().flip();
        }
    }
    GraphDevice::getInstance().unlock();
    GraphDevice::getInstance().composeSurfaces();
    // Kick off the enter transition once the content's first frame is drawn. The Activity set
    // mEnterTransition (and pre-snapped to the start state) in onCreate; we start the animator here
    // so the slide/fade begins from a laid-out, drawn window rather than a blank one.
    if (mPendingEnterAnim && !mInTransition) {
        mPendingEnterAnim = false;
        startEnterAnimation();
    }
}

bool Window::dispatchTouchEvent(MotionEvent& event){
    // AOSP DecorView.dispatchTouchEvent: cb != null -> cb.dispatchTouchEvent
    // (the Dialog chain = superDispatchTouchEvent || onTouchEvent covers both
    // the tree pass and the owner's own handling).
    if (mCallback != nullptr && mCallback != this) {
        return mCallback->dispatchTouchEvent(event);
    }
    return FrameLayout::dispatchTouchEvent(event);
}

bool Window::onTouchEvent(MotionEvent& event){
    return FrameLayout::onTouchEvent(event);
}

void Window::dispatchInvalidateOnAnimation(View*view){
    mInvalidateOnAnimationRunnable.setOwner(this);
    mInvalidateOnAnimationRunnable.addView(view);
}

void Window::dispatchInvalidateRectOnAnimation(View*view,const Rect&rect){
    mInvalidateOnAnimationRunnable.setOwner(this);
    mInvalidateOnAnimationRunnable.addViewRect(view,rect);
}

void Window::dispatchInvalidateDelayed(View*view, long delayMilliseconds){
    LOGW_IF(delayMilliseconds,"Delay is NOT IMPLEMENTED");
    if(0==delayMilliseconds) dispatchInvalidateOnAnimation(view);
}

void Window::dispatchInvalidateRectDelayed(const AttachInfo::InvalidateInfo*info,long delayMilliseconds){
    LOGW_IF(delayMilliseconds,"Delay is NOT IMPLEMENTED");
    if(0==delayMilliseconds) dispatchInvalidateRectOnAnimation(info->target,info->rect);
}

void Window::cancelInvalidate(View* view){
    mInvalidateOnAnimationRunnable.removeView(view);
}

Window::InvalidateOnAnimationRunnable::InvalidateOnAnimationRunnable(){
    mOwner = nullptr;
    mPosted= false;
}

Window::InvalidateOnAnimationRunnable::~InvalidateOnAnimationRunnable(){
    drain();
}

void Window::InvalidateOnAnimationRunnable::setOwner(Window*w){
    mOwner = w;
}

std::vector<View::AttachInfo::InvalidateInfo*>::iterator Window::InvalidateOnAnimationRunnable::find(View*v){
    for(auto it = mInvalidateViews.begin();it!=mInvalidateViews.end();it++){
        if((*it)->target == v)
            return it;
    }
    return mInvalidateViews.end();
}

void Window::InvalidateOnAnimationRunnable::addView(View* view){
    // Full invalidate: a pending rect for the same view collapses back to
    // "invalidate everything" (AOSP ViewRootImpl's InvalidateInfo null-rect).
    addViewRect(view, Rect::Make(0,0,0,0), true);
}

void Window::InvalidateOnAnimationRunnable::addViewRect(View* view,const Rect&rect,bool full){
    auto it = find(view);
    AttachInfo::InvalidateInfo* info;
    if(it == mInvalidateViews.end()){
        info = AttachInfo::InvalidateInfo::obtain();
        info->target = view;
        info->rect = full ? Rect::Make(0,0,0,0) : rect;
        mInvalidateViews.push_back(info);
    }else{
        info = (*it);
        if (full)
            info->rect.set(0,0,0,0);
        else if(!info->rect.empty())
            info->rect.Union(rect);
    }
    postIfNeededLocked();
}

void Window::InvalidateOnAnimationRunnable::removeView(View* view){
    auto it = find(view);
    if(it != mInvalidateViews.end()){
        (*it)->recycle();
        mInvalidateViews.erase(it);
    }
    if(mInvalidateViews.size()==0){
        mPosted = false;
    }
}

void Window::InvalidateOnAnimationRunnable::run(){
    mPosted = false;
    drain();
}

void Window::InvalidateOnAnimationRunnable::drain(){
    std::vector<View::AttachInfo::InvalidateInfo*>& temp = mInvalidateViews;
    for (auto i:temp){
        Rect&r = i->rect;
        View*v = i->target;
        if(r.width<=0||r.height<=0) v->invalidate();
        else  v->invalidate(r);
        i->recycle();
    }
    mInvalidateViews.clear();
}

void Window::InvalidateOnAnimationRunnable::postIfNeededLocked() {
    if (!mPosted) {
        Runnable run(std::bind(&InvalidateOnAnimationRunnable::run,this));
        mOwner->postDelayed(run,AnimationHandler::getFrameDelay());
        mPosted = true;
    }
}

void Window::drawAccessibilityFocusedDrawableIfNeeded(Canvas& canvas){
    Rect bounds;
    if(!isAttachedToWindow()){
        return;
    }
    if (getAccessibilityFocusedRect(bounds)) {
        Drawable* drawable = getAccessibilityFocusedDrawable();
        if (drawable != nullptr) {
            drawable->setBounds(bounds);
            drawable->draw(canvas);
        }
    } else if (mAttachInfo->mAccessibilityFocusDrawable != nullptr) {
        mAttachInfo->mAccessibilityFocusDrawable->setBounds(0, 0, 0, 0);
    }    
}

bool Window::getAccessibilityFocusedRect(Rect& bounds){
    AccessibilityManager& manager = AccessibilityManager::getInstance(mContext);
    if (!manager.isEnabled() || !manager.isTouchExplorationEnabled()) {
        return false;
    }

    View* host = mAccessibilityFocusedHost;
    if (host == nullptr || host->mAttachInfo == nullptr) {
        return false;
    }

    AccessibilityNodeProvider* provider = host->getAccessibilityNodeProvider();
    if (provider == nullptr) {
        host->getBoundsOnScreen(bounds, true);
    } else if (mAccessibilityFocusedVirtualView != nullptr) {
        mAccessibilityFocusedVirtualView->getBoundsInScreen(bounds);
    } else {
        return false;
    }

    // Transform the rect into window-relative coordinates.
    AttachInfo* attachInfo = mAttachInfo;
    bounds.offset(0, attachInfo->mRootView->mScrollY);
    bounds.offset(-attachInfo->mWindowLeft, -attachInfo->mWindowTop);
    if (!bounds.intersect(0, 0, attachInfo->mRootView->getWidth(),
            attachInfo->mRootView->getHeight())) {
        // If no intersection, set bounds to empty.
        bounds.setEmpty();
    }
    return !bounds.empty();
}

Drawable* Window::getAccessibilityFocusedDrawable(){
    // Lazily load the accessibility focus drawable from the theme
    // (AOSP ViewRootImpl: accessibilityFocusedDrawable -> view_accessibility_focused).
    if (mAttachInfo->mAccessibilityFocusDrawable == nullptr) {
        TypedValue value;
        const bool resolved = mContext->getTheme().resolveAttribute(
                (int)internal::R::attr::accessibilityFocusedDrawable, &value, true);
        if (resolved && value.resourceId != 0) {
            mAttachInfo->mAccessibilityFocusDrawable = mContext->getDrawable(value.resourceId);
        }
    }
    return mAttachInfo->mAccessibilityFocusDrawable;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

Window::SendWindowContentChangedAccessibilityEvent::SendWindowContentChangedAccessibilityEvent(Window*w):mWin(w){
    mSource   = nullptr;
    mLastEventTimeMillis =0;
    mRunnable = std::bind(&SendWindowContentChangedAccessibilityEvent::run,this);
}

void Window::SendWindowContentChangedAccessibilityEvent::run(){
    //Protect against re-entrant code and attempt to do the right thing in the case that
    // we're multithreaded.
    View* source = mSource;
    mSource = nullptr;
    if (source == nullptr) {
        LOGE("Accessibility content change has no source");
        return;
    }
    if (!source->isAttachedToWindow()) {
        // Detached between the post and the run (Window::dispatchDetachedFromWindow
        // normally removes the pending callback; this guards teardowns that free
        // the tree without the detach dispatch). Reading the freed subtree from
        // here crashed AdapterView::onInitializeAccessibilityEventInternal.
        return;
    }
    // The accessibility may be turned off while we were waiting so check again.
    if (AccessibilityManager::getInstance(mWin->getContext()).isEnabled()) {
        mLastEventTimeMillis = SystemClock::uptimeMillis();
        AccessibilityEvent* event = AccessibilityEvent::obtain();
        event->setEventType(AccessibilityEvent::TYPE_WINDOW_CONTENT_CHANGED);
        event->setContentChangeTypes(mChangeTypes);
        source->sendAccessibilityEventUnchecked(*event);
    } else {
        mLastEventTimeMillis = 0;
    }
    // In any case reset to initial state.
    source->resetSubtreeAccessibilityStateChanged();
    mChangeTypes = 0;
}

void Window::SendWindowContentChangedAccessibilityEvent::runOrPost(View* source, int changeType){
     if (mSource != nullptr) {
         // If there is no common predecessor, then mSource points to
         // a removed view, hence in this case always prefer the source.
         View* predecessor = mWin->getCommonPredecessor(mSource, source);
         if (predecessor != nullptr) {
             predecessor = predecessor->getSelfOrParentImportantForA11y();
         }
         mSource = (predecessor != nullptr) ? predecessor : source;
         mChangeTypes |= changeType;
         return;
     }
     mSource = source;
     mChangeTypes = changeType;
     const int64_t timeSinceLastMillis = SystemClock::uptimeMillis() - mLastEventTimeMillis;
     const int64_t minEventIntevalMillis = ViewConfiguration::getSendRecurringAccessibilityEventsInterval();
     if (timeSinceLastMillis >= minEventIntevalMillis) {
         removeCallbacksAndRun();
     } else {
         mWin->postDelayed(mRunnable, minEventIntevalMillis - timeSinceLastMillis);
     }
}

void Window::SendWindowContentChangedAccessibilityEvent::removeCallbacks(){
    mWin->removeCallbacks(mRunnable);
    // CDROID addition beyond AOSP: also drop the source — detach-then-delete
    // teardown frees the source between the post and the run, and both
    // runOrPost's dedup path (getCommonPredecessor) and run() would read it.
    mSource = nullptr;
    mChangeTypes = 0;
}

void Window::SendWindowContentChangedAccessibilityEvent::removeCallbacksAndRun() {
    mWin->removeCallbacks(mRunnable);
    run();
}

}  //endof namespace
