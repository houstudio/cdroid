/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/

#include <windowmanager.h>
#include <cdlog.h>
#include <graphdevice.h>
#include <uieventsource.h>

using namespace Cairo;
namespace cdroid {
// Initialize the instance of the singleton to nullptr
WindowManager* WindowManager::mInst = nullptr;

WindowManager::WindowManager(){
    mActiveWindow = nullptr;
    mDisplayRotation =0;
    mHoveredWindow = nullptr;
}

WindowManager&WindowManager::getInstance(){
    if(nullptr==mInst){
        mInst = new WindowManager();
    }
    return *mInst;
};

WindowManager::~WindowManager() {
    for(Window*w:mWindows){
        View::AttachInfo*info = w->mAttachInfo;
        w->dispatchDetachedFromWindow();
        delete info;
        delete w;
    };
    mWindows.clear();
    LOGD("%p Destroied",this);
}

void WindowManager::setDisplayRotation(int display,int rot){
    mDisplayRotation = rot;
    if((display>=0) && (display<mDisplays.size()))
        mDisplays[display].mDisplayInfo.rotation=rot;
    LOGD("display %d rotation = %d",display,rot);
}

int WindowManager::getDisplayRotation(int display)const{
    if((display>=0) && (display<mDisplays.size()))
        return mDisplays[display].mDisplayInfo.rotation;
    return -1;
}

Display& WindowManager::getDefaultDisplay(){
    if(mDisplays.size()==0){
	    size_t dc = GFXGetDisplayCount();
	    for(size_t i = 0;i < dc ;i++){
	        DisplayInfo info;
	        info.rotation = mDisplayRotation;
	        GFXGetDisplaySize(i,(uint32_t*)&info.logicalWidth,(uint32_t*)&info.logicalHeight);
	        Display d(i,info);
	        mDisplays.push_back(d);
        }
    }
    return mDisplays.at(Display::DEFAULT_DISPLAY); 
}

Display*WindowManager::getDisplay(int display){
    if((display>=0) && (display<mDisplays.size()))
        return &mDisplays.at(display);
    return nullptr;
}

void WindowManager::addWindow(Window*win){
    mWindows.push_back(win);
    win->mLayer = (win->window_type<<16)|0x7FFF;
    std::sort(mWindows.begin(),mWindows.end(),[](Window*w1,Window*w2){
        return (w2->mLayer - w1->mLayer)>0;
    });

    for(int idx = 0 ;idx < mWindows.size();idx++){
        Window *w = mWindows.at(idx);
        w->mLayer = (w->window_type<<16)|(idx+1);
        LOGV("%p window %p[%s] type=%d layer=%d",win,w,w->getText().c_str(),w->window_type,w->mLayer);
    }
    if(mActiveWindow){
        Window*deactWin = mActiveWindow;
        mActiveWindow->post([deactWin](){deactWin->onDeactive();});
        mActiveWindow->mAttachInfo->mTreeObserver->dispatchOnWindowFocusChange(false);
    }

    View::AttachInfo*info = new View::AttachInfo(win->getContext());
    info->mContentInsets.setEmpty();
    info->mRootView = win;
    info->mEventSource=win->mUIEventHandler;
    win->dispatchAttachedToWindow(info,win->getVisibility());
#if USE_UIEVENTHANDLER    
    Looper::getMainLooper()->addHandler(win->mUIEventHandler);
#else
    Looper::getMainLooper()->addEventHandler(win->mUIEventHandler);
#endif
    win->post([win](){win->onCreate();});
    //the first create only call onCreate,no onActive 
    win->post([info](){
        info->mTreeObserver->dispatchOnWindowFocusChange(true);
    });
    mActiveWindow = win;
    LOGV("win=%p Handler=%p windows.size=%d",win,win->mUIEventHandler,mWindows.size());
}

void WindowManager::removeWindow(Window*w){
    if(w == mActiveWindow){
        mActiveWindow = nullptr;
        w->mAttachInfo->mTreeObserver->dispatchOnWindowFocusChange(false);
    }
    if(w->hasFlag(View::FOCUSABLE)){
        w->dispatchWindowFocusChanged(false);
        w->onDeactive();
    }
    auto itw = std::find(mWindows.begin(),mWindows.end(),w);
    const Rect wrect = w->getBound();
    mWindows.erase(itw);
    for(auto itr=mWindows.begin();itr!=mWindows.end();itr++){
        Window*w1 = (*itr);
        Rect rc = w1->getBound();
        rc.intersect(wrect);
        rc.offset(-w1->getLeft(),-w1->getTop());
        w1->mPendingRgn->do_union({rc.left,rc.top,rc.width,rc.height});
    }
#if USE_UIEVENTHANDLER
    Looper::getMainLooper()->removeHandler(w->mUIEventHandler);
#else
    Looper::getMainLooper()->removeEventHandler(w->mUIEventHandler);
#endif
    View::AttachInfo*info = w->mAttachInfo;
    w->dispatchDetachedFromWindow();
    delete info;
    delete w;
    for(auto it=mWindows.rbegin();it!=mWindows.rend();it++){
        if((*it)->hasFlag(View::FOCUSABLE)&&(*it)->getVisibility()==View::VISIBLE){
            if((*it)!=mActiveWindow){
                 (*it)->dispatchWindowFocusChanged(true);
                 (*it)->onActive();
            }
            mActiveWindow = (*it);
            break;
        } 
    }
    //GraphDevice::getInstance().invalidate(wrect);
    GraphDevice::getInstance().flip();
    LOGI("w=%p windows.size=%d",w,mWindows.size());
}

void WindowManager::removeWindows(const std::vector<Window*>&ws){
    Cairo::RefPtr<Cairo::Region>rgn=Cairo::Region::create();
    for(auto w:ws){
        if(w == mActiveWindow){
            mActiveWindow = nullptr;
            w->mAttachInfo->mTreeObserver->dispatchOnWindowFocusChange(false);
        }
        if(w->hasFlag(View::FOCUSABLE)){
            w->dispatchWindowFocusChanged(false);
            w->onDeactive();
        }
        auto itw = std::find(mWindows.begin(),mWindows.end(),w);
        const Rect rw = w->getBound();
        mWindows.erase(itw);
        rgn->do_union({rw.left,rw.top,rw.width,rw.height});
        for(auto itr=mWindows.begin();itr!=mWindows.end();itr++){
            Window*w1 = (*itr);
            Rect rc = w1->getBound();
            rc.intersect(rw);
            rc.offset(-w1->getLeft(),-w1->getTop());
            w1->invalidate((const Rect*)&rc);
            w1->mPendingRgn->do_union({rc.left,rc.top,rc.width,rc.height});
        }
    #if USE_UIEVENTHANDLER
        Looper::getMainLooper()->removeHandler(w->mUIEventHandler);
    #else
        Looper::getMainLooper()->removeEventHandler(w->mUIEventHandler);
    #endif
        View::AttachInfo*info = w->mAttachInfo;
        w->dispatchDetachedFromWindow();
        delete info;
        delete w;
    }
    for(auto it=mWindows.rbegin();it!=mWindows.rend();it++){
        if((*it)->hasFlag(View::FOCUSABLE)&&(*it)->getVisibility()==View::VISIBLE){
            if((*it)!=mActiveWindow){
                (*it)->dispatchWindowFocusChanged(true);
                (*it)->onActive();
            }
            mActiveWindow = (*it);
            break;
        }
    }
    //const Cairo::RectangleInt re = rgn->get_extents();
    //GraphDevice::getInstance().invalidate({re.x,re.y,re.width,re.height});
    GraphDevice::getInstance().flip();
}

void WindowManager::moveWindow(Window*w,int x,int y){
    Rect rcw = w->getBound();
    Rect rcw2 =rcw;
    rcw2.left = x;
    rcw2.top = y;
    w->setFrame(x,y,rcw.width,rcw.height);
    const auto itw = std::find(mWindows.begin(),mWindows.end(),w);
    if( w->isAttachedToWindow() && (w->getVisibility()==View::VISIBLE)){
        //GraphDevice::getInstance().invalidate(rcw);
        //GraphDevice::getInstance().invalidate(rcw2);
        for(auto it = mWindows.begin();it<itw;it++){
           Rect rc = w->getBound();
           RefPtr<Cairo::Region>newrgn = Cairo::Region::create((RectangleInt&)rc);
           for( auto it2 = it+1 ; it2 < itw ; it2++){
               Rect r = (*it)->getBound();
               newrgn->subtract((const RectangleInt&)r);
           }
           newrgn->translate(-rcw.left,-rcw.top);
           (*it)->mPendingRgn->do_union((RectangleInt&)rcw);
           (*it)->mPendingRgn->subtract((RectangleInt&)rcw2);
        }
        GraphDevice::getInstance().flip();
    }
}

Window*WindowManager::getActiveWindow()const{
    return mActiveWindow;
}

void WindowManager::sendToBack(Window*win){
    win->mLayer = (win->window_type<<16);/*make win's layer to lowerest*/
    std::sort(mWindows.begin(),mWindows.end(),[](Window*w1,Window*w2){
        return (w2->mLayer - w1->mLayer)>0;
    });

    for(int idx = 0 ; idx < mWindows.size();idx++){
        Window *w = mWindows.at(idx);
        w->mLayer = (w->window_type<<16)|(idx+1);
    }
    mActiveWindow = mWindows.back();
    mActiveWindow->mPendingRgn->do_union({0,0,win->getWidth(),win->getHeight()});
    win->post([win](){win->onDeactive();});
    Window*newActWin = mActiveWindow;
    mActiveWindow->post([newActWin](){
        newActWin->onActive();
    });
    GraphDevice::getInstance().flip();
}

void WindowManager::bringToFront(Window*win){
    win->mLayer = (win->window_type<<16)|0x7FFF;
    if(mActiveWindow==win) return;
    std::sort(mWindows.begin(),mWindows.end(),[](Window*w1,Window*w2){
        return (w2->mLayer - w1->mLayer)>0;
    });

    for(int idx = 0 ; idx < mWindows.size();idx++){
        Window *w = mWindows.at(idx);
        w->mLayer = (w->window_type<<16)|(idx+1);
    }
    win->post([win](){win->onActive();});

    Window*deactWin= mActiveWindow;
    mActiveWindow->post([deactWin](){
        deactWin->onDeactive();
    });
    mActiveWindow = win;
    win->mPendingRgn->do_union({0,0,win->getWidth(),win->getHeight()});
    GraphDevice::getInstance().flip();
}

int WindowManager::enumWindows(WNDENUMPROC cbk){
    int rc = 0;
    for(auto& w:mWindows)
       rc+=cbk(w);
    return rc;
}

int WindowManager::getWindows(std::vector<Window*>&wins){
    wins = mWindows;
    return mWindows.size();
}

int WindowManager::getVisibleWindows(std::vector<Window*>&wins){
    for(auto& w:mWindows){
        if(w->getVisibility()==View::VISIBLE)
           wins.push_back(w);
    }
    return wins.size();
}

void WindowManager::processEvent(InputEvent&e){
   switch(e.getType()){
   case InputEvent::INPUT_EVENT_TYPE_KEY: onKeyEvent((KeyEvent&)e); break;
   case InputEvent::INPUT_EVENT_TYPE_MOTION: onMotion((MotionEvent&)e);break;
   default:break;
   }
}


void WindowManager::onMotion(MotionEvent&event) {
   // Notify the focused child
   const int x = event.getX();
   const int y = event.getY();
   const int action = event.getActionMasked();
   // If this is a touchscreen/stylus/touchpad event, keep existing behavior.
   if (event.isFromSource(InputDevice::SOURCE_CLASS_POINTER)){
       for (auto itr = mWindows.rbegin(); itr != mWindows.rend(); itr++) {
           auto w = (*itr);
           ViewTreeObserver*obv = w->getViewTreeObserver();
           if (action == MotionEvent::ACTION_DOWN) {
               w->mAttachInfo->mInTouchMode = true;
               obv->dispatchOnTouchModeChanged(true);
           } else if (action == MotionEvent::ACTION_UP) {
               w->mAttachInfo->mInTouchMode = false;
               obv->dispatchOnTouchModeChanged(false);
           }
           LOGV_IF(action != MotionEvent::ACTION_MOVE, "%s at(%d,%d)", MotionEvent::actionToString(action).c_str(), x, y);
           if ((w->getVisibility() == View::VISIBLE) && w->getBound().contains(x, y)) {
               event.offsetLocation(-w->getLeft(), -w->getTop());
               w->dispatchPointerEvent(event);
               event.offsetLocation(w->getLeft(), w->getTop());
               break;
           }
       }
       return;
   }

   // Pointer (mouse) handling: support hover enter/exit/move, button press/release, scroll.
   Window* target = nullptr;
   for (auto itr = mWindows.rbegin(); itr != mWindows.rend(); itr++) {
       auto w = (*itr);
       if ((w->getVisibility() == View::VISIBLE) && w->getBound().contains(x, y)) {
           target = w;
           break;
       }
   }

   // Hover exit for previously hovered window when pointer moved out
   if (mHoveredWindow && mHoveredWindow != target) {
       MotionEvent* exitEvent = MotionEvent::obtain(event);
       exitEvent->setAction(MotionEvent::ACTION_HOVER_EXIT);
       exitEvent->offsetLocation(-mHoveredWindow->getLeft(), -mHoveredWindow->getTop());
       mHoveredWindow->dispatchPointerEvent(*exitEvent);
       exitEvent->recycle();
       mHoveredWindow = nullptr;
   }

   if (target) {
       // If entering a new window, send hover enter
       if (mHoveredWindow != target) {
           MotionEvent* enterEvent = MotionEvent::obtain(event);
           enterEvent->setAction(MotionEvent::ACTION_HOVER_ENTER);
           enterEvent->offsetLocation(-target->getLeft(), -target->getTop());
           target->dispatchPointerEvent(*enterEvent);
           enterEvent->recycle();
           mHoveredWindow = target;
       }

       // Dispatch the original event to target window (offset to its coords)
       event.offsetLocation(-target->getLeft(), -target->getTop());

       // If this is a button press, bring window to front / activate it
       if ((action == MotionEvent::ACTION_DOWN) || (action == MotionEvent::ACTION_BUTTON_PRESS)) {
           if (mActiveWindow != target) {
               bringToFront(target);
               if (target->hasFlag(View::FOCUSABLE)) {
                   target->dispatchWindowFocusChanged(true);
                   target->onActive();
               }
               mActiveWindow = target;
           }
       }

       target->dispatchPointerEvent(event);
       event.offsetLocation(target->getLeft(), target->getTop());
       return;
   }

   // If no target and we had a hovered window already, clear it (hover exit already sent above).
   return;
}

void WindowManager::onKeyEvent(KeyEvent&event) {
    // Notify the focused child
    if(mActiveWindow){
        mActiveWindow->processKeyEvent(event);
        return ;
    }
    for (auto itr = mWindows.rbegin() ;itr != mWindows.rend();itr++) {
        Window*win = (*itr);
        if ( win->hasFlag(View::FOCUSABLE) && (win->getVisibility()==View::VISIBLE) ) {
            const int keyCode = event.getKeyCode();
            LOGV("Window:%p Key:%s[%x] action=%d",win,KeyEvent::keyCodeToString(keyCode).c_str(),keyCode,event.getAction());
            win->processKeyEvent(event);
            //dispatchKeyEvent(event);
            return;
        }
    }
}

void WindowManager::clip(Window*win){
    Rect rcw = win->getBound();
    for (auto wind = mWindows.rbegin() ;wind != mWindows.rend();wind++){
        if( (*wind)==win )break;
        if( (*wind)->getVisibility()!=View::VISIBLE)continue;
        Rect rc = rcw;
        rc.intersect((*wind)->getBound());
        if(rc.empty())continue;
        rc.offset(-win->getX(),-win->getY());
        win->mInvalidRgn->subtract((const RectangleInt&)rc); 
    }
}

}  // namespace ui
