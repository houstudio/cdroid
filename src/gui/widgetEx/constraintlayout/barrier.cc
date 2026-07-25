/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Barrier.
 */
#include <widgetEx/constraintlayout/barrier.h>

#include <porting/cdlog.h>
#include <view/view.h>

DECLARE_WIDGET(Barrier)

namespace cdroid {

// out-of-line definitions (odr-used as map values / runtime args)
constexpr int Barrier::LEFT;
constexpr int Barrier::TOP;
constexpr int Barrier::RIGHT;
constexpr int Barrier::BOTTOM;
constexpr int Barrier::START;
constexpr int Barrier::END;

namespace {
// barrierDirection enum (XML string) -> int. Matches Android's enum order plus start/end.
const std::unordered_map<std::string, int> kBarrierDirection = {
    {"left", Barrier::LEFT}, {"top", Barrier::TOP},
    {"right", Barrier::RIGHT}, {"bottom", Barrier::BOTTOM},
    {"start", Barrier::START}, {"end", Barrier::END}
};
} // namespace

Barrier::Barrier(Context* ctx, const AttributeSet& attrs)
    : ConstraintHelper(ctx, attrs) {
    setVisibility(View::GONE);
    mHelperWidget = std::make_unique<clcore::Barrier>();

    int dir = attrs.getInt("barrierDirection", kBarrierDirection, LEFT);
    setType(dir);
    static_cast<clcore::Barrier*>(mHelperWidget.get())
            ->setAllowsGoneWidget(attrs.getBoolean("barrierAllowsGoneWidgets", true));
    int margin = attrs.getDimensionPixelSize("barrierMargin", 0);
    static_cast<clcore::Barrier*>(mHelperWidget.get())->setMargin(margin);

    // Resolve the indicated type against the (deferred) RTL flag. LTR: START->LEFT, END->RIGHT.
    updateType(mHelperWidget.get(), mIndicatedType, /*isRtl=*/false);
    validateParams();
}

Barrier::Barrier(int width, int height)
    : ConstraintHelper(width, height) {
    setVisibility(View::GONE);
    mHelperWidget = std::make_unique<clcore::Barrier>();
    setType(LEFT);
    updateType(mHelperWidget.get(), mIndicatedType, /*isRtl=*/false);
    validateParams();
}

int Barrier::getType() const {
    return mIndicatedType;
}

void Barrier::setType(int type) {
    mIndicatedType = type;
    updateType(mHelperWidget.get(), mIndicatedType, /*isRtl=*/false);
}

void Barrier::updateType(ConstraintWidget* widget, int type, bool /*isRtl*/) {
    mResolvedType = type;
    // CDROID is LTR-only for now (no JB-MR1 RTL resolution). START -> LEFT, END -> RIGHT.
    if (mIndicatedType == START) {
        mResolvedType = LEFT;
    } else if (mIndicatedType == END) {
        mResolvedType = RIGHT;
    }
    if (auto* barrier = dynamic_cast<clcore::Barrier*>(widget)) {
        barrier->setBarrierType(mResolvedType);
    }
}

void Barrier::resolveRtl(ConstraintWidget* widget, bool isRtl) {
    updateType(widget, mIndicatedType, isRtl);
}

bool Barrier::getAllowsGoneWidget() const {
    return static_cast<clcore::Barrier*>(mHelperWidget.get())->getAllowsGoneWidget();
}

void Barrier::setAllowsGoneWidget(bool supportGone) {
    static_cast<clcore::Barrier*>(mHelperWidget.get())->setAllowsGoneWidget(supportGone);
}

int Barrier::getMargin() const {
    return static_cast<clcore::Barrier*>(mHelperWidget.get())->getMargin();
}

void Barrier::setMargin(int margin) {
    static_cast<clcore::Barrier*>(mHelperWidget.get())->setMargin(margin);
}

void Barrier::setDpMargin(int margin) {
    // TODO: multiply by display density. CDROID embedded targets are often px-oriented; treat as px.
    setMargin(margin);
}

} // namespace cdroid
