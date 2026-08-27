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
#include <preference/internal/preferenceimageview.h>
#include <widget/framework_styleable.h>
#include <content/typedarray.h>

namespace cdroid {

PreferenceImageView::PreferenceImageView(Context& context)
    : PreferenceImageView(context, AttributeSet()) {
}

PreferenceImageView::PreferenceImageView(Context* context, const AttributeSet* attrs,
        int defStyleAttr)
    : ImageView(context, attrs, defStyleAttr) {
    namespace ns = internal::R::styleable;

    auto a = context->obtainStyledAttributes(attrs, ns::PreferenceImageView, defStyleAttr, 0);

    setMaxWidth(a->getDimensionPixelSize(ns::PreferenceImageView_maxWidth, 0x7FFFFFFF));
    setMaxHeight(a->getDimensionPixelSize(ns::PreferenceImageView_maxHeight, 0x7FFFFFFF));
}

PreferenceImageView::PreferenceImageView(Context& context, const AttributeSet& attrs)
    : PreferenceImageView(context, attrs, 0) {
}

PreferenceImageView::PreferenceImageView(Context& context, const AttributeSet& attrs,
        int defStyleAttr)
    : PreferenceImageView(&context, &attrs, defStyleAttr) {
}

void PreferenceImageView::setMaxWidth(int maxWidth) {
    mMaxWidthValue = maxWidth;
    ImageView::setMaxWidth(maxWidth);
}

int PreferenceImageView::getMaxWidth() const {
    return mMaxWidthValue;
}

void PreferenceImageView::setMaxHeight(int maxHeight) {
    mMaxHeightValue = maxHeight;
    ImageView::setMaxHeight(maxHeight);
}

int PreferenceImageView::getMaxHeight() const {
    return mMaxHeightValue;
}

void PreferenceImageView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    if (widthMode == MeasureSpec::AT_MOST || widthMode == MeasureSpec::UNSPECIFIED) {
        const int widthSize = MeasureSpec::getSize(widthMeasureSpec);
        const int maxWidth = getMaxWidth();
        if (maxWidth != 0x7FFFFFFF
                && (maxWidth < widthSize || widthMode == MeasureSpec::UNSPECIFIED)) {
            widthMeasureSpec = MeasureSpec::makeMeasureSpec(maxWidth, MeasureSpec::AT_MOST);
        }
    }

    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    if (heightMode == MeasureSpec::AT_MOST || heightMode == MeasureSpec::UNSPECIFIED) {
        const int heightSize = MeasureSpec::getSize(heightMeasureSpec);
        const int maxHeight = getMaxHeight();
        if (maxHeight != 0x7FFFFFFF
                && (maxHeight < heightSize || heightMode == MeasureSpec::UNSPECIFIED)) {
            heightMeasureSpec = MeasureSpec::makeMeasureSpec(maxHeight, MeasureSpec::AT_MOST);
        }
    }

    ImageView::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

} // namespace cdroid

// The framework preference_material layout references this view by its
// com.android.internal.widget name; LayoutInflater::getInflater strips the
// package prefix before the registry lookup, so the registration key is the
// bare class name (hand-written registration: the DECLARE_WIDGET macros
// token-paste the name into an identifier, which a dotted tag cannot provide).
static cdroid::InflaterRegister<cdroid::PreferenceImageView>
        widget_inflater_preference_image_view("PreferenceImageView", 0);
