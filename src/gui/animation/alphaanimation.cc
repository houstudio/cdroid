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
#include<animation/alphaanimation.h>

namespace cdroid{

AlphaAnimation::AlphaAnimation(const AlphaAnimation&o)
	:Animation(o){
    mFromAlpha = o.mFromAlpha;
    mToAlpha = o.mToAlpha;
}

AlphaAnimation::AlphaAnimation(Context* context,const AttributeSet& attrs):Animation(context,attrs){
    mFromAlpha = attrs.getFloat("fromAlpha",1.f);
    mToAlpha   = attrs.getFloat("toAlpha",1.f);
}

AlphaAnimation::AlphaAnimation(float fromAlpha, float toAlpha){
    mFromAlpha = fromAlpha;
    mToAlpha = toAlpha;
}

void AlphaAnimation::applyTransformation(float interpolatedTime, Transformation& t) {
    t.setAlpha(mFromAlpha + ((mToAlpha - mFromAlpha) * interpolatedTime));
}

bool AlphaAnimation::willChangeTransformationMatrix()const{
    return false;
}

bool AlphaAnimation::willChangeBounds()const{
    return false;
}

bool AlphaAnimation::hasAlpha(){
    return true;
}

AlphaAnimation*AlphaAnimation::clone()const{
    return new AlphaAnimation(*this);
}

}
