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
#ifndef __ABS_SPINNER_H__
#define __ABS_SPINNER_H__
#include <widget/adapterview.h>
#include <core/sparsearray.h>
namespace cdroid{
class AbsSpinner:public AdapterView{
protected:
    class RecycleBin{
    private:
        SparseArray<View*> mScrapHeap;
        AbsSpinner*ABS;
    public:
        RecycleBin(AbsSpinner*);
        void put(int position, View* v);
        View* get(int position);
        void clear();
    };
    RecycleBin*mRecycler;
    DataSetObserver* mDataSetObserver;
    int mHeightMeasureSpec;
    int mWidthMeasureSpec;
    int mSelectionLeftPadding;
    int mSelectionTopPadding;
    int mSelectionRightPadding;
    int mSelectionBottomPadding;
    Rect mSpinnerPadding;
    Rect mTouchFrame;
    void initAbsSpinner();
    void resetList();
    void recycleAllViews();
    int getChildWidth(View* child);
    virtual int getChildHeight(View* child);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    virtual void layout(int delta, bool animate)=0;
public:
    AbsSpinner(int w,int h);
    AbsSpinner(Context*,const AttributeSet&atts);
    virtual ~AbsSpinner();
    int getCount()override;
    View* getSelectedView()override;
    void setAdapter(Adapter*adapter)override;
    void setSelection(int position, bool animate);
    void setSelection(int position)override;
    virtual void setSelectionInt(int position, bool animate);
    int pointToPosition(int x, int y);
    void requestLayout()override;
};
}/*endof namespace*/
#endif
