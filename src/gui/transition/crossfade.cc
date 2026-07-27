/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * (LGPL 2.1+) — ported from android-36 android.transition.Crossfade.
 *********************************************************************************/
#include <transition/crossfade.h>

#include <animation/animator.h>
#include <animation/animatorset.h>
#include <animation/objectanimator.h>
#include <animation/property.h>
#include <animation/typeevaluators.h>
#include <animation/valueanimator.h>
#include <cairomm/surface.h>
#include <core/any.h>
#include <core/canvas.h>
#include <core/rect.h>
#include <drawable/bitmapdrawable.h>
#include <drawable/drawable.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/viewoverlay.h>

namespace cdroid {

namespace {

// android drives Drawable "alpha"/"bounds" by name. CDROID uses explicit Property objects
// (the by-name form resolves through a different path); behavior is identical.
class AlphaProperty: public Property {
  public:
    AlphaProperty(): Property("alpha", INT_TYPE) {}
    AnimateValue get(void* object) const override {
        return ((Drawable*)object)->getAlpha();
    }
    void set(void* object, const AnimateValue& v) const override {
        ((Drawable*)object)->setAlpha(GET_VARIANT(v, int));
    }
};
AlphaProperty ALPHA_PROPERTY;

class BoundsProperty: public Property {
  public:
    BoundsProperty(): Property("bounds") {}
    AnimateValue get(void* object) const override {
        return ((Drawable*)object)->getBounds();
    }
    void set(void* object, const AnimateValue& v) const override {
        ((Drawable*)object)->setBounds(GET_VARIANT(v, Rect));
    }
};
BoundsProperty BOUNDS_PROPERTY;

// android: Bitmap.sameAs — pixel comparison. Stubbed as always-different (always crossfade).
bool sameAs(const Cairo::RefPtr<Cairo::ImageSurface>&, const Cairo::RefPtr<Cairo::ImageSurface>&) {
    return false;
}

} // anonymous namespace

Crossfade& Crossfade::setFadeBehavior(int fadeBehavior) {
    if (fadeBehavior >= FADE_BEHAVIOR_CROSSFADE && fadeBehavior <= FADE_BEHAVIOR_OUT_IN) {
        mFadeBehavior = fadeBehavior;
    }
    return *this;
}

Crossfade& Crossfade::setResizeBehavior(int resizeBehavior) {
    if (resizeBehavior >= RESIZE_BEHAVIOR_NONE && resizeBehavior <= RESIZE_BEHAVIOR_SCALE) {
        mResizeBehavior = resizeBehavior;
    }
    return *this;
}

void Crossfade::captureValues(TransitionValues& transitionValues) {
    View* view = transitionValues.view;
    Rect bounds{0, 0, view->getWidth(), view->getHeight()};
    if (mFadeBehavior != FADE_BEHAVIOR_REVEAL) {
        bounds.offset(view->getLeft(), view->getTop());
    }
    transitionValues.values[PROPNAME_BOUNDS] = bounds;

    // android: Bitmap.createBitmap + Canvas + view.draw (TextureView.getBitmap skipped here).
    Cairo::RefPtr<Cairo::ImageSurface> bitmap = Cairo::ImageSurface::create(
                Cairo::Surface::Format::ARGB32, view->getWidth(), view->getHeight());
    Canvas c(bitmap);
    view->draw(c);
    transitionValues.values[PROPNAME_BITMAP] = bitmap;

    BitmapDrawable* drawable = new BitmapDrawable(bitmap);
    drawable->setBounds(bounds);
    transitionValues.values[PROPNAME_DRAWABLE] = drawable;
}

void Crossfade::captureStartValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

void Crossfade::captureEndValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

Animator* Crossfade::createAnimator(ViewGroup* /*sceneRoot*/,
                                    TransitionValues* startValues, TransitionValues* endValues) {
    if (startValues == nullptr || endValues == nullptr) {
        return nullptr;
    }
    const bool useParentOverlay = mFadeBehavior != FADE_BEHAVIOR_REVEAL;
    View* view = endValues->view;
    Rect startBounds = nonstd::any_cast<Rect>(startValues->values.at(PROPNAME_BOUNDS));
    Rect endBounds   = nonstd::any_cast<Rect>(endValues->values.at(PROPNAME_BOUNDS));
    auto startBitmap = nonstd::any_cast<Cairo::RefPtr<Cairo::ImageSurface>>(startValues->values.at(PROPNAME_BITMAP));
    auto endBitmap   = nonstd::any_cast<Cairo::RefPtr<Cairo::ImageSurface>>(endValues->values.at(PROPNAME_BITMAP));
    BitmapDrawable* startDrawable = nonstd::any_cast<BitmapDrawable*>(startValues->values.at(PROPNAME_DRAWABLE));
    BitmapDrawable* endDrawable   = nonstd::any_cast<BitmapDrawable*>(endValues->values.at(PROPNAME_DRAWABLE));

    if (startDrawable != nullptr && endDrawable != nullptr && !sameAs(startBitmap, endBitmap)) {
        ViewOverlay* overlay = useParentOverlay
                               ? static_cast<ViewGroup*>(view->getParent())->getOverlay()
                               : view->getOverlay();
        if (mFadeBehavior == FADE_BEHAVIOR_REVEAL) {
            overlay->add(endDrawable);
        }
        overlay->add(startDrawable);

        ObjectAnimator* anim;
        if (mFadeBehavior == FADE_BEHAVIOR_OUT_IN) {
            // Fade out completely halfway through the transition.
            anim = ObjectAnimator::ofInt(startDrawable, &ALPHA_PROPERTY, {255, 0, 0});
        } else {
            anim = ObjectAnimator::ofInt(startDrawable, &ALPHA_PROPERTY, {0});
        }
        anim->addUpdateListener([view, startDrawable](ValueAnimator&) {
            view->invalidate(startDrawable->getBounds());
        });

        ObjectAnimator* anim1 = nullptr;
        if (mFadeBehavior == FADE_BEHAVIOR_OUT_IN) {
            anim1 = ObjectAnimator::ofFloat(view, View::ALPHA, {0.0f, 0.0f, 1.0f});
        } else if (mFadeBehavior == FADE_BEHAVIOR_CROSSFADE) {
            anim1 = ObjectAnimator::ofFloat(view, View::ALPHA, {0.0f, 1.0f});
        }

        int fadeBehavior = mFadeBehavior;
        Animator::AnimatorListener endListener;
        endListener.onAnimationEnd = [overlay, startDrawable, endDrawable, fadeBehavior](Animator&, bool) {
            overlay->remove(startDrawable);
            if (fadeBehavior == FADE_BEHAVIOR_REVEAL) {
                overlay->remove(endDrawable);
            }
        };
        anim->addListener(endListener);

        AnimatorSet* set = new AnimatorSet();
        set->playTogether({anim});
        if (anim1 != nullptr) {
            set->playTogether({anim1});
        }
        if (mResizeBehavior == RESIZE_BEHAVIOR_SCALE && !(startBounds == endBounds)) {
            ObjectAnimator* anim2 = ObjectAnimator::ofObject(startDrawable, &BOUNDS_PROPERTY,
                                    RectEvaluator, {startBounds, endBounds});
            set->playTogether({anim2});
            ObjectAnimator* anim3 = ObjectAnimator::ofObject(endDrawable, &BOUNDS_PROPERTY,
                                    RectEvaluator, {startBounds, endBounds});
            set->playTogether({anim3});
        }
        return set;
    }
    return nullptr;
}

} // namespace cdroid
