#include <widget/yearpickerview.h>
#include <widget/textview.h>
#include <widget/layoutinflater.h>

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
    YearAdapter(Context* context) {
        mInflater = LayoutInflater::from(context);
    }

    void setRange(int minDate,int maxDate/*Calendar minDate, Calendar maxDate*/) {
        int minYear = minDate;//minDate.get(Calendar.YEAR);
        int count = maxDate;//maxDate.get(Calendar.YEAR) - minYear + 1;

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

        /*if ((convertView == nullptr) || v->isActivated() != activated) {
            int textAppearanceResId;
            if (activated && ITEM_TEXT_ACTIVATED_APPEARANCE != 0) {
                textAppearanceResId = ITEM_TEXT_ACTIVATED_APPEARANCE;
            } else {
                textAppearanceResId = ITEM_TEXT_APPEARANCE;
            }
            v->setTextAppearance(textAppearanceResId);
            v->setActivated(activated);
        }*/
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
}

void YearPickerView::setOnYearSelectedListener(OnYearSelectedListener listener) {
    mOnYearSelectedListener = listener;
}

void YearPickerView::setYear(int year) {
    mAdapter->setSelection(year);
    int position=mAdapter->getPositionForYear(year);
    if (position >= 0 && position < getCount())
        setSelectionCentered(position);
    /*post(new Runnable() {
        void run() {
            int position = mAdapter.getPositionForYear(year);
            if (position >= 0 && position < getCount()) {
                setSelectionCentered(position);
            }
        }
    });*/
}

void YearPickerView::setSelectionCentered(int position) {
    int offset = mViewSize / 2 - mChildSize / 2;
    setSelectionFromTop(position, offset);
}

void YearPickerView::setRange(int min,int max/*Calendar min, Calendar max*/) {
    mAdapter->setRange(min, max);
}

}
