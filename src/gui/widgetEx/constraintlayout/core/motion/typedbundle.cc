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

void TypedBundle::add(int type, int value)          {
    mTypeInt.push_back(type);
    mValueInt.push_back(value);
}
void TypedBundle::add(int type, float value)        {
    mTypeFloat.push_back(type);
    mValueFloat.push_back(value);
}
void TypedBundle::add(int type, const std::string& value) {
    mTypeString.push_back(type);
    mValueString.push_back(value);
}
void TypedBundle::addIfNotNull(int type, const std::string& value) {
    if (!value.empty()) add(type, value);
}
void TypedBundle::add(int type, bool value)         {
    mTypeBoolean.push_back(type);
    mValueBoolean.push_back(value);
}

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
    mTypeInt.clear();
    mValueInt.clear();
    mTypeFloat.clear();
    mValueFloat.clear();
    mTypeString.clear();
    mValueString.clear();
    mTypeBoolean.clear();
    mValueBoolean.clear();
}

} // namespace cdroid
