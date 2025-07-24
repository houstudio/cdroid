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
#ifndef __RELATIVE_LAYOUT_H__
#define __RELATIVE_LAYOUT_H__
#include <view/viewgroup.h>
#include <list>
#include <set>
namespace cdroid{

class RelativeLayout:public ViewGroup{
public:
    static constexpr int LTRUE = -1;
    static constexpr int LEFT_OF                  = 0;
    static constexpr int RIGHT_OF                 = 1;
    static constexpr int ABOVE                    = 2;
    static constexpr int BELOW                    = 3;
    static constexpr int ALIGN_BASELINE           = 4;
    static constexpr int ALIGN_LEFT               = 5;
    static constexpr int ALIGN_TOP                = 6;
    static constexpr int ALIGN_RIGHT              = 7;
    static constexpr int ALIGN_BOTTOM             = 8;
    static constexpr int ALIGN_PARENT_LEFT        = 9;
    static constexpr int ALIGN_PARENT_TOP         = 10;
    static constexpr int ALIGN_PARENT_RIGHT       = 11;
    static constexpr int ALIGN_PARENT_BOTTOM      = 12;
    static constexpr int CENTER_IN_PARENT         = 13;
    static constexpr int CENTER_HORIZONTAL        = 14;
    static constexpr int CENTER_VERTICAL          = 15;
    static constexpr int START_OF                 = 16;
    static constexpr int END_OF                   = 17;
    static constexpr int ALIGN_START              = 18;
    static constexpr int ALIGN_END                = 19;
    static constexpr int ALIGN_PARENT_START       = 20;
    static constexpr int ALIGN_PARENT_END         = 21;
    static constexpr int VERB_COUNT               = 22;

    class LayoutParams:public MarginLayoutParams{
    private:
        int mRules[VERB_COUNT];
        int mInitialRules[VERB_COUNT];
        int mLeft, mTop, mRight, mBottom;
        bool mNeedsLayoutResolution;
        bool mRulesChanged = false;
        bool mIsRtlCompatibilityMode = false;
        friend class RelativeLayout;

        bool hasRelativeRules();
        bool isRelativeRule(int rule);
        void resolveRules(int layoutDirection);
        bool shouldResolveLayoutDirection(int layoutDirection);
    public:
        LayoutParams(int w, int h);
        LayoutParams(const ViewGroup::LayoutParams& source);
        LayoutParams(const ViewGroup::MarginLayoutParams& source);
        LayoutParams(const LayoutParams& source);
        LayoutParams(Context*,const AttributeSet&atts);
        void resolveLayoutDirection(int layoutDirection)override;
        bool alignWithParent;
        void addRule(int verb);
        void addRule(int verb, int subject);
        void removeRule(int verb);
        int getRule(int verb);
        const int* getRules(int layoutDirection);
        const int* getRules();
    };
private:
    class DependencyGraph{
    public:
        class Node{
        public:
            View*view;
            /*The list of dependents for this node; a dependent is a node
             * that needs this node to be processed first.*/
            std::unordered_map<Node*,DependencyGraph*>dependents;
            /** The list of dependencies for this node.*/
            SparseArray<Node*> dependencies;
            Node(View*v);
            ~Node();
        };
        std::vector<Node*>mNodes;
        SparseArray<Node*> mKeyNodes;
        std::list<Node*>mRoots;
        DependencyGraph();
        ~DependencyGraph();
        void clear();
        void add(View* view);
        void getSortedViews(std::vector<View*>&sorted,const int* rules,size_t ruleCount);
        std::list<Node*> findRoots(const int* rulesFilter,size_t ruleCount);
    };
    struct TopToBottomLeftToRightComparator{
        bool operator()(const View* lhs, const View* rhs)const;
    };
private:

    /* Used to indicate left/right/top/bottom should be inferred from constraints*/
    static constexpr int VALUE_NOT_SET = INT_MIN;

    View* mBaselineView = nullptr;

    int mGravity = Gravity::START | Gravity::TOP;
    Rect mContentBounds;
    Rect mSelfBounds;
    int mIgnoreGravity;

    std::set<View*,TopToBottomLeftToRightComparator> mTopToBottomLeftToRightSet;

    bool mDirtyHierarchy;
    std::vector<View*> mSortedHorizontalChildren;
    std::vector<View*> mSortedVerticalChildren;
    DependencyGraph *mGraph;

    // Compatibility hack. Old versions of the platform had problems
    // with MeasureSpec value overflow and RelativeLayout was one source of them.
    // Some apps came to rely on them. :(
    bool mAllowBrokenMeasureSpecs = false;
    // Compatibility hack. Old versions of the platform would not take
    // margins and padding into account when generating the height measure spec
    // for children during the horizontal measure pass.
    bool mMeasureVerticalWithPaddingMargin = true;

    // A default width used for RTL measure pass
    /** Value reduced so as not to interfere with View's measurement spec. flags. See:
     * {@link View#MEASURED_SIZE_MASK}.
     * {@link View#MEASURED_STATE_TOO_SMALL}.*/
    static constexpr int DEFAULT_WIDTH = 0x00010000; 

private://function
    void sortChildren();
    int compareLayoutPosition(const LayoutParams* p1,const LayoutParams* p2);
    void measureChild(View* child, LayoutParams* params, int myWidth, int myHeight);
    void measureChildHorizontal(View* child, LayoutParams* params, int myWidth, int myHeight);
    int getChildMeasureSpec(int childStart, int childEnd, int childSize, 
           int startMargin, int endMargin, int startPadding,int endPadding, int mySize);
    bool positionChildHorizontal(View* child, LayoutParams* params, int myWidth,bool wrapContent);
    void positionAtEdge(View* child, LayoutParams* params, int myWidth);
    bool positionChildVertical(View* child, LayoutParams* params, int myHeight, bool wrapContent);
    void applyHorizontalSizeRules(RelativeLayout::LayoutParams* childParams, int myWidth,const int*rules);
    void applyVerticalSizeRules(RelativeLayout::LayoutParams* childParams, int myHeight, int myBaseline);
    View* getRelatedView(const int* rules, int relation);
    RelativeLayout::LayoutParams* getRelatedViewParams(const int* rules, int relation);
    int getRelatedViewBaselineOffset(const int* rules);
    static void centerHorizontal(View* child, LayoutParams* params, int myWidth);
    static void centerVertical(View* child, LayoutParams* params, int myHeight);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* lp)const override;
public:
    RelativeLayout(int w,int h);
    RelativeLayout(Context* context,const AttributeSet& attrs);
    ~RelativeLayout()override;
    bool shouldDelayChildPressedState()override;
    int getIgnoreGravity()const;
    int getGravity()const;
    void setGravity(int);
    void setHorizontalGravity(int horizontalGravity);
    void setVerticalGravity(int verticalGravity);
    int getBaseline()override;
    void requestLayout()override;
    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;

    bool dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event)override;
    std::string getAccessibilityClassName()const override;
};

}//endof namespace
#endif //__RELATIVE_LAYOUT_H____RELATIVE_LAYOUT_H__
