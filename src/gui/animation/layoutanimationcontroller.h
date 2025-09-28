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
#ifndef __LAYOUT_ANIMATION_CONTROLLER_H__
#define __LAYOUT_ANIMATION_CONTROLLER_H__
#include <animation/animation.h>

namespace cdroid{
class View;
class LayoutAnimationController{
public:
    static constexpr int ORDER_NORMAL  = 0;
    static constexpr int ORDER_REVERSE = 1;
    static constexpr int ORDER_RANDOM  = 2;
    class AnimationParameters {
    /* The number of children in the view group containing the view to which
     * these parameters are attached.*/
    public: int count;

    /** The index of the view to which these parameters are attached in its
     * containing view group.*/
    public: int index;
    };
private:
    float mDelay;
    int mOrder;
    int64_t mDuration;
    int64_t mMaxDelay; 
protected:
    Animation* mAnimation;
    Interpolator* mInterpolator;

    virtual int64_t getDelayForView(View* view);
    int getTransformedIndex(const AnimationParameters* params);
public:
    LayoutAnimationController(Context* context, const AttributeSet& attrs);
    LayoutAnimationController(Animation* animation,float delay);
    LayoutAnimationController(Animation* animation);
    virtual ~LayoutAnimationController();
    int getOrder()const;
    void setOrder(int);
    void setAnimation(Context* context,const std::string&resourceID);
    void setAnimation(Animation* animation);
    Animation* getAnimation();
    void setInterpolator(Context* context,const std::string&resourceID);
    void setInterpolator(Interpolator* interpolator);
    Interpolator* getInterpolator();
    float getDelay()const;
    void setDelay(float);
    virtual bool willOverlap();
    void start();
    Animation* getAnimationForView(View* view);
    bool isDone()const;
};

}//endof namespace
#endif //__LAYOUT_ANIMATION_CONTROLLER_H__
