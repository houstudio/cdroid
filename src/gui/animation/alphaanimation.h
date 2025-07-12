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
#ifndef __ALPHA_ANIMAION_H__
#define __ALPHA_ANIMAION_H__
#include <animation/animation.h>
namespace cdroid{

class AlphaAnimation:public Animation{
private:
    float mFromAlpha;
    float mToAlpha;
protected:
    AlphaAnimation(const AlphaAnimation&other);
    void applyTransformation(float interpolatedTime, Transformation& t)override;
public:
    AlphaAnimation(Context* context,const AttributeSet& attrs);
    AlphaAnimation(float fromAlpha, float toAlpha);
    bool willChangeTransformationMatrix()const override;
    bool willChangeBounds()const override;
    bool hasAlpha() override;
    AlphaAnimation*clone()const override;
};
}
#endif//__ALPHA_ANIMAION_H__
