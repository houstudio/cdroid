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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0211-1301  USA
 *********************************************************************************/
#include <app/buttonbarlayout.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <view/layoutinflater.h>
#include <view/view.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET(ButtonBarLayout);

ButtonBarLayout::ButtonBarLayout(Context* context,const AttributeSet* attrs)
  :ButtonBarLayout(context, attrs,0){
}

ButtonBarLayout::ButtonBarLayout(Context* context,const AttributeSet* attrs,int defStyleAttr)
  :LinearLayout(context,attrs,defStyleAttr){
    auto ta = context->obtainStyledAttributes(attrs, R::styleable::ButtonBarLayout);
    mAllowStacking = ta->getBoolean(R::styleable::ButtonBarLayout_allowStacking, true);
}

void ButtonBarLayout::setAllowStacking(bool allowStacking){
    if (mAllowStacking != allowStacking) {
        mAllowStacking = allowStacking;
        if (!mAllowStacking && getOrientation() == LinearLayout::VERTICAL) {
            setStacked(false);
        }
        requestLayout();
    }
}

void ButtonBarLayout::onMeasure(int widthMeasureSpec,int heightMeasureSpec){
    const int widthSize = MeasureSpec::getSize(widthMeasureSpec);

    if (mAllowStacking) {
        if (widthSize > mLastWidthSize && isStacked()) {
            // We're being measured wider this time, try un-stacking.
            setStacked(false);
        }

        mLastWidthSize = widthSize;
    }

    bool needsRemeasure = false;

    // If we're not stacked, make sure the measure spec is AT_MOST rather
    // than EXACTLY. This ensures that we'll still get TOO_SMALL so that we
    // know to stack the buttons.
    int initialWidthMeasureSpec;
    if (!isStacked() && MeasureSpec::getMode(widthMeasureSpec) == MeasureSpec::EXACTLY) {
        initialWidthMeasureSpec = MeasureSpec::makeMeasureSpec(widthSize, MeasureSpec::AT_MOST);

        // We'll need to remeasure again to fill excess space.
        needsRemeasure = true;
    } else {
        initialWidthMeasureSpec = widthMeasureSpec;
    }

    LinearLayout::onMeasure(initialWidthMeasureSpec, heightMeasureSpec);

    if (mAllowStacking && !isStacked()) {
        const int measuredWidth = getMeasuredWidthAndState();
        const int measuredWidthState = measuredWidth & View::MEASURED_STATE_MASK;
        if (measuredWidthState == View::MEASURED_STATE_TOO_SMALL) {
            setStacked(true);

            // Measure again in the new orientation.
            needsRemeasure = true;
        }
    }

    if (needsRemeasure) {
        LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    }

    // Compute minimum height such that, when stacked, some portion of the
    // second button is visible.
    int minHeight = 0;
    const int firstVisible = getNextVisibleChildIndex(0);
    if (firstVisible >= 0) {
        const View* firstButton = getChildAt(firstVisible);
        const LinearLayout::LayoutParams* firstParams =
                (const LinearLayout::LayoutParams*)firstButton->getLayoutParams();
        minHeight += getPaddingTop() + firstButton->getMeasuredHeight()
                + firstParams->topMargin + firstParams->bottomMargin;
        if (isStacked()) {
            const int secondVisible = getNextVisibleChildIndex(firstVisible + 1);
            if (secondVisible >= 0) {
                minHeight += getChildAt(secondVisible)->getPaddingTop()
                        + PEEK_BUTTON_DP * getResources().getDisplayMetrics().density;
            }
        } else {
            minHeight += getPaddingBottom();
        }
    }

    if (getMinimumHeight() != minHeight) {
        setMinimumHeight(minHeight);
    }
}

int ButtonBarLayout::getNextVisibleChildIndex(int index){
    for (int i = index, count = (int)getChildCount(); i < count; i++) {
        if (getChildAt(i)->getVisibility() == View::VISIBLE) {
            return i;
        }
    }
    return -1;
}

int ButtonBarLayout::getMinimumHeight(){
    return std::max(mMinimumHeight, LinearLayout::getMinimumHeight());
}

void ButtonBarLayout::setStacked(bool stacked){
    setOrientation(stacked ? LinearLayout::VERTICAL : LinearLayout::HORIZONTAL);
    setGravity(stacked ? Gravity::END : Gravity::BOTTOM);

    View* spacer = findViewById(R::id::spacer);
    if (spacer != nullptr) {
        spacer->setVisibility(stacked ? View::GONE : View::INVISIBLE);
    }

    // Reverse the child order. This is specific to the Material button
    // bar's layout XML and will probably not generalize.
    const int childCount = (int)getChildCount();
    for (int i = childCount - 2; i >= 0; i--) {
        bringChildToFront(getChildAt(i));
    }
}

bool ButtonBarLayout::isStacked()const{
    return getOrientation() == LinearLayout::VERTICAL;
}
};//endof namespace

// The material alert layout carries the internal FQCN com.android.internal
// .widget.ButtonBarLayout; the inflater matches tags on their last '.'-segment,
// so registering the bare name serves both spellings.
static const bool _cdroid_reg_buttonbarlayout =
    cdroid::LayoutInflater::registerInflater("ButtonBarLayout", 0,
        [](cdroid::Context* ctx, const cdroid::AttributeSet& attrs) -> cdroid::View* {
            return new cdroid::ButtonBarLayout(ctx, &attrs);
        });
