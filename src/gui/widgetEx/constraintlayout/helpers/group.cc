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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Group.
 */
#include <widgetEx/constraintlayout/helpers/group.h>

#include <porting/cdlog.h>
#include <view/view.h>
#include <widgetEx/constraintlayout/constraintlayout.h>

DECLARE_WIDGET(Group)

namespace cdroid {

Group::Group(Context* ctx, const AttributeSet& attrs)
    : ConstraintHelper(ctx, attrs) {
    mUseViewMeasure = false;
}

Group::Group(int width, int height)
    : ConstraintHelper(width, height) {
    mUseViewMeasure = false;
}

void Group::setVisibility(int visibility) {
    ConstraintHelper::setVisibility(visibility);
    applyLayoutFeatures();
}

void Group::onAttachedToWindow() {
    View::onAttachedToWindow();
    applyLayoutFeatures();
}

void Group::updatePostLayout(ConstraintLayout* /*container*/) {
    // The Group itself is a zero-sized anchor in the solver (Java: params.mWidget.setWidth/Height(0)).
    auto* lp = dynamic_cast<ConstraintLayout::LayoutParams*>(getLayoutParams());
    if (lp != nullptr && lp->mWidget) {
        lp->mWidget->setWidth(0);
        lp->mWidget->setHeight(0);
    }
}

} // namespace cdroid
