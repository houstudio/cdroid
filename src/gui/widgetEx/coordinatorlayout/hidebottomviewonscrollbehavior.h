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
#ifndef __HIDE_BOTTOMVIEW_ONSCROLL_BEHAVIOR_H__
#define __HIDE_BOTTOMVIEW_ONSCROLL_BEHAVIOR_H__
#include <widgetEx/coordinatorlayout/coordinatorlayout.h>
namespace cdroid{
class HideBottomViewOnScrollBehavior:public CoordinatorLayout::Behavior{
public:
    static constexpr int STATE_SCROLLED_DOWN = 1;
    static constexpr int STATE_SCROLLED_UP = 2;
    typedef CallbackBase<void,View&,int> OnScrollStateChangedListener;//onStateChanged(View& var1, int var2);
private:
    std::vector<OnScrollStateChangedListener> mOnScrollStateChangedListeners;
    static constexpr int DEFAULT_ENTER_ANIMATION_DURATION_MS = 225;
    static constexpr int DEFAULT_EXIT_ANIMATION_DURATION_MS = 175;
    int mEnterAnimDuration;
    int mExitAnimDuration;
    TimeInterpolator* mEnterAnimInterpolator;
    TimeInterpolator* mExitAnimInterpolator;
    int mHeight = 0;
    int mCurrentState = 2;
    int mAdditionalHiddenOffsetY = 0;
    ViewPropertyAnimator* mCurrentAnimator;
private:
    void updateCurrentState(View* child, int state);
    void animateChildTo(View* child, int targetY, long duration, TimeInterpolator* interpolator);
public:
    HideBottomViewOnScrollBehavior();
    HideBottomViewOnScrollBehavior(Context* context,const AttributeSet& attrs);
    ~HideBottomViewOnScrollBehavior()override;
    bool onLayoutChild(CoordinatorLayout& parent, View& child, int layoutDirection)override;

    void setAdditionalHiddenOffsetY(View& child, int offset);

    bool onStartNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& directTargetChild, View& target, int nestedScrollAxes, int type)override;
    void onNestedScroll(CoordinatorLayout& coordinatorLayout, View& child, View& target, int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed, int type, int* consumed)override;

    bool isScrolledUp() const;
    void slideUp(View* child);
    void slideUp(View* child, bool animate);

    bool isScrolledDown() const;
    void slideDown(View* child);
    void slideDown(View* child, bool animate);

    void addOnScrollStateChangedListener(const OnScrollStateChangedListener& listener);
    void removeOnScrollStateChangedListener(const OnScrollStateChangedListener& listener);
    void clearOnScrollStateChangedListeners();
};
}/*endof namespace*/
#endif/*__HIDE_BOTTOMVIEW_ONSCROLL_BEHAVIOR_H__*/
