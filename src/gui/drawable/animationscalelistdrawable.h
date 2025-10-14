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
#ifndef __ANIMATION_SCALE_LIST_DRAWABLE_H__
#define __ANIMATION_SCALE_LIST_DRAWABLE_H__
#include <drawable/drawablecontainer.h>
namespace cdroid{
class AnimationScaleListDrawable:public DrawableContainer,public Animatable {
protected:
    class AnimationScaleListState:public DrawableContainerState {
        // The index of the last static drawable.
        int mStaticDrawableIndex = -1;
        // The index of the last animatable drawable.
        int mAnimatableDrawableIndex = -1;
    public:
        AnimationScaleListState(const AnimationScaleListState* orig, AnimationScaleListDrawable* owner);
        void mutate()override;
        int addDrawable(Drawable* drawable);
        AnimationScaleListDrawable* newDrawable() override;
        //bool canApplyTheme() override;
        int getCurrentDrawableIndexBasedOnScale();
    };
private:
    bool mMutated;
    std::shared_ptr<AnimationScaleListState> mAnimationScaleListState;
private:
    AnimationScaleListDrawable(std::shared_ptr<AnimationScaleListState> state);
    void inflateChildElements(XmlPullParser& parser,const AttributeSet& attrs);
protected:
    bool onStateChange(const std::vector<int>& stateSet)override;
    void setConstantState(std::shared_ptr<DrawableContainerState> state) override;
public:
    AnimationScaleListDrawable();
    void inflate(XmlPullParser& parser,const AttributeSet& attrs)override;
    AnimationScaleListDrawable* mutate() override;
    void clearMutated() override;
    void start() override;
    void stop() override;
    bool isRunning()override;
    //void applyTheme(@NonNull Theme theme);
};
}/*endof namespace*/
#endif/*__ANIMATION_SCALE_LIST_DRAWABLE_H__*/
