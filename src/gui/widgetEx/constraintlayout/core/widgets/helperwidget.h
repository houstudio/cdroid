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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.HelperWidget.
 *
 * Base for virtual helpers that hold a list of referenced ConstraintWidgets (Barrier, Group's
 * core side, ...). Extends ConstraintWidget and implements the Helper interface (add /
 * removeAllIds / updateConstraints). The referenced-widget array grows geometrically, mirroring
 * Java's Arrays.copyOf doubling.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_HELPER_WIDGET_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_HELPER_WIDGET_H

#include <vector>

#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/helper.h>

namespace cdroid {

class ConstraintWidgetContainer;

class HelperWidget : public ConstraintWidget, public Helper {
  public:
    HelperWidget();
    ~HelperWidget() override;

    // The referenced widgets (Java: ConstraintWidget[] mWidgets, mWidgetsCount). Stored as a
    // vector for simplicity; add() mirrors the doubling-array growth semantics.
    std::vector<ConstraintWidget*> mWidgets;

    // --- Helper interface ---
    void updateConstraints(ConstraintWidgetContainer* container) override; // no-op base
    void add(ConstraintWidget* widget) override;
    void removeAllIds() override;

    void copy(ConstraintWidget* src,
              std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map) override;

    // Analyzer grouping hooks (OPTIMIZATION_GROUPING). Present for fidelity; the grouping pass is
    // deferred so these are not yet exercised by the MVP driver.
    void addDependents(std::vector<class WidgetGroup*>& dependencyLists, int orientation,
                       class WidgetGroup* group);
    int  findGroupInDependents(int orientation) const;

    std::string getType() const override; // "HelperWidget"
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_HELPER_WIDGET_H
