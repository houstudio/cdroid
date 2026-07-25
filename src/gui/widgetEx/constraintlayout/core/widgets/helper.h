/*
 * Copyright (C) 2018 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Helper.
 * Header-only: interface to virtual objects (Guideline/Barrier/Group/...).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_HELPER_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_HELPER_H

namespace cdroid {

class ConstraintWidget;
class ConstraintWidgetContainer;

/** Interface to virtual objects */
class Helper {
public:
    virtual ~Helper() = default;
    virtual void updateConstraints(ConstraintWidgetContainer* container) = 0;
    virtual void add(ConstraintWidget* widget) = 0;
    virtual void removeAllIds() = 0;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_HELPER_H
