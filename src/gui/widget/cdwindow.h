#ifndef __CDROID_WINDOW_H__
#define __CDROID_WINDOW_H__
#include <cdtypes.h>
#include <view/viewgroup.h>
#include <widget/framelayout.h>
#include <core/handler.h>
#include <core/uieventsource.h>

#define USE_UIEVENTHANDLER 0

namespace cdroid {
class Window : public ViewGroup {
protected:
    friend class WindowManager;
    friend class GraphDevice;
    friend class UIEventSource;
     struct InvalidateInfo{
         View* target;
         Rect rect;
     };
    class InvalidateOnAnimationRunnable:public Runnable{
    private:
        bool mPosted;
        Window*mOwner;
        std::vector<InvalidateInfo>mInvalidateViews;
        void postIfNeededLocked();
        std::vector<InvalidateInfo>::iterator find(View*v);
    public:
        InvalidateOnAnimationRunnable();
        void setOwner(Window*w);
        void addView(View* view);
        void addViewRect(View*view,const Rect&);
        void removeView(View* view);
        void run();
    };
private:
    class UIEventHandler:public Handler{
    private:
        View*mAttachedView;
        std::function<void()> mLayoutRunner;
    public:
        UIEventHandler(View*,std::function<void()>run);
        void handleIdle()override;
    };
    bool mInLayout;
    bool mHandingLayoutInLayoutRequest;
    Rect mRectOfFocusedView;
    void doLayout();
    bool performFocusNavigation(KeyEvent& event);
    static View*inflate(Context*ctx,std::istream&stream);
protected:
    std::vector<View*>mLayoutRequesters;
    Cairo::RefPtr<Cairo::Region>mVisibleRgn;
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
    ViewGroup::LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    ViewGroup::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* lp)const override;
public:
    typedef enum{
        TYPE_WALLPAPER    =1,
        TYPE_APPLICATION  =2,
        TYPE_SYSTEM_WINDOW=2000,
        TYPE_STATUS_BAR   =2001,
        TYPE_SEARCH_BAR   =2002,
        TYPE_SYSTEM_ALERT =2003,
        TYPE_KEYGUARD     =2004,
        TYPE_TOAST        =2005,
    }WindowType;
    Window(int x,int y,int w,int h,int type=TYPE_APPLICATION);
    Window(Context*,const AttributeSet&);
    ~Window()override;
    void setRegion(const Cairo::RefPtr<Cairo::Region>&region);
    void draw();
    virtual void setText(const std::string&);
    const std::string getText()const;
    virtual View& setPos(int x,int y)override;
    View& setAlpha(float a);
    virtual bool onKeyUp(int keyCode,KeyEvent& evt) override;
    virtual bool onKeyDown(int keyCode,KeyEvent& evt) override;
    virtual void onBackPressed();
    virtual void onActive();
    virtual void onDeactive();
    bool dispatchKeyEvent(KeyEvent&event)override;
    bool isInLayout()const override;
    bool postDelayed(Runnable& what,uint32_t delay)override;
    bool removeCallbacks(const Runnable& what)override;
    void dispatchInvalidateOnAnimation(View* view)override;
    void dispatchInvalidateRectOnAnimation(View*,const Rect&)override;
    bool dispatchTouchEvent(MotionEvent& event)override;
    void cancelInvalidate(View* view)override;
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet&)const override;
    void close();
protected:
};

}  // namespace cdroid

#endif  // UI_LIBUI_WINDOW_H_
