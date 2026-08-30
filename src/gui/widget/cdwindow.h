#ifndef __CDROID_WINDOW_H__
#define __CDROID_WINDOW_H__
#include <widget/framelayout.h>
#include <core/handler.h>
#include <core/windowmanager.h>
#include <view/choreographer.h>
#include <view/actionmode.h>
#include <widget/windowcallback.h>
#include <core/intent.h>
#include <widget/activitytransition.h>
#include <functional>

namespace cdroid {
class Bundle; // forward declaration, for Activity-style onCreate(Bundle*)
class ActionBar;
class Toolbar;
class Menu;
class MenuItem;
class MenuInflater;
class ContextMenu;
class ContextMenuInfo;
class Animator;  // forward — drives Window-level Activity transitions (ObjectAnimator/ValueAnimator)
class Window : public FrameLayout, public WindowCallback {
protected:
    friend class WindowManager;
    friend class View;  // View::invalidateInternal/requestLayout → Window::scheduleTraversals
    friend class GraphDevice;
    class InvalidateOnAnimationRunnable:public Runnable{
    private:
        bool mPosted;
        Window*mOwner;
        std::vector<AttachInfo::InvalidateInfo*>mInvalidateViews;
        void postIfNeededLocked();
        std::vector<AttachInfo::InvalidateInfo*>::iterator find(View*v);
    public:
        InvalidateOnAnimationRunnable();
        ~InvalidateOnAnimationRunnable();
        void setOwner(Window*w);
        void addView(View* view);
        void addViewRect(View*view,const Rect&);
        void removeView(View* view);
        void run();
    };
private:
    class SendWindowContentChangedAccessibilityEvent;
    // Liveness guard for the accessibility-state listener registered on the
    // (static, longer-lived) AccessibilityManager: the manager fires listeners
    // at exit-time service unregistration, after windows may be gone.
    std::shared_ptr<bool> mA11yListenerAlive;
    friend SendWindowContentChangedAccessibilityEvent;
    bool mInLayout;
    bool mHandingLayoutInLayoutRequest;
    Rect mRectOfFocusedView;
    AccessibilityManager*mAccessibilityManager;
    ActionMode* mActionMode = nullptr;
    ActionBar*  mActionBar  = nullptr; // owned; created by setActionBar(Toolbar*)
    MenuInflater* mMenuInflater = nullptr; // owned; lazy, from getMenuInflater()
    Intent mIntent; // the Intent this Window was started with (Activity.getIntent); set by startActivity
    bool mNoHistory = false; // FLAG_ACTIVITY_NO_HISTORY: auto-close when another Window is shown
    int mResultCode = 0;     // Activity result (set by setResult, delivered on close)
    Intent* mResultData = nullptr; // borrowed (set by setResult, not deleted by ~Window)
    SendWindowContentChangedAccessibilityEvent* mSendWindowContentChangedAccessibilityEvent;
    std::vector<LayoutTransition*> mPendingTransitions;
    // Window-level Activity transitions (owned). Mirror android.app.Activity transition API names,
    // but implemented Window-level (setAlpha/setPos), not android.transition content-level — CDROID's
    // Window is the composition root, so only moving the Window / setting surface opacity is visible.
    ActivityTransition* mEnterTransition   = nullptr; // shown on startActivity open
    ActivityTransition* mExitTransition    = nullptr; // shown when another Window opens over this (MVP: not driven)
    ActivityTransition* mReturnTransition  = nullptr; // shown on close/back (null => use mExitTransition)
    ActivityTransition* mReenterTransition = nullptr; // shown returning to this Window (null => use mEnterTransition)
    Animator* mCurrentTransitionAnimator   = nullptr; // owned (cancel+delete on replace/~Window)
    // Compose-time visual translation — CDROID's SurfaceControl::setPosition. The SLIDE activity
    // transition animates ONLY this offset (composeSurfaces adds it to the blit); the real frame
    // (getBound/mLeft/mTop) stays at the resting position, so a11y bounds, input hit-testing and
    // WMS placement are stable mid-animation — AOSP semantics (window animations are surface-side
    // transforms; WindowState's frame never moves).
    int  mSurfaceDx = 0;
    int  mSurfaceDy = 0;
    bool mPendingEnterAnim  = false; // run mEnterTransition after the first doTraversal (content drawn)
    bool mInTransition      = false; // close()/re-enter re-entrancy guard
    bool mDestroyed         = false; // set in ~Window so the animator end-callback skips finishClose
    bool mClosePending      = false; // close() idempotence: a second close must not post a second delete
    // AOSP LayoutParams.windowAnimations source: an explicit animation STYLE overriding the
    // theme's windowAnimationStyle (setWindowAnimations). 0 -> resolve from the theme.
    int mWindowAnimationStyle = 0;
    bool mWindowExitAnimationsEnabled = true; // setWindowAnimations(enableExit=false) skips the exit pair
    // True when the Context ctor auto-wrapped the caller's plain context in a
    // ContextThemeWrapper (AOSP: an Activity IS a themed context); freed in ~Window.
    bool mOwnsContext       = false;
    // Activity name stamped by REGISTER_ACTIVITY's factory; empty for anonymous
    // windows. Window::recreate() relaunches through the ActivityFactory by it.
    std::string mActivityName;
    // AOSP ActivityInfo.configChanges bits (android:configChanges).
    int mConfigChanges = 0;
private:
    void doLayout();
    // Schedule a traversal (layout + draw + flip + compose) via Choreographer CALLBACK_TRAVERSAL.
    // Moves draw from the doEventHandlers phase (UIEventSource poll) into drainMessageQueue (the
    // Choreographer posts MSG_DO_FRAME as a Handler message), so draw and effect-end posts share the
    // same FIFO queue — clone playTransition runs before any posted view delete. Re-entrancy guard
    // via mTraversalScheduled (multiple invalidate/requestLayout coalesce into one traversal).
    void scheduleTraversals();
    void doTraversal();
    bool performFocusNavigation(KeyEvent& event);
    static View*inflate(Context*ctx,std::istream&stream);
    static ViewGroup*findAncestorToTakeFocusInTouchMode(View* focused);
    void initWindow();
    bool ensureTouchModeLocally(bool);
    bool enterTouchMode();
    bool leaveTouchMode();
    void playSoundImpl(int);
    View* getCommonPredecessor(View* first, View* second);
    void postSendWindowContentChangedCallback(View*source,int changeType);
    void removeSendWindowContentChangedCallback();
    friend class AlertController;  // flushes pending posts before freeing swapped panels
    void drawAccessibilityFocusedDrawableIfNeeded(Canvas& canvas);
    bool getAccessibilityFocusedRect(Rect& bounds);
    // CDROID-specific: detach-then-delete teardown (AlertController's panel
    // swap) flushes the pending content-changed post BEFORE freeing the tree —
    // the runnable holds a raw source pointer into it (AOSP: GC keeps it).
    Drawable* getAccessibilityFocusedDrawable();
    void handleWindowContentChangedEvent(AccessibilityEvent& event);
    ActionMode* startActionModeInternal(View* originatingView, const ActionMode::Callback& callback, int type);
    // Activity-transition driver. enter=true plays the open animation; enter=false plays the close
    // one. onEnd (may be empty) runs when the animation completes (or immediately if NONE).
    void runActivityTransition(ActivityTransition* t, bool enter, const std::function<void()>& onEnd);
    void startEnterAnimation();
    void startExitAnimation(const std::function<void()>& onEnd);
    void snapEnterStart(ActivityTransition* t); // pre-snap to the start state so the first frame isn't a fully-shown flash
    static void computeSlidePos(int edge, int ox, int oy, int w, int h, bool offscreen, int& x, int& y);
    void finishClose(); // close()'s tail: post (onDestroy + delete) + removeWindow
protected:
    // The teardown callback handed to close(cb): invoked at finishClose time
    // (after the exit transition, view tree still intact). Stored as a member
    // so the owner can CANCEL it (see PopupDecorView::detachOwner) when it
    // dies before the animation ends - a pending notification must never fire
    // into a freed owner chain.
    std::function<void()> mTeardownCb;
private:
    // Build the default enter/exit ActivityTransitions from mWindowAnimationStyle or (when 0)
    // the theme's windowAnimationStyle (AOSP PhoneWindow.generateLayout records the style;
    // AppTransition resolves the actual animations from it — simplified to an enter/exit pair).
    // Called from the Context-taking ctors, so a later programmatic setEnter/ExitTransition
    // simply replaces these, like AOSP's overridePendingTransition over the theme.
    void loadThemeWindowAnimations();
    // Shared body of loadThemeWindowAnimations/setWindowAnimations: resolve enter/exit anims
    // out of `styleRes` and install them (capturing the resting pos / snapping pre-first-frame).
    void applyWindowAnimationStyle(int styleRes);
protected:
    std::vector<View*>mLayoutRequesters;
    Cairo::RefPtr<Cairo::Region>mVisibleRgn;
    /*mPendingRgn init by mInvalidRgn,and also can be modified by windowmanager,if the window above the window
     *is resized or moved*/
    Cairo::RefPtr<Cairo::Region>mPendingRgn;
    int window_type = TYPE_APPLICATION;/*window type*/
    // AOSP Window.mWindowAttributes: the WindowManager::LayoutParams this window
    // is placed by (WindowManager::relayoutWindow — the WMS applyGravityAndUpdateFrame
    // equivalent). Kept in sync with window_type in initWindow().
    WindowManager::LayoutParams mWindowAttributes;
    int mLayer;/*surface layer*/
    std::string mText;
    InvalidateOnAnimationRunnable mInvalidateOnAnimationRunnable;
    bool mTraversalScheduled = false;  // scheduleTraversals re-entrancy guard
    void onFinishInflate()override;
    void onSizeChanged(int w,int h,int oldw,int oldh)override;
    void onVisibilityChanged(View& changedView,int visibility)override;
    ViewGroup*invalidateChildInParent(int* location,Rect& dirty)override;
    int processInputEvent(InputEvent&event);
    int processKeyEvent(KeyEvent&event);
    int processPointerEvent(MotionEvent&event);
    Cairo::RefPtr<Canvas>getCanvas();
    void setAccessibilityFocus(View* view, AccessibilityNodeInfo* node);
public:
    // The host of the accessibility-focused (virtual) view — the in-process
    // AccessibilityService's FOCUS_ACCESSIBILITY query (AOSP goes through the
    // interaction connection; here the window is the connection).
    View* getAccessibilityFocusedHost() const { return mAccessibilityFocusedHost; }

    // Terminal focus-search resolver (public like AOSP's ViewRootImpl.focusSearch —
    // a ViewParent interface method). The Window IS the root view in CDROID
    // (mParent == nullptr), so the ViewGroup parent-chain ends here — this
    // override stands in for AOSP's ViewRootImpl.focusSearch, which is where
    // the chain terminates on Android.
    View* focusSearch(View* focused, int direction)override;

    using Callback = WindowCallback;
    typedef enum{
        TYPE_WALLPAPER    = 1,
        TYPE_APPLICATION  = 2,
        TYPE_SYSTEM_WINDOW= 2000,
        TYPE_STATUS_BAR   = 2001,
        TYPE_SEARCH_BAR   = 2002,
        TYPE_SYSTEM_ALERT = 2003,
        TYPE_KEYGUARD     = 2004,
        TYPE_TOAST        = 2005,
    }WindowType;
    Window(int x,int y,int w,int h,int type=TYPE_APPLICATION);
    // AOSP PhoneWindow(context): themed (ContextThemeWrapper) dialog contexts
    // drive inflation through this overload instead of the global App.
    // themeWindowAnimations=false opts out of the theme windowAnimationStyle load —
    // AOSP's windowAnimationStyle belongs to app/activity windows only; popup decor
    // windows (PopupDecorView) animate via their own popup window animation style,
    // never the theme (and CDROID popups align to their anchor after creation, so a
    // ctor-time snap would use a stale resting position — see loadThemeWindowAnimations).
    Window(Context*ctx,int x,int y,int w,int h,int type=TYPE_APPLICATION,
           bool themeWindowAnimations = true);
    Window(Context*,const AttributeSet*);
    ~Window()override;
    void setRegion(const Cairo::RefPtr<Cairo::Region>&region);
    void draw();
    virtual void setText(const std::string&);
    const std::string getText()const;
    void setPos(int x,int y);
    /* Visual-only surface translation (see mSurfaceDx). Real moves go through setPos. */
    void setSurfaceTranslation(int dx,int dy);
    // AOSP Window.getAttributes/setAttributes. getAttributes returns the LIVE
    // object — mutate fields on it and call WindowManager::relayoutWindow,
    // exactly how AOSP dialogs tune their window before showing.
    WindowManager::LayoutParams& getAttributes();
    const WindowManager::LayoutParams& getAttributes()const;
    void setAttributes(const WindowManager::LayoutParams& a);
    bool ensureTouchMode(bool inTouchMode)override;
    View& setAlpha(float a);
    void sendToBack();
    void bringToFront();
    void notifySubtreeAccessibilityStateChanged(View* child, View* source, int changeType)override;
    bool requestSendAccessibilityEvent(View* child, AccessibilityEvent& event)override;
    virtual bool onKeyUp(int keyCode,KeyEvent& evt) override;
    virtual bool onKeyDown(int keyCode,KeyEvent& evt) override;
    virtual void onBackPressed();
    // Activity-aligned lifecycle callbacks. Window plays the role of an Activity
    // (typedef Window Activity), so it exposes the standard Activity lifecycle.
    virtual void onCreate(Bundle* savedInstanceState);
    // android.app.Activity.onNewIntent: called when a singleTop/singleTask Window is reused (the
    // system delivers a new Intent to an existing instance instead of creating a new one).
    // Default: no-op (setIntent has already been called by startActivity; override to react).
    virtual void onNewIntent(const Intent& /*intent*/) {}
    // android.app.Activity result API: startActivityForResult → target setResult → close →
    // caller.onActivityResult. App mediates the result delivery (see App::dispatchPendingResult).
    void startActivityForResult(const Intent& intent, int requestCode);
    void setResult(int resultCode, Intent* data = nullptr) { mResultCode = resultCode; mResultData = data; }
    int getResultCode() const { return mResultCode; }
    Intent* getResultData() const { return mResultData; }
    virtual void onActivityResult(int /*requestCode*/, int /*resultCode*/, Intent* /*data*/) {}
    virtual void onStart();
    virtual void onResume();
    virtual void onPause();
    virtual void onStop();
    virtual void onDestroy();
    // Legacy hooks, kept for backward compatibility. The default onCreate(Bundle*)/
    // onResume()/onPause() forward to these so existing Window subclasses that
    // override them keep firing unchanged. New code should override the Activity-
    // named callbacks above instead.
    [[deprecated("Use onCreate(Bundle*) instead")]]
    virtual void onCreate();
    [[deprecated("Use onResume() instead")]]
    virtual void onActive();
    [[deprecated("Use onPause() instead")]]
    virtual void onDeactive();

    // Panel feature id used as an options-menu dispatch sentinel. CDROID does not
    // model the full requestWindowFeature()/FEATURE_ACTION_BAR theme machinery
    // (no DecorView); only this id is referenced internally.
    static constexpr int FEATURE_OPTIONS_PANEL = 0;
    // androidx AppCompatDelegate panel id used by ToolbarActionBar's menu callbacks
    // (onMenuOpened/onPanelClosed). CDROID has only the one options panel, so this aliases it.
    static constexpr int FEATURE_SUPPORT_ACTION_BAR = 0;

    // Adopts toolbar as this Activity's ActionBar (mirrors framework
    // Activity.setActionBar / androidx AppCompatActivity.setSupportActionBar — CDROID's
    // Activity plays the AppCompatActivity role). The created ToolbarActionBar is owned
    // by this Window and freed in ~Window(). Pass nullptr to clear.
    void setActionBar(Toolbar* toolbar);
    ActionBar* getActionBar();

    // AOSP Activity.setTheme(@StyleRes int): ContextThemeWrapper.setTheme applies
    // the style to the LIVE theme object, so views inflated afterwards — and lazy
    // ?attr resolution — pick it up. Already-inflated views are NOT re-themed in
    // place (AOSP behavior too); call recreate() to rebuild under the new theme.
    void setTheme(int resid);
    // AOSP Activity.recreate(): cause this Activity to be relaunched with a new
    // instance (which inflates under the theme selected before recreation).
    // CDROID: closes this window and instantiates a fresh one through the
    // ActivityFactory (REGISTER_ACTIVITY name). Needs the name — only registered
    // activities can relaunch themselves.
    void recreate();
    // Stamp used by REGISTER_ACTIVITY's factory so recreate() can relaunch by name.
    void setActivityName(const std::string& name) { mActivityName = name; }

    // AOSP ComponentCallbacks.onConfigurationChanged: override to receive
    // configuration changes the activity declared it handles (setConfigChanges).
    virtual void onConfigurationChanged(Configuration& newConfig);
    // AOSP Activity.dispatchConfigurationChanged: delivered by the system
    // (App::handleConfigurationChanged) — runs the callback and dispatches
    // through the content view tree (AOSP ViewRootImpl does the tree walk).
    void dispatchConfigurationChanged(Configuration& newConfig);
    // AOSP ActivityInfo.configChanges (manifest android:configChanges): the
    // CONFIG_* bits this activity handles itself — an activity keeps alive only
    // when EVERY changed bit is declared; otherwise the system recreates it.
    int  getConfigChanges() const { return mConfigChanges; }
    void setConfigChanges(int configChanges) { mConfigChanges = configChanges; }

    // Options-menu dispatch chain. Override in subclasses to populate / handle items.
    virtual bool onCreateOptionsMenu(Menu& menu);
    virtual bool onPrepareOptionsMenu(Menu& menu);
    // Non-home options items. (Home/up is folded to onNavigateUp() upstream, in
    // onMenuItemSelected — mirrors AOSP Activity.onMenuItemSelected for FEATURE_OPTIONS_PANEL.)
    virtual bool onOptionsItemSelected(MenuItem& item);
    virtual bool onContextItemSelected(MenuItem& item);
    virtual bool onNavigateUp();
    virtual void invalidateOptionsMenu();
    virtual MenuInflater* getMenuInflater();
    // The Intent that started this Window (android.app.Activity.getIntent/getIntent). ActivityNavigator
    // stamps EXTRA_NAV_SOURCE/CURRENT on it; a real Context.startActivity would setIntent on the new Window.
    Intent getIntent() const { return mIntent; }
    void setIntent(const Intent& intent) { mIntent = intent; }
    // android FLAG_ACTIVITY_NO_HISTORY: if set, this Window auto-closes when another Window is shown.
    bool isNoHistory() const { return mNoHistory; }
    void setNoHistory(bool b) { mNoHistory = b; }
    // Programmatic options-menu open/close — delegate to the ActionBar (ToolbarActionBar).
    virtual void openOptionsMenu();
    virtual void closeOptionsMenu();

    // --- WindowCallback (android.view.Window.Callback, panel/options subset) ---
    // These wrap the options-menu methods above with the featureId dimension that androidx
    // ToolbarActionBar dispatches through. CDROID has a single options panel, so only
    // FEATURE_OPTIONS_PANEL is honored; other feature ids are no-ops.
    View* onCreatePanelView(int featureId) override;
    bool onCreatePanelMenu(int featureId, Menu& menu) override;
    bool onPreparePanel(int featureId, View* view, Menu& menu) override;
    bool onMenuOpened(int featureId, Menu& menu) override;
    bool onMenuItemSelected(int featureId, MenuItem& item) override;
    void onPanelClosed(int featureId, Menu& menu) override;

    // Context menu (android.app.Activity context menu dispatch). The long-press ->
    // showContextMenu -> showContextMenuForChild chain terminates here; Window builds and
    // shows the menu (MenuDialogHelper) and routes item selection to onContextItemSelected.
    bool showContextMenuForChild(View* originalView) override;
    bool showContextMenuForChild(View* originalView, float x, float y) override;
    void registerForContextMenu(View* view);
    void unregisterForContextMenu(View* view);
    void openContextMenu(View* view);
    void closeContextMenu();
    virtual void onCreateContextMenu(ContextMenu& menu, View& v, ContextMenuInfo* menuInfo);

    bool dispatchKeyEvent(KeyEvent&event)override;
    bool isInLayout()const override;
    void dispatchInvalidateOnAnimation(View* view)override;
    void dispatchInvalidateRectOnAnimation(View*,const Rect&)override;
    void dispatchInvalidateDelayed(View*, long delayMilliseconds)override;
    void dispatchInvalidateRectDelayed(const AttachInfo::InvalidateInfo*,long delayMilliseconds)override;
    bool dispatchTouchEvent(MotionEvent& event)override;
    ActionMode* startActionModeForChild(View* originalView, const ActionMode::Callback& callback, int type)override;
    void cancelInvalidate(View* view)override;
    void requestTransitionStart(LayoutTransition* transition)override;
    // AOSP Window.setWindowAnimations: an explicit animation STYLE res id overriding the theme's
    // windowAnimationStyle for this window's enter/exit (0 restores the theme resolution).
    // enableExit=false installs the ENTER animation only. AOSP always installs the full pair
    // (it survives animated popup exits on GC); popups also pass true now - the no-GC
    // discipline for the deferred teardown (borrowed-content owners must detach their
    // adapter before freeing it) is documented on PopupWindow::dismiss. The parameter stays
    // for substrate callers that need the legacy synchronous-teardown behavior.
    void setWindowAnimations(int resId, bool enableExit = true);
    // Window-level Activity transitions (android.app.Activity transition API names). Each setter
    // takes ownership of the passed ActivityTransition* (replacing/deleting any previous one).
    void setEnterTransition(ActivityTransition* t);
    void setExitTransition(ActivityTransition* t);
    void setReturnTransition(ActivityTransition* t);
    void setReenterTransition(ActivityTransition* t);
    ActivityTransition* getEnterTransition()   const { return mEnterTransition; }
    ActivityTransition* getExitTransition()    const { return mExitTransition; }
    ActivityTransition* getReturnTransition()  const { return mReturnTransition; }
    ActivityTransition* getReenterTransition() const { return mReenterTransition; }
    void close();
    // Substrate extension (not an AOSP mirror): close() with a teardown
    // callback, invoked at finishClose time - after the exit transition (when
    // one plays) but while the view tree is still intact, i.e. the caller's
    // last safe point before the window teardown cascade. PopupWindow's exit
    // branch uses it to return borrowed content at the AOSP-specified moment.
    void close(const std::function<void()>& onTeardown);
};
using Activity=Window;

class Window::SendWindowContentChangedAccessibilityEvent{
private:
    int mChangeTypes = 0;
    View* mSource;
    Runnable mRunnable;
    Window*mWin;
public:
    int64_t mLastEventTimeMillis;
    SendWindowContentChangedAccessibilityEvent(Window*w);
    void run();
    void runOrPost(View* source, int changeType);
    void removeCallbacks();
    void removeCallbacksAndRun();
};
}  // namespace cdroid

#endif  // UI_LIBUI_WINDOW_H_
