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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.VirtualLayout.
 */
#include <widgetEx/constraintlayout/helpers/virtuallayout.h>

#include <widgetEx/constraintlayout/constraintlayout.h>

namespace cdroid {

VirtualLayout::VirtualLayout(Context* ctx)
    : VirtualLayout(ctx, nullptr) {
}

VirtualLayout::VirtualLayout(Context* ctx, const AttributeSet* attrs)
    : VirtualLayout(ctx, attrs, 0) {
}

VirtualLayout::VirtualLayout(Context* ctx, const AttributeSet* attrs, int defStyleAttr)
    : ConstraintHelper(ctx, attrs, defStyleAttr) {
}

void VirtualLayout::init(const AttributeSet* attrs) {
    ConstraintHelper::init(attrs);
    if (attrs == nullptr) return;
    // AndroidX scans the ConstraintLayout_Layout styleable for android:visibility/elevation and
    // remembers that they were set (VirtualLayout.java:46-63). The framework android:* attrs are
    // not in CDROID's styleable, so probe the raw parser — same "was the attr present" test.
    if (!attrs->getAttributeValue("http://schemas.android.com/apk/res/android", "visibility").empty()) {
        mApplyVisibilityOnAttach = true;
    }
    if (!attrs->getAttributeValue("http://schemas.android.com/apk/res/android", "elevation").empty()) {
        mApplyElevationOnAttach = true;
    }
}

void VirtualLayout::onMeasure(clcore::VirtualLayout* /*layout*/,
                              int /*widthMeasureSpec*/, int /*heightMeasureSpec*/) {
    // nothing (VirtualLayout.java:71-75)
}

void VirtualLayout::onAttachedToWindow() {
    ConstraintHelper::onAttachedToWindow();
    if (mApplyVisibilityOnAttach || mApplyElevationOnAttach) {
        if (auto* container = dynamic_cast<ConstraintLayout*>(getParent())) {
            const int visibility = getVisibility();
            const float elevation = getElevation();
            for (int id : mIds) {
                View* view = container->getViewById(id);
                if (view != nullptr) {
                    if (mApplyVisibilityOnAttach) {
                        view->setVisibility(visibility);
                    }
                    if (mApplyElevationOnAttach && elevation > 0) {
                        view->setTranslationZ(view->getTranslationZ() + elevation);
                    }
                }
            }
        }
    }
}

void VirtualLayout::setVisibility(int visibility) {
    View::setVisibility(visibility);
    applyLayoutFeatures();
}

void VirtualLayout::setElevation(float elevation) {
    View::setElevation(elevation);
    applyLayoutFeatures();
}

} // namespace cdroid
