/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.ChangeImageTransform.
 *********************************************************************************/
#include <transition/changeimagetransform.h>

#include <cairomm/matrix.h>
#include <core/any.h>
#include <core/rect.h>
#include <drawable/drawable.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/imageview.h>

#include <animation/valueanimator.h>

namespace cdroid {

namespace {

bool matrixEquals(const Cairo::Matrix& a, const Cairo::Matrix& b) {
    return a.xx == b.xx && a.yx == b.yx && a.xy == b.xy && a.yy == b.yy && a.x0 == b.x0 && a.y0 == b.y0;
}

// android: TransitionUtils.MatrixEvaluator — lerp the 6 affine fields. (Cairo uses 6 values;
// android's Matrix is 9 with a constant [0,0,1] bottom row — the lerp of the 6 is equivalent.)
Cairo::Matrix lerpMatrix(const Cairo::Matrix& start, const Cairo::Matrix& end, float fraction) {
    Cairo::Matrix m;
    m.xx = start.xx + (end.xx - start.xx) * fraction;
    m.yx = start.yx + (end.yx - start.yx) * fraction;
    m.xy = start.xy + (end.xy - start.xy) * fraction;
    m.yy = start.yy + (end.yy - start.yy) * fraction;
    m.x0 = start.x0 + (end.x0 - start.x0) * fraction;
    m.y0 = start.y0 + (end.y0 - start.y0) * fraction;
    return m;
}

ImageView* asImageView(View* v) {
    return dynamic_cast<ImageView*>(v);
}

} // anonymous namespace

const std::vector<std::string> ChangeImageTransform::sTransitionProperties = {PROPNAME_MATRIX, PROPNAME_BOUNDS};

std::vector<std::string> ChangeImageTransform::getTransitionProperties() {
    return sTransitionProperties;
}

void ChangeImageTransform::captureValues(TransitionValues& transitionValues) {
    View* view = transitionValues.view;
    ImageView* imageView = asImageView(view);
    if (imageView == nullptr || view->getVisibility() != View::VISIBLE) {
        return;
    }
    Drawable* drawable = imageView->getDrawable();
    if (drawable == nullptr) {
        return;
    }
    Rect bounds = Rect::MakeLTRB(view->getLeft(), view->getTop(), view->getRight(), view->getBottom());
    transitionValues.values[PROPNAME_BOUNDS] = bounds;

    Cairo::Matrix matrix;
    int scaleType = imageView->getScaleType();
    int drawableWidth = drawable->getIntrinsicWidth();
    int drawableHeight = drawable->getIntrinsicHeight();
    if (scaleType == ScaleType::FIT_XY && drawableWidth > 0 && drawableHeight > 0) {
        float scaleX = (float)bounds.width / drawableWidth;
        float scaleY = (float)bounds.height / drawableHeight;
        matrix = Cairo::Matrix(); // identity
        matrix.scale(scaleX, scaleY); // setScale (post-multiply from identity)
    } else {
        matrix = imageView->getImageMatrix();
    }
    transitionValues.values[PROPNAME_MATRIX] = matrix;
}

void ChangeImageTransform::captureStartValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

void ChangeImageTransform::captureEndValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

Animator* ChangeImageTransform::createAnimator(ViewGroup* /*sceneRoot*/,
        TransitionValues* startValues, TransitionValues* endValues) {
    if (startValues == nullptr || endValues == nullptr) {
        return nullptr;
    }
    if (startValues->values.count(PROPNAME_BOUNDS) == 0 || endValues->values.count(PROPNAME_BOUNDS) == 0) {
        return nullptr;
    }
    Rect startBounds = nonstd::any_cast<Rect>(startValues->values.at(PROPNAME_BOUNDS));
    Rect endBounds   = nonstd::any_cast<Rect>(endValues->values.at(PROPNAME_BOUNDS));
    Cairo::Matrix startMatrix = nonstd::any_cast<Cairo::Matrix>(startValues->values.at(PROPNAME_MATRIX));
    Cairo::Matrix endMatrix   = nonstd::any_cast<Cairo::Matrix>(endValues->values.at(PROPNAME_MATRIX));

    if (startBounds == endBounds && matrixEquals(startMatrix, endMatrix)) {
        return nullptr;
    }

    ImageView* imageView = asImageView(endValues->view);
    Drawable* drawable = imageView->getDrawable();
    int drawableWidth = drawable->getIntrinsicWidth();
    int drawableHeight = drawable->getIntrinsicHeight();

    if (drawableWidth <= 0 || drawableHeight <= 0) {
        return createNullAnimator(endValues->view);
    }
    imageView->animateTransform(&startMatrix); // ANIMATED_TRANSFORM_PROPERTY.set(start)
    return createMatrixAnimator(endValues->view, startMatrix, endMatrix);
}

Animator* ChangeImageTransform::createNullAnimator(View* imageView) {
    // android: NULL_MATRIX_EVALUATOR returns null → animateTransform(null) each frame.
    ValueAnimator* anim = ValueAnimator::ofFloat({0.0f, 1.0f});
    anim->addUpdateListener([imageView](ValueAnimator&) {
        ((ImageView*)imageView)->animateTransform(nullptr);
    });
    return anim;
}

Animator* ChangeImageTransform::createMatrixAnimator(View* imageView,
        const Cairo::Matrix& startMatrix, const Cairo::Matrix& endMatrix) {
    ValueAnimator* anim = ValueAnimator::ofFloat({0.0f, 1.0f});
    anim->addUpdateListener([imageView, startMatrix, endMatrix](ValueAnimator& a) {
        float fraction = a.getAnimatedFraction();
        Cairo::Matrix m = lerpMatrix(startMatrix, endMatrix, fraction);
        ((ImageView*)imageView)->animateTransform(&m);
    });
    return anim;
}

} // namespace cdroid
