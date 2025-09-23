#ifndef __DISMISSABLE_FRAME_LAYOUT_H__
#define __DISMISSABLE_FRAME_LAYOUT_H__
#include <widget/framelayout.h>
#include <widgetEx/wear/swipedismisscontroller.h>

namespace cdroid{
class SwipeDismissController;
class BackButtonDismissController;
class DismissibleFrameLayout:public FrameLayout {
public:
    class Callback:public EventSet{
    public:
        std::function<void(DismissibleFrameLayout&)> onDismissStarted;
        std::function<void(DismissibleFrameLayout&)>onDismissCanceled;
        std::function<void(DismissibleFrameLayout&)>onDismissFinished;
    };
private:
    Context* mContext;
    SwipeDismissController* mSwipeDismissController = nullptr;
    BackButtonDismissController* mBackButtonDismissController = nullptr;
    DismissController::OnDismissListener mDismissListener;
    std::vector<Callback> mCallbacks;
protected:
    void performDismissFinishedCallbacks();
    void performDismissStartedCallbacks();
    void performDismissCanceledCallbacks();
public:
    //DismissibleFrameLayout(Context* context);
    DismissibleFrameLayout(Context* context, const AttributeSet& attrs);
    //DismissibleFrameLayout(Context* context,const AttributeSet& attrs, int defStyle);
    //DismissibleFrameLayout(Context* context,const AttributeSet& attrs,int defStyle,int defStyleRes);
    /** Registers a callback for dismissal. */
    void registerCallback(const Callback& callback);
    void unregisterCallback(const Callback& callback);

    void setSwipeDismissible(bool swipeDismissible);
    bool isDismissableBySwipe() const;

    /**
     * Sets the frame layout to be back button dismissible or not.
     * @param backButtonDismissible bool value to enable/disable the back button dismiss
     */
    void setBackButtonDismissible(bool backButtonDismissible);
    bool isDismissableByBackButton() const;

    SwipeDismissController* getSwipeDismissController() const;


    void requestDisallowInterceptTouchEvent(bool disallowIntercept) override;
    bool onInterceptTouchEvent(MotionEvent& ev) override;

    bool canScrollHorizontally(int direction) override;
    bool onTouchEvent(MotionEvent& ev) override;
    
};
}/*endof namespace*/
#endif/*__DISMISSABLE_FRAME_LAYOUT_H__*/
