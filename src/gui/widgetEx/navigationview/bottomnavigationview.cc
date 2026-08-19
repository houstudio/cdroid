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
#include <widgetEx/navigationview/bottomnavigationview.h>
#include <widgetEx/widgetex_styleable.h>
#include <core/typedarray.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET(BottomNavigationView)

BottomNavigationView::BottomNavigationView(Context* context, const AttributeSet& attrs)
    : BottomNavigationView(context, &attrs, 0) {}

BottomNavigationView::BottomNavigationView(Context* context, const AttributeSet* attrs, int defStyleAttr)
    : NavigationBarView(context, attrs, defStyleAttr) {
    // BottomNavigationView styleable (0x02).
    auto ta = context->obtainStyledAttributes(attrs,
            cdroid::internal::R::styleable::BottomNavigationView, defStyleAttr);
    (void)ta;
    // android:minHeight comes through the framework attr by name.
    const int minHeight = attrs ? attrs->getAttributeIntValue(std::string(), "minHeight", 0) : 0;
    if (minHeight > 0) setMinimumHeight(minHeight);
    // itemHorizontalTranslationEnabled / window-inset dodging: not ported.
}

bool BottomNavigationView::onTouchEvent(MotionEvent& event) {
    FrameLayout::onTouchEvent(event);
    // AOSP: consume all events so views underneath never see them.
    return true;
}

void BottomNavigationView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // AOSP BottomNavigationView.onMeasure: honor minHeight when height isn't EXACT.
    const int minHeight = getSuggestedMinimumHeight();
    if (MeasureSpec::getMode(heightMeasureSpec) != MeasureSpec::EXACTLY && minHeight > 0) {
        const int want = minHeight + getPaddingTop() + getPaddingBottom();
        heightMeasureSpec = MeasureSpec::makeMeasureSpec(
                std::max(MeasureSpec::getSize(heightMeasureSpec), want), MeasureSpec::AT_MOST);
    }
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    if (MeasureSpec::getMode(heightMeasureSpec) != MeasureSpec::EXACTLY) {
        setMeasuredDimension(getMeasuredWidth(),
                std::max(getMeasuredHeight(),
                         getSuggestedMinimumHeight() + getPaddingTop() + getPaddingBottom()));
    }
}

}//namespace cdroid
