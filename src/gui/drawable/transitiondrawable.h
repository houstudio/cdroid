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
#ifndef __TRANSMITION_DRAWABLE_H__
#define __TRANSMITION_DRAWABLE_H__
#include <drawable/layerdrawable.h>

namespace cdroid{

class TransitionDrawable:public LayerDrawable{
private:
    static constexpr int TRANSITION_STARTING = 0;
    static constexpr int TRANSITION_RUNNING = 1;
    static constexpr int TRANSITION_NONE = 2;
private:
    int  mTransitionState;
    bool mReverse = false;
    bool mCrossFade = false;
    int64_t mStartTimeMillis;
    int  mFrom = 0;
    int  mTo = 0;
    int  mDuration = 0;
    int  mOriginalDuration = 0;
    int  mAlpha = 0;        // no-arg ctor path leaves these uninit otherwise

    class TransitionState:public LayerDrawable::LayerState{
    public:
        TransitionState(TransitionState* orig, TransitionDrawable* owner, Resources* res);
        TransitionDrawable*newDrawable()override;
        Drawable*newDrawable(Resources* res)override;
        // Mirrors AOSP TransitionDrawable.TransitionState: reports only this state's own
        // changing configurations (drops the children aggregate folded in by LayerState).
        int getChangingConfigurations()const override;
    };

    TransitionDrawable(std::shared_ptr<TransitionState> state, Resources* res);
    std::shared_ptr<LayerDrawable::LayerState> createConstantState(LayerState* state,Resources* res)override;
public:
    TransitionDrawable();
    TransitionDrawable(const std::vector<Drawable*>drawables);
    void startTransition(int durationMillis);
    // @hide AOSP API (TransitionDrawable.java): snap to the second layer with
    // no transition — framework/SystemUI callers migrating from Android need it.
    void showSecondLayer();
    void resetTransition();
    void reverseTransition(int duration);
    bool isCrossFadeEnabled()const;
    void setCrossFadeEnabled(bool enabled);
    // Mirrors AOSP LayerDrawable.getChangingConfigurations: instance value ORed with the
    // backing LayerState's value so callers see the effective configuration of this drawable.
    int getChangingConfigurations()const override;
    void draw(Canvas&canvas)override;
};

}/*endof namespace*/
#endif
