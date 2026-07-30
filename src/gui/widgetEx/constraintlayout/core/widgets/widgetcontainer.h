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
