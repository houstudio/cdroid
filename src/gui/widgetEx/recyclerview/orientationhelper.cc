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
#include <widgetEx/recyclerview/orientationhelper.h>
namespace cdroid{

OrientationHelper::OrientationHelper(RecyclerView::LayoutManager* layoutManager) {
    mLayoutManager = layoutManager;
}

RecyclerView::LayoutManager* OrientationHelper::getLayoutManager() {
    return mLayoutManager;
}

void OrientationHelper::onLayoutComplete() {
    mLastTotalSpace = getTotalSpace();
}

int OrientationHelper::getTotalSpaceChange() {
    return INVALID_SIZE == mLastTotalSpace ? 0 : getTotalSpace() - mLastTotalSpace;
}

OrientationHelper* OrientationHelper::createOrientationHelper(RecyclerView::LayoutManager* layoutManager,int orientation) {
    switch (orientation) {
    case HORIZONTAL:
        return createHorizontalHelper(layoutManager);
    case VERTICAL:
        return createVerticalHelper(layoutManager);
    }
    LOGE("invalid orientation");
    return nullptr;
}

class HorizontalOrientationHelper:public OrientationHelper{
public:
    HorizontalOrientationHelper(RecyclerView::LayoutManager*layoutManager)
	:OrientationHelper(layoutManager){
    }
    int getEndAfterPadding()override{
        return mLayoutManager->getWidth() - mLayoutManager->getPaddingRight();
    }

    int getEnd()override{
        return mLayoutManager->getWidth();
    }

    void offsetChildren(int amount)override{
        mLayoutManager->offsetChildrenHorizontal(amount);
    }

    int getStartAfterPadding()override{
        return mLayoutManager->getPaddingLeft();
    }

    int getDecoratedMeasurement(View* view)override{
        const RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)
                view->getLayoutParams();
        return mLayoutManager->getDecoratedMeasuredWidth(view) + params->leftMargin
                + params->rightMargin;
    }

    int getDecoratedMeasurementInOther(View* view)override{
        const RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)
                view->getLayoutParams();
        return mLayoutManager->getDecoratedMeasuredHeight(view) + params->topMargin
                + params->bottomMargin;
    }

    int getDecoratedEnd(View* view)override{
        const RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)
                view->getLayoutParams();
        return mLayoutManager->getDecoratedRight(view) + params->rightMargin;
    }

    int getDecoratedStart(View* view)override{
        const RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)
                view->getLayoutParams();
        return mLayoutManager->getDecoratedLeft(view) - params->leftMargin;
    }

    int getTransformedEndWithDecoration(View* view)override{
        mLayoutManager->getTransformedBoundingBox(view, true, mTmpRect);
        return mTmpRect.right();
    }

    int getTransformedStartWithDecoration(View* view)override{
        mLayoutManager->getTransformedBoundingBox(view, true, mTmpRect);
        return mTmpRect.left;
    }

    int getTotalSpace()override{
        return mLayoutManager->getWidth() - mLayoutManager->getPaddingLeft()
                - mLayoutManager->getPaddingRight();
    }

    void offsetChild(View* view, int offset)override{
        view->offsetLeftAndRight(offset);
    }

    int getEndPadding()override{
        return mLayoutManager->getPaddingRight();
    }

    int getMode()override{
        return mLayoutManager->getWidthMode();
    }

    int getModeInOther()override{
        return mLayoutManager->getHeightMode();
    }
};

OrientationHelper* OrientationHelper::createHorizontalHelper(RecyclerView::LayoutManager* layoutManager){
    return new HorizontalOrientationHelper(layoutManager);
}

class VerticalOrientationHehlper:public OrientationHelper{
public:
    VerticalOrientationHehlper(RecyclerView::LayoutManager*layoutManager)
	:OrientationHelper(layoutManager){
    }
    int getEndAfterPadding()override{
        return mLayoutManager->getHeight() - mLayoutManager->getPaddingBottom();
    }

    int getEnd()override{
        return mLayoutManager->getHeight();
    }

    void offsetChildren(int amount)override{
        mLayoutManager->offsetChildrenVertical(amount);
    }

    int getStartAfterPadding()override{
        return mLayoutManager->getPaddingTop();
    }

    int getDecoratedMeasurement(View* view)override{
        const RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)
                view->getLayoutParams();
        return mLayoutManager->getDecoratedMeasuredHeight(view) + params->topMargin
                + params->bottomMargin;
    }

    int getDecoratedMeasurementInOther(View* view)override{
        RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)
                view->getLayoutParams();
        return mLayoutManager->getDecoratedMeasuredWidth(view) + params->leftMargin
                + params->rightMargin;
    }

    int getDecoratedEnd(View* view)override{
        const RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)
                view->getLayoutParams();
        return mLayoutManager->getDecoratedBottom(view) + params->bottomMargin;
    }

    int getDecoratedStart(View* view)override{
        const RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)
                view->getLayoutParams();
        return mLayoutManager->getDecoratedTop(view) - params->topMargin;
    }

    int getTransformedEndWithDecoration(View* view)override{
        mLayoutManager->getTransformedBoundingBox(view, true, mTmpRect);
        return mTmpRect.bottom();
    }

    int getTransformedStartWithDecoration(View* view)override{
        mLayoutManager->getTransformedBoundingBox(view, true, mTmpRect);
        return mTmpRect.top;
    }

    int getTotalSpace()override{
        return mLayoutManager->getHeight() - mLayoutManager->getPaddingTop()
                - mLayoutManager->getPaddingBottom();
    }

    void offsetChild(View* view, int offset)override{
        view->offsetTopAndBottom(offset);
    }

    int getEndPadding()override{
        return mLayoutManager->getPaddingBottom();
    }

    int getMode()override{
        return mLayoutManager->getHeightMode();
    }

    int getModeInOther() override{
        return mLayoutManager->getWidthMode();
    }
};

OrientationHelper* OrientationHelper::createVerticalHelper(RecyclerView::LayoutManager* layoutManager) {
    return new VerticalOrientationHehlper(layoutManager);
}

}/*endof namespace*/
