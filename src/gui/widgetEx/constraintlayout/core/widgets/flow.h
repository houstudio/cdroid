/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Flow.
 *
 * A virtual layout that arranges its referenced widgets in a flowing sequence, wrapping to a new
 * row/column when the available space is exceeded (flexbox-like). Wraps via a list of chain rows
 * (WidgetsList); measure() computes the wrapped size + builds the rows, addToSolver() emits each
 * row's per-widget constraints. Lives in clcore:: to avoid clashing with the widget-layer
 * cdroid::Flow View subclass.
 *
 * MVP: HORIZONTAL orientation + WRAP_CHAIN + WRAP_NONE are fully ported. VERTICAL rows,
 * WRAP_ALIGNED and WRAP_CHAIN_NEW are stubbed (faithful TODO).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_FLOW_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_FLOW_H

#include <vector>

#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/widgets/virtuallayout.h>

namespace cdroid::clcore {

class Flow : public VirtualLayout {
  public:
    // horizontalAlign
    static const int HORIZONTAL_ALIGN_START  = 0;
    static const int HORIZONTAL_ALIGN_END    = 1;
    static const int HORIZONTAL_ALIGN_CENTER = 2;
    // verticalAlign
    static const int VERTICAL_ALIGN_TOP     = 0;
    static const int VERTICAL_ALIGN_BOTTOM  = 1;
    static const int VERTICAL_ALIGN_CENTER  = 2;
    static const int VERTICAL_ALIGN_BASELINE = 3;
    // wrap mode
    static const int WRAP_NONE      = 0;
    static const int WRAP_CHAIN     = 1;
    static const int WRAP_ALIGNED   = 2;
    static const int WRAP_CHAIN_NEW = 3;

    Flow();

    void copy(ConstraintWidget* src,
              std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map) override;

    // --- configuration setters ---
    void setOrientation(int value);
    void setHorizontalStyle(int value);
    void setVerticalStyle(int value);
    void setFirstHorizontalStyle(int value);
    void setFirstVerticalStyle(int value);
    void setLastHorizontalStyle(int value);
    void setLastVerticalStyle(int value);
    void setHorizontalBias(float value);
    void setVerticalBias(float value);
    void setFirstHorizontalBias(float value);
    void setFirstVerticalBias(float value);
    void setLastHorizontalBias(float value);
    void setLastVerticalBias(float value);
    void setHorizontalAlign(int value);
    void setVerticalAlign(int value);
    void setWrapMode(int value);
    void setHorizontalGap(int value);
    void setVerticalGap(int value);
    void setMaxElementsWrap(int value);

    // --- solver lifecycle ---
    void measure(int widthMode, int widthSize, int heightMode, int heightSize) override;
    void addToSolver(LinearSystem* system, bool optimize) override;

    std::string getType() const override; // "Flow"

    // A single chain row. Java's non-static inner class; holds a Flow back-pointer to reach the
    // owning Flow's gap/padding/displayed-widgets helpers (mirrors Flow.this access).
    class WidgetsList {
      public:
        WidgetsList(Flow* flow, int orientation,
                    ConstraintAnchor* left, ConstraintAnchor* top,
                    ConstraintAnchor* right, ConstraintAnchor* bottom, int max);

        ConstraintAnchor* mLeft = nullptr;
        ConstraintAnchor* mTop = nullptr;
        ConstraintAnchor* mRight = nullptr;
        ConstraintAnchor* mBottom = nullptr;
        int mPaddingLeft = 0, mPaddingTop = 0, mPaddingRight = 0, mPaddingBottom = 0;
        int mWidth = 0, mHeight = 0;
        int mStartIndex = 0, mCount = 0;
        int mNbMatchConstraintsWidgets = 0;
        int mMax = 0;
        ConstraintWidget* mBiggest = nullptr;
        int mBiggestDimension = 0;

        void setup(int orientation, ConstraintAnchor* left, ConstraintAnchor* top,
                   ConstraintAnchor* right, ConstraintAnchor* bottom,
                   int paddingLeft, int paddingTop, int paddingRight, int paddingBottom, int max);
        void clear();
        void setStartIndex(int value);
        int  getWidth() const;
        int  getHeight() const;
        void add(ConstraintWidget* widget);
        void createConstraints(bool isInRtl, int chainIndex, bool isLastChain);
        void measureMatchConstraints(int availableSpace);
        void recomputeDimensions();

      private:
        Flow* mFlow;
        int mOrientation = HORIZONTAL;
    };

    std::vector<WidgetsList> mChainList;

  private:
    // Bring VirtualLayout's protected measure(Widget*,...) helper into scope (the public 4-int
    // measure override would otherwise hide it).
    using VirtualLayout::measure;

    int getWidgetWidth(ConstraintWidget* widget, int max);
    int getWidgetHeight(ConstraintWidget* widget, int max);
    void measureChainWrap(std::vector<ConstraintWidget*>& widgets, int count, int orientation,
                          int max, std::vector<int>& measured);
    void measureNoWrap(std::vector<ConstraintWidget*>& widgets, int count, int orientation,
                       int max, std::vector<int>& measured);
    void measureAligned(std::vector<ConstraintWidget*>& widgets, int count, int orientation,
                        int max, std::vector<int>& measured);
    void measureChainWrap_new(std::vector<ConstraintWidget*>& widgets, int count, int orientation,
                              int max, std::vector<int>& measured);

    int mHorizontalStyle = UNKNOWN;
    int mVerticalStyle = UNKNOWN;
    int mFirstHorizontalStyle = UNKNOWN;
    int mFirstVerticalStyle = UNKNOWN;
    int mLastHorizontalStyle = UNKNOWN;
    int mLastVerticalStyle = UNKNOWN;
    float mHorizontalBias = 0.5f;
    float mVerticalBias = 0.5f;
    float mFirstHorizontalBias = 0.5f;
    float mFirstVerticalBias = 0.5f;
    float mLastHorizontalBias = 0.5f;
    float mLastVerticalBias = 0.5f;
    int mHorizontalGap = 0;
    int mVerticalGap = 0;
    int mHorizontalAlign = HORIZONTAL_ALIGN_CENTER;
    int mVerticalAlign = VERTICAL_ALIGN_CENTER;
    int mWrapMode = WRAP_NONE;
    int mMaxElementsWrap = UNKNOWN;
    int mOrientation = HORIZONTAL;

    std::vector<ConstraintWidget*> mDisplayedWidgets;
    int mDisplayedWidgetsCount = 0;
};

} // namespace cdroid::clcore

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_FLOW_H
