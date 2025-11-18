#ifndef __CDROID_WINDOW_H__
#define __CDROID_WINDOW_H__
#include <cdtypes.h>
#include <widget/framelayout.h>
#include <core/handler.h>
#include <core/uieventsource.h>

#define USE_UIEVENTHANDLER 0

namespace cdroid {
class Window : public FrameLayout {
protected:
    friend class WindowManager;
    friend class GraphDevice;
    friend class UIEventSource;
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
    class UIEventHandler;
    friend UIEventSource;
    class SendWindowContentChangedAccessibilityEvent;
    friend SendWindowContentChangedAccessibilityEvent;
    bool mInLayout;
    bool mHandingLayoutInLayoutRequest;
    Rect mRectOfFocusedView;
    AccessibilityManager*mAccessibilityManager;
    SendWindowContentChangedAccessibilityEvent* mSendWindowContentChangedAccessibilityEvent;
    std::vector<LayoutTransition*> mPendingTransitions;
private:
    void doLayout();
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
protected:
    std::vector<View*>mLayoutRequesters;
    Cairo::RefPtr<Cairo::Region>mVisibleRgn;
    /*mPendingRgn init by mInvalidRgn,and also can be modified by windowmanager,if the window above the window 
     *is resized or moved*/
    Cairo::RefPtr<Cairo::Region>mPendingRgn;
    int window_type;/*window type*/
    int mLayer;/*surface layer*/
    std::string mText;
    InvalidateOnAnimationRunnable mInvalidateOnAnimationRunnable;
#if USE_UIEVENTHANDLER	
    UIEventHandler* mUIEventHandler;
#else
    UIEventSource *mUIEventHandler;
#endif
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
    virtual void onCreate();
    virtual void onActive();
    virtual void onDeactive();
    bool dispatchKeyEvent(KeyEvent&event)override;
    bool isInLayout()const override;
    void dispatchInvalidateOnAnimation(View* view)override;
    void dispatchInvalidateRectOnAnimation(View*,const Rect&)override;
    void dispatchInvalidateDelayed(View*, long delayMilliseconds)override;
    void dispatchInvalidateRectDelayed(const AttachInfo::InvalidateInfo*,long delayMilliseconds)override;
    bool dispatchTouchEvent(MotionEvent& event)override;
    void cancelInvalidate(View* view)override;
    void requestTransitionStart(LayoutTransition* transition)override;
    void close();
};
typedef Window Activity;
class Window::UIEventHandler:public Handler{
private:
    View*mAttachedView;
    std::function<void()> mLayoutRunner;
public:
    UIEventHandler(View*,std::function<void()>run);
    void handleIdle()override;
};

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
