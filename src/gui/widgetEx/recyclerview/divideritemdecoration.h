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
