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
#ifndef __CLIPRECT_ANIMATION_H__
#define __CLIPRECT_ANIMATION_H__
#include <animation/animation.h>

namespace cdroid{

class ClipRectAnimation:public Animation{
private:
    int mFromLeftType = ABSOLUTE;
    int mFromTopType = ABSOLUTE;
    int mFromRightType = ABSOLUTE;
    int mFromBottomType = ABSOLUTE;

    int mToLeftType = ABSOLUTE;
    int mToTopType = ABSOLUTE;
    int mToRightType = ABSOLUTE;
    int mToBottomType = ABSOLUTE;

    float mFromLeftValue;
    float mFromTopValue;
    float mFromRightValue;
    float mFromBottomValue;

    float mToLeftValue;
    float mToTopValue;
    float mToRightValue;
    float mToBottomValue;
protected:
    Rect mFromRect;
    Rect mToRect;
    void applyTransformation(float it, Transformation& tr)override;
    ClipRectAnimation(const ClipRectAnimation&);
public:
    ClipRectAnimation(Context* context, const AttributeSet& attrs);
    ClipRectAnimation(const Rect& fromClip,const Rect& toClip);
    ClipRectAnimation(int fromL, int fromT, int fromR, int fromB,
            int toL, int toT, int toR, int toB);
    bool willChangeTransformationMatrix()const override;
    void initialize(int width, int height, int parentWidth, int parentHeight)override;
    ClipRectAnimation*clone()const override;
};

}/*endof namespace*/
#endif/*__CLIPRECT_ANIMATION_H__*/
