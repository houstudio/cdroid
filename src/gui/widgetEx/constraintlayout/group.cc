/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Group.
 */
#include <widgetEx/constraintlayout/group.h>

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
