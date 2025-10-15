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
#ifndef __ANIMATED_ROTATE_DRAWABLE_H__
#define __ANIMATED_ROTATE_DRAWABLE_H__
#include <drawable/drawablewrapper.h>

namespace cdroid{

class AnimatedRotateDrawable:public DrawableWrapper,public Animatable{
private:
    class AnimatedRotateState:public DrawableWrapperState{
    public:
        std::vector<int> mThemeAttrs;
        bool mPivotXRel;
        float mPivotX;
        bool mPivotYRel;
        float mPivotY;
        int mFrameDuration;
        int mFramesCount;
        AnimatedRotateState();
        AnimatedRotateState(const AnimatedRotateState& orig);
        AnimatedRotateDrawable* newDrawable()override;
        int getChangingConfigurations()const override;
    };
    Runnable mNextFrame;
    float mCurrentDegrees;
    float mIncrement;
    bool mRunning;
    void updateLocalState();
    std::shared_ptr<AnimatedRotateState>mState;
    AnimatedRotateDrawable(std::shared_ptr<AnimatedRotateState> state);
    void updateStateFromTypedArray(const AttributeSet&atts);
protected:
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    AnimatedRotateDrawable();
    ~AnimatedRotateDrawable()override;
    float getPivotX()const;
    float getPivotY()const;
    void setPivotX(float pivotX);
    void setPivotY(float pivotX);

    bool isPivotXRelative()const;
    void setPivotXRelative(bool relative);
    bool isPivotYRelative()const;
    void setPivotYRelative(bool relative);

    void setFramesCount(int framesCount);
    void setFramesDuration(int framesDuration);
    bool setVisible(bool visible, bool restart)override;
    void start()override;
    void stop()override;
    bool isRunning()override;
    void nextFrame();
    void draw(Canvas& canvas)override;
    void inflate(XmlPullParser&,const AttributeSet&atts)override;
};

}
#endif
