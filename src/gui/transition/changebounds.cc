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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *********************************************************************************/
#include <transition/changebounds.h>

#include <algorithm>
#include <cmath>

#include <animation/animator.h>
#include <animation/animatorset.h>
#include <animation/objectanimator.h>
#include <animation/property.h>      // Property, GET_VARIANT
#include <animation/typeevaluators.h>// RectEvaluator
#include <core/attributeset.h>
#include <core/context.h>
#include <core/path.h>
#include <core/rect.h>
#include <view/view.h>
#include <view/viewgroup.h>

#include <transition/transitionlisteneradapter.h>
#include <transition/transitionutils.h>

namespace cdroid {

namespace {

// ---- PointF properties driving View bounds along a Path ----
// android: anonymous Property<View,PointF> subclasses. Named here.

class PositionProperty: public Property {
  public:
    PositionProperty():Property("position") {}
    void set(void* object, const AnimateValue& value) const override {
        View* view = (View*)object;
        PointF p = GET_VARIANT(value, PointF);
        int left = (int)lround(p.x);
        int top  = (int)lround(p.y);
        view->setLeftTopRightBottom(left, top, left + view->getWidth(), top + view->getHeight());
    }
    AnimateValue get(void*) const override {
        return PointF{};
    }
};

class TopLeftOnlyProperty: public Property {
  public:
    TopLeftOnlyProperty():Property("topLeft") {}
    void set(void* object, const AnimateValue& value) const override {
        View* view = (View*)object;
        PointF p = GET_VARIANT(value, PointF);
        view->setLeftTopRightBottom((int)lround(p.x), (int)lround(p.y), view->getRight(), view->getBottom());
    }
    AnimateValue get(void*) const override {
        return PointF{};
    }
};

class BottomRightOnlyProperty: public Property {
  public:
    BottomRightOnlyProperty():Property("bottomRight") {}
    void set(void* object, const AnimateValue& value) const override {
        View* view = (View*)object;
        PointF p = GET_VARIANT(value, PointF);
        view->setLeftTopRightBottom(view->getLeft(), view->getTop(), (int)lround(p.x), (int)lround(p.y));
    }
    AnimateValue get(void*) const override {
        return PointF{};
    }
};

// android: private static nested ViewBounds. File-local here (pure impl helper); receives
// the two PointF animators (top-left + bottom-right) and applies both to the view once each
// pair arrives, avoiding intermediate layout glitches.
class ViewBounds {
  public:
    explicit ViewBounds(View* view): mView(view) {}
    void setTopLeft(const PointF& topLeft) {
        mLeft = (int)lround(topLeft.x);
        mTop  = (int)lround(topLeft.y);
        mTopLeftCalls++;
        if (mTopLeftCalls == mBottomRightCalls) {
            apply();
        }
    }
    void setBottomRight(const PointF& bottomRight) {
        mRight  = (int)lround(bottomRight.x);
        mBottom = (int)lround(bottomRight.y);
        mBottomRightCalls++;
        if (mTopLeftCalls == mBottomRightCalls) {
            apply();
        }
    }
  private:
    void apply() {
        mView->setLeftTopRightBottom(mLeft, mTop, mRight, mBottom);
        mTopLeftCalls = 0;
        mBottomRightCalls = 0;
    }
    View* mView;
    int mLeft = 0, mTop = 0, mRight = 0, mBottom = 0;
    int mTopLeftCalls = 0, mBottomRightCalls = 0;
};

class TopLeftProperty: public Property {
  public:
    TopLeftProperty():Property("topLeft") {}
    void set(void* object, const AnimateValue& value) const override {
        ((ViewBounds*)object)->setTopLeft(GET_VARIANT(value, PointF));
    }
    AnimateValue get(void*) const override {
        return PointF{};
    }
};

class BottomRightProperty: public Property {
  public:
    BottomRightProperty():Property("bottomRight") {}
    void set(void* object, const AnimateValue& value) const override {
        ((ViewBounds*)object)->setBottomRight(GET_VARIANT(value, PointF));
    }
    AnimateValue get(void*) const override {
        return PointF{};
    }
};

PositionProperty       POSITION_PROPERTY;
TopLeftOnlyProperty    TOP_LEFT_ONLY_PROPERTY;
BottomRightOnlyProperty BOTTOM_RIGHT_ONLY_PROPERTY;
TopLeftProperty        TOP_LEFT_PROPERTY;
BottomRightProperty    BOTTOM_RIGHT_PROPERTY;

// android: anonymous TransitionListenerAdapter that suppresses layout on the parent during
// the bounds animation. Named here.
struct SuppressLayoutListener: public TransitionListenerAdapter {
    ViewGroup* parent;
    bool mCanceled = false;
    void onTransitionCancel(Transition&) override {
        parent->suppressLayout(false);
        mCanceled = true;
    }
    void onTransitionEnd(Transition& t) override {
        if (!mCanceled) {
            parent->suppressLayout(false);
        }
        t.removeListener(this);
    }
    void onTransitionPause(Transition&) override {
        parent->suppressLayout(false);
    }
    void onTransitionResume(Transition&) override {
        parent->suppressLayout(true);
    }
};

} // anonymous namespace

const std::vector<std::string> ChangeBounds::sTransitionProperties = {
    PROPNAME_BOUNDS, PROPNAME_CLIP, PROPNAME_PARENT, PROPNAME_WINDOW_X, PROPNAME_WINDOW_Y
};

ChangeBounds::ChangeBounds() = default;

ChangeBounds::ChangeBounds(Context* context, AttributeSet* attrs)
    : Transition(context, attrs) {
    // android: obtainStyledAttributes(attrs, R.styleable.ChangeBounds) → resizeClip.
    if (attrs != nullptr) {
        std::string v = attrs->getAttributeValue("resizeClip");
        setResizeClip(v == "true" || v == "1");
    }
}

std::vector<std::string> ChangeBounds::getTransitionProperties() {
    return sTransitionProperties;
}

void ChangeBounds::captureValues(TransitionValues& values) {
    View* view = values.view;
    if (view->isLaidOut() || view->getWidth() != 0 || view->getHeight() != 0) {
        values.values[PROPNAME_BOUNDS] = Rect::MakeLTRB(view->getLeft(), view->getTop(),
                                         view->getRight(), view->getBottom());
        values.values[PROPNAME_PARENT] = view->getParent(); // ViewGroup*
        if (mReparent) {
            // CDROID: getLocationInWindow not wired for the reparent bitmap path; values stay
            // absent and the reparent branch in createAnimator is a no-op.
        }
        if (mResizeClip) {
            Rect clip;
            if (view->getClipBounds(clip)) {
                values.values[PROPNAME_CLIP] = clip;
            }
        }
    }
}

void ChangeBounds::captureStartValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

void ChangeBounds::captureEndValues(TransitionValues& transitionValues) {
    captureValues(transitionValues);
}

bool ChangeBounds::parentMatches(ViewGroup* startParent, ViewGroup* endParent) {
    if (mReparent) {
        TransitionValues* endValues = getMatchedTransitionValues(startParent, true);
        if (endValues == nullptr) {
            return startParent == endParent;
        }
        return endParent == endValues->view;
    }
    return true;
}

Animator* ChangeBounds::createAnimator(ViewGroup* /*sceneRoot*/,
                                       TransitionValues* startValues, TransitionValues* endValues) {
    if (startValues == nullptr || endValues == nullptr) {
        return nullptr;
    }
    ViewGroup* startParent = nonstd::any_cast<ViewGroup*>(startValues->values.at(PROPNAME_PARENT));
    ViewGroup* endParent   = nonstd::any_cast<ViewGroup*>(endValues->values.at(PROPNAME_PARENT));
    if (startParent == nullptr || endParent == nullptr) {
        return nullptr;
    }
    View* view = endValues->view;
    if (parentMatches(startParent, endParent)) {
        Rect startBounds = nonstd::any_cast<Rect>(startValues->values.at(PROPNAME_BOUNDS));
        Rect endBounds   = nonstd::any_cast<Rect>(endValues->values.at(PROPNAME_BOUNDS));
        const int startLeft = startBounds.left,    endLeft = endBounds.left;
        const int startTop  = startBounds.top,     endTop  = endBounds.top;
        const int startRight = startBounds.right(),  endRight = endBounds.right();
        const int startBottom = startBounds.bottom(), endBottom = endBounds.bottom();
        const int startWidth = startRight - startLeft, startHeight = startBottom - startTop;
        const int endWidth   = endRight - endLeft,     endHeight   = endBottom - endTop;

        bool startClipPresent = startValues->values.count(PROPNAME_CLIP) != 0;
        bool endClipPresent   = endValues->values.count(PROPNAME_CLIP) != 0;
        Rect startClip, endClip;
        if (startClipPresent) {
            startClip = nonstd::any_cast<Rect>(startValues->values.at(PROPNAME_CLIP));
        }
        if (endClipPresent) {
            endClip = nonstd::any_cast<Rect>(endValues->values.at(PROPNAME_CLIP));
        }

        int numChanges = 0;
        if ((startWidth != 0 && startHeight != 0) || (endWidth != 0 && endHeight != 0)) {
            if (startLeft != endLeft || startTop != endTop) {
                ++numChanges;
            }
            if (startRight != endRight || startBottom != endBottom) {
                ++numChanges;
            }
        }
        if ((startClipPresent && !endClipPresent) ||
                (startClipPresent && endClipPresent && !(startClip == endClip)) ||
                (!startClipPresent && endClipPresent)) {
            ++numChanges;
        }
        if (numChanges == 0) {
            return nullptr;
        }

        if (dynamic_cast<ViewGroup*>(view->getParent()) != nullptr) {
            ViewGroup* parent = static_cast<ViewGroup*>(view->getParent());
            parent->suppressLayout(true);
            SuppressLayoutListener* l = new SuppressLayoutListener();
            l->parent = parent;
            addListener(l);
        }

        Animator* anim = nullptr;
        if (!mResizeClip) {
            view->setLeftTopRightBottom(startLeft, startTop, startRight, startBottom);
            if (numChanges == 2) {
                if (startWidth == endWidth && startHeight == endHeight) {
                    Path topLeftPath = getPathMotion()->getPath(startLeft, startTop, endLeft, endTop);
                    anim = ObjectAnimator::ofObject(view, &POSITION_PROPERTY, nullptr, topLeftPath);
                } else {
                    ViewBounds* viewBounds = new ViewBounds(view);
                    Path topLeftPath = getPathMotion()->getPath(startLeft, startTop, endLeft, endTop);
                    ObjectAnimator* topLeftAnimator = ObjectAnimator::ofObject(viewBounds, &TOP_LEFT_PROPERTY, nullptr, topLeftPath);
                    Path bottomRightPath = getPathMotion()->getPath(startRight, startBottom, endRight, endBottom);
                    ObjectAnimator* bottomRightAnimator = ObjectAnimator::ofObject(viewBounds, &BOTTOM_RIGHT_PROPERTY, nullptr, bottomRightPath);
                    AnimatorSet* set = new AnimatorSet();
                    set->playTogether({topLeftAnimator, bottomRightAnimator});
                    anim = set;
                }
            } else if (startLeft != endLeft || startTop != endTop) {
                Path topLeftPath = getPathMotion()->getPath(startLeft, startTop, endLeft, endTop);
                anim = ObjectAnimator::ofObject(view, &TOP_LEFT_ONLY_PROPERTY, nullptr, topLeftPath);
            } else {
                Path bottomRightPath = getPathMotion()->getPath(startRight, startBottom, endRight, endBottom);
                anim = ObjectAnimator::ofObject(view, &BOTTOM_RIGHT_ONLY_PROPERTY, nullptr, bottomRightPath);
            }
        } else {
            const int maxWidth = std::max(startWidth, endWidth);
            const int maxHeight = std::max(startHeight, endHeight);
            view->setLeftTopRightBottom(startLeft, startTop, startLeft + maxWidth, startTop + maxHeight);

            ObjectAnimator* positionAnimator = nullptr;
            if (startLeft != endLeft || startTop != endTop) {
                Path topLeftPath = getPathMotion()->getPath(startLeft, startTop, endLeft, endTop);
                positionAnimator = ObjectAnimator::ofObject(view, &POSITION_PROPERTY, nullptr, topLeftPath);
            }

            Rect startClipLocal = startClipPresent ? startClip : Rect{0, 0, startWidth, startHeight};
            Rect endClipLocal   = endClipPresent   ? endClip   : Rect{0, 0, endWidth, endHeight};
            ObjectAnimator* clipAnimator = nullptr;
            if (!(startClipLocal == endClipLocal)) {
                view->setClipBounds(&startClipLocal);
                clipAnimator = ObjectAnimator::ofObject(view, "clipBounds", RectEvaluator,
                {startClipLocal, endClipLocal});
                Animator::AnimatorListener clipListener;
                Rect finalClip = endClip;
                clipListener.onAnimationEnd = [view, finalClip, endClipPresent, endLeft, endTop, endRight, endBottom](Animator&, bool) {
                    if (view) {
                        view->setClipBounds(endClipPresent ? &finalClip : nullptr);
                        view->setLeftTopRightBottom(endLeft, endTop, endRight, endBottom);
                    }
                };
                clipAnimator->addListener(clipListener);
            }
            anim = TransitionUtils::mergeAnimators(positionAnimator, clipAnimator);
        }
        return anim;
    }
    // Reparent path (mReparent) is stubbed — requires a Bitmap/Canvas snapshot of the view
    // (cairo 2D has no DisplayList). Returns no animator.
    return nullptr;
}

} // namespace cdroid
