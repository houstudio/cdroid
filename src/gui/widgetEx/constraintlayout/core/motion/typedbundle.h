/*
 * Copyright (C) 2020 The Android Open Source Project
 *
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
