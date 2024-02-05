#ifndef __VIEWPAGER2_H__
#define __VIEWPAGER2_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/pagersnaphelper.h>
namespace cdroid{
class PagerSnapHelper;
class FakeDrag;
class ScrollEventAdapter;
class CompositeOnPageChangeCallback;
class PageTransformerAdapter;
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
    class RecyclerViewImpl;
    class PagerSnapHelperImpl;
    RecyclerView::AdapterDataObserver* mCurrentItemDataSetChangeObserver;
    // reused in layout(...)
    Rect mTmpContainerRect;
    Rect mTmpChildRect;
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
    //AccessibilityProvider* mAccessibilityProvider; // to avoid creation of a synthetic accessor
private:
    class RecyclerViewImpl;
    void initialize(Context* context, const AttributeSet& attrs);
    //RecyclerView::OnChildAttachStateChangeListener enforceChildFillListener();
    void setOrientation(Context* context,const AttributeSet& attrs);
    void restorePendingState();
    void unregisterCurrentItemDataSetTracker(RecyclerView::Adapter*adapter);
protected:
    friend class FakeDrag;
    friend class ScrollEventAdapter;
    //Parcelable* onSaveInstanceState()override;
    //void onRestoreInstanceState(Parcelable& state)override;
    //void dispatchRestoreInstanceState(SparseArray<Parcelable*>& container)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;

    void onViewAdded(View* child)override;
    void updateCurrentItem();
    int getPageSize();
    void setCurrentItemInternal(int item, bool smoothScroll);
    void snapToPage();
public:
    ViewPager2(Context* context, const AttributeSet& attrs);
    //CharSequence getAccessibilityClassName()override;

    void setAdapter(RecyclerView::Adapter* adapter);
    void registerCurrentItemDataSetTracker(RecyclerView::Adapter* adapter);
    RecyclerView::Adapter* getAdapter();
    void setOrientation(int orientation);
    int getOrientation();
    bool isRtl();
    void setCurrentItem(int item);
    void setCurrentItem(int item, bool smoothScroll);
    int getCurrentItem()const;
    int getScrollState();
    bool beginFakeDrag();
    bool fakeDragBy(float offsetPxFloat);
    bool endFakeDrag();
    bool isFakeDragging();
    void setUserInputEnabled(bool enabled);

    bool isUserInputEnabled();

    void setOffscreenPageLimit(int limit);
    int getOffscreenPageLimit()const;
    virtual bool canScrollHorizontally(int direction)const;
    virtual bool canScrollVertically(int direction)const;
    void registerOnPageChangeCallback(OnPageChangeCallback callback);
    void unregisterOnPageChangeCallback(OnPageChangeCallback callback);
    void setPageTransformer(PageTransformer* transformer);
    void requestTransform();
    View&setLayoutDirection(int layoutDirection)override;
    //void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info)override;
    bool performAccessibilityAction(int action, Bundle arguments);

#if 0
#endif

    void addItemDecoration(RecyclerView::ItemDecoration* decor);
    void addItemDecoration(RecyclerView::ItemDecoration* decor, int index);
    RecyclerView::ItemDecoration* getItemDecorationAt(int index);
    int getItemDecorationCount();
    void invalidateItemDecorations();
    void removeItemDecorationAt(int index);
    void removeItemDecoration(RecyclerView::ItemDecoration* decor);
#if 0
    private class AccessibilityProvider {
        void onInitialize(@NonNull CompositeOnPageChangeCallback pageChangeEventDispatcher,RecyclerView recyclerView);
        virtual bool handlesGetAccessibilityClassName();
        virtual String onGetAccessibilityClassName() {
        virtual void onRestorePendingState(){}
        virtual void onAttachAdapter(@Nullable Adapter<?> newAdapter){}
        virtual void onDetachAdapter(@Nullable Adapter<?> oldAdapter){}
        virtual void onSetOrientation(){}
        virtual void onSetNewCurrentItem(){}
        virtual void onSetUserInputEnabled(){}
        virtual void onSetLayoutDirection(){}
        virtual void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info) { }
        virtual bool handlesPerformAccessibilityAction(int action, Bundle arguments) {
            return false;
        }

        virtual bool onPerformAccessibilityAction(int action, Bundle arguments) {
            throw new IllegalStateException("Not implemented.");
        }

        virtual void onRvInitializeAccessibilityEvent(@NonNull AccessibilityEvent event){} 

        virtual bool handlesLmPerformAccessibilityAction(int action) {
            return false;
        }

        virtual bool onLmPerformAccessibilityAction(int action) {
            throw new IllegalStateException("Not implemented.");
        }

        virtual void onLmInitializeAccessibilityNodeInfo(@NonNull AccessibilityNodeInfoCompat info) {
        }

        virtual bool handlesRvGetAccessibilityClassName() {
            return false;
        }

        virtual CharSequence onRvGetAccessibilityClassName() {
            throw new IllegalStateException("Not implemented.");
        }
    };

    class BasicAccessibilityProvider:public AccessibilityProvider {
    public:
	bool handlesLmPerformAccessibilityAction(int action)override;
        bool onLmPerformAccessibilityAction(int action)override
        void onLmInitializeAccessibilityNodeInfo(AccessibilityNodeInfoCompat info)override

        bool handlesRvGetAccessibilityClassName()override
        CharSequence onRvGetAccessibilityClassName()override;
    };

    class PageAwareAccessibilityProvider:public AccessibilityProvider {
    private:
	AccessibilityViewCommand mActionPageForward;
        AccessibilityViewCommand mActionPageBackward;
        RecyclerView::AdapterDataObserver mAdapterDataObserver;
        void addCollectionInfo(AccessibilityNodeInfo info);
        void addScrollActions(AccessibilityNodeInfo info);
    protected:
        void setCurrentItemFromAccessibilityCommand(int item);
        void updatePageAccessibilityActions();
    public:
        void onInitialize(@NonNull CompositeOnPageChangeCallback pageChangeEventDispatcher,
                @NonNull RecyclerView recyclerView)override;
        bool handlesGetAccessibilityClassName()override;
        String onGetAccessibilityClassName()override;
        void onRestorePendingState()override;
        void onAttachAdapter(@Nullable Adapter<?> newAdapter)override;
        void onDetachAdapter(@Nullable Adapter<?> oldAdapter)override;
        void onSetOrientation()override;
        void onSetNewCurrentItem()override;
        void onSetUserInputEnabled()override;
        void onSetLayoutDirection()override;
        void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info)override;
        bool handlesPerformAccessibilityAction(int action, Bundle arguments)override;
        bool onPerformAccessibilityAction(int action, Bundle arguments)override;
        void onRvInitializeAccessibilityEvent(@NonNull AccessibilityEvent event)override;
    };
#endif
};/*endof ViewPager2*/
#if 0
class ViewPager2::SavedState:public AbsSavedState{//BaseSavedState {
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
#endif
class ViewPager2::RecyclerViewImpl:public RecyclerView {
private:
    friend ViewPager2;
    ViewPager2*mVP;
public:
    RecyclerViewImpl(Context* context,const AttributeSet&);
    //CharSequence getAccessibilityClassName()override;
    //void onInitializeAccessibilityEvent(AccessibilityEvent event)override;
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
    /*bool performAccessibilityAction(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, int action, Bundle args)override;
    void onInitializeAccessibilityNodeInfo(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, AccessibilityNodeInfo& info)override;*/
    bool requestChildRectangleOnScreen(RecyclerView& parent,View& child,const Rect& rect, bool immediate,bool focusedChildVisible)override;
};

class ViewPager2::PagerSnapHelperImpl:public PagerSnapHelper {
public:
    PagerSnapHelperImpl();
    View* findSnapView(RecyclerView::LayoutManager& layoutManager);
};

class ViewPager2DataSetChangeObserver:public RecyclerView::AdapterDataObserver {
public:
    virtual void onChanged()=0;

    virtual void onItemRangeChanged(int positionStart, int itemCount)final{
        onChanged();
    }
    virtual void onItemRangeChanged(int positionStart, int itemCount,
        Object* payload)final{
        onChanged();
    }
    void onItemRangeInserted(int positionStart, int itemCount) {
        onChanged();
    }
    void onItemRangeRemoved(int positionStart, int itemCount)final{
        onChanged();
    }
    void onItemRangeMoved(int fromPosition, int toPosition, int itemCount)final{
        onChanged();
    }
};

}/*endof namespace*/
#endif/*__VIEWPAGER2_H__*/

