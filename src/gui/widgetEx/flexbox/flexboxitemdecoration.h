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
#ifndef __FLEXBOX_ITEM_DECORATION_H__
#define __FLEXBOX_ITEM_DECORATION_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/flexbox/flexboxlayoutmanager.h>

namespace cdroid{

/**
 * {@link RecyclerView::ItemDecoration} implementation that can be used as item decorations between
 * view holders within the {@link FlexboxLayoutManager}.
 *
 * Orientation for the decoration can be either of:
 * <ul>
 * <li> Horizontal (setOrientation(HORIZONTAL)</li>
 * <li> Vertical (setOrientation(VERTICAL)</li>
 * <li> Both orientation (setOrientation(BOTH)</li>
 * </ul>
 * The default value is set to both.
 */
class FlexboxItemDecoration:public RecyclerView::ItemDecoration {
public:
    static constexpr int HORIZONTAL = 1;
    static constexpr int VERTICAL = 1 << 1;
    static constexpr int BOTH = HORIZONTAL | VERTICAL;
private:
    Drawable* mDrawable;
    int mOrientation;
private:
    void drawHorizontalDecorations(Canvas& canvas, RecyclerView& parent);
    void drawVerticalDecorations(Canvas& canvas, RecyclerView& parent);
    void setOffsetAlongCrossAxis(Rect& outRect, int position,
            FlexboxLayoutManager* layoutManager, std::vector<FlexLine>& flexLines);
    void setOffsetAlongMainAxis(Rect& outRect, int position,
            FlexboxLayoutManager* layoutManager, std::vector<FlexLine>& flexLines, int flexDirection);
    bool isFirstItemInLine(int position, std::vector<FlexLine>& flexLines,
            FlexboxLayoutManager* layoutManager);
    bool needsHorizontalDecoration();
    bool needsVerticalDecoration();
public:
    FlexboxItemDecoration(Context* context);
    ~FlexboxItemDecoration()override;

    /** Set the drawable used as the item decoration. */
    void setDrawable(Drawable* drawable);
    /** Set the orientation for the decoration. */
    void setOrientation(int orientation);

    void onDraw(Canvas& c, RecyclerView& parent, RecyclerView::State& state)override;
    void getItemOffsets(Rect& outRect, View& view, RecyclerView& parent,RecyclerView::State& state)override;
};

}/*endof namespace*/
#endif/*__FLEXBOX_ITEM_DECORATION_H__*/
