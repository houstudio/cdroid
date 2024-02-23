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
#include <windowmanager.h>
#include <cdlog.h>
#include <uieventsource.h>
#include <expat.h>
#include <systemclock.h>
#include <fstream>

using namespace Cairo;
namespace cdroid {
constexpr int FORWARD = 0;
constexpr int FINISH_HANDLED = 1;
constexpr int FINISH_NOT_HANDLED = 2;

Window::Window(Context*ctx,const AttributeSet&atts)
  :ViewGroup(ctx,atts){
    initWindow();
    Point pt;
    WindowManager::getInstance().getDefaultDisplay().getSize(pt);
    setFrame(0,0,pt.x,pt.y);
    WindowManager::getInstance().addWindow(this);
    mAttachInfo->mPlaySoundEffect = std::bind(&Window::playSoundImpl,this,std::placeholders::_1);
}

Window::Window(int x,int y,int width,int height,int type)
  : ViewGroup(width,height),window_type(type){
    initWindow();
    // Set the boundary
    // Do the resizing at first time in order to invoke the OnLayout
    mContext = &App::getInstance();
    Point size;
    WindowManager::getInstance().getDefaultDisplay().getSize(size);
    if(width<0)  width = size.x;
    if(height<0) height= size.y;
    setFrame(x, y, width, height);
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
    mPendingRgn = Cairo::Region::create();
    setBackground(nullptr);
    /*mLayoutRequested = false;
    mTraversalScheduled = false;
    mTraversalRunnable = [this](){
        LOGD("mTraversalRunnable.run");
        doTraversal();
    };*/
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    setFocusable(true);
    setKeyboardNavigationCluster(true);
}

Window::~Window(){
    LOGD("%p:%d destroied!",this,mID);
}

void Window::playSoundImpl(int effectId){
    LOGD("%d",effectId);
}

ViewGroup::LayoutParams* Window::generateDefaultLayoutParams()const{
    return new MarginLayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
}

bool Window::checkLayoutParams(const ViewGroup::LayoutParams* p)const{
    return dynamic_cast<const MarginLayoutParams*>(p);
}

ViewGroup::LayoutParams* Window::generateLayoutParams(const ViewGroup::LayoutParams* lp)const{
    return new MarginLayoutParams(*lp);
}

ViewGroup::LayoutParams* Window::generateLayoutParams(const AttributeSet&atts)const{
    return new MarginLayoutParams(getContext(),atts);
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
    if(mVisibleRgn&&mVisibleRgn->get_num_rectangles()==0){
        return;
    }
    RefPtr<Canvas>canvas = getCanvas();
    mAttachInfo->mDrawingTime = SystemClock::uptimeMillis();
    ViewGroup::draw(*canvas);
    if(View::VIEW_DEBUG)drawInvalidateRegion(*canvas);
    LOGI_IF(View::VIEW_DEBUG,"%p:%d used %dms",this,mID,SystemClock::uptimeMillis() - mAttachInfo->mDrawingTime);
    mPendingRgn->do_union(mInvalidRgn);
    mInvalidRgn->subtract(mInvalidRgn);
    GraphDevice::getInstance().flip();
}

View& Window::setPos(int x,int y){
    const bool changed =(x!=mLeft)||(mTop!=y);
    if( changed && isAttachedToWindow()){
        WindowManager::getInstance().moveWindow(this,x,y);
        ViewGroup::setPos(x,y);
        mAttachInfo->mWindowLeft= x;
        mAttachInfo->mWindowTop = y;
    }
    GraphDevice::getInstance().flip();
    return *this;
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
    GraphDevice::getInstance().invalidate(getBound());
    GraphDevice::getInstance().flip();
}

ViewGroup*Window::invalidateChildInParent(int* location,Rect& dirty){
    ViewGroup::invalidateChildInParent(location,dirty);
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
	LOGV("rotation=%d window.size=%dx%d canvas.size=%dx%d",rotation*90,getWidth(),getHeight(),canvasWidth,canvasHeight);
	switch(rotation){
        case Display::ROTATION_0:break;
        case Display::ROTATION_90:
	   matrix.rotate(-90.*M_PI/180.);
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
	   matrix.rotate(90.*M_PI/180.);
	   canvas->transform(matrix);
	   break;
	}
    }
    const int num = mInvalidRgn->get_num_rectangles();
    canvas->reset_clip();
    for(int i=0;i<num;i++){
        RectangleInt r = mInvalidRgn->get_rectangle(i);
        canvas->rectangle(r.x,r.y,r.width,r.height);
    }
    if(num>0)canvas->clip();
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
    LOGV_IF(action==KeyEvent::ACTION_DOWN,"key:0x%x %s %x",event.getKeyCode(),KeyEvent::getLabel(event.getKeyCode()),KEY_DPAD_DOWN);
    if(dispatchKeyEvent(event))
        return FINISH_HANDLED;
    if (action == KeyEvent::ACTION_DOWN  && event.getKeyCode() == KEY_TAB) {
        if (KeyEvent::metaStateHasModifiers(event.getMetaState(), KeyEvent::META_META_ON)) {
            groupNavigationDirection = View::FOCUS_FORWARD;
        } else if (KeyEvent::metaStateHasModifiers(event.getMetaState(),
              KeyEvent::META_META_ON | KeyEvent::META_SHIFT_ON)) {
            groupNavigationDirection = View::FOCUS_BACKWARD;
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
    View* focused = getFocusedChild();
    bool handled  = false;
    const int action = event.getAction();
    if(focused && focused->dispatchKeyEvent(event))
        return true;
    int groupNavigationDirection = 0;
    if (action == KeyEvent::ACTION_DOWN  && event.getKeyCode() == KEY_TAB) {
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
        case KeyEvent::ACTION_UP  ://handled = onKeyUp(event.getKeyCode(),event);break;
        case KeyEvent::ACTION_DOWN://handled = onKeyDown(event.getKeyCode(),event);break;
             handled = ViewGroup::dispatchKeyEvent(event);break;
        default:break;
        }
    }
    return handled;
}

bool Window::performFocusNavigation(KeyEvent& event){
    int direction = -1;
    switch (event.getKeyCode()) {
    case KEY_DPAD_LEFT:  direction = View::FOCUS_LEFT;    break;
    case KEY_DPAD_RIGHT: direction = View::FOCUS_RIGHT;   break;
    case KEY_DPAD_UP:    direction = View::FOCUS_UP;      break;
    case KEY_DPAD_DOWN:  direction = View::FOCUS_DOWN;    break;
    case KEY_TAB:
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
    case KEY_ESCAPE:
        evt.startTracking();
        LOGD("recv %d %s flags=%x",keyCode,evt.getLabel(),evt.getFlags());
        return true;
    default:
        //return performFocusNavigation(evt);
        LOGV("recv %d %s",keyCode,evt.getLabel());
        return ViewGroup::onKeyDown(keyCode,evt);
    } 
    return false;
}

bool Window::onKeyUp(int keyCode,KeyEvent& evt){
    LOGV("recv %d %s flags=%x track=%d cance=%d",keyCode,evt.getLabel(),evt.getFlags(),evt.isTracking(),evt.isCanceled());
    switch(keyCode){
    case KEY_ESCAPE:
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

bool Window::postDelayed(Runnable& what,uint32_t delay){
    return mUIEventHandler && mUIEventHandler->postDelayed(what,delay);
}

bool Window::removeCallbacks(const Runnable& what){
    if(mUIEventHandler)
        return mUIEventHandler->removeCallbacks((Runnable&)what);
    return !(mUIEventHandler==nullptr);
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
    mPrivateFlags&=~PFLAG_FORCE_LAYOUT;
    mInLayout = false;  
}


void Window::close(){
    post([this](){WindowManager::getInstance().removeWindow(this);} );
}

bool Window::dispatchTouchEvent(MotionEvent& event){
    return ViewGroup::dispatchTouchEvent(event);
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
        else  v->invalidate(r.left,r.top,r.width,r.height);
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

}  //endof namespace
