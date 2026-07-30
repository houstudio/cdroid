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
 * Ported to C++ for CDROID from androidx.constraintlayout.widget.Placeholder.
 *
 * A virtual object that positions an existing content view. When a content id is set, the
 * placeholder takes the content's size and the content view is drawn at the placeholder's frame
 * (treated as gone at its original location). The placeholder itself is otherwise invisible.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_PLACEHOLDER_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_PLACEHOLDER_H

#include <view/view.h>
#include <core/attributeset.h>

namespace cdroid {

class ConstraintLayout;

class Placeholder : public View {
  public:
    Placeholder(Context* ctx, const AttributeSet& attrs);
    explicit Placeholder(int width, int height);

    int  getEmptyVisibility() const;
    void setEmptyVisibility(int visibility);
    View* getContent() const {
        return mContent;
    }

    // Set the content view by id (the content is looked up among the placeholder's siblings).
    void setContentId(int id);
    int  getContentId() const {
        return mContentId;
    }

    // --- lifecycle hooks (invoked by ConstraintLayout) ---
    void updatePreLayout(ConstraintLayout* container);
    void updatePostMeasure(ConstraintLayout* container);

  private:
    void init(const AttributeSet& attrs);

    int   mContentId = -1;
    View* mContent = nullptr;
    int   mEmptyVisibility = View::INVISIBLE;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_PLACEHOLDER_H
