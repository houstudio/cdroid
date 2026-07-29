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
 */
#include <widgetEx/constraintlayout/core/widgets/helperwidget.h>

#include <porting/cdlog.h>

namespace cdroid {

// Forward-declared analyzer grouping types (OPTIMIZATION_GROUPING, deferred). Present only so the
// addDependents/findGroupInDependents signatures compile; their bodies are stubbed.
class WidgetGroup {};

HelperWidget::HelperWidget() = default;
HelperWidget::~HelperWidget() = default;

void HelperWidget::updateConstraints(ConstraintWidgetContainer* /*container*/) {
    // nothing here (base)
}

void HelperWidget::add(ConstraintWidget* widget) {
    if (widget == this || widget == nullptr) {
        return;
    }
    mWidgets.push_back(widget);
}

void HelperWidget::removeAllIds() {
    mWidgets.clear();
}

void HelperWidget::copy(ConstraintWidget* src,
                        std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map) {
    ConstraintWidget::copy(src, map);
    auto* srcHelper = static_cast<HelperWidget*>(src);
    mWidgets.clear();
    for (ConstraintWidget* w : srcHelper->mWidgets) {
        auto it = map.find(w);
        if (it != map.end()) {
            add(it->second);
        }
    }
}

void HelperWidget::addDependents(std::vector<WidgetGroup*>& /*dependencyLists*/, int /*orientation*/,
                                 WidgetGroup* /*group*/) {
    // OPTIMIZATION_GROUPING deferred. Java iterates mWidgets adding each to `group`, then calls
    // Grouping.findDependents per widget. Restored when the grouping pass is ported.
}

int HelperWidget::findGroupInDependents(int /*orientation*/) const {
    // OPTIMIZATION_GROUPING deferred.
    return -1;
}

std::string HelperWidget::getType() const {
    return "HelperWidget";
}

} // namespace cdroid
