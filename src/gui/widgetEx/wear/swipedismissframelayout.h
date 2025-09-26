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
#ifndef __SWIPE_DISMISS_FRAMELAYOUT_H__
#define __SWIPE_DISMISS_FRAMELAYOUT_H__
#include <widgetEx/wear/dismissableframelayout.h>
namespace cdroid{
class SwipeDismissFrameLayout:public DismissibleFrameLayout {
public:
    static constexpr float DEFAULT_DISMISS_DRAG_WIDTH_RATIO = 0.33f;

    class Callback:public EventSet{
    public:
        std::function<void(SwipeDismissFrameLayout&)> onSwipeStarted;
        std::function<void(SwipeDismissFrameLayout&)> onSwipeCanceled;
        std::function<void(SwipeDismissFrameLayout&)> onDismissed;
    };
private:
    std::vector<Callback> mCallbacksCompat;
protected:
    void performDismissFinishedCallbacks() override;
    void performDismissStartedCallbacks() override;
    void performDismissCanceledCallbacks() override;
public:
    SwipeDismissFrameLayout(Context* context,const AttributeSet& attrs);

    void addCallback(const Callback& callback);
    void removeCallback(const Callback& callback);
    
    void setSwipeable(bool swipeable);
    bool isSwipeable() const;

    void setDismissMinDragWidthRatio(float ratio);
    float getDismissMinDragWidthRatio()const;

};
}/*endof namespace*/
#endif/*__SWIPE_DISMISS_FRAMELAYOUT_H__*/
