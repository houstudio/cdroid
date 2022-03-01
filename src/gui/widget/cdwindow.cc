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
#include <widget/measurespec.h>
#include <fstream>

namespace cdroid {
constexpr int FORWARD = 0;
constexpr int FINISH_HANDLED = 1;
constexpr int FINISH_NOT_HANDLED = 2;

Window::Window(Context*ctx,const AttributeSet&atts)
  :ViewGroup(ctx,atts){
    source=new UIEventSource(this,[this](){ doLayout(); });
    setFrame(0,0,1280,720);
    setFocusable(true);
    mInLayout=false;
    setKeyboardNavigationCluster(true);
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    WindowManager::getInstance().addWindow(this);
}

Window::Window(int x,int y,int width,int height,int type)
  : ViewGroup(width,height),source(nullptr),window_type(type){
    // Set the boundary
    // Do the resizing at first time in order to invoke the OnLayout
    source=new UIEventSource(this,[this](){ doLayout(); });
    mContext=&App::getInstance();
    mInLayout=false;
    LOGV("%p source=%p visible=%d size=%dx%d",this,source,hasFlag(VISIBLE),width,height);
    setFrame(x, y, width, height);
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    setFocusable(true);
    setKeyboardNavigationCluster(true);
    WindowManager::getInstance().addWindow(this);
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

void Window::setText(const std::string&txt){
    mText=txt;
}

const std::string Window::getText()const{
    return mText;
}

void Window::setRegion(const RefPtr<Region>&rgn){
    mWindowRgn=rgn->copy();
}

void Window::draw(){
    RefPtr<Canvas>canvas=getCanvas();
    mAttachInfo->mDrawingTime=SystemClock::uptimeMillis();
    ViewGroup::draw(*canvas);
    //if(DEBUG_DRAW)drawInvalidateRegion(*canvas);
    mInvalidRgn->subtract(mInvalidRgn);
    GraphDevice::getInstance().flip();
}

void Window::show(){
    if(isAttachedToWindow()&&getVisibility()!=VISIBLE){
        setVisibility(VISIBLE);
        if(mAttachInfo->mCanvas==nullptr)
            invalidate(true);
        WindowManager::getInstance().resetVisibleRegion();
    }
}

void Window::hide(){
    if(getVisibility()==VISIBLE){
        setVisibility(INVISIBLE);
        WindowManager::getInstance().resetVisibleRegion();
    }
}

View& Window::setPos(int x,int y){
    const bool changed =(x!=mLeft)||(mTop!=y);
    if( changed && isAttachedToWindow()){
        WindowManager::getInstance().moveWindow(this,x,y);
        ViewGroup::setPos(x,y);
        WindowManager::getInstance().resetVisibleRegion();
    }
    GraphDevice::getInstance().flip();
    return *this;
}

void Window::onSizeChanged(int w,int h,int oldw,int oldh){
    WindowManager::getInstance().resetVisibleRegion();
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
    //for children's canvas is allcated by it slef and delete by drawing thread(UIEventSource)
    if(mAttachInfo==nullptr)return nullptr;
    RefPtr<Canvas> canvas=mAttachInfo->mCanvas;
    if((canvas==nullptr)&&(getVisibility()==VISIBLE)){
        //GraphDevice::getInstance().createContext(getBound());
        canvas=make_refptr_for_instance<Canvas>(new Canvas(getWidth(),getHeight()));		
        mAttachInfo->mCanvas=canvas;
    }
    const int num=mInvalidRgn->get_num_rectangles();
    canvas->reset_clip();
    for(int i=0;i<num;i++){
        RectangleInt r=mInvalidRgn->get_rectangle(i);
        canvas->rectangle(r.x,r.y,r.width,r.height);
    }
    if(num>0)canvas->clip();
    return canvas;
}

Window::~Window() {
    delete mAttachInfo;
}

void Window::onActive(){
    LOGV("%p[%s]",this,getText().c_str());
}

void Window::onDeactive(){
    LOGV("%p[%s]",this,getText().c_str());
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
    const int action=event.getAction();
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
    if(action==KeyEvent::ACTION_DOWN){
        if(groupNavigationDirection!=0){
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
    View* focused =getFocusedChild();
    bool handled=false;
    const int action=event.getAction();
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
    if(action==KeyEvent::ACTION_DOWN){
        if(groupNavigationDirection!=0)
            return performKeyboardGroupNavigation(groupNavigationDirection);
        handled=performFocusNavigation(event);
    }
    if(!handled){
        switch(action){
        case KeyEvent::ACTION_UP  : 
        case KeyEvent::ACTION_DOWN: 
             handled = ViewGroup::dispatchKeyEvent(event);break;
        default:break;
        }
    }
    return handled;
}

bool Window::performFocusNavigation(KeyEvent& event){
    int direction = -1;
    switch (event.getKeyCode()) {
    case KEY_DPAD_LEFT: direction = View::FOCUS_LEFT;    break;
    case KEY_DPAD_RIGHT: direction = View::FOCUS_RIGHT;   break;
    case KEY_DPAD_UP:    direction = View::FOCUS_UP;      break;
    case KEY_DPAD_DOWN:   direction = View::FOCUS_DOWN;    break;
    case KEY_TAB:
        if (event.hasNoModifiers()) {
            direction = View::FOCUS_FORWARD;
        } else if (event.hasModifiers(KeyEvent::META_SHIFT_ON)) {
            direction = View::FOCUS_BACKWARD;
        }
        break;
    }

    if (direction != -1){
        ViewGroup*mView=(ViewGroup*)this;
        View* focused = mView->findFocus();
        Rect& mTempRect=mRectOfFocusedView;
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
    switch(keyCode){
    case KEY_ESCAPE:
        LOGD("recv %d %s flags=%x track=%d cance=%d",keyCode,evt.getLabel(),evt.getFlags(),evt.isTracking(),evt.isCanceled());
        if(evt.isTracking()&&!evt.isCanceled()){
            onBackPressed();
            return true;
        }//pass throught return false;
    default:return false;
    }
}

void Window::onBackPressed(){
    post([this](){WindowManager::getInstance().removeWindow(this);} );
}

bool Window::postDelayed(Runnable& what,uint32_t delay){
    return source && source->post(what,delay);
}

bool Window::removeCallbacks(const Runnable& what){
    return source && source->removeCallbacks(what);
}

bool Window::isInLayout()const{
    return mInLayout;
}

void Window::doLayout(){
    LOGV("requestLayout(%dx%d)child.count=%d HAS_BOUNDS=%x",getWidth(),getHeight(),
                getChildCount(),(mPrivateFlags&PFLAG_HAS_BOUNDS));
    for(auto c:mChildren){
        int x=0,y=0;
        int widthSpec,heightSpec;
        ViewGroup*vg=dynamic_cast<ViewGroup*>(c);
        if(vg==nullptr)continue;
        LayoutParams*lp=vg->getLayoutParams();
        LOGV("lp=%p  layoutsize=%d,%d",lp,lp->width,lp->height);

        if(vg->getWidth()>0) widthSpec =MeasureSpec::makeMeasureSpec(vg->getWidth(),MeasureSpec::EXACTLY);
        else if(vg->getWidth()<0)widthSpec=MeasureSpec::makeMeasureSpec(lp->width,MeasureSpec::AT_MOST);
        else widthSpec=MeasureSpec::makeMeasureSpec(mRight-mLeft,MeasureSpec::EXACTLY);
    
        if(vg->getHeight()>0) heightSpec =MeasureSpec::makeMeasureSpec(vg->getHeight(),MeasureSpec::EXACTLY);
        else if(vg->getHeight()<0)heightSpec=MeasureSpec::makeMeasureSpec(lp->height,MeasureSpec::AT_MOST);
        else heightSpec=MeasureSpec::makeMeasureSpec(mBottom-mTop,MeasureSpec::EXACTLY);

        if(vg->getHeight()>0&&vg->getWidth()>0){
            x=vg->getLeft();
            y=vg->getTop();
        }
        vg->measure(widthSpec, heightSpec);
        vg->layout(x,y,vg->getMeasuredWidth(),vg->getMeasuredHeight());
    }
    mPrivateFlags&=~PFLAG_FORCE_LAYOUT;
    mInLayout=false;
}

void Window::broadcast(DWORD msgid,DWORD wParam,ULONG lParam){
    WindowManager::getInstance().broadcast(msgid,wParam,lParam);
}

void Window::close(){
    //sendMessage(View::WM_DESTROY,0,0);
}

void Window::dispatchInvalidateOnAnimation(View*view){
    mInvalidateOnAnimationRunnable.setOwner(this);
    mInvalidateOnAnimationRunnable.addView(view);
}

void Window::cancelInvalidate(View* view){
    mInvalidateOnAnimationRunnable.removeView(view);
}

Window::InvalidateOnAnimationRunnable::InvalidateOnAnimationRunnable(){
    mOwner =nullptr;
    mPosted=false;
}

void Window::InvalidateOnAnimationRunnable::setOwner(Window*w){
    mOwner=w;
}

void Window::InvalidateOnAnimationRunnable::addView(View* view){
    if(std::find(mViews.begin(),mViews.end(),view)==mViews.end()){
        mViews.push_back(view);
        postIfNeededLocked();
    }
}

void Window::InvalidateOnAnimationRunnable::removeView(View* view){
    auto it=std::find(mViews.begin(),mViews.end(),view);
    if(it!=mViews.end())mViews.erase(it);
    if(mViews.size()==0){
        mPosted = false;
    }
}

void Window::InvalidateOnAnimationRunnable::run(){
    int viewCount;
    int viewRectCount;
    mPosted = false;

    std::vector<View*>tempViews=mViews; 
    mViews.clear();

    for (auto view:tempViews){
        view->invalidate();
    }
}

void Window::InvalidateOnAnimationRunnable::postIfNeededLocked() {
    if (!mPosted) {
        //Choreographer::getInstance().postCallback(Choreographer::CALLBACK_ANIMATION,nullptr,this);
        Runnable run;run=std::bind(&InvalidateOnAnimationRunnable::run,this);
        mOwner->postDelayed(run,AnimationHandler::getFrameDelay());
        mPosted = true;
    }
}

}  //endof namespace
