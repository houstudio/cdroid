#ifndef __CDROID_WINDOW_H__
#define __CDROID_WINDOW_H__
#include <widget/framelayout.h>
#include <core/handler.h>
#include <view/choreographer.h>
#include <view/actionmode.h>
#include <widget/windowcallback.h>
#include <core/intent.h>

namespace cdroid {
class Bundle; // forward declaration, for Activity-style onCreate(Bundle*)
class ActionBar;
class Toolbar;
class Menu;
class MenuItem;
class MenuInflater;
class ContextMenu;
class ContextMenuInfo;
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
    void drawAccessibilityFocusedDrawableIfNeeded(Canvas& canvas);
    bool getAccessibilityFocusedRect(Rect& bounds);
    Drawable* getAccessibilityFocusedDrawable();
    void handleWindowContentChangedEvent(AccessibilityEvent& event);
    ActionMode* startActionModeInternal(View* originatingView, const ActionMode::Callback& callback, int type);
protected:
    std::vector<View*>mLayoutRequesters;
    Cairo::RefPtr<Cairo::Region>mVisibleRgn;
    /*mPendingRgn init by mInvalidRgn,and also can be modified by windowmanager,if the window above the window 
     *is resized or moved*/
    Cairo::RefPtr<Cairo::Region>mPendingRgn;
    int window_type = TYPE_APPLICATION;/*window type*/
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
    Window(Context*,const AttributeSet&);
    ~Window()override;
    void setRegion(const Cairo::RefPtr<Cairo::Region>&region);
    void draw();
    virtual void setText(const std::string&);
    const std::string getText()const;
    void setPos(int x,int y);
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
    void close();
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
