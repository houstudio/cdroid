/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.view.GhostView.
 *********************************************************************************/
#include <view/ghostview.h>

#include <core/canvas.h>
#include <view/viewgroup.h>
#include <view/viewgroupoverlay.h>
#include <widget/framelayout.h>

namespace cdroid{

GhostView::GhostView(View* view)
    : View(0, 0), mView(view){
    mView->mGhostView = this;
    mView->setTransitionVisibility(View::INVISIBLE);
    if (ViewGroup* parent = mView->getParent()){
        parent->invalidate();
    }
}

void GhostView::onDraw(Canvas& canvas){
    // android uses RenderNode display list. cairo: draw the ghosted view directly with matrix.
    canvas.save();
    canvas.transform(mMatrix);
    mView->draw(canvas);
    canvas.restore();
}

void GhostView::setMatrix(const Cairo::Matrix* matrix){
    if (matrix != nullptr){
        mMatrix = *matrix;
    }
}

void GhostView::setVisibility(int visibility){
    View::setVisibility(visibility);
    if (mView->mGhostView == this){
        int inverseVisibility = (visibility == View::VISIBLE) ? View::INVISIBLE : View::VISIBLE;
        mView->setTransitionVisibility(inverseVisibility);
    }
}

void GhostView::onDetachedFromWindow(){
    View::onDetachedFromWindow();
    if (!mBeingMoved){
        mView->setTransitionVisibility(View::VISIBLE);
        mView->mGhostView = nullptr;
        if (ViewGroup* parent = mView->getParent()){
            parent->invalidate();
        }
    }
}

void GhostView::calculateMatrix(View* view, ViewGroup* host, Cairo::Matrix& matrix){
    ViewGroup* parent = view->getParent();
    matrix = Cairo::Matrix(); // reset → identity
    parent->transformMatrixToGlobal(matrix);
    // android preTranslate(-scrollX,-scrollY); cairo translate is post-multiply — approximate.
    matrix.translate(-parent->getScrollX(), -parent->getScrollY());
    host->transformMatrixToLocal(matrix);
}

static void copySize(View* from, View* to){
    to->setLeft(0);
    to->setTop(0);
    to->setRight(from->getWidth());
    to->setBottom(from->getHeight());
}

GhostView* GhostView::addGhost(View* view, ViewGroup* viewGroup, const Cairo::Matrix* matrix){
    if (view->getParent() == nullptr){
        return nullptr; // must be parented by a ViewGroup
    }
    ViewGroupOverlay* overlay = static_cast<ViewGroupOverlay*>(viewGroup->getOverlay());
    GhostView* ghostView = view->mGhostView;
    if (ghostView == nullptr){
        Cairo::Matrix m;
        if (matrix == nullptr){
            calculateMatrix(view, viewGroup, m);
            ghostView = new GhostView(view);
            ghostView->setMatrix(&m);
        } else {
            ghostView = new GhostView(view);
            ghostView->setMatrix(matrix);
        }
        FrameLayout* wrapper = new FrameLayout(0, 0);
        wrapper->setClipChildren(false);
        copySize(viewGroup, wrapper);
        copySize(viewGroup, ghostView);
        wrapper->addView(ghostView);
        overlay->add(wrapper); // simplified: append (no binary-search z-order)
    } else if (matrix != nullptr){
        ghostView->setMatrix(matrix);
    }
    ghostView->mReferences++;
    return ghostView;
}

GhostView* GhostView::addGhost(View* view, ViewGroup* viewGroup){
    return addGhost(view, viewGroup, nullptr);
}

void GhostView::removeGhost(View* view){
    GhostView* ghostView = view->mGhostView;
    if (ghostView != nullptr){
        ghostView->mReferences--;
        if (ghostView->mReferences == 0){
            // Remove the FrameLayout wrapper from the overlay.
            if (ViewGroup* wrapper = static_cast<ViewGroup*>(ghostView->getParent())){
                if (ViewGroupOverlay* overlay = static_cast<ViewGroupOverlay*>(
                        static_cast<ViewGroup*>(wrapper->getParent())->getOverlay())){
                    overlay->remove(wrapper);
                }
            }
        }
    }
}

GhostView* GhostView::getGhost(View* view){
    return view->mGhostView;
}

} // namespace cdroid
