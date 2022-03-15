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

#include <windowmanager.h>
#include <ngl_msgq.h>
#include <ngl_os.h>
#include <cdlog.h>
#include <ngl_timer.h>
#include <graphdevice.h>
#include <uieventsource.h>


namespace cdroid {
// Initialize the instance of the singleton to nullptr
WindowManager* WindowManager::instance = nullptr;

WindowManager::WindowManager(){
     GraphDevice::getInstance();
     activeWindow=nullptr;
}

WindowManager&WindowManager::getInstance(){
    if(nullptr==instance){
        instance=new WindowManager();
    }
    return *instance;
};

WindowManager::~WindowManager() {
    for_each(windows.begin(),windows.end(),[](Window*w){delete w;});
    windows.erase(windows.begin(),windows.end());
    LOGD("%p Destroied",this);
}

void WindowManager::addWindow(Window*win){
    windows.push_back(win);
    std::sort(windows.begin(),windows.end(),[](Window*w1,Window*w2){
        return (w2->window_type-w1->window_type)>0;
    });
    
    for(int idx=0,type_idx=0;idx<windows.size();idx++){
        Window*w=windows.at(idx);
        if(w->window_type!=windows[type_idx]->window_type)
            type_idx=idx;
        w->mLayer=w->window_type*10000+(idx-type_idx)*5;
        LOGV("%p window %p[%s] type=%d layer=%d",win,w,w->getText().c_str(),w->window_type,w->mLayer);
    }
    if(activeWindow)activeWindow->post(std::bind(&Window::onDeactive,activeWindow));
    
    View::AttachInfo*info=new View::AttachInfo();
    info->mContentInsets.set(5,5,5,5);
    info->mRootView=win;
    win->dispatchAttachedToWindow(info,win->getVisibility());
    
    Looper::getDefault()->addEventHandler(win->source);
    win->post(std::bind(&Window::onActive,win));
    activeWindow=win;
    LOGV("win=%p source=%p windows.size=%d",win,win->source,windows.size());
}

void WindowManager::removeWindow(Window*w){
    if(w==activeWindow)activeWindow=nullptr;
    if(w->hasFlag(View::FOCUSABLE))
        w->onDeactive();
    auto itw=std::find(windows.begin(),windows.end(),w);
    const Rect wrect=w->getBound();
    windows.erase(itw);
    for(auto itr=windows.begin();itr!=windows.end();itr++){
        Window*w1=(*itr);
        RECT rc=w1->getBound();
        rc.intersect(wrect);
        rc.offset(-w1->getX(),-w1->getY());
        w1->invalidate(&rc);
    }
    Looper::getDefault()->removeEventHandler(w->source);
    View::AttachInfo*info=w->mAttachInfo;
    w->dispatchDetachedFromWindow();
    delete info;
    delete w;
    for(auto it=windows.rbegin();it!=windows.rend();it++){
        if((*it)->hasFlag(View::FOCUSABLE)&&(*it)->getVisibility()==View::VISIBLE){
            (*it)->onActive();
            activeWindow=(*it);
            break;
        } 
    }
    GraphDevice::getInstance().invalidate(wrect);
    GraphDevice::getInstance().flip();
    LOGV("w=%p windows.size=%d",w,windows.size());
}

void WindowManager::moveWindow(Window*w,int x,int y){
    if( (w->isAttachedToWindow()==false)||(w->getVisibility()!=View::VISIBLE))
        return;
    Rect rcw=w->getBound();
    GraphDevice::getInstance().invalidate(rcw);
    rcw.left=x;rcw.top=y;
    GraphDevice::getInstance().invalidate(rcw);
    GraphDevice::getInstance().flip();
}

int WindowManager::enumWindows(WNDENUMPROC cbk){
    int rc=0;
    for(auto win:windows)
       rc+=cbk(win);
    return rc;
}

int WindowManager::getWindows(std::vector<Window*>&wins){
    wins=windows;
    return windows.size();
}

int WindowManager::getVisibleWindows(std::vector<Window*>&wins){
    for(auto w:windows){
        if(w->getVisibility()==View::VISIBLE)
           wins.push_back(w);
    }
    return wins.size();
}

void WindowManager::processEvent(InputEvent&e){
   switch(e.getType()){
   case EV_KEY: onKeyEvent((KeyEvent&)e); break;
   case EV_ABS: onMotion((MotionEvent&)e);break;
   default:break;
   }
}


void WindowManager::onMotion(MotionEvent&event) {
   // Notify the focused child
   const int x=event.getX();
   const int y=event.getY();
   for (auto itr=windows.rbegin();itr!=windows.rend();itr++) {
       auto w=(*itr);
       if ((w->getVisibility()==View::VISIBLE)&&w->getBound().contains(x,y)) {
           event.offsetLocation(-w->getX(),-w->getY());
           w->dispatchTouchEvent(event);
           event.offsetLocation(w->getX(),w->getY());
           break;
       }
   }
}

void WindowManager::onKeyEvent(KeyEvent&event) {
    // Notify the focused child
    for (auto itr=windows.rbegin() ;itr!= windows.rend();itr++) {
        Window*win=(*itr);
        if ( win->hasFlag(View::FOCUSABLE)&&(win->getVisibility()==View::VISIBLE) ) {
            int keyCode=event.getKeyCode();
            LOGV("Window:%p Key:%s[%x] action=%d",win,event.getLabel(keyCode),keyCode,event.getAction());
            win->processKeyEvent(event);
            //dispatchKeyEvent(event);
            return;
        }
  }
}

void WindowManager::clip(Window*win){
    RECT rcw=win->getBound();
    for (auto wind=windows.rbegin() ;wind!= windows.rend();wind++){
        if( (*wind)==win )break;
        if( (*wind)->getVisibility()!=View::VISIBLE)continue;
        Rect rc=rcw;
        rc.intersect((*wind)->getBound());
        if(rc.empty())continue;
        rc.offset(-win->getX(),-win->getY());
        win->mInvalidRgn->subtract((const RectangleInt&)rc); 
    }
}

}  // namespace ui
