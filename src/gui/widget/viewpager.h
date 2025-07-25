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

#ifndef __VIEW_PAGER_H__
#define __VIEW_PAGER_H__
#include <view/viewgroup.h>
#include <widget/adapter.h>
#include <widget/edgeeffect.h>
#include <math.h>
#include <limits>
namespace cdroid{

class ViewPager:public ViewGroup{
public:
    class PageTransformer{
    public:
        virtual void transformPage(View& page, float position)=0;//typedef std::function<void(View&v,float position)>PageTransformer; 
    };
    class OnPageChangeListener:public virtual EventSet{
    public:
        std::function<void(int,float,int)>onPageScrolled;//void onPageScrolled(int position, float positionOffset, int positionOffsetPixels)
        std::function<void(int)>onPageSelected;//void onPageSelected(int position);
        std::function<void(int)>onPageScrollStateChanged;//void onPageScrollStateChanged(int state);
    };
private:
    static constexpr int MAX_SCROLL_X =2<<23;
    static constexpr bool USE_CACHE = true;
    static constexpr int DEFAULT_OFFSCREEN_PAGES = 1;
    static constexpr int MAX_SETTLE_DURATION = 600; // ms
    static constexpr int MIN_DISTANCE_FOR_FLING =25; // dips
    static constexpr int DEFAULT_GUTTER_SIZE = 16; // dips
    static constexpr int MIN_FLING_VELOCITY = 400; // dips
    static constexpr int INVALID_POINTER = -1;
    static constexpr int CLOSE_ENOUGH = 2; // dp
    static constexpr int DRAW_ORDER_DEFAULT = 0;
    static constexpr int DRAW_ORDER_FORWARD = 1;
    static constexpr int DRAW_ORDER_REVERSE = 2;
    
    class PagerObserver:public DataSetObserver {
    private:
        ViewPager*mVP;
    public:
        PagerObserver(ViewPager*vp){  mVP=vp; }
        void onChanged()override {  mVP->dataSetChanged(); }
        void onInvalidated()override { mVP->dataSetChanged(); }
        void clearSavedState()override{}
    };
public:
    static constexpr int SCROLL_STATE_IDLE = 0;
    /**Indicates that the pager is currently being dragged by the user.*/
    static constexpr int SCROLL_STATE_DRAGGING = 1;
    /** Indicates that the pager is in the process of settling to a final position.*/
    static constexpr int SCROLL_STATE_SETTLING = 2;
    DECLARE_UIEVENT(void,OnAdapterChangeListener,ViewPager&,PagerAdapter*oldAdapter,PagerAdapter*newAdapter);
    class LayoutParams :public ViewGroup::LayoutParams{
    public:
        /* true if this view is a decoration on the pager itself and not
         * a view supplied by the adapter. */
        bool isDecor;
        int gravity;
        float widthFactor = 0.f;
        bool needsMeasure;
        int position;
        int childIndex;
        LayoutParams();
        LayoutParams(Context* context,const AttributeSet& attrs);
    };
protected:
    struct ItemInfo {
        void* object;
        //Logical position of the item within the pager adapter
        int position;
        bool scrolling;
        float widthFactor;
        float offset;
        ItemInfo();
    };
    int mCurItem;
private:
    Interpolator* mInterpolator;
    Runnable mEndScrollRunnable;
    int mExpectedAdapterCount;
    std::vector<ItemInfo*>mItems ;
    ItemInfo mTempItem;
    int mRestoredCurItem = -1;
    Scroller* mScroller;
    bool mIsScrollStarted;
    int mScrollState;
    PagerObserver * mObserver;
    int mPageMargin;
    Drawable* mMarginDrawable;
    int mTopPageBounds;
    int mBottomPageBounds;

    int mLeftIncr = -1;
    // Offsets of the first and last items, if known.
    // Set during population, used to determine if we are at the beginning
    // or end of the pager data set during touch scrolling.
    float mFirstOffset;//Float.MAX_VALUE;
    float mLastOffset ;//Float.MAX_VALUE;
    int mChildWidthMeasureSpec;
    int mChildHeightMeasureSpec;
    int mPageTransformerLayerType;
    bool mInLayout;
    std::vector<OnPageChangeListener> mOnPageChangeListeners;
    OnPageChangeListener mInternalPageChangeListener;
    PageTransformer* mPageTransformer;

    bool mScrollingCacheEnabled;

    bool mPopulatePending;
    int mOffscreenPageLimit = DEFAULT_OFFSCREEN_PAGES;

    bool mIsBeingDragged;
    bool mIsUnableToDrag;
    int mDefaultGutterSize;
    int mGutterSize;
    int mTouchSlop;

    /**Position of the last motion event.*/
    float mLastMotionX;
    float mLastMotionY;
    float mInitialMotionX;
    float mInitialMotionY;
    int mActivePointerId = INVALID_POINTER;

    /**
     * Determines speed during touch scrolling
     */
    VelocityTracker* mVelocityTracker;
    int mMinimumVelocity;
    int mMaximumVelocity;
    int mFlingDistance;
    int mCloseEnough;
    bool mFakeDragging;
    int64_t mFakeDragBeginTime;


    EdgeEffect* mLeftEdge;
    EdgeEffect* mRightEdge;

    bool mFirstLayout = true;
    bool mNeedCalculatePageOffsets =false;
    bool mCalledSuper;
    int mDecorChildCount;
    int mDrawingOrder;
    std::vector<View*>mDrawingOrderedChildren;
    PagerAdapter* mAdapter;
    std::vector<OnAdapterChangeListener> mAdapterChangeListeners;
private:
    void removeNonDecorViews();
    int getClientWidth();
    bool isGutterDrag(float x, float dx);
    void scrollToItem(int item, bool smoothScroll, int velocity, bool dispatchSelected);
    void completeScroll(bool postEvents);
    bool pageScrolled(int xpos);
    void sortChildDrawingOrder();
    void calculatePageOffsets(ItemInfo* curItem, int curIndex, ItemInfo* oldCurInfo);
    void recomputeScrollPosition(int width, int oldWidth, int margin, int oldMargin);
    int determineTargetPage(int currentPage, float pageOffset, int velocity, int deltaX);
    Rect getChildRectInPagerCoordinates(Rect& outRect, View* child);
    void onSecondaryPointerUp(MotionEvent& ev);
    bool canScroll();
    bool canScroll(View* v, bool checkV, int dx, int x, int y);
    bool performDrag(float x);
    ItemInfo* infoForCurrentScrollPosition();
    void endDrag();
    bool resetTouch();
    void requestParentDisallowInterceptTouchEvent(bool disallowIntercept);
    void setScrollingCacheEnabled(bool enabled);

    void dispatchOnPageScrolled(int position, float offset, int offsetPixels);
    void dispatchOnPageSelected(int position);
    void dispatchOnScrollStateChanged(int state);
    void enableLayers(bool enable);
protected:
    ItemInfo* infoForChild(View* child);
    ItemInfo* infoForAnyChild(View* child);
    ItemInfo* infoForPosition(int position)const;
    void initViewPager();
    int getChildDrawingOrder(int childCount, int i)override;
    void onDetachedFromWindow()override;
    void setScrollState(int newState);
    void setCurrentItemInternal(int item, bool smoothScroll, bool always);
    void setCurrentItemInternal(int item, bool smoothScroll, bool always,int velocity);
    void drawableStateChanged()override;
    bool verifyDrawable(Drawable* who)const override;
    ItemInfo* addNewItem(int position, int index);
    void dataSetChanged();
    void populate();
    void populate(int newCurrentItem);
    bool pageLeft();
    bool pageRight();
    bool onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect)override;
    void onAttachedToWindow()override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void onPageScrolled(int position, float offset, int offsetPixels);
    void draw(Canvas& canvas)override;
    void onDraw(Canvas& canvas)override;
    LayoutParams* generateDefaultLayoutParams();
    LayoutParams* generateLayoutParams(ViewGroup::LayoutParams* p);
    bool checkLayoutParams(ViewGroup::LayoutParams* p);
public:
    ViewPager(int w,int h);
    ViewPager(Context* context,const AttributeSet& attrs);
    ~ViewPager();
    void setAdapter(PagerAdapter* adapter);
    PagerAdapter* getAdapter();
    void addOnAdapterChangeListener(OnAdapterChangeListener listener);
    void removeOnAdapterChangeListener(OnAdapterChangeListener listener);

    void addOnPageChangeListener(const OnPageChangeListener& listener);
    void removeOnPageChangeListener(const OnPageChangeListener& listener);
    void clearOnPageChangeListeners();
    OnPageChangeListener setInternalPageChangeListener(OnPageChangeListener listener);
    int getCurrentItem()const;
    void setCurrentItem(int item);
    void setCurrentItem(int item, bool smoothScroll);
    void setPageTransformer(bool reverseDrawingOrder, PageTransformer* transformer);
    int getOffscreenPageLimit()const;
    void setOffscreenPageLimit(int limit);
    int getPageMargin()const;
    void setPageMargin(int marginPixels);
    void setPageMarginDrawable(Drawable* d);
    void setPageMarginDrawable(const std::string&resId);
    void smoothScrollTo(int x, int y);
    void smoothScrollTo(int x, int y, int velocity);
    void addView(View* child, int index, ViewGroup::LayoutParams* params)override;
    void removeView(View* view)override;
    void* getCurrent()const;
    void computeScroll()override;
    bool executeKeyEvent(KeyEvent& event);
    bool dispatchKeyEvent(KeyEvent& event)override;
    bool onInterceptTouchEvent(MotionEvent& ev)override;
    bool onTouchEvent(MotionEvent& ev)override;
    bool arrowScroll(int direction);
    bool canScrollHorizontally(int direction)override;
    void addFocusables(std::vector<View*>& views, int direction, int focusableMode)override;
    void addTouchables(std::vector<View*>& views)override;
    LayoutParams* generateLayoutParams(const AttributeSet& attrs);
    void onRtlPropertiesChanged(int layoutDirection)override;

    bool beginFakeDrag();
    void endFakeDrag();
    void fakeDragBy(float xOffset);
    bool isFakeDragging()const;
};

}//endof namespace

#endif
