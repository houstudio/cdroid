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
#ifndef __CDROID_PREFERENCE_IMAGE_VIEW_H__
#define __CDROID_PREFERENCE_IMAGE_VIEW_H__

#include <widget/imageview.h>

namespace cdroid {

/**
 * Port of androidx.preference.internal.PreferenceImageView — extension of
 * ImageView that correctly applies maxWidth and maxHeight. Registered under
 * the framework tag name com.android.internal.widget.PreferenceImageView
 * (that is the element used by the framework preference_material layouts).
 */
class PreferenceImageView : public ImageView {
public:
    PreferenceImageView(Context& context);
    PreferenceImageView(Context& context, const AttributeSet& attrs);
    PreferenceImageView(Context& context, const AttributeSet& attrs, int defStyleAttr);
    // InflaterRegister factory signature (widget-side pointer convention).
    PreferenceImageView(Context* context, const AttributeSet* attrs, int defStyleAttr = 0);

    // ImageView's accessors are non-virtual in the CDROID port; these hide
    // them (Java @Override has no C++ counterpart on a non-virtual base).
    void setMaxWidth(int maxWidth);
    int getMaxWidth() const;

    void setMaxHeight(int maxHeight);
    int getMaxHeight() const;

protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;

private:
    int mMaxWidthValue = 0x7FFFFFFF;
    int mMaxHeightValue = 0x7FFFFFFF;
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_IMAGE_VIEW_H__
