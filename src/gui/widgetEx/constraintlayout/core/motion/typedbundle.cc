/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.TypedBundle.
 */
#include <widgetEx/constraintlayout/core/motion/typedbundle.h>

#include <widgetEx/constraintlayout/core/motion/typedvalues.h>

namespace cdroid {

int TypedBundle::getInteger(int type) const {
    for (size_t i = 0; i < mTypeInt.size(); i++) {
        if (mTypeInt[i] == type) return mValueInt[i];
    }
    return -1;
}

void TypedBundle::add(int type, int value)          { mTypeInt.push_back(type);     mValueInt.push_back(value); }
void TypedBundle::add(int type, float value)        { mTypeFloat.push_back(type);   mValueFloat.push_back(value); }
void TypedBundle::add(int type, const std::string& value) { mTypeString.push_back(type); mValueString.push_back(value); }
void TypedBundle::addIfNotNull(int type, const std::string& value) { if (!value.empty()) add(type, value); }
void TypedBundle::add(int type, bool value)         { mTypeBoolean.push_back(type); mValueBoolean.push_back(value); }

void TypedBundle::applyDelta(TypedValues& values) const {
    for (size_t i = 0; i < mTypeInt.size(); i++)     values.setValue(mTypeInt[i], mValueInt[i]);
    for (size_t i = 0; i < mTypeFloat.size(); i++)   values.setValue(mTypeFloat[i], mValueFloat[i]);
    for (size_t i = 0; i < mTypeString.size(); i++)  values.setValue(mTypeString[i], mValueString[i]);
    for (size_t i = 0; i < mTypeBoolean.size(); i++) values.setValue(mTypeBoolean[i], mValueBoolean[i]);
}

void TypedBundle::applyDelta(TypedBundle& values) const {
    for (size_t i = 0; i < mTypeInt.size(); i++)     values.add(mTypeInt[i], mValueInt[i]);
    for (size_t i = 0; i < mTypeFloat.size(); i++)   values.add(mTypeFloat[i], mValueFloat[i]);
    for (size_t i = 0; i < mTypeString.size(); i++)  values.add(mTypeString[i], mValueString[i]);
    for (size_t i = 0; i < mTypeBoolean.size(); i++) values.add(mTypeBoolean[i], mValueBoolean[i]);
}

void TypedBundle::clear() {
    mTypeInt.clear(); mValueInt.clear();
    mTypeFloat.clear(); mValueFloat.clear();
    mTypeString.clear(); mValueString.clear();
    mTypeBoolean.clear(); mValueBoolean.clear();
}

} // namespace cdroid
