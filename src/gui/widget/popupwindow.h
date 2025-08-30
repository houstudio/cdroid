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
#ifndef __POPUP_WINDOW_H__
#define __POPUP_WINDOW_H__
#include <widget/framelayout.h>
#include <widget/cdwindow.h>
#include <core/windowmanager.h>
namespace cdroid{
class Transition;
class PopupWindow{
public:
    static constexpr int INPUT_METHOD_FROM_FOCUSABLE = 0;
    static constexpr int INPUT_METHOD_NEEDED = 1;
    static constexpr int INPUT_METHOD_NOT_NEEDED = 2;
    DECLARE_UIEVENT(void,OnDismissListener);
private:
    class PopupDecorView:public Window{
    private:
        PopupWindow*mPop;
    public:
        PopupDecorView(int w,int h);
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
    Context* mContext;
    View* mParentRootView;
    bool mIsShowing;
    bool mIsTransitioningToDismiss;
    bool mIsDropdown;

    /** View that handles event dispatch and content transitions. maby we can use it as Window??*/
    PopupDecorView* mDecorView;

    /** View that holds the background and may animate during a transition. */
    View* mBackgroundView;

    /** The contents of the popup. May be identical to the background view. */
    View* mContentView;

    int mInputMethodMode = INPUT_METHOD_FROM_FOCUSABLE;
    int mSoftInputMode;//= WindowManager.LayoutParams.SOFT_INPUT_STATE_UNCHANGED;
    int mSplitTouchEnabled;
    bool mFocusable;
    bool mTouchable;
    bool mOutsideTouchable;
    bool mClippingEnabled;
    bool mLayoutInScreen;
    bool mClipToScreen;
    bool mAllowScrollingAnchorParent;
    bool mLayoutInsetDecor;
    bool mNotTouchModal;
    bool mAttachedInDecor;
    bool mAttachedInDecorSet;

    View::OnTouchListener mTouchInterceptor;

    int mWidthMode;
    int mWidth;
    int mLastWidth;
    int mHeightMode;
    int mHeight;
    int mLastHeight;

    float mElevation;

    Drawable* mBackground;
    Drawable* mAboveAnchorBackgroundDrawable;
    Drawable* mBelowAnchorBackgroundDrawable;

    Transition* mEnterTransition;
    Transition* mExitTransition;
    Rect mEpicenterBounds;

    int mWindowLayoutType ;//= WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;

    OnDismissListener mOnDismissListener;

    int mAnimationStyle = ANIMATION_STYLE_DEFAULT;
    int mGravity;
    View* mAnchor;
    View* mAnchorRoot;
    View::OnAttachStateChangeListener mOnAnchorDetachedListener;
    View::OnAttachStateChangeListener mOnAnchorRootDetachedListener;
    ViewTreeObserver::OnScrollChangedListener mOnScrollChangedListener;
    View::OnLayoutChangeListener mOnLayoutChangeListener;
    int mAnchorXoff;
    int mAnchorYoff;
    int mAnchoredGravity;

    bool mAboveAnchor;
    bool mIgnoreCheekPress = false;
    bool mOverlapAnchor;
    bool mIsAnchorRootAttached;
    bool mPopupViewInitialLayoutDirectionInherited;
private:
    void init();
    int computeGravity();
    int computeFlags(int curFlags);
    PopupDecorView* createDecorView(View* contentView);
    void invokePopup(WindowManager::LayoutParams* p);
    void setLayoutDirectionFromAnchor();
    const std::string computeAnimationResource();
    void update(View* anchor, bool updateLocation, int xoff, int yoff, int width, int height);
    bool tryFitVertical(WindowManager::LayoutParams* outParams, int yOffset, int height, int anchorHeight,
           int drawingLocationY, int screenLocationY, int displayFrameTop,int displayFrameBottom, bool allowResize);
    bool positionInDisplayVertical(WindowManager::LayoutParams* outParams, int height, int drawingLocationY,
           int screenLocationY, int displayFrameTop, int displayFrameBottom,  bool canResize);
    bool tryFitHorizontal(WindowManager::LayoutParams* outParams, int xOffset, int width,int anchorWidth,
           int drawingLocationX, int screenLocationX, int displayFrameLeft,int displayFrameRight, bool allowResize);
    bool positionInDisplayHorizontal(WindowManager::LayoutParams* outParams, int width,int drawingLocationX,
           int screenLocationX, int displayFrameLeft, int displayFrameRight, bool canResize);
    void dismissImmediate(View* decorView, ViewGroup* contentHolder, View* contentView);
    void alignToAnchor();
protected:
    void setShowing(bool);
    void setDropDown(bool isDropDown);
    void updateAboveAnchor(bool aboveAnchor);
    OnDismissListener getOnDismissListener();
    bool hasContentView()const;
    bool hasDecorView()const;
    void detachFromAnchor();
    WindowManager::LayoutParams* getDecorViewLayoutParams();
    WindowManager::LayoutParams* createPopupLayoutParams(long token);
    void attachToAnchor(View* anchor, int xoff, int yoff, int gravity);
    
    bool isLayoutInScreenEnabled()const;
    void preparePopup(WindowManager::LayoutParams*p);
    PopupBackgroundView*createBackgroundView(View* contentView);
    void update(View* anchor,WindowManager::LayoutParams* params);
    bool findDropDownPosition(View* anchor,WindowManager::LayoutParams* outParams,
            int xOffset, int yOffset, int width, int height, int gravity, bool allowScroll);
     Rect getTransitionEpicenter();
public:
    PopupWindow(Context* context,const AttributeSet& attrs);
    PopupWindow(Context* context,const AttributeSet& attrs, const std::string& defStyleAttr);
    PopupWindow(Context* context,const AttributeSet& attrs, const std::string& defStyleAttr, const std::string& defStyleRes);
    PopupWindow(View* contentView, int width, int height,bool focusable=false);
    PopupWindow(int width, int height);
    virtual ~PopupWindow();
    void setEnterTransition(Transition*enterTransition);
    Transition* getEnterTransition()const;
    void setExitTransition(Transition* exitTransition);
    Transition* getExitTransition()const;
    void setEpicenterBounds(const Rect& bounds);
    Drawable* getBackground();
    void  setBackgroundDrawable(Drawable* background);
    float getElevation()const;
    void setElevation(float elevation);
    View*getContentView();
    void setContentView(View* contentView);
    void setTouchInterceptor(const View::OnTouchListener& l);
    bool isFocusable()const;
    void setFocusable(bool focusable);
    int  getInputMethodMode()const;
    void setInputMethodMode(int mode);
    void setSoftInputMode(int mode);
    int getSoftInputMode()const;
    bool isTouchable()const;
    void setTouchable(bool);
    bool isOutsideTouchable()const;
    void setOutsideTouchable(bool);
    bool isClippingEnabled()const;
    void setClippingEnabled(bool enabled);
    bool isClippedToScreen()const;
    void setIsClippedToScreen(bool enabled);
    bool isSplitTouchEnabled()const;
    void setSplitTouchEnabled(bool enabled);
    bool isLaidOutInScreen()const;
    void setIsLaidOutInScreen(bool enabled);
    void setLayoutInScreenEnabled(bool);
    bool isAttachedInDecor()const;
    void setAttachedInDecor(bool);
    void setLayoutInsetDecor(bool enabled);
    bool isLayoutInsetDecor()const;
    void setWindowLayoutType(int layoutType);
    int  getWindowLayoutType()const;
    bool isTouchModal()const;
    void setTouchModal(bool touchModal);
    bool getOverlapAnchor()const;
    void setOverlapAnchor(bool overlapAnchor);
    int  getWidth()const;
    void setWidth(int );
    int  getHeight()const;
    void setHeight(int);
    bool isShowing()const;
    void showAtLocation(View* parent, int gravity, int x, int y);
    void showAsDropDown(View* anchor);
    void showAsDropDown(View* anchor, int xoff, int yoff);
    void showAsDropDown(View* anchor, int xoff, int yoff,int gravity);
    bool isAboveAnchor()const;

    int getMaxAvailableHeight(View* anchor);
    int getMaxAvailableHeight(View* anchor, int yOffset,bool ignoreBottomDecorations);
    void dismiss();
    void setOnDismissListener(const OnDismissListener& onDismissListener);
    void update();
    void update(int width, int height);
    void update(int x, int y, int width, int height,bool force=false);
    void update(View* anchor, int xoff, int yoff, int width, int height);
};
}
#endif//__POPUP_WINDOW_H__
