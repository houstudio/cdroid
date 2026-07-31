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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.WidgetContainer.
 */
#include <widgetEx/constraintlayout/core/widgets/widgetcontainer.h>
#include <widgetEx/constraintlayout/core/cache.h>

#include <algorithm>

namespace cdroid {

WidgetContainer::WidgetContainer() = default;

WidgetContainer::WidgetContainer(int x, int y, int width, int height)
    : ConstraintWidget(x, y, width, height) {
}

WidgetContainer::WidgetContainer(int width, int height)
    : ConstraintWidget(width, height) {
}

void WidgetContainer::reset() {
    mChildren.clear();
    ConstraintWidget::reset();
}

void WidgetContainer::add(ConstraintWidget* widget) {
    mChildren.push_back(widget);
    if (widget->getParent() != nullptr) {
        WidgetContainer* container = dynamic_cast<WidgetContainer*>(widget->getParent());
        if (container != nullptr) {
            container->remove(widget);
        }
    }
    widget->setParent(this);
}

void WidgetContainer::add(const std::vector<ConstraintWidget*>& widgets) {
    for (ConstraintWidget* widget : widgets) {
        add(widget);
    }
}

void WidgetContainer::remove(ConstraintWidget* widget) {
    auto it = std::find(mChildren.begin(), mChildren.end(), widget);
    if (it != mChildren.end()) {
        mChildren.erase(it);
    }
    widget->reset();
}

std::vector<ConstraintWidget*>& WidgetContainer::getChildren() {
    return mChildren;
}

ConstraintWidgetContainer* WidgetContainer::getRootConstraintContainer() const {
    // DEFERRED: needs ConstraintWidgetContainer complete (dynamic_cast up the parent chain).
    // Java walks item.getParent() casting each ConstraintWidgetContainer found; restore once
    // that class is ported.
    // TODO(container): walk mParent, dynamic_cast<ConstraintWidgetContainer*> at each step.
    return nullptr;
}

void WidgetContainer::setOffset(int x, int y) {
    ConstraintWidget::setOffset(x, y);
    // DEFERRED: Java propagates to children via widget.setOffset(getRootX(), getRootY()).
    // ConstraintWidget.getRootX/Y (root-relative position) are not yet ported; restore the
    // children loop when those land.
    // TODO(geom): for each child: child->setOffset(getRootX(), getRootY());
    (void)mChildren;
}

void WidgetContainer::layout() {
    if (mChildren.empty()) {
        return;
    }
    for (ConstraintWidget* widget : mChildren) {
        WidgetContainer* container = dynamic_cast<WidgetContainer*>(widget);
        if (container != nullptr) {
            container->layout();
        }
    }
}

void WidgetContainer::resetSolverVariables(Cache* cache) {
    ConstraintWidget::resetSolverVariables(cache);
    for (ConstraintWidget* widget : mChildren) {
        widget->resetSolverVariables(cache);
    }
}

void WidgetContainer::removeAllChildren() {
    mChildren.clear();
}

} // namespace cdroid
