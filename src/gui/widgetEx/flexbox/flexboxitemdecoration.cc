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
#include <widgetEx/flexbox/flexboxitemdecoration.h>
namespace cdroid{

FlexboxItemDecoration::FlexboxItemDecoration(Context* context) {
    //final TypedArray a = context.obtainStyledAttributes(LIST_DIVIDER_ATTRS);
    //mDrawable = a.getDrawable(0);
    AttributeSet a = context->obtainStyledAttributes("cdroid:attr/listDivider");
    mDrawable = a.getDrawable("listDivider");
    LOGW_IF(mDrawable == nullptr,"@android:attr/listDivider was not set in the theme used for this "
               "FlexboxItemDecoration. Please set that attribute all call setDrawable()");
    setOrientation(BOTH);
}

FlexboxItemDecoration::~FlexboxItemDecoration(){
    delete mDrawable;
}

/**
 * Set the drawable used as the item decoration.
 * If the drawable is not set, the default list divider is used as the
 * item decoration.
 */
void FlexboxItemDecoration::setDrawable(Drawable* drawable) {
    FATAL_IF(drawable==nullptr,"Drawable cannot be null.");
    delete mDrawable;
    mDrawable = drawable;
}

/**
 * Set the orientation for the decoration.
 */
void FlexboxItemDecoration::setOrientation(int orientation) {
    mOrientation = orientation;
}

void FlexboxItemDecoration::onDraw(Canvas& c, RecyclerView& parent, RecyclerView::State& state) {
    drawHorizontalDecorations(c, parent);
    drawVerticalDecorations(c, parent);
}

void FlexboxItemDecoration::getItemOffsets(Rect& outRect, View& view, RecyclerView& parent,
        RecyclerView::State& state) {
    int position = parent.getChildAdapterPosition(&view);
    if (position == 0) {
        return;
    }
    if (!needsHorizontalDecoration() && !needsVerticalDecoration()) {
        outRect.set(0, 0, 0, 0);
        return;
    }
    FlexboxLayoutManager* layoutManager = dynamic_cast<FlexboxLayoutManager*>(parent.getLayoutManager());
    std::vector<FlexLine> flexLines = layoutManager->getFlexLines();
    int flexDirection = layoutManager->getFlexDirection();
    setOffsetAlongMainAxis(outRect, position, layoutManager, flexLines, flexDirection);
    setOffsetAlongCrossAxis(outRect, position, layoutManager, flexLines);
}

void FlexboxItemDecoration::setOffsetAlongCrossAxis(Rect& outRect, int position,
        FlexboxLayoutManager* layoutManager, std::vector<FlexLine>& flexLines) {
    if (flexLines.size() == 0) {
        return;
    }
    int flexLineIndex = layoutManager->getPositionToFlexLineIndex(position);
    if (flexLineIndex == 0) {
        return;
    }
    if (layoutManager->isMainAxisDirectionHorizontal()) {
        if (!needsHorizontalDecoration()) {
            outRect.top = 0;
            outRect.height = 0;
            return;
        }
        outRect.top = mDrawable->getIntrinsicHeight();
        outRect.height = 0;
    } else {
        if (!needsVerticalDecoration()) {
            return;
        }
        if (layoutManager->isLayoutRtl()) {
            outRect.width = mDrawable->getIntrinsicWidth();
            outRect.left = 0;
        } else {
            outRect.left = mDrawable->getIntrinsicWidth();
            outRect.width = 0;
        }
    }
}

void FlexboxItemDecoration::setOffsetAlongMainAxis(Rect& outRect, int position,
        FlexboxLayoutManager* layoutManager, std::vector<FlexLine>& flexLines, int flexDirection) {
    if (isFirstItemInLine(position, flexLines, layoutManager)) {
        return;
    }
    if (layoutManager->isMainAxisDirectionHorizontal()) {
        if (!needsVerticalDecoration()) {
            outRect.left = 0;
            outRect.width = 0;
            return;
        }
        if (layoutManager->isLayoutRtl()) {
            outRect.width = mDrawable->getIntrinsicWidth();
            outRect.left = 0;
        } else {
            outRect.left = mDrawable->getIntrinsicWidth();
            outRect.width = 0;
        }
    } else {
        if (!needsHorizontalDecoration()) {
            outRect.top = 0;
            outRect.height = 0;
            return;
        }
        if (flexDirection == FlexDirection::COLUMN_REVERSE) {
            outRect.height = mDrawable->getIntrinsicHeight();
            outRect.top = 0;
        } else {
            outRect.top = mDrawable->getIntrinsicHeight();
            outRect.height = 0;
        }
    }
}

void FlexboxItemDecoration::drawVerticalDecorations(Canvas& canvas, RecyclerView& parent) {
    if (!needsVerticalDecoration()) {
        return;
    }
    FlexboxLayoutManager* layoutManager = dynamic_cast<FlexboxLayoutManager*>(parent.getLayoutManager());
    int parentTop = parent.getTop() - parent.getPaddingTop();
    int parentBottom = parent.getBottom() + parent.getPaddingBottom();
    int childCount = parent.getChildCount();
    int flexDirection = layoutManager->getFlexDirection();
    for (int i = 0; i < childCount; i++) {
        View* child = parent.getChildAt(i);
        RecyclerView::LayoutParams* lp = (RecyclerView::LayoutParams*) child->getLayoutParams();
        int left, right;
        if (layoutManager->isLayoutRtl()) {
            left = child->getRight() + lp->rightMargin;
            right = left + mDrawable->getIntrinsicWidth();
        } else {
            right = child->getLeft() - lp->leftMargin;
            left = right - mDrawable->getIntrinsicWidth();
        }
        int top, bottom;
        if (layoutManager->isMainAxisDirectionHorizontal()) {
            top = child->getTop() - lp->topMargin;
            bottom = child->getBottom() + lp->bottomMargin;
        } else {
            if (flexDirection == FlexDirection::COLUMN_REVERSE) {
                bottom = child->getBottom() + lp->bottomMargin + mDrawable->getIntrinsicHeight();
                bottom = std::min(bottom, parentBottom);
                top = child->getTop() - lp->topMargin;
            } else {
                top = child->getTop() - lp->topMargin - mDrawable->getIntrinsicHeight();
                top = std::max(top, parentTop);
                bottom = child->getBottom() + lp->bottomMargin;
            }
        }
        mDrawable->setBounds(left, top, right - left, bottom - top);
        mDrawable->draw(canvas);
    }
}

void FlexboxItemDecoration::drawHorizontalDecorations(Canvas& canvas, RecyclerView& parent) {
    if (!needsHorizontalDecoration()) {
        return;
    }
    FlexboxLayoutManager* layoutManager = dynamic_cast<FlexboxLayoutManager*>(parent.getLayoutManager());
    int flexDirection = layoutManager->getFlexDirection();
    int parentLeft = parent.getLeft() - parent.getPaddingLeft();
    int parentRight = parent.getRight() + parent.getPaddingRight();
    int childCount = parent.getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = parent.getChildAt(i);
        RecyclerView::LayoutParams* lp = (RecyclerView::LayoutParams*) child->getLayoutParams();
        int top, bottom;
        if (flexDirection == FlexDirection::COLUMN_REVERSE) {
            top = child->getBottom() + lp->bottomMargin;
            bottom = top + mDrawable->getIntrinsicHeight();
        } else {
            bottom = child->getTop() - lp->topMargin;
            top = bottom - mDrawable->getIntrinsicHeight();
        }
        int left, right;
        if (layoutManager->isMainAxisDirectionHorizontal()) {
            if (layoutManager->isLayoutRtl()) {
                right = child->getRight() + lp->rightMargin + mDrawable->getIntrinsicWidth();
                right = std::min(right, parentRight);
                left = child->getLeft() - lp->leftMargin;
            } else {
                left = child->getLeft() - lp->leftMargin - mDrawable->getIntrinsicWidth();
                left = std::max(left, parentLeft);
                right = child->getRight() + lp->rightMargin;
            }
        } else {
            left = child->getLeft() - lp->leftMargin;
            right = child->getRight() + lp->rightMargin;
        }
        mDrawable->setBounds(left, top, right - left, bottom - top);
        mDrawable->draw(canvas);
    }
}

bool FlexboxItemDecoration::needsHorizontalDecoration() {
    return (mOrientation & HORIZONTAL) > 0;
}

bool FlexboxItemDecoration::needsVerticalDecoration() {
    return (mOrientation & VERTICAL) > 0;
}

/**
 * @return {@code true} if the given position is the first item in a flex line.
 */
bool FlexboxItemDecoration::isFirstItemInLine(int position, std::vector<FlexLine>& flexLines,
        FlexboxLayoutManager* layoutManager) {
    int flexLineIndex = layoutManager->getPositionToFlexLineIndex(position);
    std::vector<FlexLine>& flexLinesInternal = layoutManager->getFlexLinesInternal();
    if (flexLineIndex != RecyclerView::NO_POSITION &&
            flexLineIndex < (int)flexLinesInternal.size() &&
            flexLinesInternal[flexLineIndex].getFirstIndex() == position) {
        return true;
    }
    if (position == 0) {
        return true;
    }
    if (flexLines.size() == 0) {
        return false;
    }
    // Check if the position is the "lastIndex + 1" of the last line in case the FlexLine which
    // has the View, whose index is position is not included in the flexLines. (E.g. flexLines
    // is being calculated
    FlexLine& lastLine = flexLines[flexLines.size() - 1];
    return lastLine.getLastIndex() == position - 1;
}

}/*endof namespace*/
