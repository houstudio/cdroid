/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Placeholder.
 */
#include <widgetEx/constraintlayout/placeholder.h>

#include <porting/cdlog.h>
#include <view/view.h>
#include <widgetEx/constraintlayout/constraintlayout.h>

DECLARE_WIDGET(Placeholder)

namespace cdroid {

Placeholder::Placeholder(Context* ctx, const AttributeSet& attrs)
    : View(ctx, attrs) {
    init(attrs);
}

Placeholder::Placeholder(int width, int height)
    : View(width, height) {
    setVisibility(mEmptyVisibility);
    mContentId = -1;
}

void Placeholder::init(const AttributeSet& attrs) {
    setVisibility(mEmptyVisibility);
    mContentId = attrs.getResourceId("content", -1);
    int emptyVis = attrs.getInt("placeholder_emptyVisibility",std::unordered_map<std::string,int>{
        {"visible", View::VISIBLE},
        { "invisible",View::INVISIBLE },
        {"gone", View::GONE}
    }, mEmptyVisibility);
    if (emptyVis == View::VISIBLE || emptyVis == View::INVISIBLE || emptyVis == View::GONE) {
        mEmptyVisibility = emptyVis;
        setVisibility(mEmptyVisibility);
    }
}

int Placeholder::getEmptyVisibility() const {
    return mEmptyVisibility;
}

void Placeholder::setEmptyVisibility(int visibility) {
    mEmptyVisibility = visibility;
}

void Placeholder::setContentId(int id) {
    if (mContentId == id) {
        return;
    }
    if (mContent != nullptr) {
        mContent->setVisibility(View::VISIBLE);
        if (auto* lp = dynamic_cast<ConstraintLayout::LayoutParams*>(mContent->getLayoutParams())) {
            lp->mIsInPlaceholder = false;
        }
        mContent = nullptr;
    }
    mContentId = id;
    if (id != ConstraintLayout::LayoutParams::UNSET) {
        View* parent = getParent();
        if (parent != nullptr) {
            View* v = parent->findViewById(id);
            if (v != nullptr) {
                v->setVisibility(View::GONE);
            }
        }
    }
}

void Placeholder::updatePreLayout(ConstraintLayout* container) {
    if (mContentId == -1) {
        setVisibility(mEmptyVisibility);
        return;
    }
    mContent = (container != nullptr) ? container->findViewById(mContentId) : nullptr;
    if (mContent != nullptr) {
        if (auto* lp = dynamic_cast<ConstraintLayout::LayoutParams*>(mContent->getLayoutParams())) {
            lp->mIsInPlaceholder = true;
        }
        mContent->setVisibility(View::VISIBLE);
        setVisibility(View::VISIBLE);
    }
}

void Placeholder::updatePostMeasure(ConstraintLayout* /*container*/) {
    if (mContent == nullptr) {
        return;
    }
    auto* lp = dynamic_cast<ConstraintLayout::LayoutParams*>(getLayoutParams());
    auto* lpContent = dynamic_cast<ConstraintLayout::LayoutParams*>(mContent->getLayoutParams());
    if (lp == nullptr || lpContent == nullptr || lp->mWidget == nullptr || lpContent->mWidget == nullptr) {
        return;
    }
    lpContent->mWidget->setVisibility(View::VISIBLE);
    // Adopt the content's resolved size unless the placeholder is FIXED on that axis.
    if (lp->mWidget->getHorizontalDimensionBehaviour()
            != ConstraintWidget::DimensionBehaviour::FIXED) {
        lp->mWidget->setWidth(lpContent->mWidget->getWidth());
    }
    if (lp->mWidget->getVerticalDimensionBehaviour()
            != ConstraintWidget::DimensionBehaviour::FIXED) {
        lp->mWidget->setHeight(lpContent->mWidget->getHeight());
    }
    lpContent->mWidget->setVisibility(View::GONE);
}

} // namespace cdroid
