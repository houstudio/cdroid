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
#include <animation/cliprectanimation.h>
#include <core/rect.h>
#include <core/typedarray.h>
#include <core/typedvalue.h>   // TypedValue (Description::parseValue)
#include <widget/framework_styleable.h>
namespace cdroid{
using namespace cdroid::internal;

ClipRectAnimation::ClipRectAnimation(const ClipRectAnimation&o):Animation(o){
    mFromLeftValue  = o.mFromLeftValue;
    mFromTopValue   = o.mFromTopValue;
    mFromRightValue = o.mFromRightValue;
    mFromBottomValue= o.mFromBottomValue;
    mToLeftValue    = o.mToLeftValue;
    mToTopValue     = o.mToTopValue;
    mToRightValue   = o.mToRightValue;
    mToBottomValue  = o.mToBottomValue;
}

ClipRectAnimation::ClipRectAnimation(Context* context, const AttributeSet& attrs)
:Animation(context,attrs){
    auto a = context->obtainStyledAttributes(attrs, R::styleable::ClipRectAnimation);

    // AOSP: Description.parseValue(a.peekValue(idx), context) — null when absent.
    TypedValue v;
    auto parse = [&](int idx)->Description {
        if (!a->peekValue(idx, &v)) return Description::parseValue(nullptr, context);
        return Description::parseValue(&v, context);
    };

    Description d = parse(R::styleable::ClipRectAnimation_fromLeft);
    mFromLeftType = d.type;
    mFromLeftValue= d.value;

    d = parse(R::styleable::ClipRectAnimation_fromTop);
    mFromTopType = d.type;
    mFromTopValue= d.value;

    d = parse(R::styleable::ClipRectAnimation_fromRight);
    mFromRightType = d.type;
    mFromRightValue = d.value;

    d = parse(R::styleable::ClipRectAnimation_fromBottom);
    mFromBottomType = d.type;
    mFromBottomValue= d.value;

    d = parse(R::styleable::ClipRectAnimation_toLeft);
    mToLeftType = d.type;
    mToLeftValue= d.value;

    d = parse(R::styleable::ClipRectAnimation_toTop);
    mToTopType = d.type;
    mToTopValue= d.value;

    d = parse(R::styleable::ClipRectAnimation_toRight);
    mToRightType = d.type;
    mToRightValue= d.value;

    d = parse(R::styleable::ClipRectAnimation_toBottom);
    mToBottomType = d.type;
    mToBottomValue= d.value;
}

ClipRectAnimation::ClipRectAnimation(const Rect& fromClip,const Rect& toClip)
:Animation(){
    mFromLeftValue = (float)fromClip.left;
    mFromTopValue = (float)fromClip.top;
    mFromRightValue= (float)fromClip.right();
    mFromBottomValue = (float)fromClip.bottom();

    mToLeftValue = (float)toClip.left;
    mToTopValue = (float)toClip.top;
    mToRightValue= (float)toClip.right();
    mToBottomValue = (float)toClip.bottom();    
}

ClipRectAnimation::ClipRectAnimation(int fromL, int fromT, int fromR, int fromB,
        int toL, int toT, int toR, int toB)
: ClipRectAnimation(Rect::MakeLTRB(fromL, fromT, fromR, fromB),Rect::MakeLTRB(toL, toT, toR, toB)){
}

ClipRectAnimation* ClipRectAnimation::clone()const{
    return new ClipRectAnimation(*this);
}

void ClipRectAnimation::applyTransformation(float it, Transformation& tr){
    int l = mFromRect.left + (int) ((mToRect.left - mFromRect.left) * it);
    int t = mFromRect.top + (int) ((mToRect.top - mFromRect.top) * it);
    int r = mFromRect.right() + (int) ((mToRect.right() - mFromRect.right()) * it);
    int b = mFromRect.bottom() + (int) ((mToRect.bottom() - mFromRect.bottom()) * it);
    tr.setClipRect(l, t, r, b);
}

bool ClipRectAnimation::willChangeTransformationMatrix()const{
    return false;
}

void ClipRectAnimation::initialize(int width, int height, int parentWidth, int parentHeight) {
     Animation::initialize(width, height, parentWidth, parentHeight);

     mFromRect.set((int) resolveSize(mFromLeftType, mFromLeftValue, width, parentWidth),
            (int) resolveSize(mFromTopType, mFromTopValue, height, parentHeight),
            (int) resolveSize(mFromRightType, mFromRightValue, width, parentWidth),
            (int) resolveSize(mFromBottomType, mFromBottomValue, height, parentHeight));
     mToRect.set((int) resolveSize(mToLeftType, mToLeftValue, width, parentWidth),
            (int) resolveSize(mToTopType, mToTopValue, height, parentHeight),
            (int) resolveSize(mToRightType, mToRightValue, width, parentWidth),
            (int) resolveSize(mToBottomType, mToBottomValue, height, parentHeight));
}

}
