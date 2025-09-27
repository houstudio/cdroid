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
*/
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
    virtual void performDismissFinishedCallbacks();
    virtual void performDismissStartedCallbacks();
    virtual void performDismissCanceledCallbacks();
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
