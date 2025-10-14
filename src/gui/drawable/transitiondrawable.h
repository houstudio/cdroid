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
    bool mReverse;
    int64_t mStartTimeMillis;
    int  mFrom;
    int  mTo;
    int  mDuration;
    int  mOriginalDuration;
    int  mAlpha;
    bool mCrossFade;

    class TransitionState:public LayerDrawable::LayerState{
    public:
        TransitionState(TransitionState* orig, TransitionDrawable* owner);
        TransitionDrawable*newDrawable()override;
    };

    TransitionDrawable(std::shared_ptr<TransitionState> state);
    std::shared_ptr<LayerDrawable::LayerState> createConstantState(LayerState* state,const AttributeSet*)override;
public:
    TransitionDrawable();
    TransitionDrawable(const std::vector<Drawable*>drawables);
    void startTransition(int durationMillis);
    void resetTransition();
    void reverseTransition(int duration);
    bool isCrossFadeEnabled()const;
    void setCrossFadeEnabled(bool enabled);
    void draw(Canvas&canvas)override;
};

}/*endof namespace*/
#endif
