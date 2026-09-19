#ifndef __WINDOWCALLBACK_H__
#define __WINDOWCALLBACK_H__
/*********************************************************************************
 * Port of android.view.Window.Callback (API set of android-12; the android-36
 * set adds nothing CDROID consumers need).
 *
 * androidx's ToolbarActionBar / ToolbarWidgetWrapper hold a `Window.Callback`
 * and dispatch the options panel / home-up through it; AOSP's Dialog and
 * Activity implement the SAME interface and install themselves via
 * mWindow.setCallback(this) — the window then delegates its input dispatch and
 * lifecycle notifications to that owner (DecorView.dispatchKeyEvent:
 * `cb != null ? cb.dispatchKeyEvent(event) : super.dispatchKeyEvent(event)`).
 *
 * CDROID's Window also plays the Activity role, so Window ITSELF derives from
 * WindowCallback (the fallback implementation); a Dialog grafts on top by
 * calling mWindow->setCallback(this) exactly like AOSP.
 *
 * C++ shape note: AOSP is a Java interface (implement-everything) with four
 * default methods. The C++ port gives EVERY method an inline empty/fallback
 * body — the Java "must implement" pressure has no useful C++ equivalent, and
 * implementers overriding only what they care about mirrors how the four
 * AOSP default methods already behave. AOSP bodies that are trivially empty
 * stay empty here; AOSP bodies with real logic (Dialog's dispatch chain, the
 * panel forwarding) live in the implementing class.
 *
 * Deviations from the AOSP surface (each recorded at the method):
 *  - dispatchTrackballEvent/onSearchRequested(SearchEvent)/onPointerCaptureChanged/
 *    onProvideKeyboardShortcuts: kept for shape; CDROID has no trackball,
 *    SearchEvent or pointer-capture plumbing, so they default to false/no-op.
 *  - onWindowDismissed keeps AOSP's two-parameter signature.
 *********************************************************************************/
#include <core/windowmanager.h>   // WindowManager::LayoutParams (nested class — no forward decl possible)
#include <view/actionmode.h>      // ActionMode::Callback (nested class — same reason)

namespace cdroid{
class Menu;
class MenuItem;
class View;
class KeyEvent;
class MotionEvent;
class AccessibilityEvent;

class WindowCallback{
public:
    virtual ~WindowCallback() = default;

    // --- Input-dispatch family (AOSP Window.Callback, in declaration order) ---

    // Called to process key events. At the very least, your implementation
    // must call the window's superDispatchKeyEvent to propagate (AOSP Dialog:
    // onKeyListener -> superDispatchKeyEvent -> event.dispatch(this)).
    virtual bool dispatchKeyEvent(KeyEvent& event) { return false; }
    // Called to process key shortcut events.
    virtual bool dispatchKeyShortcutEvent(KeyEvent& event) { return false; }
    // Called to process touch screen events. At the very least your
    // implementation must call the window's superDispatchTouchEvent.
    virtual bool dispatchTouchEvent(MotionEvent& event) { return false; }
    // Called to process trackball events (no trackball source in CDROID).
    virtual bool dispatchTrackballEvent(MotionEvent& event) { return false; }
    // Called to process generic motion events.
    virtual bool dispatchGenericMotionEvent(MotionEvent& event) { return false; }
    // Called to process population of AccessibilityEvents. AOSP sets the
    // class name here; CDROID's AccessibilityEvent has no className yet.
    virtual bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) { return false; }

    // --- Panel / options-menu family ---

    // Instantiate the view to display in the panel for 'featureId' (null = use the standard menu).
    virtual View* onCreatePanelView(int featureId) { return nullptr; }
    // Initialize the contents of the menu for panel 'featureId' (called once, first show).
    virtual bool onCreatePanelMenu(int featureId, Menu& menu) { return false; }
    // Prepare a panel right before it is shown (called every show).
    virtual bool onPreparePanel(int featureId, View* view, Menu& menu) { return true; }
    // A panel's menu was opened by the user.
    virtual bool onMenuOpened(int featureId, Menu& menu) { return true; }
    // A menu item was selected — this is the home/up entry point (featureId == FEATURE_OPTIONS_PANEL).
    virtual bool onMenuItemSelected(int featureId, MenuItem& item) { return false; }
    // A panel was closed.
    virtual void onPanelClosed(int featureId, Menu& menu) { }

    // --- Window lifecycle family ---

    // This is called whenever the current window attributes change.
    virtual void onWindowAttributesChanged(WindowManager::LayoutParams& attrs) { }
    // This hook is called whenever the content view of the screen changes.
    virtual void onContentChanged() { }
    // This hook is called whenever the window focus changes.
    virtual void onWindowFocusChanged(bool hasFocus) { }
    // Called when the window has been attached to the window manager.
    virtual void onAttachedToWindow() { }
    // Called when the window has been detached from the window manager.
    virtual void onDetachedFromWindow() { }
    // Called when the window dismisses itself (AOSP Window.OnWindowDismissedCallback
    // riding the Callback surface; Dialog maps this to dismiss()).
    virtual void onWindowDismissed(bool finishTask, bool suppressWindowTransition) { }

    // --- Search family ---

    // Called when the user signals the desire to start a search (CDROID has no
    // SearchManager; the SearchEvent overload is not ported).
    virtual bool onSearchRequested() { return false; }

    // --- Action-mode family ---

    // Called when an action mode is about to be started by this window
    // (AOSP Dialog delegates to the ActionBar; CDROID dialogs have none).
    virtual ActionMode* onWindowStartingActionMode(ActionMode::Callback* callback) { return nullptr; }
    // The type variant of the above (AOSP's two-argument overload).
    virtual ActionMode* onWindowStartingActionMode(ActionMode::Callback* callback, int type) { return nullptr; }
    // Called when an action mode has been started.
    virtual void onActionModeStarted(ActionMode& mode) { }
    // Called when an action mode has been finished.
    virtual void onActionModeFinished(ActionMode& mode) { }

    // AOSP default methods not ported (no CDROID counterpart): onProvideKeyboardShortcuts
    // (KeyboardShortcutGroup), onPointerCaptureChanged (pointer capture). onSearchRequested's
    // SearchEvent overload is likewise absent — the no-arg version above stands in.
};

}//namespace
#endif
