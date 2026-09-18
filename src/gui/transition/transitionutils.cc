/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02101-1301  USA
 *********************************************************************************/
#include <transition/transitionutils.h>

#include <cmath>

#include <animation/animatorset.h>
#include <porting/cdlog.h>
#include <core/canvas.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/viewgroupoverlay.h>
#include <widget/imageview.h>

namespace cdroid {

namespace {
// android TransitionUtils.MAX_IMAGE_SIZE: snapshots are scaled uniformly down so
// that width * height stays within this many pixels.
constexpr int MAX_IMAGE_SIZE = 1024 * 1024;
} // namespace

Animator* TransitionUtils::mergeAnimators(Animator* animator1, Animator* animator2) {
    if (animator1 == nullptr) {
        return animator2;
    } else if (animator2 == nullptr) {
        return animator1;
    } else {
        AnimatorSet* animatorSet = new AnimatorSet();
        animatorSet->playTogether({animator1, animator2});
        return animatorSet;
    }
}

View* TransitionUtils::copyViewImage(ViewGroup* sceneRoot, View* view, ViewGroup* parent) {
    Matrix matrix = Cairo::identity_matrix();
    matrix.translate(-parent->getScrollX(), -parent->getScrollY());
    view->transformMatrixToGlobal(matrix);
    sceneRoot->transformMatrixToLocal(matrix);
    Cairo::Rectangle bounds = {0.0, 0.0, (double)view->getWidth(), (double)view->getHeight()};
    matrix.transform_rectangle(bounds);
    const int left   = (int)lround(bounds.x);
    const int top    = (int)lround(bounds.y);
    const int right  = (int)lround(bounds.x + bounds.width);
    const int bottom = (int)lround(bounds.y + bounds.height);

    // android: ImageView copy with CENTER_CROP; createViewBitmap may hand back a
    // MAX_IMAGE_SIZE-scaled bitmap, which the scale type maps onto the copy's
    // measured bounds. The copy is returned even when the bitmap is null.
    ImageView* copy = new ImageView(view->getContext());
    copy->setScaleType(CENTER_CROP);
    Cairo::RefPtr<Cairo::ImageSurface> bitmap = createViewBitmap(view, matrix, bounds, sceneRoot);
    if (bitmap) {
        copy->setImageBitmap(bitmap);
    }
    const int widthSpec  = MeasureSpec::makeMeasureSpec(right - left, MeasureSpec::EXACTLY);
    const int heightSpec = MeasureSpec::makeMeasureSpec(bottom - top, MeasureSpec::EXACTLY);
    copy->measure(widthSpec, heightSpec);
    copy->layout(left, top, right - left, bottom - top);
    return copy;
}

Cairo::RefPtr<Cairo::ImageSurface> TransitionUtils::createViewBitmap(View* view, Matrix& matrix,
        Cairo::Rectangle& bounds, ViewGroup* sceneRoot) {
    const bool needAttachment = !view->isAttachedToWindow();
    bool borrowedToOverlay = false;
    if (needAttachment) {
        if (sceneRoot == nullptr || !sceneRoot->isAttachedToWindow()) {
            return nullptr;
        }
        ViewGroup* parent = static_cast<ViewGroup*>(view->getParent());
        if (parent == nullptr) {
            // Detached with no parent: borrow into the scene root's overlay so
            // the draw below has an attached tree, then restore (AOSP recipe).
            static_cast<ViewGroupOverlay*>(sceneRoot->getOverlay())->add(view);
            borrowedToOverlay = true;
        }
        // A disappearing child still holds mParent (removeView routed it to
        // mDisappearingChildren while its subtree was detached mid-transition):
        // overlay->add would throw "already has a parent" out of addViewInner
        // and abort the process (printerdemo sweep, onDisappear -> copyViewImage
        // over a mid-teardown page). Its parent links are intact — draw in place.
    }
    Cairo::RefPtr<Cairo::ImageSurface> bitmap;
    const int bitmapWidth  = (int)lround(bounds.width);
    const int bitmapHeight = (int)lround(bounds.height);
    if (bitmapWidth > 0 && bitmapHeight > 0) {
        const float scale = std::min(1.f,
                ((float)MAX_IMAGE_SIZE) / ((float)bitmapWidth * (float)bitmapHeight));
        const int scaledWidth  = (int)(bitmapWidth * scale);
        const int scaledHeight = (int)(bitmapHeight * scale);
        matrix.translate(-bounds.x, -bounds.y);
        matrix.scale(scale, scale);
        // android records a Picture and converts it to a Bitmap; CDROID renders the
        // view straight into an ARGB32 ImageSurface (the Crossfade snapshot recipe).
        bitmap = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, scaledWidth, scaledHeight);
        Canvas canvas(bitmap);
        canvas.transform(matrix);
        view->draw(canvas);
    }
    if (borrowedToOverlay) {
        static_cast<ViewGroupOverlay*>(sceneRoot->getOverlay())->remove(view);
        // parent was null when we borrowed — nothing to restore (AOSP re-adds to the
        // captured parent/index; a parentless view has none).
    }
    return bitmap;
}

} // namespace cdroid
