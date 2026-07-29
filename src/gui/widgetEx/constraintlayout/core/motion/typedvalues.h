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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.TypedValues.
 *
 * Property-id constants + name↔id tables for the motion system. Each nested struct groups the
 * constants for a keyframe category (KeyAttributes, KeyCycle, KeyTrigger, KeyPosition, Motion,
 * Custom). getId() maps the string name used in JSON/XML to the integer id used at runtime;
 * getType() reports the value's type mask. The base TypedValues is the setValue interface
 * implemented by TypedBundle / MotionWidget / Motion.
 *
 * MVP: only the AttributesType (KeyAttributes) constants are exercised by the initial Motion
 * engine; the other categories are ported for completeness but not yet wired.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_TYPED_VALUES_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_TYPED_VALUES_H

#include <string>

namespace cdroid {

struct TypedValues {
    static constexpr const char* S_CUSTOM = "CUSTOM";
    static constexpr int BOOLEAN_MASK = 1;
    static constexpr int INT_MASK     = 2;
    static constexpr int FLOAT_MASK   = 4;
    static constexpr int STRING_MASK  = 8;

    virtual ~TypedValues() = default;
    virtual bool setValue(int id, int value);
    virtual bool setValue(int id, float value);
    virtual bool setValue(int id, const std::string& value);
    virtual bool setValue(int id, bool value);
    virtual int  getId(const std::string& name);

    static constexpr int TYPE_FRAME_POSITION = 100;
    static constexpr int TYPE_TARGET = 101;

    // --- KeyAttributes ---
    struct AttributesType {
        static constexpr const char* NAME = "KeyAttributes";
        static constexpr int TYPE_CURVE_FIT     = 301;
        static constexpr int TYPE_VISIBILITY    = 302;
        static constexpr int TYPE_ALPHA         = 303;
        static constexpr int TYPE_TRANSLATION_X = 304;
        static constexpr int TYPE_TRANSLATION_Y = 305;
        static constexpr int TYPE_TRANSLATION_Z = 306;
        static constexpr int TYPE_ELEVATION     = 307;
        static constexpr int TYPE_ROTATION_X    = 308;
        static constexpr int TYPE_ROTATION_Y    = 309;
        static constexpr int TYPE_ROTATION_Z    = 310;
        static constexpr int TYPE_SCALE_X       = 311;
        static constexpr int TYPE_SCALE_Y       = 312;
        static constexpr int TYPE_PIVOT_X       = 313;
        static constexpr int TYPE_PIVOT_Y       = 314;
        static constexpr int TYPE_PROGRESS      = 315;
        static constexpr int TYPE_PATH_ROTATE   = 316;
        static constexpr int TYPE_EASING        = 317;
        static constexpr int TYPE_PIVOT_TARGET  = 318;

        static constexpr const char* S_CURVE_FIT     = "curveFit";
        static constexpr const char* S_VISIBILITY    = "visibility";
        static constexpr const char* S_ALPHA         = "alpha";
        static constexpr const char* S_TRANSLATION_X = "translationX";
        static constexpr const char* S_TRANSLATION_Y = "translationY";
        static constexpr const char* S_TRANSLATION_Z = "translationZ";
        static constexpr const char* S_ELEVATION     = "elevation";
        static constexpr const char* S_ROTATION_X    = "rotationX";
        static constexpr const char* S_ROTATION_Y    = "rotationY";
        static constexpr const char* S_ROTATION_Z    = "rotationZ";
        static constexpr const char* S_SCALE_X       = "scaleX";
        static constexpr const char* S_SCALE_Y       = "scaleY";
        static constexpr const char* S_PIVOT_X       = "pivotX";
        static constexpr const char* S_PIVOT_Y       = "pivotY";
        static constexpr const char* S_PROGRESS      = "progress";
        static constexpr const char* S_PATH_ROTATE   = "pathRotate";
        static constexpr const char* S_EASING        = "easing";
        static constexpr const char* S_FRAME         = "frame";
        static constexpr const char* S_TARGET        = "target";
        static constexpr const char* S_PIVOT_TARGET  = "pivotTarget";

        static int getId(const std::string& name);
        static int getType(int name);
    };

    // --- KeyCycle ---
    struct CycleType {
        static constexpr const char* NAME = "KeyCycle";
        static constexpr int TYPE_CURVE_FIT         = 401;
        static constexpr int TYPE_VISIBILITY        = 402;
        static constexpr int TYPE_ALPHA             = 403;
        static constexpr int TYPE_PATH_ROTATE       = 416;
        static constexpr int TYPE_EASING            = 420;
        static constexpr int TYPE_WAVE_SHAPE        = 421;
        static constexpr int TYPE_CUSTOM_WAVE_SHAPE = 422;
        static constexpr int TYPE_WAVE_PERIOD       = 423;
        static constexpr int TYPE_WAVE_OFFSET       = 424;
        static constexpr int TYPE_WAVE_PHASE        = 425;
        // translation/rotation/scale/pivot/progress reuse AttributesType ids.
        static constexpr int TYPE_TRANSLATION_X = AttributesType::TYPE_TRANSLATION_X;
        static constexpr int TYPE_TRANSLATION_Y = AttributesType::TYPE_TRANSLATION_Y;
        static constexpr int TYPE_TRANSLATION_Z = AttributesType::TYPE_TRANSLATION_Z;
        static constexpr int TYPE_ELEVATION     = AttributesType::TYPE_ELEVATION;
        static constexpr int TYPE_ROTATION_X    = AttributesType::TYPE_ROTATION_X;
        static constexpr int TYPE_ROTATION_Y    = AttributesType::TYPE_ROTATION_Y;
        static constexpr int TYPE_ROTATION_Z    = AttributesType::TYPE_ROTATION_Z;
        static constexpr int TYPE_SCALE_X       = AttributesType::TYPE_SCALE_X;
        static constexpr int TYPE_SCALE_Y       = AttributesType::TYPE_SCALE_Y;
        static constexpr int TYPE_PIVOT_X       = AttributesType::TYPE_PIVOT_X;
        static constexpr int TYPE_PIVOT_Y       = AttributesType::TYPE_PIVOT_Y;
        static constexpr int TYPE_PROGRESS      = AttributesType::TYPE_PROGRESS;

        static constexpr const char* S_WAVE_SHAPE        = "waveShape";
        static constexpr const char* S_CUSTOM_WAVE_SHAPE = "customWave";
        static constexpr const char* S_WAVE_PERIOD       = "period";
        static constexpr const char* S_WAVE_OFFSET       = "offset";
        static constexpr const char* S_WAVE_PHASE        = "phase";

        static int getId(const std::string& name);
        static int getType(int name);
    };

    // --- KeyTrigger ---
    struct TriggerType {
        static constexpr const char* NAME = "KeyTrigger";
        static constexpr const char* VIEW_TRANSITION_ON_CROSS            = "viewTransitionOnCross";
        static constexpr const char* VIEW_TRANSITION_ON_POSITIVE_CROSS   = "viewTransitionOnPositiveCross";
        static constexpr const char* VIEW_TRANSITION_ON_NEGATIVE_CROSS   = "viewTransitionOnNegativeCross";
        static constexpr const char* POST_LAYOUT        = "postLayout";
        static constexpr const char* TRIGGER_SLACK      = "triggerSlack";
        static constexpr const char* TRIGGER_COLLISION_VIEW = "triggerCollisionView";
        static constexpr const char* TRIGGER_COLLISION_ID  = "triggerCollisionId";
        static constexpr const char* TRIGGER_ID         = "triggerID";
        static constexpr const char* POSITIVE_CROSS     = "positiveCross";
        static constexpr const char* NEGATIVE_CROSS     = "negativeCross";
        static constexpr const char* TRIGGER_RECEIVER   = "triggerReceiver";
        static constexpr const char* CROSS              = "CROSS";

        static constexpr int TYPE_VIEW_TRANSITION_ON_CROSS          = 301;
        static constexpr int TYPE_VIEW_TRANSITION_ON_POSITIVE_CROSS = 302;
        static constexpr int TYPE_VIEW_TRANSITION_ON_NEGATIVE_CROSS = 303;
        static constexpr int TYPE_POST_LAYOUT        = 304;
        static constexpr int TYPE_TRIGGER_SLACK      = 305;
        static constexpr int TYPE_TRIGGER_COLLISION_VIEW = 306;
        static constexpr int TYPE_TRIGGER_COLLISION_ID  = 307;
        static constexpr int TYPE_TRIGGER_ID         = 308;
        static constexpr int TYPE_POSITIVE_CROSS     = 309;
        static constexpr int TYPE_NEGATIVE_CROSS     = 310;
        static constexpr int TYPE_TRIGGER_RECEIVER   = 311;
        static constexpr int TYPE_CROSS              = 312;

        static int getId(const std::string& name);
    };

    // --- KeyPosition ---
    struct PositionType {
        static constexpr const char* NAME = "KeyPosition";
        static constexpr const char* S_TRANSITION_EASING = "transitionEasing";
        static constexpr const char* S_DRAWPATH          = "drawPath";
        static constexpr const char* S_PERCENT_WIDTH     = "percentWidth";
        static constexpr const char* S_PERCENT_HEIGHT    = "percentHeight";
        static constexpr const char* S_SIZE_PERCENT      = "sizePercent";
        static constexpr const char* S_PERCENT_X         = "percentX";
        static constexpr const char* S_PERCENT_Y         = "percentY";

        static constexpr int TYPE_TRANSITION_EASING = 501;
        static constexpr int TYPE_DRAWPATH          = 502;
        static constexpr int TYPE_PERCENT_WIDTH     = 503;
        static constexpr int TYPE_PERCENT_HEIGHT    = 504;
        static constexpr int TYPE_SIZE_PERCENT      = 505;
        static constexpr int TYPE_PERCENT_X         = 506;
        static constexpr int TYPE_PERCENT_Y         = 507;
        static constexpr int TYPE_CURVE_FIT         = 508;
        static constexpr int TYPE_PATH_MOTION_ARC   = 509;
        static constexpr int TYPE_POSITION_TYPE     = 510;

        static int getId(const std::string& name);
        static int getType(int name);
    };

    // --- Motion (per-widget transition defaults) ---
    struct MotionType {
        static constexpr const char* NAME = "Motion";
        static constexpr const char* S_STAGGER                    = "Stagger";
        static constexpr const char* S_PATH_ROTATE                = "PathRotate";
        static constexpr const char* S_QUANTIZE_MOTION_PHASE      = "QuantizeMotionPhase";
        static constexpr const char* S_EASING                     = "TransitionEasing";
        static constexpr const char* S_QUANTIZE_INTERPOLATOR      = "QuantizeInterpolator";
        static constexpr const char* S_ANIMATE_RELATIVE_TO        = "AnimateRelativeTo";
        static constexpr const char* S_ANIMATE_CIRCLEANGLE_TO     = "AnimateCircleAngleTo";
        static constexpr const char* S_PATHMOTION_ARC             = "PathMotionArc";
        static constexpr const char* S_DRAW_PATH                  = "DrawPath";
        static constexpr const char* S_POLAR_RELATIVETO           = "PolarRelativeTo";
        static constexpr const char* S_QUANTIZE_MOTIONSTEPS       = "QuantizeMotionSteps";
        static constexpr const char* S_QUANTIZE_INTERPOLATOR_TYPE = "QuantizeInterpolatorType";
        static constexpr const char* S_QUANTIZE_INTERPOLATOR_ID   = "QuantizeInterpolatorID";

        static constexpr int TYPE_STAGGER                    = 600;
        static constexpr int TYPE_PATH_ROTATE                = 601;
        static constexpr int TYPE_QUANTIZE_MOTION_PHASE      = 602;
        static constexpr int TYPE_EASING                     = 603;
        static constexpr int TYPE_QUANTIZE_INTERPOLATOR      = 604;
        static constexpr int TYPE_ANIMATE_RELATIVE_TO        = 605;
        static constexpr int TYPE_ANIMATE_CIRCLEANGLE_TO     = 606;
        static constexpr int TYPE_PATHMOTION_ARC             = 607;
        static constexpr int TYPE_DRAW_PATH                  = 608;
        static constexpr int TYPE_POLAR_RELATIVETO           = 609;
        static constexpr int TYPE_QUANTIZE_MOTIONSTEPS       = 610;
        static constexpr int TYPE_QUANTIZE_INTERPOLATOR_TYPE = 611;
        static constexpr int TYPE_QUANTIZE_INTERPOLATOR_ID   = 612;

        static int getId(const std::string& name);
    };

    // --- Custom attribute types ---
    struct Custom {
        static constexpr const char* NAME = "Custom";
        static constexpr const char* S_INT = "integer";
        static constexpr const char* S_FLOAT = "float";
        static constexpr const char* S_COLOR = "color";
        static constexpr const char* S_STRING = "string";
        static constexpr const char* S_BOOLEAN = "boolean";
        static constexpr const char* S_DIMENSION = "dimension";
        static constexpr const char* S_REFERENCE = "reference";

        static constexpr int TYPE_INT       = 900;
        static constexpr int TYPE_FLOAT     = 901;
        static constexpr int TYPE_COLOR     = 902;
        static constexpr int TYPE_STRING    = 903;
        static constexpr int TYPE_BOOLEAN   = 904;
        static constexpr int TYPE_DIMENSION = 905;
        static constexpr int TYPE_REFERENCE = 906;

        static int getId(const std::string& name);
    };
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_TYPED_VALUES_H
