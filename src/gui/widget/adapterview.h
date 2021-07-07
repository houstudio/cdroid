/*************************************************************************
	> File Name: widget/adapterview.h
	> Author: 
	> Mail: 
	> Created Time: Tue 16 Mar 2021 02:24:12 PM UTC
 ************************************************************************/

#ifndef __ADAPTER_VIEW_H__
#define __ADAPTER_VIEW_H__
#include <widget/viewgroup.h>
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
private:
    int mDesiredFocusableState;
    Runnable mSelectionNotifier;
    Runnable mPendingSelectionNotifier;
    bool mDesiredFocusableInTouchModeState;
    void updateEmptyStatus(bool empty);
    void dispatchOnItemSelected();
    void fireOnSelected();
protected:
    bool mDataChanged;
    bool mInLayout;
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
    void onDatasetChange(int );
    void onLayout(bool changed, int left, int top, int width, int height)override; 
    virtual void handleDataChanged();
    void setNextSelectedPositionInt(int position);
    void setSelectedPositionInt(int position);
    void checkSelectionChanged();
    void selectionChanged();
    void checkFocus();
    void rememberSyncState();
    int findSyncPosition();
    virtual int lookForSelectablePosition(int position, bool lookDown);
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
    virtual int getCount();
    bool isInFilterMode();
    void setFocusable(int focusable)override;
    void setFocusableInTouchMode(bool focusable)override;
    virtual View* getSelectedView()=0;
    virtual void setSelection(int position)=0;
    virtual bool performItemClick(View* view, int position, long id);
    
    void setOnItemClickListener(OnItemClickListener listener);
    OnItemClickListener getOnItemClickListener() const;
    void setOnItemSelectedListener(OnItemSelectedListener listener);
    OnItemSelectedListener getOnItemSelectedListener()const;
    void setOnItemLongClickListener(OnItemLongClickListener listener);
    OnItemLongClickListener getOnItemLongClickListener() const;
};

class AdapterDataSetObserver:public DataSetObserver{
protected:
    AdapterView*adv;
public:
    AdapterDataSetObserver(AdapterView*adv);
    void onChanged()override;
    void onInvalidated()override;
    void clearSavedState()override;
};

}//namespace

#endif
