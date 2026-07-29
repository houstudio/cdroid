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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.ChainHead.
 */
#include <widgetEx/constraintlayout/core/widgets/chainhead.h>

namespace cdroid {

ChainHead::ChainHead(ConstraintWidget* first, int orientation, bool isRtl)
    : mFirst(first)
    , mOrientation(orientation)
    , mIsRtl(isRtl) {
}

bool ChainHead::isMatchConstraintEqualityCandidate(ConstraintWidget* widget, int orientation) {
    return widget->getVisibility() != ConstraintWidget::GONE
           && widget->mListDimensionBehaviors[orientation]
           == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
           && (widget->mResolvedMatchConstraintDefault[orientation] == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
               || widget->mResolvedMatchConstraintDefault[orientation] == ConstraintWidget::MATCH_CONSTRAINT_RATIO);
}

void ChainHead::defineChainProperties() {
    int offset = mOrientation * 2;
    ConstraintWidget* lastVisited = mFirst;
    mOptimizable = true;

    // Traverse the chain.
    ConstraintWidget* widget = mFirst;
    ConstraintWidget* next = mFirst;
    bool done = false;
    while (!done) {
        mWidgetsCount++;
        widget->mNextChainWidget[mOrientation] = nullptr;
        widget->mListNextMatchConstraintsWidget[mOrientation] = nullptr;
        if (widget->getVisibility() != ConstraintWidget::GONE) {
            mVisibleWidgets++;
            if (widget->getDimensionBehaviour(mOrientation)
                    != ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT) {
                mTotalSize += widget->getLength(mOrientation);
            }
            mTotalSize += widget->mListAnchors[offset]->getMargin();
            mTotalSize += widget->mListAnchors[offset + 1]->getMargin();
            mTotalMargins += widget->mListAnchors[offset]->getMargin();
            mTotalMargins += widget->mListAnchors[offset + 1]->getMargin();
            // Visible widgets linked list.
            if (mFirstVisibleWidget == nullptr) {
                mFirstVisibleWidget = widget;
            }
            mLastVisibleWidget = widget;

            // Match-constraint linked list.
            if (widget->mListDimensionBehaviors[mOrientation]
                    == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT) {
                if (widget->mResolvedMatchConstraintDefault[mOrientation]
                        == ConstraintWidget::MATCH_CONSTRAINT_SPREAD
                        || widget->mResolvedMatchConstraintDefault[mOrientation]
                        == ConstraintWidget::MATCH_CONSTRAINT_RATIO
                        || widget->mResolvedMatchConstraintDefault[mOrientation]
                        == ConstraintWidget::MATCH_CONSTRAINT_PERCENT) {
                    mWidgetsMatchCount++;
                    float weight = widget->mWeight[mOrientation];
                    if (weight > 0) {
                        mTotalWeight += widget->mWeight[mOrientation];
                    }

                    if (isMatchConstraintEqualityCandidate(widget, mOrientation)) {
                        if (weight < 0) {
                            mHasUndefinedWeights = true;
                        } else {
                            mHasDefinedWeights = true;
                        }
                        mWeightedMatchConstraintsWidgets.push_back(widget);
                    }

                    if (mFirstMatchConstraintWidget == nullptr) {
                        mFirstMatchConstraintWidget = widget;
                    }
                    if (mLastMatchConstraintWidget != nullptr) {
                        mLastMatchConstraintWidget
                        ->mListNextMatchConstraintsWidget[mOrientation] = widget;
                    }
                    mLastMatchConstraintWidget = widget;
                }
                if (mOrientation == ConstraintWidget::HORIZONTAL) {
                    if (widget->mMatchConstraintDefaultWidth
                            != ConstraintWidget::MATCH_CONSTRAINT_SPREAD) {
                        mOptimizable = false;
                    } else if (widget->mMatchConstraintMinWidth != 0
                               || widget->mMatchConstraintMaxWidth != 0) {
                        mOptimizable = false;
                    }
                } else {
                    if (widget->mMatchConstraintDefaultHeight
                            != ConstraintWidget::MATCH_CONSTRAINT_SPREAD) {
                        mOptimizable = false;
                    } else if (widget->mMatchConstraintMinHeight != 0
                               || widget->mMatchConstraintMaxHeight != 0) {
                        mOptimizable = false;
                    }
                }
                if (widget->mDimensionRatio != 0.0f) {
                    mOptimizable = false;
                    mHasRatio = true;
                }
            }
        }
        if (lastVisited != widget) {
            lastVisited->mNextChainWidget[mOrientation] = widget;
        }
        lastVisited = widget;

        // go to the next widget
        ConstraintAnchor* nextAnchor = widget->mListAnchors[offset + 1]->mTarget;
        if (nextAnchor != nullptr) {
            next = nextAnchor->mOwner;
            if (next->mListAnchors[offset]->mTarget == nullptr
                    || next->mListAnchors[offset]->mTarget->mOwner != widget) {
                next = nullptr;
            }
        } else {
            next = nullptr;
        }
        if (next != nullptr) {
            widget = next;
        } else {
            done = true;
        }
    }
    if (mFirstVisibleWidget != nullptr) {
        mTotalSize -= mFirstVisibleWidget->mListAnchors[offset]->getMargin();
    }
    if (mLastVisibleWidget != nullptr) {
        mTotalSize -= mLastVisibleWidget->mListAnchors[offset + 1]->getMargin();
    }
    mLast = widget;

    if (mOrientation == ConstraintWidget::HORIZONTAL && mIsRtl) {
        mHead = mLast;
    } else {
        mHead = mFirst;
    }

    mHasComplexMatchWeights = mHasDefinedWeights && mHasUndefinedWeights;
}

ConstraintWidget* ChainHead::getFirst() const {
    return mFirst;
}
ConstraintWidget* ChainHead::getFirstVisibleWidget() const {
    return mFirstVisibleWidget;
}
ConstraintWidget* ChainHead::getLast() const {
    return mLast;
}
ConstraintWidget* ChainHead::getLastVisibleWidget() const {
    return mLastVisibleWidget;
}
ConstraintWidget* ChainHead::getHead() const {
    return mHead;
}
ConstraintWidget* ChainHead::getFirstMatchConstraintWidget() const {
    return mFirstMatchConstraintWidget;
}
ConstraintWidget* ChainHead::getLastMatchConstraintWidget() const {
    return mLastMatchConstraintWidget;
}
float ChainHead::getTotalWeight() const {
    return mTotalWeight;
}

void ChainHead::define() {
    if (!mDefined) {
        defineChainProperties();
    }
    mDefined = true;
}

} // namespace cdroid
