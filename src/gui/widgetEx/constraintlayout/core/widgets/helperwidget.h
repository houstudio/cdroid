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
