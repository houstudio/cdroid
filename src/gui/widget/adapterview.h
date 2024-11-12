#ifndef __ADAPTER_VIEW_H__
#define __ADAPTER_VIEW_H__
#include <view/menu.h>
#include <view/viewgroup.h>
#include <widget/adapter.h>
namespace cdroid{
class AdapterView:public ViewGroup{
friend class AdapterDataSetObserver;
public:
    enum{
        ITEM_VIEW_TYPE_IGNORE=-1,
	    ITEM_VIEW_TYPE_HEADER_OR_FOOTER=-2	
    };
    enum{
        SYNC_SELECTED_POSITION =0,
        SYNC_FIRST_POSITION =1,
        INVALID_ROW_ID   =-1,
        INVALID_POSITION =-1,
        SYNC_MAX_DURATION_MILLIS=100
    };
    struct OnItemSelectedListener{
        std::function<void(AdapterView&parent, View& view, int position, long id)>onItemSelected;
        std::function<void(AdapterView&parent)>onNothingSelected;
    };
    typedef std::function<void (AdapterView& parent,View& view, int position, long id)>OnItemClickListener;
    typedef std::function<bool (AdapterView& parent,View& view, int position, long id)>OnItemLongClickListener;
    
    class AdapterContextMenuInfo:public ContextMenu::ContextMenuInfo {
    public: 
        AdapterContextMenuInfo(View* targetView, int position, long id);
        /**
         * The child view for which the context menu is being displayed. This
         * will be one of the children of this AdapterView.
         */
        View* targetView;

        /**
         * The position in the adapter for which the context menu is being
         * displayed.
         */
        int position;

        /**
         * The row id of the item for which the context menu is being displayed.
         */
        long id;
    };
private:
    int mDesiredFocusableState;
    Parcelable mInstanceState;
    Runnable mSelectionNotifier;
    Runnable mPendingSelectionNotifier;
    bool mDesiredFocusableInTouchModeState;
    void initAdapterView();
    void updateEmptyStatus(bool empty);
    void dispatchOnItemSelected();
    void fireOnSelected();
    void doSectionNotify();
protected:
    bool mDataChanged;
    bool mInLayout;
    bool mIsVertical;
    int mLayoutHeight;
    int mFirstPosition;
    int mSpecificTop;
    int mOldItemCount;
    int mItemCount;
    int mSelectedPosition;
    long mSelectedRowId;
    int mNextSelectedPosition;
    long mNextSelectedRowId;
    int mOldSelectedPosition;
    long mOldSelectedRowId;
    int mSyncMode;
    int mSyncPosition;
    long mSyncRowId;
    long mSyncHeight;
    bool mNeedSync;
    bool mBlockLayoutRequests;
    Adapter *mAdapter;
    View*mEmptyView;
    OnItemSelectedListener mOnItemSelectedListener;
    OnItemClickListener mOnItemClickListener;
    OnItemLongClickListener mOnItemLongClickListener;
    void onDetachedFromWindow()override;
    void onDatasetChange(int );
    void onLayout(bool changed, int left, int top, int width, int height)override; 
    virtual void handleDataChanged();
    void checkSelectionChanged();
    virtual void selectionChanged();
    void checkFocus();
    void rememberSyncState();
    int  findSyncPosition();
public:
    AdapterView(int w,int h);
    AdapterView(Context*ctx,const AttributeSet&atts);
    virtual ~AdapterView();
    virtual Adapter*getAdapter();
    virtual void setAdapter(Adapter*)=0;
    int getFirstVisiblePosition();
    int getLastVisiblePosition();
    int getPositionForView(View* view);
    //Sets the view to show if the adapter is empty
    void setEmptyView(View* emptyView);
    View* getEmptyView();
    void *getItemAtPosition(int position);
    long getItemIdAtPosition(int position);
    View&addView(View*,int,LayoutParams*)override;
    void removeView(View* child)override;
    void removeViewAt(int index)override;
    void removeAllViews()override;
    int getSelectedItemPosition();
    long getSelectedItemId();
    void*getSelectedItem();
    void setNextSelectedPositionInt(int position);
    virtual void setSelectedPositionInt(int position);
    virtual int lookForSelectablePosition(int position, bool lookDown);
    virtual int getCount();
    bool isInFilterMode();
    void setFocusable(int focusable)override;
    void setFocusableInTouchMode(bool focusable)override;
    virtual View* getSelectedView()=0;
    virtual void setSelection(int position)=0;
    virtual bool performItemClick(View& view, int position, long id);
    
    void setOnItemClickListener(OnItemClickListener listener);
    OnItemClickListener getOnItemClickListener() const;
    void setOnItemSelectedListener(OnItemSelectedListener listener);
    OnItemSelectedListener getOnItemSelectedListener()const;
    void setOnItemLongClickListener(OnItemLongClickListener listener);
    OnItemLongClickListener getOnItemLongClickListener() const;
};

class AdapterDataSetObserver:public DataSetObserver{
protected:
    AdapterView*mAdapterView;
public:
    AdapterDataSetObserver(AdapterView*adv);
    AdapterView*getAdapterView()const;
    void onChanged()override;
    void onInvalidated()override;
    void clearSavedState()override;
};

}//namespace

#endif
