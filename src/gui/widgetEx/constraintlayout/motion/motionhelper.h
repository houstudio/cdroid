/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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
