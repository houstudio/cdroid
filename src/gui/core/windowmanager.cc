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
#include <graph_device.h>
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
    if(activeWindow)activeWindow->source->sendMessage(activeWindow,View::WM_ACTIVE,0,0);
    win->source->sendMessage(win,View::WM_ACTIVE,1,0);
    activeWindow=win;
    Looper::getDefault()->add_event_source(win->source,[](EventSource&e)->bool{
        return ((UIEventSource&)e).processEvents();
    });
    resetVisibleRegion();
    LOGV("win=%p source=%p windows.size=%d",win,win->source,windows.size());
}

void WindowManager::removeWindow(Window*w){
    if(w==activeWindow)activeWindow=nullptr;
    if(w->hasFlag(View::FOCUSABLE))
        w->onDeactive();
    auto itw=std::find(windows.begin(),windows.end(),w);
    const RECT wrect=w->getBound();
    windows.erase(itw);
    resetVisibleRegion();
    for(auto itr=windows.begin();itr!=windows.end();itr++){//!=windows.end()){
        Window*w1=(*itr);
        RECT rc=w1->getBound();
        rc.intersect(wrect);
	rc.offset(-w1->getX(),-w1->getY());
        w1->invalidate(&rc);
    }
    delete w;
    for(auto it=windows.rbegin();it!=windows.rend();it++){
        if((*it)->hasFlag(View::FOCUSABLE)&&(*it)->getVisibility()==View::VISIBLE){
            (*it)->onActive();
            activeWindow=(*it);
            break;
        } 
    }
    GraphDevice::getInstance().flip();
    LOGV("w=%p windows.size=%d",w,windows.size());
}

void WindowManager::moveWindow(Window*w,int x,int y){
    auto itw=std::find(windows.begin(),windows.end(),w);
    for(auto itr=windows.begin();itr!=itw;itr++){
        RECT rcw=w->getBound();
        Window*w1=(*itr);
	Canvas*c=w1->getCanvas();
	RECT r=w1->getBound();
	if(w1->getVisibility()!=View::VISIBLE)continue;
	r.intersect(rcw);
	r.offset(-w1->getX(),-w1->getY());
	c->mInvalidRgn->do_union((const RectangleInt&)r);
	r=w1->getBound();
	rcw.x=x;rcw.y=y;
	r.intersect(rcw);
	r.offset(-w1->getX(),-w1->getY());
	c->mInvalidRgn->subtract((const RectangleInt&)r);
    }
}

void WindowManager::broadcast(DWORD msgid,DWORD wParam,ULONG lParam){
    for(auto win:windows)
       win->sendMessage(msgid,wParam,lParam);
}

int WindowManager::enumWindows(WNDENUMPROC cbk){
    int rc=0;
    for(auto win:windows)
       rc+=cbk(win);
    return rc;
}

void WindowManager::processEvent(InputEvent&e){
   switch(e.getType()){
   case EV_KEY: onKeyEvent((KeyEvent&)e); break;
   case EV_ABS: onMotion((MotionEvent&)e);break;
   default:break;
   }
}

void WindowManager::onBtnPress(MotionEvent&event) {
  // Notify the focused child
  for (auto& wind : windows) {
    if (wind->isFocused() == true) {
       //wind->onMousePress(event);
    }
  }
}

void WindowManager::onBtnRelease(MotionEvent&event) {
  // Notify the focused child
  for (auto& wind : windows) {
    if (wind->isFocused() == true) {
       //wind->onMouseRelease(event);
    }
  }
}

void WindowManager::onMotion(MotionEvent&event) {
   // Notify the focused child
   int x=event.getX();
   int y=event.getY();
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
            win->dispatchKeyEvent(event);return;
            switch(event.getAction()){
            case KeyEvent::ACTION_UP  :
                win->onKeyUp(keyCode,event);break;
            case KeyEvent::ACTION_DOWN:
                win->onKeyDown(keyCode,event);break;
            default:break;
            }
            return;
        }
  }
}

void WindowManager::clip(Window*win){
    RECT rcw=win->getBound();
    for (auto wind=windows.rbegin() ;wind!= windows.rend();wind++){
        if( (*wind)==win )break;
        if( (*wind)->getVisibility()!=View::VISIBLE)continue;
        RECT rc=rcw;
        rc.intersect((*wind)->getBound());
        if(rc.empty())continue;
        rc.offset(-win->getX(),-win->getY());
        win->mInvalidRgn->subtract((const RectangleInt&)rc); 
    }
}

void WindowManager::resetVisibleRegion(){
    for (auto w=windows.begin() ;w!= windows.end();w++){
        RECT rcw=(*w)->getBound();
        RefPtr<Region>newrgn=Region::create((RectangleInt&)rcw);
        if((*w)->getVisibility()!=View::VISIBLE)continue;

        for(auto w1=w+1;w1!=windows.end();w1++){
            RECT r=(*w1)->getBound();
            if((*w1)->getVisibility()==View::VISIBLE)
               newrgn->subtract((const RectangleInt&)r);
        }
        newrgn->translate(-rcw.x,-rcw.y);
        (*w)->getCanvas()->set_layer((*w)->mLayer,newrgn);
        LOGV("window %p[%s] Layer=%d %d rects visible=%d",(*w),(*w)->getText().c_str(),(*w)->mLayer,
                   newrgn->get_num_rectangles(),(*w)->getVisibility());
    }
}

}  // namespace ui
