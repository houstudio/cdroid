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
#include <core/contextthemewrapper.h>
#include <core/intent.h>
#include <core/componentname.h>
#include <core/looper.h>
#include <widget/cdwindow.h>
#include <widget/toolbar.h>
#include <widget/toolbaractionbar.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <menu/menuinflater.h>
#include <menu/contextmenubuilder.h>
#include <menu/contextmenu.h>
#include <menu/menudialoghelper.h>
#include <widget/textview.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/floatingactionmode.h>
#include <view/focusfinder.h>
#include <core/systemclock.h>
#include <content/typedvalue.h>
#include <core/windowmanager.h>
#include <animation/animator.h>
#include <animation/objectanimator.h>
#include <animation/valueanimator.h>
#include <animation/animationutils.h>
#include <animation/animationset.h>
#include <animation/alphaanimation.h>
#include <animation/translateanimation.h>
#include <view/gravity.h>
#include <porting/cdlog.h>
#include <porting/cdgraph.h>
#include <fstream>

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
    setFrame(0,0,pt.x,pt.y);
    WindowManager::getInstance().addWindow(this);
    mAttachInfo->mPlaySoundEffect = std::bind(&Window::playSoundImpl,this,std::placeholders::_1);
    loadThemeWindowAnimations();
}

Window::Window(int x,int y,int width,int height,int type)
  : FrameLayout(&App::getInstance()),window_type(type){
    initWindow();
    LOGD("Window::Window(%p)",this);
    // Set the boundary
    // Do the resizing at first time in order to invoke the OnLayout
    mContext = &App::getInstance();
    Point size;
    WindowManager::getInstance().getDefaultDisplay().getSize(size);
    if(width<0)  width = size.x;
    if(height<0) height= size.y;
    setFrame(x, y, width, height);
    mPendingRgn->do_union({0,0,width,height});
    WindowManager::getInstance().addWindow(this);
    mAttachInfo->mPlaySoundEffect = std::bind(&Window::playSoundImpl,this,std::placeholders::_1);
}

// AOSP PhoneWindow(context): same window, but the caller's (possibly themed —
// ContextThemeWrapper) context drives inflation instead of the global App.
// AOSP windows belong to an Activity, which IS a ContextThemeWrapper — CDROID
// windows are the Activity, so a plain context is wrapped in an empty
// ContextThemeWrapper overlay (inherits the app theme via lazy setTo(base)),
// giving every window its own theme for Window::setTheme()/recreate().
Window::Window(Context*ctx,int x,int y,int width,int height,int type, bool themeWindowAnimations)
  : Window(x,y,width,height,type){
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
    // Theme-driven window animations resolve against the FINAL context (the themed overlay
    // above), which did not exist when the delegated geometric ctor ran — load them here.
    // PopupDecorView opts out (see the ctor declaration note).
    if (themeWindowAnimations) loadThemeWindowAnimations();
}

void Window::initWindow(){
    mInLayout= false;
    mAccessibilityManager =&AccessibilityManager::getInstance(mContext);
    mSendWindowContentChangedAccessibilityEvent = nullptr;
    mPendingRgn = Cairo::Region::create();
    mActionBar = nullptr;
    mActionMode = nullptr;
    mMenuInflater = nullptr;
    setBackground(nullptr);
    setLayoutDirection(View::LAYOUT_DIRECTION_LTR);
    setTextDirection(View::TEXT_DIRECTION_LTR);
    /*mLayoutRequested = false;
    mTraversalScheduled = false;
    mTraversalRunnable = [this](){
        LOGD("mTraversalRunnable.run");
        doTraversal();
    };*/
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    setFocusable(true);
    setKeyboardNavigationCluster(true);
    AccessibilityManager::AccessibilityStateChangeListener acsl([this](bool enabled) {
        LOGD("%d",enabled);
        if (enabled||1) {
            if (mAttachInfo->mHasWindowFocus||1) {
                sendAccessibilityEvent(AccessibilityEvent::TYPE_WINDOW_STATE_CHANGED);
                View* focusedView = findFocus();
                if ((focusedView != nullptr) && (focusedView != this)) {
                    focusedView->sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_FOCUSED);
                }
                LOGD("focusedView=%d",focusedView);
            }
        } else {
            //mHandler.obtainMessage(MSG_CLEAR_ACCESSIBILITY_FOCUS_HOST).sendToTarget();
        }
    });
    mAccessibilityManager->addAccessibilityStateChangeListener(acsl);
}

Window::~Window(){
    if (mOwnsContext) delete mContext;   // the auto-wrapped ContextThemeWrapper
    if (mActionMode != nullptr) {
        ActionMode* mode = mActionMode;
        mActionMode = nullptr;
        mode->finish();
    }
    delete mActionBar;
    delete mMenuInflater;
    delete mSendWindowContentChangedAccessibilityEvent;
    mDestroyed = true;  // signal the transition end-callback to skip finishClose (we're tearing down)
    if (mCurrentTransitionAnimator) {
        Animator* a = mCurrentTransitionAnimator;
        mCurrentTransitionAnimator = nullptr;  // end-callback sees null + mDestroyed, skips onEnd
        a->cancel();   // cancel() fires onAnimationEnd (see animator.cc) — guarded by mDestroyed above
        delete a;
    }
    delete mEnterTransition;
    delete mExitTransition;
    delete mReturnTransition;
    delete mReenterTransition;
    // NOTE: the AttachInfo is freed by the lambda posted in close() (which stashed it before
    // removeWindow detached/null'd mAttachInfo); ~Window does not touch mAttachInfo.
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

void Window::setActionBar(Toolbar* toolbar){
    delete mActionBar;
    // CDROID's Activity plays the AppCompatActivity role: adopting a Toolbar builds a
    // ToolbarActionBar that bridges it (mirrors androidx AppCompatDelegateImpl +
    // framework Activity.setActionBar).
    mActionBar = toolbar ? new ToolbarActionBar(toolbar, getText(), this) : nullptr;
    if(mActionBar) mActionBar->invalidateOptionsMenu();
}

ActionBar* Window::getActionBar(){
    return mActionBar;
}

bool Window::onCreateOptionsMenu(Menu& /*menu*/){
    return true;
}

bool Window::onPrepareOptionsMenu(Menu& /*menu*/){
    return true;
}

bool Window::onOptionsItemSelected(MenuItem& /*item*/){
    // Non-home options items reach FragmentActivity's override (which dispatches to Fragments).
    // Home/up is folded to onNavigateUp() upstream in onMenuItemSelected (mirrors AOSP
    // Activity.onMenuItemSelected for FEATURE_OPTIONS_PANEL), so it never arrives here.
    return false;
}

bool Window::onContextItemSelected(MenuItem& /*item*/){
    return false;
}

bool Window::onNavigateUp(){
    // CDROID has no manifest parentActivityIntent; the default Up behavior finishes the
    // activity (mirrors androidx Activity.onNavigateUp -> finish when no parent). Override
    // in subclasses (e.g. NavController-driven hosts) for custom Up handling.
    close();
    return true;
}

void Window::invalidateOptionsMenu(){
    if(mActionBar) mActionBar->invalidateOptionsMenu();
}

MenuInflater* Window::getMenuInflater(){
    if(!mMenuInflater) mMenuInflater = new MenuInflater(getContext());
    return mMenuInflater;
}

void Window::openOptionsMenu(){
    if(mActionBar) mActionBar->openOptionsMenu();
}

void Window::closeOptionsMenu(){
    if(mActionBar) mActionBar->closeOptionsMenu();
}

// --- WindowCallback (android.view.Window.Callback, panel/options subset) ---
// CDROID honours a single options panel (FEATURE_OPTIONS_PANEL); other feature ids are no-ops.
View* Window::onCreatePanelView(int /*featureId*/){
    return nullptr; // no custom panel view -> standard options menu
}

bool Window::onCreatePanelMenu(int featureId, Menu& menu){
    return (featureId == FEATURE_OPTIONS_PANEL) ? onCreateOptionsMenu(menu) : false;
}

bool Window::onPreparePanel(int featureId, View* /*view*/, Menu& menu){
    return (featureId == FEATURE_OPTIONS_PANEL) ? onPrepareOptionsMenu(menu) : true;
}

bool Window::onMenuOpened(int /*featureId*/, Menu& /*menu*/){
    return true;
}

bool Window::onMenuItemSelected(int featureId, MenuItem& item){
    // Home -> Up fold. AOSP does this in Activity.onMenuItemSelected for FEATURE_OPTIONS_PANEL;
    // CDROID folds it here (the Window.Callback entry point ToolbarActionBar dispatches through).
    if(featureId == FEATURE_OPTIONS_PANEL && item.getItemId() == R::id::home && mActionBar &&
       (mActionBar->getDisplayOptions() & ActionBar::DISPLAY_HOME_AS_UP)){
        return onNavigateUp();
    }
    return onOptionsItemSelected(item);
}

void Window::onPanelClosed(int /*featureId*/, Menu& /*menu*/){
    // No PhoneWindow panel state machine beyond the toolbar popup; nothing to do here.
}

// =====================================================================================
//  Context menu
// =====================================================================================
bool Window::showContextMenuForChild(View* originalView){
    if(originalView == nullptr) return false;
    ContextMenuBuilder* builder = new ContextMenuBuilder(getContext());
    MenuBuilder::Callback cb;
    cb.onMenuItemSelected = [this](MenuBuilder&, MenuItem& item)->bool{
        return onContextItemSelected(item);
    };
    builder->setCallback(cb);
    // showDialog builds the menu via originalView.createContextMenu (which invokes the
    // OnCreateContextMenuListener registered by registerForContextMenu -> onCreateContextMenu)
    // and presents it as a dialog; item selection routes back through the callback above.
    MenuDialogHelper* helper = builder->showDialog(originalView);
    return helper != nullptr;
}

bool Window::showContextMenuForChild(View* originalView, float /*x*/, float /*y*/){
    // Anchored variant: CDROID shows the context menu as a centered AlertDialog, so the
    // touch coordinates are not used (no floating popup anchored to (x,y) here).
    return showContextMenuForChild(originalView);
}

void Window::registerForContextMenu(View* view){
    if(!view) return;
    view->setOnCreateContextMenuListener(
        [this](ContextMenu& menu, View& v, ContextMenuInfo* info){ onCreateContextMenu(menu, v, info); });
}

void Window::unregisterForContextMenu(View* view){
    if(view) view->setOnCreateContextMenuListener(View::OnCreateContextMenuListener{});
}

void Window::openContextMenu(View* view){
    if(view) view->showContextMenu();
}

void Window::onCreateContextMenu(ContextMenu&, View&, ContextMenuInfo*){}

void Window::closeContextMenu(){
    // CDROID shows the context menu as a self-dismissing AlertDialog via MenuDialogHelper;
    // there is no window panel to close programmatically (no FEATURE_CONTEXT_MENU).
}

// =====================================================================================
//  ActionMode (DecorView)
// =====================================================================================
ActionMode* Window::startActionModeForChild(View* originalView, const ActionMode::Callback& callback, int type){
    return startActionModeInternal(originalView, callback, type);
}

ActionMode* Window::startActionModeInternal(View* originatingView, const ActionMode::Callback& callback, int type){
    if (mActionMode != nullptr) {
        ActionMode* prev = mActionMode;
        mActionMode = nullptr;
        prev->finish();
    }

    // DecorView analog: FloatingActionMode creates its own FloatingToolbar from the root view.
    FloatingActionMode* mode = new FloatingActionMode(getContext(), callback, originatingView);
    mode->setType(type);
    mode->setOnFinishedListener([this, mode]() {
        mActionMode = nullptr;
        post(Runnable([mode] { delete mode; }));
    });
    if (!mode->show()) {
        delete mode;
        return nullptr;
    }
    mActionMode = mode;
    return mode;
}

void Window::playSoundImpl(int effectId){
    LOGD("%d",effectId);
}

View* Window::getCommonPredecessor(View* first, View* second){
     std::set<View*> seen;
     View* firstCurrent = first;
     while (firstCurrent != nullptr) {
         seen.insert(firstCurrent);
         ViewGroup* firstCurrentParent = firstCurrent->mParent;
         firstCurrent = firstCurrentParent;
     }
     View* secondCurrent = second;
     while (secondCurrent != nullptr) {
         if (seen.find(secondCurrent)!=seen.end()) {
             seen.clear();
             return secondCurrent;
         }
         ViewGroup* secondCurrentParent = secondCurrent->mParent;
         secondCurrent = secondCurrentParent;
     }
     seen.clear();
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
    postSendWindowContentChangedCallback(source, changeType);
}

void Window::requestTransitionStart(LayoutTransition* transition){
    auto it = std::find(mPendingTransitions.begin(),mPendingTransitions.end(),transition);
    if(it==mPendingTransitions.end())
        mPendingTransitions.push_back(transition);
}

void Window::setText(const std::string&txt){
    mText=txt;
}

const std::string Window::getText()const{
    return mText;
}

void Window::sendToBack(){
    WindowManager::getInstance().sendToBack(this);
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
        mAccessibilityFocusedHost = nullptr;
        mAccessibilityFocusedVirtualView = nullptr;
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
    mAccessibilityFocusedVirtualView = provider->createAccessibilityNodeInfo(focusedChildId);
    if (mAccessibilityFocusedVirtualView == nullptr) {
        // Error state: The node no longer exists. Clear focus.
        mAccessibilityFocusedHost = nullptr;
        focusedHost->clearAccessibilityFocusNoCallbacks(0);

        // This will probably fail, but try to keep the provider's internal
        // state consistent by clearing focus.
        provider->performAction(focusedChildId,
                AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS,nullptr);
                //AccessibilityAction::ACTION_CLEAR_ACCESSIBILITY_FOCUS.getId(), nullptr);
        //invalidateRectOnScreen(oldBounds);
    } else {
        // The node was refreshed, invalidate bounds if necessary.
        Rect newBounds = mAccessibilityFocusedVirtualView->getBoundsInScreen();
        if (oldBounds!=newBounds) {
            oldBounds.Union(newBounds);
            //invalidateRectOnScreen(oldBounds);
        }
    }
}

bool Window::requestSendAccessibilityEvent(View* child, AccessibilityEvent& event) {
    if (child== nullptr/* || mStopped || mPausedForTransition*/) {
        return false;
    }

    // Immediately flush pending content changed event (if any) to preserve event order
    /*if (event.getEventType() != AccessibilityEvent::TYPE_WINDOW_CONTENT_CHANGED
            && mSendWindowContentChangedAccessibilityEvent != null
            && mSendWindowContentChangedAccessibilityEvent.mSource != null) {
        mSendWindowContentChangedAccessibilityEvent.removeCallbacksAndRun();
    }*/

    // Intercept accessibility focus events fired by virtual nodes to keep
    // track of accessibility focus position in such nodes.
    const int eventType = event.getEventType();
    long sourceNodeId =-1;
    int accessibilityViewId =-1;
    View*source = nullptr;
    switch (eventType) {
    case AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUSED:
        sourceNodeId = event.getSourceNodeId();
        accessibilityViewId = AccessibilityNodeInfo::getAccessibilityViewId(sourceNodeId);
        source = findViewByAccessibilityId(accessibilityViewId);
        if (source != nullptr) {
            AccessibilityNodeProvider* provider = source->getAccessibilityNodeProvider();
            if (provider != nullptr) {
                const int virtualNodeId = AccessibilityNodeInfo::getVirtualDescendantId(sourceNodeId);
                AccessibilityNodeInfo* node = provider->createAccessibilityNodeInfo(virtualNodeId);
                setAccessibilityFocus(source, node);
            }
        }
        break;
    case AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED:
        sourceNodeId = event.getSourceNodeId();
        accessibilityViewId = AccessibilityNodeInfo::getAccessibilityViewId(sourceNodeId);
        source = findViewByAccessibilityId(accessibilityViewId);
        if (source != nullptr) {
            AccessibilityNodeProvider* provider = source->getAccessibilityNodeProvider();
            if (provider != nullptr) {
                setAccessibilityFocus(nullptr, nullptr);
            }
        }
        break;

    case AccessibilityEvent::TYPE_WINDOW_CONTENT_CHANGED:
        handleWindowContentChangedEvent(event);
        break;
    }
    AccessibilityManager::getInstance(mContext).sendAccessibilityEvent(event);
    return true;
}

void Window::setAccessibilityFocus(View* view, AccessibilityNodeInfo* node){

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
    //mPendingRgn->do_union(mInvalidRgn);
    //mInvalidRgn->subtract(mInvalidRgn);
    GraphDevice::getInstance().flip();
}

void Window::setPos(int x,int y){
    const bool changed =(x!=mLeft)||(mTop!=y);
    if( changed && isAttachedToWindow()){
        WindowManager::getInstance().moveWindow(this,x,y);
        FrameLayout::layout(x,y,getWidth(),getHeight());
        mAttachInfo->mWindowLeft= x;
        mAttachInfo->mWindowTop = y;
    }
    GraphDevice::getInstance().flip();
}

View& Window::setAlpha(float alpha){
    // Set the VIEW alpha too: the whole-surface fade is applied at composition
    // (GraphDevice::composeSurfaces reads getAlpha() and paints with it — the X11-style
    // backends have no per-surface opacity, GFXSurfaceSetOpacity is a stub there).
    // View::setAlpha also invalidates, which re-queues the window for composition.
    View::setAlpha(alpha);
    if(isAttachedToWindow()){
        RefPtr<Canvas> canvas = getCanvas();
        LOGV("setAlpha(%p,%d)",this,(int)(alpha*255));
        GFXSurfaceSetOpacity(canvas->mHandle, (alpha*255));
    }
    return *this;
}

void Window::onSizeChanged(int w,int h,int oldw,int oldh){
    //WindowManager::getInstance().resetVisibleRegion();
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
#if 1
    Cairo::RefPtr<Cairo::Region>transRgn = Cairo::Region::create();//{0,0,getWidth(),getHeight()});
    //setWillNotDraw(true);
    int num = gatherTransparentRegion(transRgn);
    Cairo::RectangleInt rec = transRgn->get_extents();
    LOGV_IF(num,"transRgn.rects=%d extents=(%d,%d,%d,%d)",num,rec.x,rec.y,rec.width,rec.height);
    mInvalidRgn->subtract(transRgn);
#endif
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

int Window::processInputEvent(InputEvent&event){
    if(dynamic_cast<KeyEvent*>(&event))
        return processKeyEvent((KeyEvent&)event);
    else return processPointerEvent((MotionEvent&)event);
}

int Window::processPointerEvent(MotionEvent&event){
    return 0;
}

int Window::processKeyEvent(KeyEvent&event){
    int handled = FINISH_NOT_HANDLED;
    int groupNavigationDirection = 0;
    const int action = event.getAction();
    LOGV_IF(action==KeyEvent::ACTION_DOWN,"%s:0x%x %s %x",event.actionToString(action).c_str(),
            event.getKeyCode(),KeyEvent::keyCodeToString(event.getKeyCode()).c_str(),KeyEvent::KEYCODE_DPAD_DOWN);
    if(dispatchKeyEvent(event))
        return FINISH_HANDLED;
    if (action == KeyEvent::ACTION_DOWN  && event.getKeyCode() == KeyEvent::KEYCODE_TAB) {
        if (KeyEvent::metaStateHasModifiers(event.getMetaState(), KeyEvent::META_META_ON)) {
            groupNavigationDirection = View::FOCUS_FORWARD;
        } else if (KeyEvent::metaStateHasModifiers(event.getMetaState(),
              KeyEvent::META_META_ON | KeyEvent::META_SHIFT_ON)) {
            groupNavigationDirection = View::FOCUS_BACKWARD;
        }
    }
    if (event.getAction() == KeyEvent::ACTION_DOWN
            && !KeyEvent::metaStateHasNoModifiers(event.getMetaState())
            && event.getRepeatCount() == 0
            && !KeyEvent::isModifierKey(event.getKeyCode())
            && groupNavigationDirection == 0) {
        if (dispatchKeyShortcutEvent(event)) {
            return FINISH_HANDLED;
        }
        /*if (shouldDropInputEvent(q)) {
            return FINISH_NOT_HANDLED;
        }*/
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
    View* focused = getFocusedChild();
    bool handled  = false;
    const int action = event.getAction();
    if(focused && focused->dispatchKeyEvent(event))
        return true;
    int groupNavigationDirection = 0;
    if (action == KeyEvent::ACTION_DOWN  && event.getKeyCode() == KeyEvent::KEYCODE_TAB) {
        if (KeyEvent::metaStateHasModifiers(event.getMetaState(), KeyEvent::META_META_ON)) {
            groupNavigationDirection = View::FOCUS_FORWARD;
        } else if (KeyEvent::metaStateHasModifiers(event.getMetaState(),
              KeyEvent::META_META_ON | KeyEvent::META_SHIFT_ON)) {
            groupNavigationDirection = View::FOCUS_BACKWARD;
        }
    }
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
        // (Window::onKeyDown ESC startTracking; Window::onKeyUp ESC isTracking -> onBackPressed).
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
    case KeyEvent::KEYCODE_DPAD_LEFT:  direction = View::FOCUS_LEFT;  break;
    case KeyEvent::KEYCODE_DPAD_RIGHT: direction = View::FOCUS_RIGHT; break;
    case KeyEvent::KEYCODE_DPAD_UP:    direction = View::FOCUS_UP;    break;
    case KeyEvent::KEYCODE_DPAD_DOWN:  direction = View::FOCUS_DOWN;  break;
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
        evt.startTracking();
        LOGD("recv %d %s flags=%x",keyCode,KeyEvent::keyCodeToString(keyCode).c_str(),evt.getFlags());
        return true;
    default:
        //return performFocusNavigation(evt);
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


void Window::startActivityForResult(const Intent& intent, int requestCode){
    App::getInstance().startActivityForResultInternal(this, intent, requestCode);
}

void Window::close(){
    close(nullptr);
}

void Window::close(const std::function<void()>& onTeardown){
    // Deliver pending activity result synchronously (onActivityResult must not wait on the exit
    // animation). Then, if a close transition (returnTransition, else exitTransition) is configured,
    // play it before tearing down — the Window stays in mWindows (visible to composeSurfaces) until
    // the animation ends, at which point finishClose() runs removeWindow + posts the deletes.
    // Idempotence: close() may be re-entered (a dismiss listener closing again, a second close
    // during the exit animation — the else branch would run finishClose() immediately). Without
    // the guard the second call posts a second `delete self` for the same Window: a double free.
    // Mirrors Dialog::dismiss's mShowing guard.
    if (mClosePending) return;
    mClosePending = true;
    mTeardownCb = onTeardown;
    App::getInstance().dispatchPendingResult(this);
    ActivityTransition* t = mReturnTransition ? mReturnTransition : mExitTransition;
    if (t && t->getType() != ActivityTransition::Type::NONE
        && !mInTransition && isAttachedToWindow() && getVisibility() == VISIBLE) {
        startExitAnimation([this](){ finishClose(); });
    } else {
        finishClose();
    }
}

void Window::finishClose(){
    // The teardown callback runs FIRST: after the exit transition (when one
    // played) but before removeWindow detaches the tree - the caller's last
    // point with the view hierarchy intact. detachOwner() clears it when the
    // owner dies mid-animation (cancel the pending notification).
    if (mTeardownCb) {
        std::function<void()> cb = mTeardownCb;
        mTeardownCb = nullptr;
        cb();
    }
    // removeWindow detaches the view tree (nulls mAttachInfo), so stash AttachInfo first; the
    // posted lambda frees it + the window. removeWindow runs IMMEDIATELY (window leaves the
    // compositor at once). The deletes are deferred so the current call stack can still touch this
    // window safely. BUT the post must use a standalone heap Handler, NOT View::post (mAttachInfo's
    // handler = mUIEventHandler): removeWindow below calls removeEventHandler(mUIEventHandler),
    // which purges that handler's queued messages — including this delete-window post — so the
    // window would never be deleted. A heap Handler that self-deletes keeps the post alive past
    // removeWindow (same pattern as the Transition clone self-delete in Transition::end()).
    auto* info = mAttachInfo;
    Window* self = this;
    Handler* h = new Handler();
    h->post([h, self, info](){
        self->onDestroy();
        // Purge any Choreographer traversal callback that survived teardown — it captures `self`
        // and would call doTraversal() on a freed Window. Must run here (last safe point before
        // `delete self`, after removeWindow fully ran), by token=self. removeWindow purges the
        // UIEventHandler's messages but NOT Choreographer callbacks (separate queue); and
        // dispatchDetachedFromWindow -> onWindowVisibilityChanged(GONE) can re-trigger
        // scheduleTraversals(), re-posting that callback, so removal belongs at the very end.
        Choreographer::getInstance().removeCallbacks(
            Choreographer::CALLBACK_TRAVERSAL, nullptr, self);
        self->mTraversalScheduled = false;
        delete info;
        delete self;
        delete h;
    });
    WindowManager::getInstance().removeWindow(this);
}

// =====================================================================================
//  Activity transitions (Window-level: setAlpha for Fade, setPos for Slide)
//  CDROID's Window is the composition root, so only moving the Window itself (setPos) or setting
//  its surface opacity (setAlpha) produces a visible whole-window transition; a content view's
//  translationX cannot move the surface (composeSurfaces blits by getBound(), bypassing the View
//  transform). Mirrors android.app.Activity transition API names; the implementation is NOT
//  android.transition.Transition (content-level).
// =====================================================================================
void Window::setEnterTransition(ActivityTransition* t) {
    delete mEnterTransition;
    mEnterTransition = t;
    if (t && t->getType() != ActivityTransition::Type::NONE) {
        // Capture the resting position BEFORE snapEnterStart moves us offscreen: snapEnterStart calls
        // setPos (-> moveWindow -> setFrame), which overwrites mLeft/mTop with the offscreen start.
        // Reading getLeft()/getTop() later would return that offscreen value, so the enter animation
        // would slide entirely off-screen (resting pos corrupted by exactly +width/+height). The Window
        // ctor already ran setFrame(0,0,W,H), so getLeft()/getTop() here ARE the final resting pos.
        mEnterRestX = getLeft();
        mEnterRestY = getTop();
        mEnterRestValid = true;
        mPendingEnterAnim = true;
        snapEnterStart(t);  // pre-snap to the start state before the first frame (no full-show flash)
    }
}
void Window::setExitTransition(ActivityTransition* t)    { delete mExitTransition;    mExitTransition = t; }
void Window::setReturnTransition(ActivityTransition* t)  { delete mReturnTransition;  mReturnTransition = t; }
void Window::setReenterTransition(ActivityTransition* t) { delete mReenterTransition; mReenterTransition = t; }

// Delta reader for the slide mapping: TranslateAnimation's deltas are protected and carry no
// getters, so a derived shim exposes them (the standard protected-member access idiom).
namespace {
struct TranslateDeltaReader : TranslateAnimation {
    static float fromX(const TranslateAnimation* a) { return ((TranslateDeltaReader*)a)->mFromXDelta; }
    static float toX(const TranslateAnimation* a)   { return ((TranslateDeltaReader*)a)->mToXDelta; }
    static float fromY(const TranslateAnimation* a) { return ((TranslateDeltaReader*)a)->mFromYDelta; }
    static float toY(const TranslateAnimation* a)   { return ((TranslateDeltaReader*)a)->mToYDelta; }
};
} // namespace

// Map an AOSP window-animation resource onto the whole-Window ActivityTransition model.
// AOSP window animations are usually <set>s of alpha/translate/extend children; the
// whole-surface model can express one effect, so a translate child (the dominant motion)
// drives a SLIDE — edge from the offset delta's axis/sign — and an alpha-only set drives
// a FADE. Returns nullptr when the animation expresses nothing mappable.
static ActivityTransition* transitionFromAnimation(Animation* anim, bool enter) {
    if (anim == nullptr) return nullptr;
    // OWNS anim: the caller loads a fresh Animation just for this parameter
    // extraction (AnimationUtils::loadAnimation), and ~AnimationSet frees the
    // child parts - free the whole tree on every exit or every window-style
    // apply (every popup show!) leaks it (valgrind: 2KB per record).
    struct AnimGuard {
        Animation* a;
        ~AnimGuard() { delete a; }
    } guard{anim};
    std::vector<Animation*> parts;
    if (auto* set = dynamic_cast<AnimationSet*>(anim)) {
        parts = set->getAnimations();
    } else {
        parts.push_back(anim);
    }
    int64_t duration = 0;
    const TranslateAnimation* slide = nullptr;
    bool fades = false;
    for (Animation* a : parts) {
        if (a == nullptr) continue;
        duration = std::max<int64_t>(duration, a->getDuration());
        if (slide == nullptr) slide = dynamic_cast<TranslateAnimation*>(a);
        if (dynamic_cast<AlphaAnimation*>(a) != nullptr) fades = true;
    }
    if (slide != nullptr) {
        // The nonzero delta gives the motion axis+direction: an enter animation's from-delta is
        // the side the window comes FROM; an exit animation's to-delta is the side it leaves TO.
        const float dx = enter ? TranslateDeltaReader::fromX(slide) : TranslateDeltaReader::toX(slide);
        const float dy = enter ? TranslateDeltaReader::fromY(slide) : TranslateDeltaReader::toY(slide);
        int edge = Gravity::RIGHT; // computeSlidePos's default for a degenerate zero delta
        if      (dx < 0) edge = Gravity::LEFT;
        else if (dx == 0 && dy < 0) edge = Gravity::TOP;
        else if (dx == 0 && dy > 0) edge = Gravity::BOTTOM;
        return ActivityTransition::slide(edge, duration > 0 ? duration : 300);
    }
    if (fades) {
        // A bare whole-surface fade is nearly imperceptible at the resource's own
        // 150-220ms — the scale component it normally pairs with (the visible part of
        // grow_fade_in) is not expressible window-level, so hold the fade long enough
        // to read (window scaling is unsupported).
        constexpr int64_t MIN_FADE_DURATION_MS = 350;
        return ActivityTransition::fade(std::max<int64_t>(duration, MIN_FADE_DURATION_MS));
    }
    return nullptr;
}

void Window::setWindowAnimations(int resId, bool enableExit) {
    mWindowAnimationStyle = resId;
    mWindowExitAnimationsEnabled = enableExit;
    if (resId != 0) applyWindowAnimationStyle(resId); // resolve now (AOSP: params.windowAnimations)
}

void Window::applyWindowAnimationStyle(int styleRes) {
    if (styleRes == 0 || mContext == nullptr) return;
    // AOSP R.styleable.WindowAnimation: the plain window names, falling back to the Activity
    // open/close names Animation.Activity carries (the windowAnimationStyle target). The
    // generated styleable array carries its terminating 0 sentinel (gen_styleable.py appends
    // one), so the hand-maintained attr list and sentinel are gone.
    auto ta = mContext->getTheme().obtainStyledAttributes(styleRes, R::styleable::WindowAnimation);
    if (!ta) return;
    const int enterRes = ta->getResourceId(R::styleable::WindowAnimation_windowEnterAnimation,
                          ta->getResourceId(R::styleable::WindowAnimation_activityOpenEnterAnimation, 0));
    const int exitRes  = ta->getResourceId(R::styleable::WindowAnimation_windowExitAnimation,
                          ta->getResourceId(R::styleable::WindowAnimation_activityCloseExitAnimation, 0));

    // Install like setEnterTransition would, but reuse the resting position captured by an
    // earlier install — after the first snap getLeft()/getTop() are the OFFSCREEN start, and
    // re-capturing them would corrupt the resting point (the exact bug its comment describes).
    auto enterT = enterRes != 0
        ? transitionFromAnimation(AnimationUtils::loadAnimation(mContext, enterRes), true) : nullptr;
    if (enterT != nullptr) {
        delete mEnterTransition;
        mEnterTransition = enterT;
        if (!mEnterRestValid) {
            mEnterRestX = getLeft();
            mEnterRestY = getTop();
            mEnterRestValid = true;
        }
        mPendingEnterAnim = true;
        snapEnterStart(enterT);
    }
    auto exitT = (exitRes != 0 && mWindowExitAnimationsEnabled)
        ? transitionFromAnimation(AnimationUtils::loadAnimation(mContext, exitRes), false) : nullptr;
    if (exitT != nullptr) {
        delete mExitTransition;
        mExitTransition = exitT;
    }
}

void Window::loadThemeWindowAnimations() {
    if (mContext == nullptr) return;
    if (mWindowAnimationStyle != 0) { // explicit override (AOSP LayoutParams.windowAnimations)
        applyWindowAnimationStyle(mWindowAnimationStyle);
        return;
    }
    // AOSP PhoneWindow.generateLayout: the theme's windowAnimationStyle carries the window
    // animation style; a compiled @style item resolves as a reference whose data is the style id.
    TypedValue styleValue;
    if (!mContext->getTheme().resolveAttribute(R::attr::windowAnimationStyle, &styleValue, true)) return;
    const int styleRes = (styleValue.type == TypedValue::TYPE_REFERENCE)
            ? (int)styleValue.data : (int)styleValue.resourceId;
    if (styleRes != 0) applyWindowAnimationStyle(styleRes);
}

void Window::startEnterAnimation() {
    runActivityTransition(mEnterTransition, true, std::function<void()>());
}

void Window::startExitAnimation(const std::function<void()>& onEnd) {
    ActivityTransition* t = mReturnTransition ? mReturnTransition : mExitTransition;
    runActivityTransition(t, false, onEnd);
}

void Window::computeSlidePos(int edge, int ox, int oy, int w, int h, bool offscreen, int& x, int& y) {
    if (edge != Gravity::LEFT && edge != Gravity::RIGHT
        && edge != Gravity::TOP && edge != Gravity::BOTTOM) edge = Gravity::RIGHT;
    x = ox; y = oy;
    if (!offscreen) return;
    if (edge == Gravity::LEFT)        x = ox - w;
    else if (edge == Gravity::RIGHT)  x = ox + w;
    else if (edge == Gravity::TOP)    y = oy - h;
    else                              y = oy + h;  // BOTTOM
}

void Window::snapEnterStart(ActivityTransition* t) {
    if (!t || !isAttachedToWindow()) return;
    if (t->getType() == ActivityTransition::Type::FADE) {
        setAlpha(0.f);
    } else if (t->getType() == ActivityTransition::Type::SLIDE) {
        int x, y;
        computeSlidePos(t->getSlideEdge(), getLeft(), getTop(), getWidth(), getHeight(), true, x, y);
        setPos(x, y);
    }
}

void Window::runActivityTransition(ActivityTransition* t, bool enter, const std::function<void()>& onEnd) {
    if (!t || t->getType() == ActivityTransition::Type::NONE || !isAttachedToWindow()) {
        mInTransition = false;
        if (onEnd) onEnd();
        return;
    }
    // Replace any in-flight transition animator. cancel() fires onAnimationEnd (animator.cc) — the
    // replaced animator is always an enter (onEnd empty), and ~Window's path is guarded by mDestroyed.
    if (mCurrentTransitionAnimator) {
        Animator* prev = mCurrentTransitionAnimator;
        mCurrentTransitionAnimator = nullptr;
        prev->cancel();
        delete prev;
    }
    mInTransition = true;
    const int64_t duration = t->getDuration();
    Animator::AnimatorListener endListener;
    endListener.onAnimationEnd = [this, onEnd](Animator&, bool) {
        if (mDestroyed) return;  // ~Window is tearing us down — don't run finishClose / replace
        mInTransition = false;
        if (onEnd) onEnd();
        // The animator is NOT deleted here (delete-in-end-callback). It stays in
        // mCurrentTransitionAnimator and is freed by ~Window or the next runActivityTransition.
    };

    if (t->getType() == ActivityTransition::Type::FADE) {
        // ObjectAnimator "alpha" dispatches to View::setAlpha, which Window overrides to
        // GFXSurfaceSetOpacity (whole-surface opacity). Each frame must re-compose, so schedule
        // a traversal (setAlpha itself does not invalidate).
        ObjectAnimator* anim = ObjectAnimator::ofFloat(this, "alpha",
            std::vector<float>{enter ? 0.f : 1.f, enter ? 1.f : 0.f});
        anim->setDuration(duration);
        anim->addUpdateListener([this](ValueAnimator&) { scheduleTraversals(); });
        anim->addListener(endListener);
        mCurrentTransitionAnimator = anim;
        anim->start();
    } else { // SLIDE — translate the whole window surface via setPos/moveWindow.
        const int w = getWidth(), h = getHeight();
        // Enter slides back to the resting position captured in setEnterTransition (snapEnterStart has
        // since moved the window offscreen, so getLeft()/getTop() are no longer the resting pos).
        // Exit slides from the live resting pos — the window is at rest when close() starts the exit.
        const int ox = (enter && mEnterRestValid) ? mEnterRestX : getLeft();
        const int oy = (enter && mEnterRestValid) ? mEnterRestY : getTop();
        int offX, offY;
        computeSlidePos(t->getSlideEdge(), ox, oy, w, h, true, offX, offY);
        const int startX = enter ? offX : ox;
        const int startY = enter ? offY : oy;
        const int endX   = enter ? ox  : offX;
        const int endY   = enter ? oy  : offY;
        ValueAnimator* anim = ValueAnimator::ofFloat(std::vector<float>{0.f, 1.f});
        anim->setDuration(duration);
        anim->addUpdateListener([this, startX, startY, endX, endY](ValueAnimator& a) {
            const float f = a.getAnimatedFraction();
            setPos((int)(startX + (endX - startX) * f), (int)(startY + (endY - startY) * f));
        });
        anim->addListener(endListener);
        mCurrentTransitionAnimator = anim;
        if (enter) setPos(startX, startY);  // ensure offscreen before the first animated frame
        anim->start();
    }
}

void Window::scheduleTraversals(){
    if(mTraversalScheduled) return;
    mTraversalScheduled = true;
    Choreographer::getInstance().postCallback(
        Choreographer::CALLBACK_TRAVERSAL,
        [this](){ doTraversal(); },
        this);  // token=this so close() can purge the pending callback by window identity
}

void Window::doTraversal(){
    mTraversalScheduled = false;
    GraphDevice::getInstance().lock();
    if(isAttachedToWindow()){
        if(isLayoutRequested()) doLayout();
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
    return FrameLayout::dispatchTouchEvent(event);
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
    for (auto i:mInvalidateViews){
        Rect&r = i->rect;
        View*v = i->target;
        if(r.width<=0||r.height<=0) v->invalidate();
        else  v->invalidate(r);
        i->recycle();
    }
    mInvalidateViews.clear();
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
    auto it = find(view);
    if(it == mInvalidateViews.end()){
        AttachInfo::InvalidateInfo* info = AttachInfo::InvalidateInfo::obtain();
        info->target = view;
        info->rect.set(0,0,0,0);
        mInvalidateViews.push_back(info);
    }else{
        AttachInfo::InvalidateInfo* info = (*it);
        info->rect.set(0,0,0,0);
    }
    postIfNeededLocked();
}

void Window::InvalidateOnAnimationRunnable::addViewRect(View* view,const Rect&rect){
    auto it = find(view);
    if(it == mInvalidateViews.end()){
        AttachInfo::InvalidateInfo* info = AttachInfo::InvalidateInfo::obtain();
        info->target =view;
        info->rect = rect;
        mInvalidateViews.push_back(info);
    }else{
        AttachInfo::InvalidateInfo* info = (*it);
        if(!info->rect.empty())
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
    // Lazily load the accessibility focus drawable.
    if (mAttachInfo->mAccessibilityFocusDrawable == nullptr) {
        LOGD("TODO");
        //mAttachInfo->mAccessibilityFocusDrawable = mContext->getDrawable(value.resourceId);
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
    LOGD("mSource=%p mChangeTypes=%d",source,mChangeTypes);
    if (source == nullptr) {
        LOGE("Accessibility content change has no source");
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
}

void Window::SendWindowContentChangedAccessibilityEvent::removeCallbacksAndRun() {
    mWin->removeCallbacks(mRunnable);
    run();
}

}  //endof namespace
