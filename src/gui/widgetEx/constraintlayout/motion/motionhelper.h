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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.MotionHelper.
 *
 * Base for MotionLayout "decorators" — helpers that mutate the Motion controllers during a
 * transition's setup (e.g. MotionEffect inserts fade/stick keyframes). MotionLayout scans its
 * children for MotionHelper decorators after building its Motion controllers and calls onPreSetup,
 * handing each the live per-view controller map (id -> Motion).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_HELPER_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_HELPER_H

#include <unordered_map>

#include <widgetEx/constraintlayout/helpers/constrainthelper.h>

namespace cdroid {

class MotionLayout;
class Motion;

class MotionHelper : public ConstraintHelper {
  public:
    using MotionMap = std::unordered_map<int, Motion*>;

    MotionHelper(Context* ctx, const AttributeSet& attrs) : ConstraintHelper(ctx, attrs) {}
    explicit MotionHelper(int width, int height) : ConstraintHelper(width, height) {}

    // Decorators are invoked via onPreSetup during transition setup; plain MotionHelpers are not.
    virtual bool isDecorator() const { return false; }

    // Called by MotionLayout after its Motion controllers are built (start/end positions known) and
    // before the transition is applied. A decorator may add keyframes to the controllers. The map is
    // keyed by view id (CDROID's MotionLayout stores id -> Motion*, vs AndroidX's View -> controller).
    virtual void onPreSetup(MotionLayout* /*motionLayout*/, MotionMap& /*motions*/) {}
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_MOTION_HELPER_H
