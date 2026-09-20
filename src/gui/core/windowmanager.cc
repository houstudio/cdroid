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

#include <core/app.h>
#include <core/looper.h>
#include <porting/cdlog.h>
#include <porting/cdgraph.h>
#include <core/graphdevice.h>
#include <core/windowmanager.h>
#include <widget/cdwindow.h>
#include <view/gravity.h>
#include <mutex>

namespace cdroid {
// Initialize the instance of the singleton to nullptr

/* GLOBAL rect -> the window's LOCAL pending region (clipped, translated). */
void WindowManager::damageWindow(Window*w,const Rect&grc){
    Rect rc = grc;
    rc.intersect(w->getBound());
    rc.offset(-w->getLeft(),-w->getTop());
    w->mPendingRgn->do_union((const Cairo::RectangleInt&)rc);
}

/* onPause + first-stop: the idempotent AOSP pair. */
void WindowManager::pauseAndStop(Window*w){
    w->onPause();
    if (!w->mStoppedByWm) {
        w->mStoppedByWm = true;
        w->onStop();
    }
}

WindowManager::WindowManager(){
    mActiveWindow = nullptr;
    mDisplayRotation =0;
    mHoveredWindow = nullptr;
    LOGD("WindowManager %p",this);
}

WindowManager&WindowManager::getInstance(){
    static WindowManager* mInstance = nullptr;
    static std::once_flag flag;
    std::call_once(flag, []() {
        mInstance = new WindowManager();
    });
    return *mInstance;
};

WindowManager::~WindowManager() {
    App::getInstance().exit(0);
    // Sweep the remaining windows by dispatching detach through each tree —
    // and that cascade tears down fragments -> dialogs -> Window::close/
    // finishClose -> WindowManager::removeWindow for the OTHER windows. Those
    // nested removals must actually work: a dialog dismissed during an earlier
    // window's cascade has to be unlisted + DETACHED before its AlertDialog
    // frees the list adapter the dialog ListView still points at (emptying
    // mWindows first made removeWindow silently no-op and left the tree
    // attached — AbsListView::onDetachedFromWindow then hit the freed adapter).
    // So: keep mWindows live, iterate a COPY (nested erases can't invalidate
    // us), and set mTearingDown to suppress removeWindow's restart-next-window
    // block — onStart()/onResume()-ing a half-destroyed activity whose
    // FragmentManager is already gone crashed FragmentStateManager::
    // computeExpectedState on a dead Fragment.
    mTearingDown = true;
    // Run the still-queued due posts before sweeping. close()'s deferred
    // deletes (self/AttachInfo/Handler) ride the main-looper queue, and an
    // exit(0) that outruns them strands all three — the looper is torn down
    // with the message still queued (72B Handler + the closed window's 408B
    // AttachInfo were definite-lost whenever exit followed a close() closely).
    // Draining first lets those lambdas do their own deleting in their own
    // order (delete self before info); the sweep below then only owns windows
    // that were never closed. Cascades during the drain hit the mTearingDown
    // guards set above.
    Looper::getMainLooper()->drainMessageQueue();
    std::vector<Window*> windows = mWindows;
    for(Window*w:windows){
        View::AttachInfo*info = w->mAttachInfo;
        const bool listed = std::find(mWindows.begin(),mWindows.end(),w) != mWindows.end();
        if (listed) {
            // Full proper teardown: focus bookkeeping (suppressed parts aside),
            // erase, dispatchDetachedFromWindow. Windows already removed by a
            // cascade were detached there — re-dispatching would re-run
            // onDetachedFromWindow on a tree whose adapter is gone.
            removeWindow(w);
        }
        // close()'s posted deletes were dropped by the quitting looper — free
        // the shell here. info was stashed before the detach (which nulls
        // w->mAttachInfo); for cascade-removed windows it is already null and
        // the stash died with the dropped post (accepted quit-path leak).
        delete info;
        delete w;
    }
    mWindows.clear();
    // The sweep above ends scene-root transitions (ViewGroup::
    // dispatchDetachedFromWindow -> endTransitions -> forceToEnd), and each
    // ended throwaway clone DEFERS its delete-this to a zero-delay post
    // (Transition::end: a synchronous delete-this is unsafe while end()
    // frames unwind). Without a second drain those posts rot in the quitting
    // queue: the 72B Handler, its message closure, and the whole clone with
    // every captured TransitionValues were definite-lost at every exit that
    // tore a window down mid-transition.
    Looper::getMainLooper()->drainMessageQueue();
    // Ghost layers (removed windows' exit snapshots) have no owner window left
    // to sweep them — free them here, with the looper already quitting.
    GraphDevice::getInstance().clearGhosts();
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

void WindowManager::resortLayers(){
    std::sort(mWindows.begin(),mWindows.end(),[](Window*w1,Window*w2){
        return (w2->mLayer - w1->mLayer)>0;
    });
    for(int idx = 0 ;idx < mWindows.size();idx++){
        Window*w = mWindows.at(idx);
        w->mLayer = (w->window_type<<16)|(idx+1);
    }
}

void WindowManager::addWindow(Window*win){
    mWindows.push_back(win);
    win->mLayer = (win->window_type<<16)|0x7FFF;
    resortLayers();
    if(mActiveWindow){
        Window*deactWin = mActiveWindow;
        /* AOSP stop rule: an activity is STOPPED only when no longer visible
         * (occluded — ActivityStack stops an activity whose window is covered;
         * a dialog/popup band over a fullscreen host keeps the host visible).
         * Satellite windows (dialogs, spinner dropdowns, toasts) do not cover
         * the host, so the host keeps running — CRITICAL because the host's
         * FragmentManager also owns DialogFragments: stopping the host on a
         * mere dialog-show fired DialogFragment.onStop -> Dialog::hide() five
         * milliseconds after show (the dialog vanished before the user could
         * pick), and PreferenceFragment.onStop had already cleared the
         * tree-click listeners. Focus still moves to the new window, so the
         * deactivating window always takes onPause; onStop only when covered.
         *
         * The verdict is DEFERRED to post-run time: AOSP judges visibility
         * from laid-out, surfaced frames (WMS visibility processing), never
         * pre-layout placeholders — and addWindow runs inside the Window
         * ctor, where satellite frames are still seeds (Dialog: a hardcoded
         * 640x320; wrap-content popups: the ctor resolves negative dims to
         * the full display). Judging at ctor time wrongly stopped hosts on
         * small panels (<=640x320) and for wrap-content popups on any
         * display. By the time the post runs, Dialog::show's relayoutWindow
         * has installed the real wrap frame (same stack, earlier drain), and
         * a window removed in the same tick is skipped by the membership
         * check (pointer-value compare only — a removed window is never
         * dereferenced). */
        deactWin->post([this, deactWin, win,
                        winAlive = win->mA11yListenerAlive,
                        deactAlive = deactWin->mA11yListenerAlive](){
            // A host closed in the same tick already took its full lifecycle
            // in removeWindow — the token check keeps this post from pausing
            // it a second time (AOSP never double-dispatches pause), and a
            // satellite deleted before the drain (bulk remove, app delete —
            // paths that skip close()'s queue purge) or whose ADDRESS was
            // reused by a new window cannot feed the verdict: token +
            // membership, pointer-value compare only.
            if (!*deactAlive) return;
            const bool winLive = *winAlive
                    && std::find(mWindows.begin(), mWindows.end(), win) != mWindows.end();
            const bool occludes = winLive
                    && win->getVisibility() == View::VISIBLE
                    && win->getBound().contains(deactWin->getBound());
            deactWin->onPause();
            if (occludes && !deactWin->mStoppedByWm) {
                deactWin->mStoppedByWm = true;
                deactWin->onStop();
            }
        });
        mActiveWindow->mAttachInfo->mTreeObserver->dispatchOnWindowFocusChange(false);
    }

    View::AttachInfo*info = new View::AttachInfo(win->getContext());
    info->mContentInsets.setEmpty();
    info->mRootView = win;
    win->dispatchAttachedToWindow(info,win->getVisibility());
    win->post([win](){
        win->onCreate(nullptr);
        win->onStart();
        win->onResume();
    });
    win->post([info](){
        info->mTreeObserver->dispatchOnWindowFocusChange(true);
    });
    mActiveWindow = win;
    LOGV("win=%p AttachInfo=%p windows.size=%d",win,info,mWindows.size());
}

bool WindowManager::removeWindowCore(Window*w,bool invalidateBelow){
    // Membership check first (AOSP WMS removes by token lookup): a window can
    // reach here twice — e.g. close()'s finishClose() removes it, then the
    // posted delete self -> ~Window removes it again — and the early return
    // must also guard the lifecycle above the erase (a double remove must not
    // re-deliver pause/stop or re-detach; erasing end() is additionally UB —
    // memmove past the vector block corrupts the heap, see the DIALOG suite).
    auto itw = std::find(mWindows.begin(),mWindows.end(),w);
    if(itw == mWindows.end()) return false;
    if(w == mActiveWindow){
        mActiveWindow = nullptr;
        if (w->mAttachInfo) {   // guard: a detached-but-still-active edge
            w->mAttachInfo->mTreeObserver->dispatchOnWindowFocusChange(false);
        }
    }
    if(w->hasFlag(View::FOCUSABLE)){
        w->dispatchWindowFocusChanged(false);
        pauseAndStop(w);
    }
    const Rect wrect = w->getBound();
    mWindows.erase(itw);
    for(auto w1:mWindows){
        if(invalidateBelow){
            Rect rc = wrect;
            rc.intersect(w1->getBound());
            rc.offset(-w1->getLeft(),-w1->getTop());
            w1->invalidate((const Rect*)&rc);
        }
        damageWindow(w1,wrect);
    }
    return true;
}

Window*WindowManager::topFocusableWindow(){
    for(auto it=mWindows.rbegin();it!=mWindows.rend();it++){
        if((*it)->hasFlag(View::FOCUSABLE)&&(*it)->getVisibility()==View::VISIBLE)
            return *it;
    }
    return nullptr;
}

void WindowManager::removeWindow(Window*w){
    if(!removeWindowCore(w,false)) return;
    // Detach the view tree (derived window is still alive here, so virtual onDetachedFromWindow
    // dispatches correctly). This nulls w->mAttachInfo, so the AttachInfo cannot be freed by
    // ~Window -- Window::close() stashes it (before calling removeWindow) and hands it to the
    // posted lambda that frees it. removeWindow itself does NOT free the window or AttachInfo;
    // it only drops the window from the compositor list so a replacement shown in the same tick
    // doesn't race a still-listed window.
    w->dispatchDetachedFromWindow();
    if (!mTearingDown) {
        // Restart the next visible window. Suppressed during ~WindowManager's
        // sweep: mActiveWindow may point at a window the sweep already deleted,
        // and focusing a half-destroyed activity re-enters its dead
        // FragmentManager.
        Window*const restartWin = topFocusableWindow();
        if(restartWin){
            if(restartWin!=mActiveWindow){
                     /* POSTED, like the deactivation: addWindow queues the
                      * host's onPause, so a same-tick add+remove would run a
                      * synchronous resume BEFORE the queued pause and leave
                      * the host PAUSED; posting keeps the pair FIFO. The
                      * focus return moves inside the post too — a synchronous
                      * focus-while-paused window is a state no AOSP app ever
                      * sees. fullRestart comes from mStoppedByWm (the
                      * DELIVERED-stop bookkeeping), not a bounds re-derivation
                      * at remove time: the add-side verdict ran at post time,
                      * and geometry/visibility changes in between made
                      * re-derivation produce onStart-without-onStop pairs. A
                      * quitting looper drops the post — strictly safer than
                      * re-entering a half-destroyed activity at exit. */
                     auto alive = restartWin->mA11yListenerAlive;
                     const bool fullRestart = restartWin->mStoppedByWm;
                     restartWin->post([restartWin, alive, fullRestart](){
                         if (!*alive) return;   // deleted before the drain — skip
                         restartWin->dispatchWindowFocusChanged(true);
                         if (fullRestart) {
                             restartWin->mStoppedByWm = false;
                             restartWin->onStart();
                         }
                         restartWin->onResume();
                     });
            }
            mActiveWindow = restartWin;
        }
        // The removed window may have been the last one (single-window apps,
        // Window::recreate before the replacement is added): no window to focus.
        if(mActiveWindow) mActiveWindow->invalidate();
    }
    GraphDevice::getInstance().flip();
    LOGI("w=%p windows.size=%d",w,mWindows.size());
}

void WindowManager::removeWindows(const std::vector<Window*>&ws){
    // Bulk teardown (app delete): the shared removal core with the extra
    // View-level invalidate, then a SYNCHRONOUS restart pass — unlike
    // removeWindow's posted restart, this caller frees the windows before
    // returning, so the stack must be consistent at exit.
    for(auto w:ws){
        View::AttachInfo*info = w->mAttachInfo;
        if(!removeWindowCore(w,true)) continue;
        w->onDestroy();
        w->dispatchDetachedFromWindow();
        delete info;
        delete w;
    }
    Window*const next = topFocusableWindow();
    if(next){
        if(next!=mActiveWindow){
            next->dispatchWindowFocusChanged(true);
            next->onStart();
            next->onResume();
        }
        mActiveWindow = next;
    }
    GraphDevice::getInstance().flip();
}

void WindowManager::moveWindow(Window*w,int x,int y){
    moveWindow(w,x,y,-1,-1);
}

void WindowManager::relayoutWindow(Window*w){
    WindowManager::LayoutParams& attrs = w->getAttributes();
    Point ds;
    getDefaultDisplay().getSize(ds);
    const Rect display = Rect::Make(0, 0, ds.x, ds.y);
    const int pw = display.width, ph = display.height;

    // AOSP WindowState.applyGravityAndUpdateFrame: MATCH_PARENT resolves to the
    // containing frame; any other value is used as-is (the host pushes the
    // measured wrap-content size into the attributes — ViewRootImpl's role).
    const int wsize = (attrs.width  == LayoutParams::MATCH_PARENT) ? pw : attrs.width;
    const int hsize = (attrs.height == LayoutParams::MATCH_PARENT) ? ph : attrs.height;

    // Set the frame, then make sure the window fits in the display frame —
    // the exact Gravity::apply / Gravity::applyDisplay pair WMS runs.
    Rect frame;
    Gravity::apply(attrs.gravity, wsize, hsize, display, attrs.x, attrs.y, frame);
    Gravity::applyDisplay(attrs.gravity, display, frame);
    moveWindow(w, frame.left, frame.top, frame.width, frame.height);
}

void WindowManager::exposeRegionBelow(Window*w,const Rect&grc){
    // Repaint a global rect from the windows BELOW w (they are the ones the moving
    // surface uncovers) — the same damage pattern moveWindow/hideWindow use.
    const auto itw = std::find(mWindows.begin(), mWindows.end(), w);
    for(auto it = mWindows.begin(); it < itw; it++){
        damageWindow((*it),grc);
    }
}

void WindowManager::damageRegion(const Rect&grc){
    // Repaint a global rect from EVERY window — ghost-layer support: the region
    // under a compositor ghost (a removed window's snapshot) must re-blit from
    // the windows below before the ghost's next frame. The compose pass
    // intersects each union with the window's visible region, so over-covering
    // is clipped away.
    for(auto w : mWindows){
        damageWindow(w,grc);
    }
}


void WindowManager::moveWindow(Window*w,int x,int y,int width,int height){
    Rect rcw = w->getBound();
    Rect rcw2 =rcw;
    rcw2.left = x;
    rcw2.top = y;
    rcw2.width = ((width<0||width==INT_MAX)?rcw.width:width);
    rcw2.height= ((height<0||height==INT_MAX)?rcw.height:height);
    w->setFrame(x, y, rcw2.width,rcw2.height);
    const auto itw = std::find(mWindows.begin(),mWindows.end(),w);
    if( w->isAttachedToWindow() && (w->getVisibility()==View::VISIBLE)){
        for(auto it = mWindows.begin();it<itw;it++){
           // Vacated band = old frame minus the new one, translated into the
           // below window's LOCAL space. The old form unioned/subtracted the
           // GLOBAL rects untranslated, which only matched windows at the
           // origin (fullscreen apps) — a dialog below a mover was damaged in
           // the wrong place.
           Cairo::RefPtr<Cairo::Region>band = Cairo::Region::create((Cairo::RectangleInt&)rcw);
           band->subtract((Cairo::RectangleInt&)rcw2);
           band->translate(-(*it)->getLeft(),-(*it)->getTop());
           (*it)->mPendingRgn->do_union(band);
        }
        GraphDevice::getInstance().flip();
    }
}

void WindowManager::onSoftInputShown(Window* ime){
    // AOSP adjustResize: with the IME visible the application window lays out
    // inside the remaining screen area. The IME docks at the screen bottom, so
    // the new bottom edge is the IME's top.
    if(ime==nullptr) return;
    const int imeTop = ime->getTop();
    for(Window* w : mWindows){
        if(w==ime || w->getVisibility()!=View::VISIBLE) continue;
        if(w->getAttributes().type >= Window::TYPE_SYSTEM_WINDOW) continue; // IME/popups: no adjust
        const int adjust = w->getAttributes().softInputMode & LayoutParams::SOFT_INPUT_MASK_ADJUST;
        if(adjust == LayoutParams::SOFT_INPUT_ADJUST_NOTHING) continue;
        if(w->getBottom() <= imeTop) continue; // already clear of the IME
        // Record the backup only on the FIRST shrink: a re-run (the IME resized
        // itself while visible) would otherwise snapshot the already-shrunk frame
        // and the later restore would un-shrink to the previous IME top instead
        // of the full window.
        if(mSoftInputBackup.find(w)==mSoftInputBackup.end())
            mSoftInputBackup[w] = w->getBound();
        // View::layout takes (x, y, width, height): shrink the height to end at
        // the IME's top edge.
        w->layout(w->getLeft(), w->getTop(), w->getWidth(), imeTop - w->getTop());
        LOGV("softinput resize win=%p %d->%d",w,w->getHeight(),imeTop-w->getTop());
    }
}

void WindowManager::onSoftInputHidden(Window* ime){
    // Restore the backed-up frames; the windows relayout to full size and the
    // exposeRegionBelow damage from hideWindow repaints the uncovered band.
    for(auto& kv : mSoftInputBackup){
        Window* w = kv.first;
        const Rect& r = kv.second;
        w->layout(r.left, r.top, r.width, r.height);
    }
    mSoftInputBackup.clear();
}

void WindowManager::hideWindow(Window*w){
    if(w==nullptr) return;
    const Rect wrect = w->getBound();
    // w has just been hidden (INVISIBLE/GONE) by its caller's setVisibility,
    // which is what triggered the onVisibilityChanged that calls us. Propagate
    // the now-exposed screen area as dirty (in each window's local coords) to
    // every other window so composeSurfaces repaints it from the windows below.
    // We do NOT set visibility here (the caller already did) — same damage-
    // propagation pattern as removeWindow/moveWindow.
    for(auto itr=mWindows.begin(); itr!=mWindows.end(); itr++){
        if((*itr)==w) continue;
        damageWindow((*itr),wrect);
    }
    GraphDevice::getInstance().flip();
}

Window*WindowManager::getActiveWindow()const{
    return mActiveWindow;
}

void WindowManager::setAccessibilityFetchFlags(Window* window, int flags) {
    // AccessibilityInteractionController.java:973 — the ViewRootImpl side
    // writes the client's fetch flags into the fetched root's AttachInfo
    // (WindowManager creates and owns that AttachInfo, the ViewRootImpl role).
    if (window != nullptr && window->mAttachInfo != nullptr) {
        window->mAttachInfo->mAccessibilityFetchFlags = flags;
    }
}

Window*WindowManager::getActiveApplicationWindow(){
    // mWindows is bottom-up: the first application window from the top.
    for(auto it = mWindows.rbegin(); it != mWindows.rend(); it++){
        if((*it)->getAttributes().type < Window::TYPE_SYSTEM_WINDOW)
            return *it;
    }
    return nullptr;
}

void WindowManager::sendToBack(Window*win){
    win->mLayer = (win->window_type<<16);/*make win's layer to lowerest*/
    resortLayers();
    mActiveWindow = mWindows.back();
    mActiveWindow->mPendingRgn->do_union({0,0,win->getWidth(),win->getHeight()});
    win->post([win](){ pauseAndStop(win); });
    Window*newActWin = mActiveWindow;
    newActWin->mStoppedByWm = false;
    mActiveWindow->post([newActWin](){
        newActWin->onStart();
        newActWin->onResume();
    });
    GraphDevice::getInstance().flip();
}

void WindowManager::bringToFront(Window*win){
    if(mActiveWindow==win) return;
    win->mLayer = (win->window_type<<16)|0x7FFF;
    resortLayers();
    win->mStoppedByWm = false;
    win->post([win](){win->onStart();win->onResume();});

    Window*deactWin= mActiveWindow;
    mActiveWindow->post([deactWin](){ pauseAndStop(deactWin); });
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
   /* Observer sees EVERYTHING the user physically did, before system-key
    * policy — a recorder must capture keys the policy eats (BACK) too. */
   if (sInputEventObserver) sInputEventObserver(e);
   if(e.getType()==InputEvent::INPUT_EVENT_TYPE_KEY
           && interceptKeyBeforeQueueing((KeyEvent&)e)){
       return;   // consumed by system policy — never routed to a window
   }
   switch(e.getType()){
   case InputEvent::INPUT_EVENT_TYPE_KEY: onKeyEvent((KeyEvent&)e); break;
   case InputEvent::INPUT_EVENT_TYPE_MOTION: onMotion((MotionEvent&)e);break;
   default:break;
   }
}

InputEventObserver WindowManager::sInputEventObserver = nullptr;

void WindowManager::setInputEventObserver(InputEventObserver observer) {
    sInputEventObserver = std::move(observer);
}


void WindowManager::onMotion(MotionEvent&event) {
   // Notify the focused child
   const int x = event.getX();
   const int y = event.getY();
   const int action = event.getActionMasked();
   // If this is a touchscreen/stylus/touchpad event, keep existing behavior.
   if (event.isFromSource(InputDevice::SOURCE_CLASS_POINTER)){
       Window* hitTarget = nullptr;
       std::vector<Window*> outsideWatchers;   // FLAG_WATCH_OUTSIDE_TOUCH windows above the target
       for (auto itr = mWindows.rbegin(); itr != mWindows.rend(); itr++) {
           auto w = (*itr);
           /* Enter touch mode on ACTION_DOWN. Per Android, ACTION_UP does NOT
              exit touch mode — touch mode is only left by non-touch (key/dpad)
              input or requestFocusFromTouch(), never on finger lift. */
           if ((action == MotionEvent::ACTION_DOWN)&&!w->mAttachInfo->mInTouchMode) {
               ViewTreeObserver*obv = w->getViewTreeObserver();
               w->mAttachInfo->mInTouchMode = true;
               obv->dispatchOnTouchModeChanged(true);
           }
           LOGV_IF(action != MotionEvent::ACTION_MOVE, "%s at(%d,%d)", MotionEvent::actionToString(action).c_str(), x, y);
           /* AOSP InputDispatcher.findTouchedWindowAtLocked (InputDispatcher.cpp:1026-1047):
              a NOT_TOUCHABLE window is skipped entirely, and a TOUCH-MODAL window
              (focusable && !FLAG_NOT_TOUCH_MODAL) claims the gesture EVEN OUTSIDE its
              frame — the traversal returns there and windows below receive nothing
              (a dialog's outside-tap is consumed by the dialog itself: the real
              out-of-frame gesture reaches Dialog.onTouchEvent, whose shouldCloseOnTouch
              UP-out-of-bounds clause dismisses without click-through). Only a
              non-modal window with the point outside its frame falls through, and
              such a window that watches outside touch is then collected for an
              ACTION_OUTSIDE notification (dispatched as FLAG_DISPATCH_AS_OUTSIDE —
              the resolvedAction rewrite, :2966-2968) while the real gesture continues
              to the windows behind. The visibility gate matters: a dismissed-but-
              not-yet-removed dialog window (teardown is posted) must not be notified.
              DOWN only — no UP/POINTER_DOWN notification (:2022 addOutsideTargets=
              isDown). */
           const int wflags = w->getAttributes().flags;
           const bool touchable = !(wflags & WindowManager::LayoutParams::FLAG_NOT_TOUCHABLE);
           const bool touchModal = !(wflags & WindowManager::LayoutParams::FLAG_NOT_FOCUSABLE)
                                && !(wflags & WindowManager::LayoutParams::FLAG_NOT_TOUCH_MODAL);
           if ((w->getVisibility() == View::VISIBLE) && touchable
                        && (touchModal || w->getBound().contains(x, y))) {
               hitTarget = w;
               event.offsetLocation(-w->getLeft(), -w->getTop());
               w->dispatchPointerEvent(event);
               event.offsetLocation(w->getLeft(), w->getTop());
               break;
           }
           if ((action == MotionEvent::ACTION_DOWN)
                   && (w->getVisibility() == View::VISIBLE) && touchable && !touchModal
                   && (wflags & WindowManager::LayoutParams::FLAG_WATCH_OUTSIDE_TOUCH)) {
               outsideWatchers.push_back(w);
           }
       }
       /* Deliver ACTION_OUTSIDE only when a target exists: AOSP drops the whole
          dispatch on a target-less DOWN ("no touched foreground window",
          InputDispatcher.cpp:2249-2253, :2320 — zero watchers notified).
          Deviation from AOSP's watcher-first order (tempTouchState lists outside
          targets before the target): the real event is dispatched above, THEN the
          watchers — an OUTSIDE handler may cancel/dismiss its window, mutating
          mWindows mid-iteration, so the copy is iterated after the fact. */
       if (hitTarget != nullptr) {
           for (Window* w : outsideWatchers) {
               MotionEvent*outside = MotionEvent::obtain(event);   // owned copy (hover-synthesis pattern)
               outside->setAction(MotionEvent::ACTION_OUTSIDE);
               outside->offsetLocation(-w->getLeft(), -w->getTop());
               w->dispatchPointerEvent(*outside);
               outside->recycle();
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
                   target->onStart();
                   target->onResume();
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

/*static-like policy: runs in the non-virtual processEvent path, BEFORE
  onKeyEvent/any Window dispatch — apps overriding Window::processKeyEvent
  (their right) cannot defeat a system key. */
bool WindowManager::interceptKeyBeforeQueueing(KeyEvent& event) {
    // POWER is consumed before the app sees it — PhoneWindowManager's
    // interceptKeyBeforeQueueing position. The hal injects it as the host
    // window system's close gesture (xlib WM_DELETE_WINDOW, see graph_xlib.c):
    // closing the emulator window is the desktop's power-off. The exit request
    // stays in pure Looper vocabulary — no App reach-up from this policy path:
    // quitSafely keeps already-due messages (window teardown deletes) and
    // drops future ones; exec()'s loopOnce() returns false once they drain,
    // and ~App owns the aftermath (QueuedWork flush + subsystem teardown,
    // the "plain return from exec()" path).
    if (event.getKeyCode() == KeyEvent::KEYCODE_POWER
            && event.getAction() == KeyEvent::ACTION_UP) {
        Looper::getMainLooper()->quitSafely();
        return true;
    }
    return false;
}

Window* WindowManager::getFocusedWindow() {
    // AOSP WindowManagerService's focused window (see header): a system-layer
    // active window (an open IME keyboard) does not take the application's
    // input focus — keys keep going to the active application window.
    if (mActiveWindow != nullptr && mActiveWindow->getVisibility() == View::VISIBLE
            && mActiveWindow->getAttributes().type >= Window::TYPE_SYSTEM_WINDOW) {
        Window* appWindow = getActiveApplicationWindow();
        if (appWindow != nullptr && appWindow->getVisibility() == View::VISIBLE) {
            return appWindow;
        }
    }
    return mActiveWindow;
}

void WindowManager::onKeyEvent(KeyEvent&event) {
    // Notify the focused child. Skip an active window that is not visible (e.g.
    // a dismissed IME window hidden via setVisibility(INVISIBLE)) so it does not
    // keep consuming key events; fall through to the next visible focusable window.
    Window* keyTarget = getFocusedWindow();
    if(keyTarget && keyTarget->getVisibility()==View::VISIBLE){
        keyTarget->processKeyEvent(event);
        return ;
    }
    for (auto itr = mWindows.rbegin() ;itr != mWindows.rend();itr++) {
        Window*win = (*itr);
        if ( win->hasFlag(View::FOCUSABLE) && (win->getVisibility()==View::VISIBLE) ) {
            const int keyCode = event.getKeyCode();
            LOGV("Window:%p Key:%s[%x] action=%d",win,KeyEvent::keyCodeToString(keyCode).c_str(),keyCode,event.getAction());
            if(win->mAttachInfo->mInTouchMode){
                ViewTreeObserver*obv = win->getViewTreeObserver();
                win->mAttachInfo->mInTouchMode = false;
                obv->dispatchOnTouchModeChanged(false);
            }
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
        win->mInvalidRgn->subtract((const Cairo::RectangleInt&)rc); 
    }
}

}  // namespace ui
