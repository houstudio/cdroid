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
#include <functional>
#include <unordered_set>
#include <unordered_map>

namespace cdroid {

class Window;

/* Pre-dispatch observer over every key/motion event the WindowManager routes
 * (device AND injected — AOSP tags injected ones POLICY_FLAG_INJECTED at the
 * InputDispatcher; here the producer marks its own events instead). The
 * manual-operation script recorder (UiAutoTest) hooks this seam. Main thread. */
using InputEventObserver = std::function<void(const InputEvent&)>;

class WindowManager {
private:
    int mDisplayRotation;
    Window*mActiveWindow;
    std::vector< Window* > mWindows;
    Window* mHoveredWindow;
    // True while ~WindowManager sweeps the remaining windows. removeWindow stays
    // functional (nested removals from the sweep's detach cascades must unlist +
    // detach their windows), but the restart-next-window block and the active-
    // window invalidate are suppressed: resurrecting focus onto a half-destroyed
    // activity whose FragmentManager is already gone crashes
    // FragmentStateManager::computeExpectedState on a dead Fragment.
    bool mTearingDown = false;
    std::vector< Display > mDisplays;
    /* adjustResize backups: window -> its pre-IME frame (restored on hide). */
    std::unordered_map<Window*, Rect> mSoftInputBackup;
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
        /* AOSP WindowManager.LayoutParams softInputMode constants (values and
         * semantics match android.view.WindowManager.LayoutParams). */
        static constexpr int SOFT_INPUT_STATE_UNSPECIFIED = 0;
        static constexpr int SOFT_INPUT_STATE_UNCHANGED   = 1;
        static constexpr int SOFT_INPUT_STATE_HIDDEN      = 2;
        static constexpr int SOFT_INPUT_STATE_ALWAYS_HIDDEN = 3;
        static constexpr int SOFT_INPUT_STATE_VISIBLE     = 5;
        static constexpr int SOFT_INPUT_STATE_ALWAYS_VISIBLE = 6;
        static constexpr int SOFT_INPUT_MASK_STATE        = 0x0f;
        static constexpr int SOFT_INPUT_ADJUST_UNSPECIFIED = 0x00;
        static constexpr int SOFT_INPUT_ADJUST_NOTHING     = 0x10;
        static constexpr int SOFT_INPUT_ADJUST_PAN         = 0x20;
        static constexpr int SOFT_INPUT_ADJUST_RESIZE      = 0x30;
        static constexpr int SOFT_INPUT_MASK_ADJUST        = 0xf0;
        /* AOSP WindowManager.LayoutParams flag bits (values match
         * android.view.WindowManager.LayoutParams, android-36:2798-3089).
         * Only FLAG_WATCH_OUTSIDE_TOUCH is read by the dispatcher today —
         * the rest are the canonical values for API parity. */
        static constexpr int FLAG_NOT_FOCUSABLE       = 0x00000008;   // :2798
        static constexpr int FLAG_NOT_TOUCHABLE       = 0x00000010;   // :2819
        static constexpr int FLAG_NOT_TOUCH_MODAL     = 0x00000020;   // :2864
        static constexpr int FLAG_ALT_FOCUSABLE_IM    = 0x00020000;   // :2935
        static constexpr int FLAG_WATCH_OUTSIDE_TOUCH = 0x00040000;   // :3003 — receive ACTION_OUTSIDE for downs outside the window
        static constexpr int FLAG_SPLIT_TOUCH         = 0x00800000;   // :3089
        int type = 0;
        int format = 0;
        int x = 0, y = 0;
        int gravity = 0;          // NO_GRAVITY: WMS centers each axis
        int flags = 0;
        int privateFlags = 0;
        int windowAnimations = 0; // AOSP LayoutParams.windowAnimations (animation STYLE res id)
        int softInputMode = SOFT_INPUT_ADJUST_UNSPECIFIED; // how the window reacts to the IME
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
    void damageRegion(const Rect&grc);
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
    /* AOSP windowSoftInputMode driving (adjustResize half): the IME window
     * reports its visibility here; every visible application window below it
     * whose softInputMode is not ADJUST_NOTHING gets its bottom edge laid out
     * up to the IME's top (ADJUST_RESIZE; UNSPECIFIED defaults to resize — the
     * in-process IME rides the window stack, so the view tree relayouts and a
     * ScrollView keeps the focused field reachable). Hidden restores the
     * backed-up frames. ADJUST_PAN arrives with the insets pass. */
    void onSoftInputShown(Window* ime);
    void onSoftInputHidden(Window* ime);
    void sendToBack(Window*w);
    void bringToFront(Window*w);
    void processEvent(InputEvent&e);
    /* Install/clear the pre-dispatch input observer (recording seam, see
     * InputEventObserver above). Pass nullptr to remove. */
    static void setInputEventObserver(InputEventObserver observer);
private:
    static InputEventObserver sInputEventObserver;
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
    /* AOSP WindowManagerService's focused window: the window key input goes
     * to. mActiveWindow is "last added/touched"; a system-layer window being
     * active (an open IME) does not take the application's input focus —
     * fall through to the active application window, as AOSP keeps the app
     * window focused while the IME is up. */
    Window*getFocusedWindow();

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
