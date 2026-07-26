/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.TypedValues.
 */
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

namespace cdroid {

// Base defaults: accept nothing, unknown name.
bool TypedValues::setValue(int /*id*/, int /*value*/)             { return false; }
bool TypedValues::setValue(int /*id*/, float /*value*/)           { return false; }
bool TypedValues::setValue(int /*id*/, const std::string& /*value*/) { return false; }
bool TypedValues::setValue(int /*id*/, bool /*value*/)            { return false; }
int  TypedValues::getId(const std::string& /*name*/)              { return -1; }

int TypedValues::AttributesType::getId(const std::string& name) {
    if (name == S_CURVE_FIT)     return TYPE_CURVE_FIT;
    if (name == S_VISIBILITY)    return TYPE_VISIBILITY;
    if (name == S_ALPHA)         return TYPE_ALPHA;
    if (name == S_TRANSLATION_X) return TYPE_TRANSLATION_X;
    if (name == S_TRANSLATION_Y) return TYPE_TRANSLATION_Y;
    if (name == S_TRANSLATION_Z) return TYPE_TRANSLATION_Z;
    if (name == S_ELEVATION)     return TYPE_ELEVATION;
    if (name == S_ROTATION_X)    return TYPE_ROTATION_X;
    if (name == S_ROTATION_Y)    return TYPE_ROTATION_Y;
    if (name == S_ROTATION_Z)    return TYPE_ROTATION_Z;
    if (name == S_SCALE_X)       return TYPE_SCALE_X;
    if (name == S_SCALE_Y)       return TYPE_SCALE_Y;
    if (name == S_PIVOT_X)       return TYPE_PIVOT_X;
    if (name == S_PIVOT_Y)       return TYPE_PIVOT_Y;
    if (name == S_PROGRESS)      return TYPE_PROGRESS;
    if (name == S_PATH_ROTATE)   return TYPE_PATH_ROTATE;
    if (name == S_EASING)        return TYPE_EASING;
    if (name == S_FRAME)         return TYPE_FRAME_POSITION;
    if (name == S_TARGET)        return TYPE_TARGET;
    if (name == S_PIVOT_TARGET)  return TYPE_PIVOT_TARGET;
    return -1;
}

int TypedValues::AttributesType::getType(int name) {
    switch (name) {
        case TYPE_CURVE_FIT: case TYPE_VISIBILITY: case TYPE_FRAME_POSITION: return INT_MASK;
        case TYPE_ALPHA: case TYPE_TRANSLATION_X: case TYPE_TRANSLATION_Y: case TYPE_TRANSLATION_Z:
        case TYPE_ELEVATION: case TYPE_ROTATION_X: case TYPE_ROTATION_Y: case TYPE_ROTATION_Z:
        case TYPE_SCALE_X: case TYPE_SCALE_Y: case TYPE_PIVOT_X: case TYPE_PIVOT_Y:
        case TYPE_PROGRESS: case TYPE_PATH_ROTATE: return FLOAT_MASK;
        case TYPE_EASING: case TYPE_TARGET: case TYPE_PIVOT_TARGET: return STRING_MASK;
        default: return -1;
    }
}

int TypedValues::CycleType::getId(const std::string& name) {
    if (name == AttributesType::S_CURVE_FIT)  return TYPE_CURVE_FIT;
    if (name == AttributesType::S_VISIBILITY) return TYPE_VISIBILITY;
    if (name == AttributesType::S_ALPHA)      return TYPE_ALPHA;
    if (name == AttributesType::S_TRANSLATION_X) return TYPE_TRANSLATION_X;
    if (name == AttributesType::S_TRANSLATION_Y) return TYPE_TRANSLATION_Y;
    if (name == AttributesType::S_TRANSLATION_Z) return TYPE_TRANSLATION_Z;
    if (name == AttributesType::S_ELEVATION)     return TYPE_ELEVATION;
    if (name == AttributesType::S_ROTATION_X)    return TYPE_ROTATION_X;
    if (name == AttributesType::S_ROTATION_Y)    return TYPE_ROTATION_Y;
    if (name == AttributesType::S_ROTATION_Z)    return TYPE_ROTATION_Z;
    if (name == AttributesType::S_SCALE_X)       return TYPE_SCALE_X;
    if (name == AttributesType::S_SCALE_Y)       return TYPE_SCALE_Y;
    if (name == AttributesType::S_PIVOT_X)       return TYPE_PIVOT_X;
    if (name == AttributesType::S_PIVOT_Y)       return TYPE_PIVOT_Y;
    if (name == AttributesType::S_PROGRESS)      return TYPE_PROGRESS;
    if (name == AttributesType::S_PATH_ROTATE)   return TYPE_PATH_ROTATE;
    if (name == AttributesType::S_EASING)        return TYPE_EASING;
    return -1;
}

int TypedValues::CycleType::getType(int name) {
    int t = AttributesType::getType(name);
    if (t != -1) {
        if (name == TYPE_WAVE_PERIOD || name == TYPE_WAVE_OFFSET || name == TYPE_WAVE_PHASE) return FLOAT_MASK;
        if (name == TYPE_WAVE_SHAPE) return STRING_MASK;
        return t;
    }
    return -1;
}

int TypedValues::TriggerType::getId(const std::string& name) {
    if (name == VIEW_TRANSITION_ON_CROSS)          return TYPE_VIEW_TRANSITION_ON_CROSS;
    if (name == VIEW_TRANSITION_ON_POSITIVE_CROSS) return TYPE_VIEW_TRANSITION_ON_POSITIVE_CROSS;
    if (name == VIEW_TRANSITION_ON_NEGATIVE_CROSS) return TYPE_VIEW_TRANSITION_ON_NEGATIVE_CROSS;
    if (name == POST_LAYOUT)        return TYPE_POST_LAYOUT;
    if (name == TRIGGER_SLACK)      return TYPE_TRIGGER_SLACK;
    if (name == TRIGGER_COLLISION_VIEW) return TYPE_TRIGGER_COLLISION_VIEW;
    if (name == TRIGGER_COLLISION_ID)  return TYPE_TRIGGER_COLLISION_ID;
    if (name == TRIGGER_ID)         return TYPE_TRIGGER_ID;
    if (name == POSITIVE_CROSS)     return TYPE_POSITIVE_CROSS;
    if (name == NEGATIVE_CROSS)     return TYPE_NEGATIVE_CROSS;
    if (name == TRIGGER_RECEIVER)   return TYPE_TRIGGER_RECEIVER;
    if (name == CROSS)              return TYPE_CROSS;
    return -1;
}

int TypedValues::PositionType::getId(const std::string& name) {
    if (name == S_TRANSITION_EASING) return TYPE_TRANSITION_EASING;
    if (name == S_DRAWPATH)          return TYPE_DRAWPATH;
    if (name == S_PERCENT_WIDTH)     return TYPE_PERCENT_WIDTH;
    if (name == S_PERCENT_HEIGHT)    return TYPE_PERCENT_HEIGHT;
    if (name == S_SIZE_PERCENT)      return TYPE_SIZE_PERCENT;
    if (name == S_PERCENT_X)         return TYPE_PERCENT_X;
    if (name == S_PERCENT_Y)         return TYPE_PERCENT_Y;
    return -1;
}

int TypedValues::PositionType::getType(int name) {
    switch (name) {
        case TYPE_CURVE_FIT: case TYPE_FRAME_POSITION: return INT_MASK;
        case TYPE_PERCENT_WIDTH: case TYPE_PERCENT_HEIGHT: case TYPE_SIZE_PERCENT:
        case TYPE_PERCENT_X: case TYPE_PERCENT_Y: return FLOAT_MASK;
        case TYPE_TRANSITION_EASING: case TYPE_TARGET: case TYPE_DRAWPATH: return STRING_MASK;
        default: return -1;
    }
}

int TypedValues::MotionType::getId(const std::string& name) {
    if (name == S_STAGGER)                    return TYPE_STAGGER;
    if (name == S_PATH_ROTATE)                return TYPE_PATH_ROTATE;
    if (name == S_QUANTIZE_MOTION_PHASE)      return TYPE_QUANTIZE_MOTION_PHASE;
    if (name == S_EASING)                     return TYPE_EASING;
    if (name == S_QUANTIZE_INTERPOLATOR)      return TYPE_QUANTIZE_INTERPOLATOR;
    if (name == S_ANIMATE_RELATIVE_TO)        return TYPE_ANIMATE_RELATIVE_TO;
    if (name == S_ANIMATE_CIRCLEANGLE_TO)     return TYPE_ANIMATE_CIRCLEANGLE_TO;
    if (name == S_PATHMOTION_ARC)             return TYPE_PATHMOTION_ARC;
    if (name == S_DRAW_PATH)                  return TYPE_DRAW_PATH;
    if (name == S_POLAR_RELATIVETO)           return TYPE_POLAR_RELATIVETO;
    if (name == S_QUANTIZE_MOTIONSTEPS)       return TYPE_QUANTIZE_MOTIONSTEPS;
    if (name == S_QUANTIZE_INTERPOLATOR_TYPE) return TYPE_QUANTIZE_INTERPOLATOR_TYPE;
    if (name == S_QUANTIZE_INTERPOLATOR_ID)   return TYPE_QUANTIZE_INTERPOLATOR_ID;
    return -1;
}

int TypedValues::Custom::getId(const std::string& name) {
    if (name == S_INT)       return TYPE_INT;
    if (name == S_FLOAT)     return TYPE_FLOAT;
    if (name == S_COLOR)     return TYPE_COLOR;
    if (name == S_STRING)    return TYPE_STRING;
    if (name == S_BOOLEAN)   return TYPE_BOOLEAN;
    if (name == S_DIMENSION) return TYPE_DIMENSION;
    if (name == S_REFERENCE) return TYPE_REFERENCE;
    return -1;
}

} // namespace cdroid
