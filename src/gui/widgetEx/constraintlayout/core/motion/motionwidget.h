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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.MotionWidget.
 *
 * The widget abstraction the Motion engine operates on: a thin property bridge over a WidgetFrame
 * plus a per-widget Motion config holder. setValue(id, value) dispatches by the TypedValues id to
 * either an attribute (alpha/translation/rotation/scale/pivot — written to the frame) or a motion
 * property (easing/path-arc/stagger — written to mMotion). The Motion engine reads/writes these.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_WIDGET_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_WIDGET_H

#include <string>
#include <unordered_set>

#include <widgetEx/constraintlayout/core/motion/customattribute.h>
#include <widgetEx/constraintlayout/core/motion/customvariable.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>
#include <widgetEx/constraintlayout/core/state/widgetframe.h>

namespace cdroid {

class MotionWidget : public TypedValues {
  public:
    static constexpr int VISIBILITY_MODE_NORMAL = 0;
    static constexpr int VISIBILITY_MODE_IGNORE = 1;
    static constexpr int INVISIBLE = 0;
    static constexpr int VISIBLE   = 4;
    static constexpr int ROTATE_NONE              = 0;
    static constexpr int ROTATE_PORTRATE_OF_RIGHT = 1;
    static constexpr int ROTATE_PORTRATE_OF_LEFT  = 2;
    static constexpr int ROTATE_RIGHT_OF_PORTRATE = 3;
    static constexpr int ROTATE_LEFT_OF_PORTRATE  = 4;
    static constexpr int UNSET = -1;
    static constexpr int PARENT_ID = 0;
    static constexpr int FILL_PARENT = -1;
    static constexpr int MATCH_PARENT = -1;
    static constexpr int WRAP_CONTENT = -2;
    static constexpr int GONE_UNSET = INT_MIN;

    // Per-widget motion configuration (easing / arc / path-rotate / stagger / quantize...).
    struct Motion {
        std::string mAnimateRelativeTo;
        std::string mTransitionEasing;
        int mPathMotionArc = UNSET;
        int mDrawPath = 0;
        std::string mQuantizeInterpolatorString;
        int mQuantizeMotionSteps = 1;
        int mQuantizeInterpolatorType = 0;
        int mQuantizeInterpolatorID = UNSET;
        float mMotionStagger = NAN;
        float mPathRotate = NAN;
        float mQuantizeMotionPhase = NAN;
        int mAnimateCircleAngleTo = -1;
        int mPolarRelativeTo = UNSET;
    };

    MotionWidget();
    explicit MotionWidget(WidgetFrame* f);
    ~MotionWidget()override;
    WidgetFrame* getWidgetFrame() const {
        return mWidgetFrame;
    }

    // --- geometry ---
    int getTop() const;
    int getLeft() const;
    int getBottom() const;
    int getRight() const;
    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
    int getVisibility() const;
    void setVisibility(int visibility);
    void setBounds(int left, int top, int right, int bottom);

    // --- transforms (read/write the WidgetFrame) ---
    float getRotationX() const;
    void setRotationX(float v);
    float getRotationY() const;
    void setRotationY(float v);
    float getRotationZ() const;
    void setRotationZ(float v);
    float getTranslationX() const;
    void setTranslationX(float v);
    float getTranslationY() const;
    void setTranslationY(float v);
    float getTranslationZ() const;
    void setTranslationZ(float v);
    float getScaleX() const;
    void setScaleX(float v);
    float getScaleY() const;
    void setScaleY(float v);
    float getPivotX() const;
    void setPivotX(float v);
    float getPivotY() const;
    void setPivotY(float v);
    float getAlpha() const;
    void setAlpha(float v);

    // --- TypedValues dispatch ---
    bool setValue(int id, int value) override;
    bool setValue(int id, float value) override;
    bool setValue(int id, const std::string& value) override;
    bool setValue(int id, bool value) override;
    int  getId(const std::string& name) override;

    bool setValueMotion(int id, int value);
    bool setValueMotion(int id, float value);
    bool setValueMotion(int id, const std::string& value);
    bool setValueAttributes(int id, float value);
    float getValueAttributes(int id) const;

    void updateMotion(TypedValues& toUpdate);

    // --- custom attributes (bridge to the WidgetFrame's custom map) ---
    std::unordered_set<std::string> getCustomAttributeNames() const;
    void setCustomAttribute(const std::string& name, int type, float value);
    void setCustomAttribute(const std::string& name, int type, int value);
    void setCustomAttribute(const std::string& name, int type, bool value);
    void setCustomAttribute(const std::string& name, int type, const std::string& value);
    CustomVariable* getCustomAttribute(const std::string& name);
    void setInterpolatedValue(CustomAttribute& attribute, std::vector<float>& mCache);

    Motion mMotion;
    float mProgress = NAN;
    float mTransitionPathRotate = NAN;

  private:
    WidgetFrame* mWidgetFrame;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_MOTION_WIDGET_H
