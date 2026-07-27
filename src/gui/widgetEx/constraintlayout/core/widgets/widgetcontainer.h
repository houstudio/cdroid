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
 * A container of ConstraintWidget (children list + recursive layout). The actual solver
 * driver lives in the subclass ConstraintWidgetContainer (Stage 3 analyzer coupling).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_WIDGET_CONTAINER_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_WIDGET_CONTAINER_H

#include <vector>

#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

namespace cdroid {

class ConstraintWidgetContainer;

class WidgetContainer : public ConstraintWidget {
  public:
    std::vector<ConstraintWidget*> mChildren;

    WidgetContainer();
    WidgetContainer(int x, int y, int width, int height);
    WidgetContainer(int width, int height);

    void reset() override;

    void add(ConstraintWidget* widget);
    void add(const std::vector<ConstraintWidget*>& widgets); // Java varargs add(ConstraintWidget...)
    void remove(ConstraintWidget* widget);
    std::vector<ConstraintWidget*>& getChildren();

    // Returns the top-level ConstraintWidgetContainer up the parent chain.
    ConstraintWidgetContainer* getRootConstraintContainer() const;

    void setOffset(int x, int y) override;

    // Recursively layout child containers. Overridden by ConstraintWidgetContainer to do the
    // real solver pass.
    virtual void layout();

    void resetSolverVariables(Cache* cache) override;

    void removeAllChildren();
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_WIDGET_CONTAINER_H
