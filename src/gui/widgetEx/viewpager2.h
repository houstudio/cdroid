#ifndef __VIEWPAGER2_H__
#define __VIEWPAGER2_H__
#include <view/viewgroup.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/pagersnaphelper.h>

namespace cdroid{

class PagerSnapHelper;
class FakeDrag;
class ScrollEventAdapter;
class CompositeOnPageChangeCallback;
class PageTransformerAdapter;
class AccessibilityViewCommand;
class ViewPager2:public ViewGroup {
public:
    class PageTransformer {
    public:
        virtual void transformPage(View& var1, float var2)=0;
    };	
    class OnPageChangeCallback:public EventSet{
    public:
        CallbackBase<void,int,float,int> onPageScrolled;//(int position, float positionOffset, int positionOffsetPixels);
        CallbackBase<void,int>onPageSelected;//(int position);
        CallbackBase<void,int>onPageScrollStateChanged;//(int state);
    };
protected:
    /** Feature flag while stabilizing enhanced a11y */
    static constexpr bool sFeatureEnhancedA11yEnabled = true;
    class SavedState;
public:
    static constexpr int ORIENTATION_HORIZONTAL = RecyclerView::HORIZONTAL;
    static constexpr int ORIENTATION_VERTICAL = RecyclerView::VERTICAL;
    static constexpr int SCROLL_STATE_IDLE = 0;
    static constexpr int SCROLL_STATE_DRAGGING = 1;
    static constexpr int SCROLL_STATE_SETTLING = 2;
    static constexpr int OFFSCREEN_PAGE_LIMIT_DEFAULT = -1;
private:
    class RecyclerViewImpl;
    class LinearLayoutManagerImpl;
    class PagerSnapHelperImpl;
    class AccessibilityProvider;
    class BasicAccessibilityProvider;
    class PageAwareAccessibilityProvider;
    class DataSetChangeObserver;
    RecyclerView::AdapterDataObserver* mCurrentItemDataSetChangeObserver;
    // reused in layout(...)
    Runnable mSmoothScrollToPositionRunnable;
    CompositeOnPageChangeCallback* mExternalPageChangeCallbacks;
    LinearLayoutManager* mLayoutManager;
    int mPendingCurrentItem = RecyclerView::NO_POSITION;
    Parcelable* mPendingAdapterState;
    PagerSnapHelper* mPagerSnapHelper;
    CompositeOnPageChangeCallback* mPageChangeEventDispatcher;
    FakeDrag* mFakeDragger;
    PageTransformerAdapter* mPageTransformerAdapter;
    RecyclerView::ItemAnimator* mSavedItemAnimator = nullptr;
    bool mSavedItemAnimatorPresent = false;
    bool mUserInputEnabled = true;
    int mOffscreenPageLimit = OFFSCREEN_PAGE_LIMIT_DEFAULT;
protected:
    int mCurrentItem;
    bool mCurrentItemDirty = false;
    RecyclerViewImpl* mRecyclerView;
    ScrollEventAdapter* mScrollEventAdapter;
    AccessibilityProvider* mAccessibilityProvider; //to avoid creation of a synthetic accessor
private:
    void initialize(Context* context, const AttributeSet& attrs);
    void setOrientation(Context* context,const AttributeSet& attrs);
    void restorePendingState();
    void unregisterCurrentItemDataSetTracker(RecyclerView::Adapter*adapter);
protected:
    friend class FakeDrag;
    friend class ScrollEventAdapter;
    Parcelable* onSaveInstanceState()override;
    void onRestoreInstanceState(Parcelable& state)override;
    void dispatchRestoreInstanceState(SparseArray<Parcelable*>& container)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;

    void onViewAdded(View* child)override;
    void updateCurrentItem();
    int getPageSize()const;
    void setCurrentItemInternal(int item, bool smoothScroll);
    void snapToPage();
public:
    ViewPager2(int w,int h);
    ViewPager2(Context* context, const AttributeSet& attrs);
    ~ViewPager2()override;
    void setAdapter(RecyclerView::Adapter* adapter);
    void registerCurrentItemDataSetTracker(RecyclerView::Adapter* adapter);
    RecyclerView::Adapter* getAdapter();
    void setOrientation(int orientation);
    int getOrientation()const;
    bool isRtl()const;
    void setCurrentItem(int item);
    void setCurrentItem(int item, bool smoothScroll);
    int getCurrentItem()const;
    int getScrollState()const;
    bool beginFakeDrag();
    bool fakeDragBy(float offsetPxFloat);
    bool endFakeDrag();
    bool isFakeDragging();
    void setUserInputEnabled(bool enabled);

    bool isUserInputEnabled()const;

    void setOffscreenPageLimit(int limit);
    int getOffscreenPageLimit()const;
    virtual bool canScrollHorizontally(int direction)const;
    virtual bool canScrollVertically(int direction)const;
    void registerOnPageChangeCallback(OnPageChangeCallback callback);
    void unregisterOnPageChangeCallback(OnPageChangeCallback callback);
    void setPageTransformer(PageTransformer* transformer);
    void requestTransform();
    View&setLayoutDirection(int layoutDirection)override;
    void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info)override;
    bool performAccessibilityAction(int action, Bundle* arguments)override;

    void addItemDecoration(RecyclerView::ItemDecoration* decor);
    void addItemDecoration(RecyclerView::ItemDecoration* decor, int index);
    RecyclerView::ItemDecoration* getItemDecorationAt(int index);
    int getItemDecorationCount()const;
    void invalidateItemDecorations();
    void removeItemDecorationAt(int index);
    void removeItemDecoration(RecyclerView::ItemDecoration* decor);
};/*endof ViewPager2*/

class ViewPager2::AccessibilityProvider {
protected:
    ViewPager2*mVP;
public:
    AccessibilityProvider(ViewPager2*);
    virtual void onInitialize(CompositeOnPageChangeCallback* pageChangeEventDispatcher,RecyclerView* recyclerView);
    virtual bool handlesGetAccessibilityClassName();
    virtual std::string onGetAccessibilityClassName();
    virtual void onRestorePendingState();
    virtual void onAttachAdapter(RecyclerView::Adapter*newAdapter);
    virtual void onDetachAdapter(RecyclerView::Adapter*oldAdapter);
    virtual void onSetOrientation();
    virtual void onSetNewCurrentItem();
    virtual void onSetUserInputEnabled();
    virtual void onSetLayoutDirection();
    virtual void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info);
    virtual bool handlesPerformAccessibilityAction(int action, Bundle* arguments);
    virtual bool onPerformAccessibilityAction(int action, Bundle* arguments);
    virtual void onRvInitializeAccessibilityEvent(AccessibilityEvent& event);
    virtual bool handlesLmPerformAccessibilityAction(int action);
    virtual bool onLmPerformAccessibilityAction(int action);
    virtual void onLmInitializeAccessibilityNodeInfo(AccessibilityNodeInfo&info);
    virtual bool handlesRvGetAccessibilityClassName();
    virtual std::string onRvGetAccessibilityClassName();
};

class ViewPager2::BasicAccessibilityProvider:public ViewPager2::AccessibilityProvider {
public:
    BasicAccessibilityProvider(ViewPager2*);
    bool handlesLmPerformAccessibilityAction(int action)override;
    bool onLmPerformAccessibilityAction(int action)override;
    void onLmInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info)override;
    bool handlesRvGetAccessibilityClassName()override;
    std::string onRvGetAccessibilityClassName()override;
};

class ViewPager2::PageAwareAccessibilityProvider:public ViewPager2::AccessibilityProvider {
private:
    AccessibilityViewCommand* mActionPageForward;
    AccessibilityViewCommand* mActionPageBackward;
    RecyclerView::AdapterDataObserver* mAdapterDataObserver;
    void addCollectionInfo(AccessibilityNodeInfo& info);
    void addScrollActions(AccessibilityNodeInfo& info);
protected:
    void setCurrentItemFromAccessibilityCommand(int item);
    void updatePageAccessibilityActions();
public:
    PageAwareAccessibilityProvider(ViewPager2*);
    void onInitialize(CompositeOnPageChangeCallback* pageChangeEventDispatcher, RecyclerView* recyclerView)override;
    bool handlesGetAccessibilityClassName()override;
    std::string onGetAccessibilityClassName()override;
    void onRestorePendingState()override;
    void onAttachAdapter(RecyclerView::Adapter*newAdapter)override;
    void onDetachAdapter(RecyclerView::Adapter*oldAdapter)override;
    void onSetOrientation()override;
    void onSetNewCurrentItem()override;
    void onSetUserInputEnabled()override;
    void onSetLayoutDirection()override;
    void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info)override;
    bool handlesPerformAccessibilityAction(int action, Bundle* arguments)override;
    bool onPerformAccessibilityAction(int action, Bundle* arguments)override;
    void onRvInitializeAccessibilityEvent(AccessibilityEvent& event)override;
};

class ViewPager2::SavedState:public BaseSavedState {
protected:
    int mRecyclerViewId;
    int mCurrentItem;
    Parcelable* mAdapterState;
    friend ViewPager2;
public:
    SavedState(Parcel& source);
    SavedState(Parcelable& superState);
    //void readValues(Parcel source, ClassLoader loader);//private
    void writeToParcel(Parcel& out, int flags);
};

class ViewPager2::RecyclerViewImpl:public RecyclerView {
private:
    friend ViewPager2;
    ViewPager2*mVP;
public:
    RecyclerViewImpl(Context* context,const AttributeSet&,ViewPager2*);
    std::string getAccessibilityClassName()const override;
    void onInitializeAccessibilityEvent(AccessibilityEvent& event)override;
    bool onTouchEvent(MotionEvent& event)override;
    bool onInterceptTouchEvent(MotionEvent& ev)override;
};

class ViewPager2::LinearLayoutManagerImpl:public LinearLayoutManager {
protected:
    ViewPager2*mVP;
protected:
    void calculateExtraLayoutSpace(RecyclerView::State& state,int extraLayoutSpace[2])override;
public:
    LinearLayoutManagerImpl(Context* context,ViewPager2*vp);
    bool performAccessibilityAction(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, int action, Bundle* args)override;
    void onInitializeAccessibilityNodeInfo(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, AccessibilityNodeInfo& info)override;
    bool requestChildRectangleOnScreen(RecyclerView& parent,View& child,const Rect& rect, bool immediate,bool focusedChildVisible)override;
};

class ViewPager2::PagerSnapHelperImpl:public PagerSnapHelper {
public:
    PagerSnapHelperImpl();
    View* findSnapView(RecyclerView::LayoutManager& layoutManager);
};

class ViewPager2::DataSetChangeObserver:public RecyclerView::AdapterDataObserver {
protected:
    ViewPager2*mVP;
public:
    DataSetChangeObserver(ViewPager2*v);
    void onChanged()override;

    virtual void onItemRangeChanged(int positionStart, int itemCount)final;
    virtual void onItemRangeChanged(int positionStart, int itemCount,Object* payload)final;
    void onItemRangeInserted(int positionStart, int itemCount)override;
    void onItemRangeRemoved(int positionStart, int itemCount)override;
    void onItemRangeMoved(int fromPosition, int toPosition, int itemCount)override;
};

}/*endof namespace*/
#endif/*__VIEWPAGER2_H__*/

