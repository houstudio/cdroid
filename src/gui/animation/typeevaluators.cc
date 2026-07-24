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
#include <animation/typeevaluators.h>

namespace cdroid{

// CRect stores {left, top, width, height}. Lerping these linearly is equivalent to lerping
// android.graphics.Rect's {l,t,r,b} when the Rect was built via MakeLTRB(l,t,r,b): since
// right()==left+width and a sum of linear functions is linear, interpolating (l,t,w,h) yields
// the same endpoints and midpoints as interpolating (l,t,r,b). So transition bounds animations
// (ChangeBounds/ChangeClipBounds) stay faithful to android.
AnimateValue& RectEvaluator(float fraction, AnimateValue& out, const AnimateValue& from, const AnimateValue& to){
    const Rect& a = GET_VARIANT(from, Rect);
    const Rect& b = GET_VARIANT(to, Rect);
    Rect r;
    r.left   = (int)((1.f - fraction) * a.left   + fraction * b.left);
    r.top    = (int)((1.f - fraction) * a.top    + fraction * b.top);
    r.width  = (int)((1.f - fraction) * a.width  + fraction * b.width);
    r.height = (int)((1.f - fraction) * a.height + fraction * b.height);
    out = r;
    return out;
}

AnimateValue& RectFEvaluator(float fraction, AnimateValue& out, const AnimateValue& from, const AnimateValue& to){
    const RectF& a = GET_VARIANT(from, RectF);
    const RectF& b = GET_VARIANT(to, RectF);
    RectF r;
    r.left   = (1.f - fraction) * a.left   + fraction * b.left;
    r.top    = (1.f - fraction) * a.top    + fraction * b.top;
    r.width  = (1.f - fraction) * a.width  + fraction * b.width;
    r.height = (1.f - fraction) * a.height + fraction * b.height;
    out = r;
    return out;
}

AnimateValue& PointFEvaluator(float fraction, AnimateValue& out, const AnimateValue& from, const AnimateValue& to){
    const PointF& a = GET_VARIANT(from, PointF);
    const PointF& b = GET_VARIANT(to, PointF);
    PointF p;
    p.x = (1.f - fraction) * a.x + fraction * b.x;
    p.y = (1.f - fraction) * a.y + fraction * b.y;
    out = p;
    return out;
}

} // namespace cdroid
