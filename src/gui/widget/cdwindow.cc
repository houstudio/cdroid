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
#include <cdroid.h>
#include <widget/cdwindow.h>
#include <widget/textview.h>
#include <view/accessibility/accessibilitymanager.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <uieventsource.h>
#include <systemclock.h>
#include <fstream>

using namespace Cairo;
namespace cdroid {
constexpr int FORWARD = 0;
constexpr int FINISH_HANDLED = 1;
constexpr int FINISH_NOT_HANDLED = 2;

Window::Window(Context*ctx,const AttributeSet&atts)
  :FrameLayout(ctx,atts){
    initWindow();
    Point pt;
    WindowManager::getInstance().getDefaultDisplay().getSize(pt);
    setFrame(0,0,pt.x,pt.y);
    WindowManager::getInstance().addWindow(this);
    mAttachInfo->mPlaySoundEffect = std::bind(&Window::playSoundImpl,this,std::placeholders::_1);
}

Window::Window(int x,int y,int width,int height,int type)
  : FrameLayout(width,height),window_type(type){
    initWindow();
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

void Window::initWindow(){
#if USE_UIEVENTHANDLER
    mUIEventHandler = new UIEventHandler(this,[this](){ doLayout(); });
#else
    mUIEventHandler = new UIEventSource(this,[this](){ doLayout(); });
#endif
    mInLayout= false;
    mAccessibilityManager =&AccessibilityManager::getInstance(mContext);
    mSendWindowContentChangedAccessibilityEvent = nullptr;
    mPendingRgn = Cairo::Region::create();
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
    delete mSendWindowContentChangedAccessibilityEvent;
    LOGD("%p:%d destroied!",this,mID);
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
    //GraphDevice::getInstance().invalidate(getBound());
    GraphDevice::getInstance().flip();
}

ViewGroup*Window::invalidateChildInParent(int* location,Rect& dirty){
    FrameLayout::invalidateChildInParent(location,dirty);
    invalidate(dirty);
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
    LOGV_IF(action==KeyEvent::ACTION_DOWN|1,"%s:0x%x %s %x",event.actionToString(action).c_str(),
            event.getKeyCode(),KeyEvent::getLabel(event.getKeyCode()),KeyEvent::KEYCODE_DPAD_DOWN);
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
        switch(action){
        case KeyEvent::ACTION_UP  :
        case KeyEvent::ACTION_DOWN: handled = FrameLayout::dispatchKeyEvent(event); break;
        default:break;
        }
    }
    return handled;
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
        LOGD("recv %d %s flags=%x",keyCode,evt.getLabel(),evt.getFlags());
        return true;
    default:
        //return performFocusNavigation(evt);
        LOGV("recv %d %s",keyCode,evt.getLabel());
        return FrameLayout::onKeyDown(keyCode,evt);
    } 
    return false;
}

bool Window::onKeyUp(int keyCode,KeyEvent& evt){
    LOGV("recv %d %s flags=%x track=%d cance=%d",keyCode,evt.getLabel(),evt.getFlags(),evt.isTracking(),evt.isCanceled());
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
    post([this](){WindowManager::getInstance().removeWindow(this);} );
}

bool Window::isInLayout()const{
    return mInLayout;
}

void Window::doLayout(){
    LOGV("requestLayout(%dx%d)child.count=%d HAS_BOUNDS=%x",getWidth(),getHeight(),
                getChildCount(),(mPrivateFlags&PFLAG_HAS_BOUNDS));
    if(mChildren.size()==1){
        View*view = mChildren[0];
        const MarginLayoutParams*lp= (const MarginLayoutParams*)view->getLayoutParams();
        const int horzMargin = lp->leftMargin+lp->rightMargin;
        const int vertMargin = lp->topMargin+lp->bottomMargin;
        const int widthSpec  = MeasureSpec::makeMeasureSpec(getWidth() - horzMargin,MeasureSpec::EXACTLY);
        const int heightSpec = MeasureSpec::makeMeasureSpec(getHeight()- vertMargin,MeasureSpec::EXACTLY);
        view->measure(widthSpec, heightSpec);
        view->layout (lp->leftMargin,lp->topMargin,view->getMeasuredWidth(),view->getMeasuredHeight());
    }
    getViewTreeObserver()->dispatchOnGlobalLayout();
    mAttachInfo->mTreeObserver->dispatchOnGlobalLayout();
    mPrivateFlags&=~PFLAG_FORCE_LAYOUT;
    mInLayout = false;
}


void Window::close(){
    post([this](){WindowManager::getInstance().removeWindow(this);} );
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
    mOwner=w;
}

std::vector<View::AttachInfo::InvalidateInfo*>::iterator Window::InvalidateOnAnimationRunnable::find(View*v){
    for(auto it=mInvalidateViews.begin();it!=mInvalidateViews.end();it++){
        if((*it)->target == v)
            return it;
    }
    return mInvalidateViews.end();
}

void Window::InvalidateOnAnimationRunnable::addView(View* view){
    auto it=find(view);
    if(it==mInvalidateViews.end()){
        AttachInfo::InvalidateInfo* info = AttachInfo::InvalidateInfo::obtain();
        info->target = view;
        info->rect.set(0,0,0,0);
        mInvalidateViews.push_back(info);
    }else{
        AttachInfo::InvalidateInfo* info=(*it);
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
        AttachInfo::InvalidateInfo* info=(*it);
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
        //Choreographer::getInstance().postCallback(Choreographer::CALLBACK_ANIMATION,nullptr,this);
        Runnable run(std::bind(&InvalidateOnAnimationRunnable::run,this));
        mOwner->postDelayed(run,AnimationHandler::getFrameDelay());
        mPosted = true;
    }
}

void Window::drawAccessibilityFocusedDrawableIfNeeded(Canvas& canvas){
    Rect bounds;
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

Window::UIEventHandler::UIEventHandler(View*v,std::function<void()>r){
    mAttachedView=v;
    mLayoutRunner=r;
}

void Window::UIEventHandler::handleIdle(){
    if (mAttachedView && mAttachedView->isAttachedToWindow()){
        if(mAttachedView->isLayoutRequested())
            mLayoutRunner();
        if(mAttachedView->isDirty() && mAttachedView->getVisibility()==View::VISIBLE){
            GraphDevice::getInstance().lock();
            ((Window*)mAttachedView)->draw();
            GraphDevice::getInstance().flip();
            GraphDevice::getInstance().unlock();
        }
    }

    if(GraphDevice::getInstance().needCompose())
        GraphDevice::getInstance().requestCompose();
}

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
