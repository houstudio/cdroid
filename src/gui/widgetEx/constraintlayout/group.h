/*
 * Copyright (C) 2021 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Group.
 *
 * Controls the visibility of a set of referenced widgets. Unlike Barrier, Group owns no core
 * helper widget — it propagates its own visibility to the referenced views via applyLayoutFeatures.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_GROUP_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_GROUP_H

#include <widgetEx/constraintlayout/constrainthelper.h>

namespace cdroid {

class Group : public ConstraintHelper {
public:
    Group(Context* ctx, const AttributeSet& attrs);
    explicit Group(int width, int height);

    void setVisibility(int visibility) override;
    void onAttachedToWindow() override;

    void updatePostLayout(ConstraintLayout* container) override;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_GROUP_H
