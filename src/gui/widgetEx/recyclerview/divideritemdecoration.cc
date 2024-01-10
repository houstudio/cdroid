#include <widgetEx/recyclerview/divideritemdecoration.h>
namespace cdroid{

DividerItemDecoration::DividerItemDecoration(Context* context, int orientation) {
    //final TypedArray a = context.obtainStyledAttributes(ATTRS);
    //mDivider = a.getDrawable(0);
    mDivider = nullptr;
    LOGW_IF(mDivider == nullptr,"@android:attr/listDivider was not set in the theme used for this "
               "DividerItemDecoration. Please set that attribute all call setDrawable()");
    setOrientation(orientation);
}

void DividerItemDecoration::setOrientation(int orientation) {
    if (orientation != HORIZONTAL && orientation != VERTICAL) {
        FATAL("Invalid orientation. It should be either HORIZONTAL or VERTICAL");
    }
    mOrientation = orientation;
}

void DividerItemDecoration::setDrawable(Drawable* drawable) {
    FATAL_IF(drawable==nullptr,"Drawable cannot be null.");
    mDivider = drawable;
}

void DividerItemDecoration::onDraw(Canvas& c, RecyclerView& parent, RecyclerView::State& state) {
    if (parent.getLayoutManager() == nullptr || mDivider == nullptr) {
        return;
    }
    if (mOrientation == VERTICAL) {
        drawVertical(c, parent);
    } else {
        drawHorizontal(c, parent);
    }
}

void DividerItemDecoration::drawVertical(Canvas& canvas, RecyclerView& parent) {
    canvas.save();
    int left, right;
    //noinspection AndroidLintNewApi - NewApi lint fails to handle overrides.
    if (parent.getClipToPadding()) {
        left = parent.getPaddingLeft();
        right = parent.getWidth() - parent.getPaddingRight();
        canvas.rectangle(left, parent.getPaddingTop(), right-left,
            parent.getHeight() -parent.getPaddingTop()- parent.getPaddingBottom());
        canvas.clip();
    } else {
        left = 0;
        right = parent.getWidth();
    }

    const int childCount = parent.getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = parent.getChildAt(i);
        parent.getDecoratedBoundsWithMargins(child, mBounds);
        const int bottom = mBounds.bottom() + std::round(child->getTranslationY());
        const int top = bottom - mDivider->getIntrinsicHeight();
        mDivider->setBounds(left, top, right-left, bottom-top);
        mDivider->draw(canvas);
    }
    canvas.restore();
}

void DividerItemDecoration::drawHorizontal(Canvas& canvas, RecyclerView& parent) {
    canvas.save();
    int top,bottom;
    //noinspection AndroidLintNewApi - NewApi lint fails to handle overrides.
    if (parent.getClipToPadding()) {
        top = parent.getPaddingTop();
        bottom = parent.getHeight() - parent.getPaddingBottom();
        canvas.rectangle(parent.getPaddingLeft(), top,
            parent.getWidth() - parent.getPaddingLeft() - parent.getPaddingRight(), bottom-top);
        canvas.clip();
    } else {
        top = 0;
        bottom = parent.getHeight();
    }

    const int childCount = parent.getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = parent.getChildAt(i);
        parent.getLayoutManager()->getDecoratedBoundsWithMargins(child, mBounds);
        const int right = mBounds.right() + std::round(child->getTranslationX());
        const int left = right - mDivider->getIntrinsicWidth();
        mDivider->setBounds(left, top, right-left, bottom-top);
        mDivider->draw(canvas);
    }
    canvas.restore();
}

void DividerItemDecoration::getItemOffsets(Rect& outRect, View& view, RecyclerView& parent,
        RecyclerView::State& state) {
    if (mDivider == nullptr) {
        outRect.set(0, 0, 0, 0);
        return;
    }
    if (mOrientation == VERTICAL) {
        outRect.set(0, 0, 0, mDivider->getIntrinsicHeight());
    } else {
        outRect.set(0, 0, mDivider->getIntrinsicWidth(), 0);
    }
}

}
