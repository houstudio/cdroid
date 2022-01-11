#pragma once
#include <widget/viewgroup.h>
#include <widget/framelayout.h>

namespace cdroid{
class PopupWindow{
public:
    static constexpr int INPUT_METHOD_FROM_FOCUSABLE = 0;
    static constexpr int INPUT_METHOD_NEEDED = 1;
    static constexpr int INPUT_METHOD_NOT_NEEDED = 2;
    DECLARE_UIEVENT(void,OnDismissListener);
private:
    class PopupDecorView:public FrameLayout{
    private:
        PopupWindow*mPop;
    public:
        PopupDecorView(Context* context);
        bool dispatchKeyEvent(KeyEvent& event)override;
        bool dispatchTouchEvent(MotionEvent& ev)override;
        bool onTouchEvent(MotionEvent& event)override;
    };
    class PopupBackgroundView:public FrameLayout{
    public:
        PopupBackgroundView(Context* context);
    };
private:
    static constexpr int DEFAULT_ANCHORED_GRAVITY = Gravity::TOP | Gravity::START;
    static constexpr int ANIMATION_STYLE_DEFAULT = -1;
    int mTmpDrawingLocation[2];
    int mTmpScreenLocation[2];
    int mTmpAppLocation[2];
    Context* mContext;
    View* mParentRootView;
    bool mIsShowing;
    bool mIsTransitioningToDismiss;
    bool mIsDropdown;

    /** View that handles event dispatch and content transitions. */
    PopupDecorView* mDecorView;

    /** View that holds the background and may animate during a transition. */
    View* mBackgroundView;

    /** The contents of the popup. May be identical to the background view. */
    View* mContentView;

    bool mFocusable;
    int mInputMethodMode = INPUT_METHOD_FROM_FOCUSABLE;
    int mSoftInputMode ;//= WindowManager.LayoutParams.SOFT_INPUT_STATE_UNCHANGED;
    bool mTouchable = true;
    bool mOutsideTouchable = false;
    bool mClippingEnabled = true;
    int mSplitTouchEnabled = -1;
    bool mLayoutInScreen;
    bool mClipToScreen;
    bool mAllowScrollingAnchorParent = true;
    bool mLayoutInsetDecor = false;
    bool mNotTouchModal;
    bool mAttachedInDecor = true;
    bool mAttachedInDecorSet = false;

    View::OnTouchListener mTouchInterceptor;

    int mWidthMode;
    int mWidth = LayoutParams::WRAP_CONTENT;
    int mLastWidth;
    int mHeightMode;
    int mHeight = LayoutParams::WRAP_CONTENT;
    int mLastHeight;

    float mElevation;

    Drawable* mBackground;
    Drawable* mAboveAnchorBackgroundDrawable;
    Drawable* mBelowAnchorBackgroundDrawable;

    //Transition* mEnterTransition;
    //Transition* mExitTransition;
    Rect mEpicenterBounds;

    bool mAboveAnchor;
    int mWindowLayoutType ;//= WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;

    OnDismissListener mOnDismissListener;
    bool mIgnoreCheekPress = false;

    int mAnimationStyle = ANIMATION_STYLE_DEFAULT;
    int mGravity = Gravity::NO_GRAVITY;
    View* mAnchor;
    View* mAnchorRoot;
    bool mIsAnchorRootAttached;
    int mAnchorXoff;
    int mAnchorYoff;
    int mAnchoredGravity;
    bool mOverlapAnchor;

    bool mPopupViewInitialLayoutDirectionInherited;
private:
    int computeGravity();
    PopupDecorView* createDecorView(View* contentView);
    void invokePopup(LayoutParams* p);
    void setLayoutDirectionFromAnchor();
protected:
    void setShowing(bool);
    void setDropDown(bool isDropDown);
    void updateAboveAnchor(bool aboveAnchor);
    OnDismissListener getOnDismissListener();
    bool hasContentView()const;
    bool hasDecorView()const;
    void detachFromAnchor();
    void attachToAnchor(View* anchor, int xoff, int yoff, int gravity);
    
    bool isLayoutInScreenEnabled()const;
    void preparePopup(LayoutParams*p);
    PopupBackgroundView*createBackgroundView(View* contentView);
    void update(View* anchor,LayoutParams* params);
public:
    PopupWindow(Context* context,const AttributeSet& attrs);
    PopupWindow(View* contentView, int width, int height,bool focusable=false);
    PopupWindow(int width, int height);
    void setEpicenterBounds(const Rect& bounds);
    Drawable* getBackground();
    void  setBackgroundDrawable(Drawable* background);
    float getElevation();
    void setElevation(float elevation);
    View*getContentView();
    void setContentView(View* contentView);
    void setTouchInterceptor(View::OnTouchListener l);
    bool isFocusable();
    void setFocusable(bool focusable);
    bool isOutsideTouchable()const;
    void setOutsideTouchable(bool);
    void setLayoutInScreenEnabled(bool);
    bool isAttachedInDecor()const;
    void setAttachedInDecor(bool);
    void setLayoutInsetDecor(bool enabled);
    bool isLayoutInsetDecor()const;
    void setTouchModal(bool touchModal);
    bool getOverlapAnchor()const;
    void setOverlapAnchor(bool overlapAnchor);
    int  getWidth();
    void setWidth(int );
    int  getHeight();
    void setHeight(int);
    bool isShowing();
    void showAtLocation(View* parent, int gravity, int x, int y);
    void showAsDropDown(View* anchor);
    void showAsDropDown(View* anchor, int xoff, int yoff);
    void showAsDropDown(View* anchor, int xoff, int yoff,int gravity);
    bool isAboveAnchor();

    int getMaxAvailableHeight(View* anchor);
    int getMaxAvailableHeight(View* anchor, int yOffset,bool ignoreBottomDecorations);
    void dismiss();
    void setOnDismissListener(OnDismissListener onDismissListener);
    void update();
    void update(int width, int height);
    void update(int x, int y, int width, int height);
};
}
