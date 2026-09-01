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
#ifndef __CDROID_WINDOWMANAGER_H__
#define __CDROID_WINDOWMANAGER_H__

// cdwindow.h includes this header (Window owns a WindowManager::LayoutParams),
// so Window can only be forward-declared here.
#include <view/viewgroup.h>
#include <core/display.h>
#include <vector>
#include <stdint.h>
#include <queue>
#include <unordered_set>

namespace cdroid {

class Window;

class WindowManager {
private:
    int mDisplayRotation;
    Window*mActiveWindow;
    std::vector< Window* > mWindows;
    Window* mHoveredWindow;
    std::vector< Display > mDisplays;
private:
    friend class GraphDevice;
    WindowManager();
public:
    DECLARE_UIEVENT(bool,WNDENUMPROC,Window*);
    // AOSP WindowManager.LayoutParams: width/height are INHERITED from
    // ViewGroup.LayoutParams (the previous duplicate declaration here shadowed
    // the base members — reads and writes hit different ints). Java field
    // defaults are zero; gravity 0 == NO_GRAVITY.
    class LayoutParams:public ViewGroup::LayoutParams{
    public:
        int type = 0;
        int format = 0;
        int x = 0, y = 0;
        int gravity = 0;          // NO_GRAVITY: WMS centers each axis
        int flags = 0;
        int privateFlags = 0;
        int windowAnimations = 0; // AOSP LayoutParams.windowAnimations (animation STYLE res id)
    };
public:
    virtual ~WindowManager();
    static WindowManager& getInstance();
    void setDisplayRotation(int display,int rotation);
    int  getDisplayRotation(int display=0)const;
    Display&getDefaultDisplay();
    Display*getDisplay(int display);
    void addWindow(Window*w);
    void removeWindow(Window*w);
    void removeWindows(const std::vector<Window*>&);
    void moveWindow(Window*w,int x,int y);
    void moveWindow(Window*w,int x,int y,int width,int height);
    /* Dirty a global rect on the windows below w — surface-animation vacate damage. */
    void exposeRegionBelow(Window*w,const Rect&grc);
    /* AOSP WindowState.applyGravityAndUpdateFrame (the placement half of WMS
     * window layout), driven by the window's WindowManager::LayoutParams:
     * resolve the size (MATCH_PARENT -> display, else the value the host
     * pushed — ViewRootImpl hands WMS the measured wrap-content size), place
     * the frame with Gravity::apply, then fit it to the display with
     * Gravity::applyDisplay. Gravity 0 (NO_GRAVITY, the LayoutParams default)
     * centers each axis — this is what centers dialogs on Android. Hosts call
     * this when the size is known (Dialog::show); addWindow never places a
     * window (AOSP places on relayout, not on add — popups compute their
     * anchor position only after being added). */
    void relayoutWindow(Window*w);
    /* Hide a window: set it INVISIBLE and dirty the screen area it covered on
     * every other window, so composeSurfaces repaints the uncovered region from
     * the windows below. This is the WindowManager-owned equivalent of the old
     * GraphDevice::invalidate(getBound()) called from Window::onVisibilityChanged
     * — hiding + damage propagation is a window-stack concern, not a graph one.
     * Required by dirty-rect backends (xlib); harmless on full-flush ones. */
    void hideWindow(Window*w);
    void sendToBack(Window*w);
    void bringToFront(Window*w);
    void processEvent(InputEvent&e);
private:
    /** PhoneWindowManager.interceptKeyBeforeQueueing: system keys consumed
     *  by policy before any window sees them. True = consumed (drop). */
    bool interceptKeyBeforeQueueing(KeyEvent& event);
public:
    void clip(Window*win);
    int enumWindows(WNDENUMPROC cbk);
    int getWindows(std::vector<Window*>&);
    int getVisibleWindows(std::vector<Window*>&);
    Window*getActiveWindow()const;
    /* The top-most APPLICATION window (the stack may carry system-layer
     * windows above it — IME, toasts, system alerts — which can hold input
     * focus without being the top application window). Null when none. */
    Window*getActiveApplicationWindow();

    /* AccessibilityInteractionController.setAccessibilityFetchFlags
     * (android-36 :973): the ViewRootImpl side writes the client's fetch
     * flags into the fetched root's AttachInfo before nodes materialize —
     * FLAG_REPORT_VIEW_IDS / FLAG_INCLUDE_NOT_IMPORTANT_VIEWS gate per-node
     * reporting in View.onInitializeAccessibilityNodeInfo. In-process note:
     * nodes are created lazily while the client walks the tree, so there is
     * no reset-on-reply point like AOSP's resetAccessibilityFetchFlags();
     * the flags stay set for this single trusted in-process client. */
    void setAccessibilityFetchFlags(Window* window, int flags);
protected:
    virtual void onKeyEvent(KeyEvent&key);
    virtual void onMotion(MotionEvent&event);
    DISALLOW_COPY_AND_ASSIGN(WindowManager);
};

}  // namespace cdroid

#endif  // __CDROID WINDOWMANAGER_H__
