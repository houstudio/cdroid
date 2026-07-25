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
#ifndef __ANIMATION_TYPEEVALUATORS_H__
#define __ANIMATION_TYPEEVALUATORS_H__

#include <animation/propertyvaluesholder.h>  // TypeEvaluator, AnimateValue

namespace cdroid{

/**
 * Evaluators for the value types added to AnimateValue to support android.transition's
 * ofObject(Rect/PointF) animations. Each follows the TypeEvaluator function-pointer
 * signature: AnimateValue& (float fraction, AnimateValue& out, const AnimateValue& from, const AnimateValue& to).
 *
 * NB: there is NO MatrixEvaluator / FloatArrayEvaluator here. cairo's matrix type is kept
 * OUT of the AnimateValue variant (per port design), so ChangeTransform/ChangeImageTransform
 * animate matrices by driving a plain fraction ValueAnimator and computing the matrix directly,
 * rather than through this evaluator interface.
 */
AnimateValue& RectEvaluator(float fraction, AnimateValue& out, const AnimateValue& from, const AnimateValue& to);
AnimateValue& RectFEvaluator(float fraction, AnimateValue& out, const AnimateValue& from, const AnimateValue& to);
AnimateValue& PointFEvaluator(float fraction, AnimateValue& out, const AnimateValue& from, const AnimateValue& to);

} // namespace cdroid
#endif // __ANIMATION_TYPEEVALUATORS_H__
