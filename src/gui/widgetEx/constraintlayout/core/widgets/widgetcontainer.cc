/*
 * Copyright (C) 2016 The Android Open Source Project
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
