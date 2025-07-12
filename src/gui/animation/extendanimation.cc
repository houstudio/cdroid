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
#include <animation/extendanimation.h>

namespace cdroid{

ExtendAnimation::ExtendAnimation(Context* context,const AttributeSet& attrs)
    :Animation(context, attrs){
    Description d = Description::parseValue(attrs.getString("fromExtendLeft"));
    mFromLeftType = d.type;
    mFromLeftValue= d.value;

    d = Description::parseValue(attrs.getString("fromExtendTop"));
    mFromTopType = d.type;
    mFromTopValue= d.value;

    d = Description::parseValue(attrs.getString("fromExtendRight"));
    mFromRightType = d.type;
    mFromRightValue= d.value;

    d = Description::parseValue(attrs.getString("fromExtendBottom"));
    mFromBottomType = d.type;
    mFromBottomValue= d.value;

    d = Description::parseValue(attrs.getString("toExtendLeft"));
    mToLeftType = d.type;
    mToLeftValue= d.value;

    d = Description::parseValue(attrs.getString("toExtendTop"));
    mToTopType = d.type;
    mToTopValue= d.value;

    d = Description::parseValue(attrs.getString("toExtendRight"));
    mToRightType = d.type;
    mToRightValue= d.value;

    d = Description::parseValue(attrs.getString("toExtendBottom"));
    mToBottomType = d.type;
    mToBottomValue= d.value;
}

/**
 * Constructor to use when building an ExtendAnimation from code
 *
 * @param fromInsets the insets to animate from
 * @param toInsets the insets to animate to
 */
ExtendAnimation::ExtendAnimation(const Insets& fromInsets,const Insets& toInsets) {
    mFromLeftValue = -fromInsets.left;
    mFromTopValue  = -fromInsets.top;
    mFromRightValue= -fromInsets.right;
    mFromBottomValue= -fromInsets.bottom;

    mToLeftValue = -toInsets.left;
    mToTopValue  = -toInsets.top;
    mToRightValue = -toInsets.right;
    mToBottomValue= -toInsets.bottom;
}

/**
 * Constructor to use when building an ExtendAnimation from code
 */
ExtendAnimation::ExtendAnimation(int fromL, int fromT, int fromR, int fromB,int toL, int toT, int toR, int toB)
    :ExtendAnimation(Insets::of(-fromL, -fromT, -fromR, -fromB), Insets::of(-toL, -toT, -toR, -toB)){
}

void ExtendAnimation::ExtendAnimation::applyTransformation(float it, Transformation& tr) {
    int l = mFromInsets.left + (int) ((mToInsets.left - mFromInsets.left) * it);
    int t = mFromInsets.top + (int) ((mToInsets.top - mFromInsets.top) * it);
    int r = mFromInsets.right + (int) ((mToInsets.right - mFromInsets.right) * it);
    int b = mFromInsets.bottom + (int) ((mToInsets.bottom - mFromInsets.bottom) * it);
    tr.setInsets(l, t, r, b);
}

bool ExtendAnimation::willChangeTransformationMatrix() const{
    return false;
}

int ExtendAnimation::getExtensionEdges() const{
    return (mFromInsets.left < 0 || mToInsets.left < 0 ?  WindowInsets::Side::LEFT : 0)
        | (mFromInsets.right < 0 || mToInsets.right < 0 ?  WindowInsets::Side::RIGHT : 0)
        | (mFromInsets.top < 0 || mToInsets.top < 0 ?  WindowInsets::Side::TOP : 0)
        | (mFromInsets.bottom < 0 || mToInsets.bottom < 0 ? WindowInsets::Side::BOTTOM : 0);
}

void ExtendAnimation::initialize(int width, int height, int parentWidth, int parentHeight) {
    Animation::initialize(width, height, parentWidth, parentHeight);
    // We remove any negative extension (i.e. positive insets) and set those to 0
    mFromInsets = Insets::min(Insets::of(
                -(int) resolveSize(mFromLeftType, mFromLeftValue, width, parentWidth),
                -(int) resolveSize(mFromTopType, mFromTopValue, height, parentHeight),
                -(int) resolveSize(mFromRightType, mFromRightValue, width, parentWidth),
                -(int) resolveSize(mFromBottomType, mFromBottomValue, height, parentHeight)
            ), Insets::NONE);
    mToInsets = Insets::min(Insets::of(
                -(int) resolveSize(mToLeftType, mToLeftValue, width, parentWidth),
                -(int) resolveSize(mToTopType, mToTopValue, height, parentHeight),
                -(int) resolveSize(mToRightType, mToRightValue, width, parentWidth),
                -(int) resolveSize(mToBottomType, mToBottomValue, height, parentHeight)
            ), Insets::NONE);
}
}/*endof namespace*/
