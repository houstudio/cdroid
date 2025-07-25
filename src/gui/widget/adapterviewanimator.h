/*********************************************************************************
+ * Copyright (C) [2019] [houzh@msn.com]
+ *
+ * This library is free software; you can redistribute it and/or
+ * modify it under the terms of the GNU Lesser General Public
+ * License as published by the Free Software Foundation; either
+ * version 2.1 of the License, or (at your option) any later version.
+ *
+ * This library is distributed in the hope that it will be useful,
+ * but WITHOUT ANY WARRANTY; without even the implied warranty of
+ * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
+ * Lesser General Public License for more details.
+ *
+ * You should have received a copy of the GNU Lesser General Public
+ * License along with this library; if not, write to the Free Software
+ * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
+ *********************************************************************************/
#ifndef __ADAPTERVIEW_ANIMATOR_H__
#define __ADAPTERVIEW_ANIMATOR_H__
#include <widget/adapterview.h>
#include <widget/framelayout.h>

namespace cdroid{

class AdapterViewAnimator:public AdapterView{
private:
    static constexpr int DEFAULT_ANIMATION_DURATION = 200;
    /* Private touch states.*/
    static constexpr int TOUCH_MODE_NONE = 0;
    static constexpr int TOUCH_MODE_DOWN_IN_CURRENT_VIEW = 1;
    static constexpr int TOUCH_MODE_HANDLED = 2;
    int mRestoreWhichChild = -1;
    /* Current touch state.*/
    Runnable mPendingCheckForTap;
    int mTouchMode = TOUCH_MODE_NONE;

    class ViewAndMetaData {
    public:
        View* view;
        int relativeIndex;
        int adapterPosition;
        long itemId;
    public:
        ViewAndMetaData(View* view, int relativeIndex, int adapterPosition, long itemId);
    };
protected:
    int mWhichChild = 0;
    int mActiveOffset = 0;
    int mMaxNumActiveViews = 1;
    int mCurrentWindowStart = 0;
    int mCurrentWindowEnd = -1;
    int mCurrentWindowStartUnbounded = 0;
    int mReferenceChildWidth = -1;
    int mReferenceChildHeight = -1;

    std::unordered_map<int, ViewAndMetaData*> mViewsMap;
    std::vector<int> mPreviousViews;
    DataSetObserver* mDataSetObserver;
    Adapter* mAdapter;

    bool mAnimateFirstTime = true;
    bool mDeferNotifyDataSetChanged = false;
    bool mFirstTime = true;
    bool mLoopViews = true;

    ObjectAnimator* mInAnimation;
    ObjectAnimator* mOutAnimation;
private:
    void initViewAnimator();
    void setDisplayedChild(int whichChild, bool animate);
    ViewAndMetaData* getMetaDataForChild(View* child);
    void addChild(View* child);
    void measureChildren();
protected:
    void configureViewAnimator(int numVisibleViews, int activeOffset);
    void transformViewForTransition(int fromIndex, int toIndex, View* view, bool animate);
    ObjectAnimator* getDefaultInAnimation();
    ObjectAnimator* getDefaultOutAnimation();
    void applyTransformForChildAtIndex(View* child, int relativeIndex);
    int modulo(int pos, int size);
    View* getViewAtRelativeIndex(int relativeIndex);
    int getNumActiveViews();
    int getWindowSize();
    LayoutParams* createOrReuseLayoutParams(View* v);
    void refreshChildren();
    virtual FrameLayout* getFrameForChild();
    virtual void showOnly(int childIndex, bool animate);
    virtual void showTapFeedback(View* v);
    virtual void hideTapFeedback(View* v);
    void cancelHandleClick();
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void checkForAndHandleDataChanged();
    void onLayout(bool changed, int left, int top, int width, int height)override;
public:
    AdapterViewAnimator(Context* context,const AttributeSet& attrs);
    ~AdapterViewAnimator()override;
    void setDisplayedChild(int whichChild);
    int getDisplayedChild();
    virtual void showNext();
    virtual void showPrevious();
    bool onTouchEvent(MotionEvent& ev)override;
    View* getCurrentView();
    ObjectAnimator* getInAnimation();
    void setInAnimation(ObjectAnimator* inAnimation);
    ObjectAnimator* getOutAnimation();
    void setOutAnimation(ObjectAnimator* outAnimation);
    void setInAnimation(Context* context,const std::string& resourceID);
    void setOutAnimation(Context* context,const std::string& resourceID);
    void setAnimateFirstView(bool animate);
    int getBaseline()override;
    Adapter* getAdapter()override;
    void setAdapter(Adapter* adapter)override;
    void setSelection(int position)override;
    View* getSelectedView()override;
    void deferNotifyDataSetChanged();
    virtual void advance();
};

}//namepace
#endif
