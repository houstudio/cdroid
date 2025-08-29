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
#include <widget/yearpickerview.h>
#include <widget/textview.h>

namespace cdroid{

DECLARE_WIDGET(YearPickerView)

class YearAdapter:public ArrayAdapter<int> {
private:
    /*static int ITEM_LAYOUT = R.layout.year_label_text_view;
    static int ITEM_TEXT_APPEARANCE = R.style.TextAppearance_Material_DatePicker_List_YearLabel;
    static int ITEM_TEXT_ACTIVATED_APPEARANCE =  R.style.TextAppearance_Material_DatePicker_List_YearLabel_Activated;*/
    LayoutInflater* mInflater;
    int mActivatedYear;
    int mMinYear;
    int mCount;

public:
    YearAdapter(Context* context)
          :mActivatedYear(0)
          ,mMinYear(0)
          ,mCount(0){
        mInflater = LayoutInflater::from(context);
    }

    void setRange(Calendar& minDate, Calendar& maxDate) {
        const int minYear = minDate.get(Calendar::YEAR);
        const int count = maxDate.get(Calendar::YEAR) - minYear + 1;

        if (mMinYear != minYear || mCount != count) {
            mMinYear = minYear;
            mCount = count;
            notifyDataSetInvalidated();
        }
    }

    bool setSelection(int year) {
        if (mActivatedYear != year) {
            mActivatedYear = year;
            notifyDataSetChanged();
            return true;
        }
        return false;
    }

    int getCount()const {
        return mCount;
    }

    long getItemId(int position) {
        return getYearForPosition(position);
    }

    int getPositionForYear(int year) {
        return year - mMinYear;
    }

    int getYearForPosition(int position) {
        return mMinYear + position;
    }

    bool hasStableIds()const override {
        return true;
    }

    View* getView(int position, View* convertView, ViewGroup* parent) {
        TextView* v;
        if (convertView==nullptr) {
            v = (TextView*) mInflater->inflate("cdroid:layout/year_label_text_view.xml", parent, false);
        } else {
            v = (TextView*) convertView;
        }

        int year = getYearForPosition(position);
        bool activated = mActivatedYear == year;

        if ((convertView == nullptr) || v->isActivated() != activated) {
            std::string textAppearanceResId="YearLabel_Activated";
            if (activated /*&& ITEM_TEXT_ACTIVATED_APPEARANCE != 0*/) {
                //textAppearanceResId = ITEM_TEXT_ACTIVATED_APPEARANCE;
            } else {
                //textAppearanceResId = ITEM_TEXT_APPEARANCE;
            }
            v->setTextAppearance(textAppearanceResId);
            v->setActivated(activated);
        }
        v->setText(std::to_string(year));
        return v;
    }

    int getItemViewType(int position) {
        return 0;
    }

    int getViewTypeCount() {
        return 1;
    }

    bool isEmpty() {
        return false;
    }

    bool areAllItemsEnabled() {
        return true;
    }

    bool isEnabled(int position) {
        return true;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////
YearPickerView::YearPickerView(Context*ctx,const AttributeSet&attrs):ListView(ctx,attrs){
    mViewSize = attrs.getDimensionPixelOffset("animator_height");
    mChildSize= attrs.getDimensionPixelOffset("year_label_height");
    setOnItemClickListener([this](AdapterView& parent,View& view, int position, long id){
        const int year = mAdapter->getYearForPosition(position);
        mAdapter->setSelection(year);
        if(mOnYearSelectedListener)
            mOnYearSelectedListener(*this,year);
    });
}

void YearPickerView::setOnYearSelectedListener(const OnYearSelectedListener& listener) {
    mOnYearSelectedListener = listener;
}

void YearPickerView::setYear(int year) {
    mAdapter->setSelection(year);
    post([this,year](){
        const int position = mAdapter->getPositionForYear(year);
        if ( (position >= 0) && (position < getCount()) ) {
            setSelectionCentered(position);
        }
    });
}

void YearPickerView::setSelectionCentered(int position) {
    int offset = mViewSize / 2 - mChildSize / 2;
    setSelectionFromTop(position, offset);
}

void YearPickerView::setRange(Calendar& min, Calendar& max) {
    mAdapter->setRange(min, max);
}

int YearPickerView::getFirstPositionOffset() {
    View* firstChild = getChildAt(0);
    if (firstChild == nullptr) {
        return 0;
    }
    return firstChild->getTop();
}
}
