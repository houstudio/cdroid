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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.state.WidgetFrame.
 *
 * The per-widget frame data: position (left/top/right/bottom), transforms (pivot/rotation/
 * translation/scale), alpha, visibility, and a map of custom variables. MotionWidget wraps a
 * WidgetFrame; the Motion engine interpolates between a start and end frame.
 *
 * MVP: data fields + accessors + custom-variable handling + update-from-widget. Deferred:
 *  - interpolate(...) depends on core.state.Transition (keyframe path) — the Motion engine
 *    (chunk 5) does the interpolation instead.
 *  - setValue(String, CLElement) / parseCustom / toCLString depend on core.parser CL* (JSON) —
 *    not needed for a programmatic start/end ConstraintSet MotionLayout.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_STATE_WIDGET_FRAME_H
#define CDROID_CONSTRAINTLAYOUT_CORE_STATE_WIDGET_FRAME_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include <widgetEx/constraintlayout/core/motion/customvariable.h>
#include <widgetEx/constraintlayout/core/motion/typedbundle.h>

namespace cdroid {

class ConstraintWidget;
class CustomAttribute;

class WidgetFrame {
public:
    ConstraintWidget* widget = nullptr;
    int left = 0, top = 0, right = 0, bottom = 0;

    float pivotX = NAN, pivotY = NAN;
    float rotationX = NAN, rotationY = NAN, rotationZ = NAN;
    float translationX = NAN, translationY = NAN, translationZ = NAN;
    static float phone_orientation;
    float scaleX = NAN, scaleY = NAN;
    float alpha = NAN;
    float interpolatedPos = NAN;
    int visibility = 0 /*VISIBLE*/;
    std::string name;

    WidgetFrame();
    explicit WidgetFrame(ConstraintWidget* widget);
    WidgetFrame(const WidgetFrame& frame);

    int width() const  { return std::max(0, right - left); }
    int height() const { return std::max(0, bottom - top); }
    float centerX() const { return left + (right - left) / 2.0f; }
    float centerY() const { return top + (bottom - top) / 2.0f; }

    void updateAttributes(const WidgetFrame* frame);
    bool isDefaultTransform() const;

    WidgetFrame& update();
    WidgetFrame& update(ConstraintWidget* widget);

    bool containsCustom(const std::string& name) const;
    void addCustomColor(const std::string& name, int color);
    int  getCustomColor(const std::string& name) const;
    void addCustomFloat(const std::string& name, float value);
    float getCustomFloat(const std::string& name) const;

    void setCustomAttribute(const std::string& name, int type, float value);
    void setCustomAttribute(const std::string& name, int type, int value);
    void setCustomAttribute(const std::string& name, int type, bool value);
    void setCustomAttribute(const std::string& name, int type, const std::string& value);
    CustomVariable* getCustomAttribute(const std::string& name);
    std::unordered_set<std::string> getCustomAttributeNames() const;

    void setCustomValue(CustomAttribute& valueAt, std::vector<float>& mTempValues);
    void setMotionAttributes(TypedBundle* motionProperties) { mMotionProperties = motionProperties; }
    TypedBundle* getMotionProperties() const { return mMotionProperties; }

private:
    std::unordered_map<std::string, CustomVariable> mCustom;
    TypedBundle* mMotionProperties = nullptr;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_STATE_WIDGET_FRAME_H
