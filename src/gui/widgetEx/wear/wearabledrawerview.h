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
#ifndef __WEARABLE_DRAWER_VIEW_H__
#define __WEARABLE_DRAWER_VIEW_H__
#include <widget/framelayout.h>
#include <widget/viewdraghelper.h>
namespace cdroid{

class ImageView;
class WearableDrawerController;

/** View that contains drawer content and a peeking view for use with WearableDrawerLayout.
 *  The content and peek views can be set either by the setter methods or by the
 *  app:drawerContent / app:peekView XML attributes.
 *  androidx.wear.widget.drawer.WearableDrawerView.java (lines 91-511). */
class WearableDrawerView: public FrameLayout {
public:
    /** Indicates that the drawer is in an idle, settled state. No animation is in progress. */
    static constexpr int STATE_IDLE = ViewDragHelper::STATE_IDLE;

    /** Indicates that the drawer is currently being dragged by the user. */
    static constexpr int STATE_DRAGGING = ViewDragHelper::STATE_DRAGGING;

    /** Indicates that the drawer is in the process of settling to a final position. */
    static constexpr int STATE_SETTLING = ViewDragHelper::STATE_SETTLING;

private:
    ViewGroup* mPeekContainer = nullptr;
    ImageView* mPeekIcon = nullptr;
    View* mContent = nullptr;
    WearableDrawerController* mController = nullptr;
    /** Vertical offset of the drawer. Ranges from 0 (closed) to 1 (opened) */
    float mOpenedPercent = 0;
    /** True if the drawer's position cannot be modified by the user. This includes edge dragging,
        view dragging, and scroll based auto-peeking. */
    bool mIsLocked = false;
    bool mCanAutoPeek = true;
    bool mLockWhenClosed = false;
    bool mOpenOnlyAtTop = false;
    bool mPeekOnScrollDown = false;
    bool mIsPeeking = false;
    int mDrawerState = STATE_IDLE;
    int mPeekResId = 0;
    int mContentResId = 0;

    static Drawable* getDrawable(Context& context, const TypedArray& typedArray, size_t index);

    void parseAttributes(Context& context, const AttributeSet* attrs, int defStyleAttr);

    void setPeekContent(View* content, int index, ViewGroup::LayoutParams* params);

    /** @return {@code true} if this is a new and valid {@code content}. */
    bool setDrawerContentWithoutAdding(View* content);

protected:
    void onFinishInflate() override;

    void onAttachedToWindow() override;

public:
    WearableDrawerView(Context* context);
    WearableDrawerView(Context* context, const AttributeSet* attrs);
    WearableDrawerView(Context* context, const AttributeSet* attrs, int defStyleAttr);
    WearableDrawerView(Context* context, const AttributeSet* attrs, int defStyleAttr, int defStyleRes);

    /** Called when anything within the peek container is clicked. However, if a custom peek view
        is supplied and it handles the click, then this may not be called. The default behavior
        is to open the drawer. */
    virtual void onPeekContainerClicked(View& v);

    void addView(View* child, int index, ViewGroup::LayoutParams* params) override;
    // C++ name hiding: re-expose the remaining ViewGroup::addView overloads as one
    // flat overload set (Java's); they dispatch into the override above.
    using ViewGroup::addView;

    // package-private upstream; overridden by the drawer subclasses and called by
    // WearableDrawerLayout (C++ has no package visibility)
    virtual int preferGravity() const;
    ViewGroup* getPeekContainer();
    void setDrawerController(WearableDrawerController* controller);

    /** Returns the drawer content view. */
    View* getDrawerContent();

    /** Set the drawer content view.
        @param content The view to show when the drawer is open, or {@code null} if it should
        not open. */
    void setDrawerContent(View* content);

    /** Set the peek content view.
        @param content The view to show when the drawer peeks. */
    void setPeekContent(View* content);

    /** Called when the drawer has settled in a completely open state. The drawer is interactive
        at this point. This is analogous to
        WearableDrawerLayout.DrawerStateCallback#onDrawerOpened. */
    virtual void onDrawerOpened();

    /** Called when the drawer has settled in a completely closed state. This is analogous to
        WearableDrawerLayout.DrawerStateCallback#onDrawerClosed. */
    virtual void onDrawerClosed();

    /** Called when the drawer state changes. This is analogous to
        WearableDrawerLayout.DrawerStateCallback#onDrawerStateChanged.
        @param state one of STATE_DRAGGING, STATE_SETTLING, or STATE_IDLE */
    virtual void onDrawerStateChanged(int state);

    /** Only allow the user to open this drawer when at the top of the scrolling content. If
        there is no scrolling content, then this has no effect. Defaults to false. */
    void setOpenOnlyAtTopEnabled(bool openOnlyAtTop);

    /** Returns whether this drawer may only be opened by the user when at the top of the
        scrolling content. If there is no scrolling content, then this has no effect.
        Defaults to false. */
    bool isOpenOnlyAtTopEnabled() const;

    /** Sets whether or not this drawer should peek while scrolling down. This is currently only
        supported for bottom drawers. Defaults to false. */
    void setPeekOnScrollDownEnabled(bool peekOnScrollDown);

    /** Gets whether or not this drawer should peek while scrolling down. This is currently only
        supported for bottom drawers. Defaults to false. */
    bool isPeekOnScrollDownEnabled() const;

    /** Sets whether this drawer should be locked when the user cannot see it. @see isLocked */
    void setLockedWhenClosed(bool locked);

    /** Returns true if this drawer should be locked when the user cannot see it.
        @see isLocked */
    bool isLockedWhenClosed() const;

    /** Returns the current drawer state, which will be one of STATE_DRAGGING, STATE_SETTLING,
        or STATE_IDLE */
    int getDrawerState() const;

    /** Sets the drawer state. */
    void setDrawerState(int drawerState);

    /** Returns whether the drawer is either peeking or the peek view is animating open. */
    bool isPeeking() const;

    /** Returns true if this drawer has auto-peeking enabled. This will always return false
        for a locked drawer. */
    bool isAutoPeekEnabled() const;

    /** Sets whether or not the drawer can automatically adjust its peek state. Note that locked
        drawers will never auto-peek, but their isAutoPeekEnabled state will be maintained
        through a lock/unlock cycle. */
    void setIsAutoPeekEnabled(bool canAutoPeek);

    /** Returns true if the position of the drawer cannot be modified by user interaction.
        Specifically, a drawer cannot be opened, closed, or automatically peeked by
        WearableDrawerLayout. However, it can be explicitly opened, closed, and peeked by the
        developer. A drawer may be considered locked if the drawer is locked open, locked
        closed, or is closed and #isLockedWhenClosed returns true. */
    bool isLocked() const;

    /** Sets whether or not the position of the drawer can be modified by user interaction.
        @see isLocked */
    void setIsLocked(bool locked);

    /** Returns true if the drawer is fully open. */
    bool isOpened() const;

    /** Returns true if the drawer is fully closed. */
    bool isClosed() const;

    /** Returns the WearableDrawerController associated with this WearableDrawerView. This will
        only be valid after this View has been added to its parent. */
    WearableDrawerController* getController();

    /** Sets whether the drawer is either peeking or the peek view is animating open. */
    void setIsPeeking(bool isPeeking);

    /** Returns the percent the drawer is open, from 0 (fully closed) to 1 (fully open). */
    float getOpenedPercent() const;

    /** Sets the percent the drawer is open, from 0 (fully closed) to 1 (fully open). */
    void setOpenedPercent(float openedPercent);
};

}/*endof namespace*/
#endif/*__WEARABLE_DRAWER_VIEW_H__*/
