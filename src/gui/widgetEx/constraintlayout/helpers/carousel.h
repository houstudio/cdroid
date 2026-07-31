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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.helper.widget.Carousel.
 *
 * Carousel works within a MotionLayout to provide a recycler-like pattern over a fixed pool of
 * views: as the MotionLayout transitions forward/backward between two ConstraintSets, Carousel
 * advances its current index and re-populates the pool via the Adapter, so a large dataset is
 * paginated through a few views. Index advancement is driven by MotionLayout's TransitionListener
 * (onTransitionCompleted); on touch-up a CARRY_ON mode carries the release momentum into the next
 * item. Faithful port of the AndroidX helper; depends on MotionLayout's transition lifecycle.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_HELPERS_CAROUSEL_H
#define CDROID_CONSTRAINTLAYOUT_HELPERS_CAROUSEL_H

#include <vector>

#include <core/attributeset.h>
#include <widgetEx/constraintlayout/motion/motionhelper.h>
#include <widgetEx/constraintlayout/motion/motionlayout.h>

namespace cdroid {

class MotionLayout;
class View;

class Carousel : public MotionHelper {
  public:
    static constexpr int TOUCH_UP_IMMEDIATE_STOP = 1;
    static constexpr int TOUCH_UP_CARRY_ON = 2;

    // Supplies the carousel's content. Mirrors AndroidX Carousel.Adapter. Owned by the caller
    // (raw pointer, like RecyclerView::Adapter) — Carousel does not delete it.
    class Adapter {
      public:
        virtual ~Adapter() = default;
        virtual int  count() = 0;                              // number of items
        virtual void populate(View* view, int index) = 0;      // fill the reusable view for index
        virtual void onNewItem(int /*index*/) {}               // settled on a new index
    };

    Carousel(Context* ctx, const AttributeSet& attrs);
    explicit Carousel(int width, int height);

    void setAdapter(Adapter* adapter) { mAdapter = adapter; }
    int  getCount();
    int  getCurrentIndex() const { return mIndex; }
    // Animate to `index`; `delay` is the per-step transition duration in ms.
    void transitionToIndex(int index, int delay);
    // Jump to `index` without animation.
    void jumpToIndex(int index);
    // Rebuild the MotionLayout scene and refresh the pool.
    void refresh();

  protected:
    void init(const AttributeSet& attrs) override;
    void onAttachedToWindow() override;
    void onDetachedFromWindow() override;

  private:
    // TransitionListener handlers (forwarded from the lambdas registered on mMotionLayout).
    void onTransitionChange(int startId, int endId, float progress);
    void onTransitionCompleted(int currentId);
    void runUpdate();               // the posted "mUpdateRunnable" body
    void updateItems();
    bool updateViewVisibility(View* view, int visibility);
    bool updateViewVisibility(int constraintSetId, View* view, int visibility);
    bool enableTransition(int transitionId, bool enable);

    Adapter* mAdapter = nullptr;                 // non-owning
    MotionLayout* mMotionLayout = nullptr;
    std::vector<View*> mList;                    // the reusable carousel item views
    MotionLayout::TransitionListener mListener;  // registered on mMotionLayout in onAttachedToWindow
    int mLastStartId = -1;
    int mPreviousIndex = 0;
    int mIndex = 0;
    int mFirstViewReference = -1;
    int mStartIndex = 0;
    bool mInfiniteCarousel = false;
    int mBackwardTransition = -1;
    int mForwardTransition = -1;
    int mPreviousState = -1;
    int mNextState = -1;
    float mDampening = 0.9f;
    int mEmptyViewBehavior = View::INVISIBLE;
    int mTouchUpMode = TOUCH_UP_IMMEDIATE_STOP;
    float mVelocityThreshold = 2.0f;
    int mTargetIndex = -1;
    int mAnimateTargetDelay = 200;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_HELPERS_CAROUSEL_H
