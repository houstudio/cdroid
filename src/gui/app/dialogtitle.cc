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
#include <app/dialogtitle.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <view/layoutinflater.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET(DialogTitle);

DialogTitle::DialogTitle(Context* context):TextView(context,nullptr,0){
}

DialogTitle::DialogTitle(Context* context,const AttributeSet* attrs)
  :DialogTitle(context,attrs,0,0){
}

DialogTitle::DialogTitle(Context* context,const AttributeSet* attrs,int defStyleAttr,int defStyleRes)
  :TextView(context,attrs,defStyleAttr){
    (void)defStyleRes;   // TextView has no 4-arg ctor yet
}

void DialogTitle::onMeasure(int widthMeasureSpec,int heightMeasureSpec){
    TextView::onMeasure(widthMeasureSpec,heightMeasureSpec);

    Layout* layout = getLayout();
    if (layout != nullptr) {
        const int lineCount = layout->getLineCount();
        if (lineCount > 0) {
            const int ellipsisCount = layout->getEllipsisCount(lineCount - 1);
            if (ellipsisCount > 0) {
                setSingleLine(false);
                setMaxLines(2);

                auto a = mContext->obtainStyledAttributes(nullptr,
                        R::styleable::TextAppearance, R::attr::textAppearanceMedium,
                        R::style::TextAppearance_Medium);
                const int textSize = a->getDimensionPixelSize(
                        R::styleable::TextAppearance_textSize, 0);
                if (textSize != 0) {
                    // textSize is already expressed in pixels
                    setTextSize(TypedValue::COMPLEX_UNIT_PX, (float)textSize);
                }

                TextView::onMeasure(widthMeasureSpec,heightMeasureSpec);
            }
        }
    }
}
};//endof namespace
