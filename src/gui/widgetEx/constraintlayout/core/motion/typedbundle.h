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
 *
 * A bundle of (type-id, value) pairs split by value kind (int/float/string/boolean). Used to carry
 * per-transition motion properties; applyDelta() pushes the pairs into a TypedValues target.
 * Java's doubling Arrays.copyOf arrays map to std::vector.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_TYPED_BUNDLE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_TYPED_BUNDLE_H

#include <string>
#include <vector>

namespace cdroid {

class TypedValues;

class TypedBundle {
  public:
    int getInteger(int type) const;

    void add(int type, int value);
    void add(int type, float value);
    void add(int type, const std::string& value);
    void addIfNotNull(int type, const std::string& value);
    void add(int type, bool value);

    void applyDelta(TypedValues& values) const;
    void applyDelta(TypedBundle& values) const;

    void clear();

  private:
    std::vector<int>         mTypeInt;
    std::vector<int>         mValueInt;
    std::vector<int>         mTypeFloat;
    std::vector<float>       mValueFloat;
    std::vector<int>         mTypeString;
    std::vector<std::string> mValueString;
    std::vector<int>         mTypeBoolean;
    std::vector<bool>        mValueBoolean;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_TYPED_BUNDLE_H
