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
#ifndef __DIVIDER_ITEM_DECORATION_H__
#define __DIVIDER_ITEM_DECORATION_H__
#include <widgetEx/recyclerview/recyclerview.h>

namespace cdroid{

class DividerItemDecoration:public RecyclerView::ItemDecoration {
public:
    static constexpr int HORIZONTAL = LinearLayout::HORIZONTAL;
    static constexpr int VERTICAL = LinearLayout::VERTICAL;
private:
    Drawable* mDivider;
    int mOrientation;
    Rect mBounds;
private:
    void drawVertical(Canvas& canvas, RecyclerView& parent);
    void drawHorizontal(Canvas& canvas, RecyclerView& parent);
public:
    DividerItemDecoration(Context* context, int orientation);
    ~DividerItemDecoration()override;
    void setOrientation(int orientation);
    void setDrawable(Drawable* drawable);
    Drawable*getDrawable()const;
    void onDraw(Canvas& c, RecyclerView& parent, RecyclerView::State& state)override;
    void getItemOffsets(Rect& outRect, View& view, RecyclerView& parent,RecyclerView::State& state)override;
};
}
#endif
