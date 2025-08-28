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
#include <widget/spinner.h>
#include <widget/listview.h>
#include <widget/dropdownlistview.h>
#include <widget/forwardinglistener.h>
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>
#define MAX_ITEMS_MEASURED  15
namespace cdroid{

/////////////////////////////////////////////////////////////////////////////////////////

DECLARE_WIDGET2(Spinner,"cdroid:attr/spinnerStyle")

Spinner::SpinnerForwardingListener::SpinnerForwardingListener(View*v,Spinner::DropdownPopup*d)
:ForwardingListener(v){
    mDropDown = d;
}

ShowableListMenu Spinner::SpinnerForwardingListener::getPopup(){
    ShowableListMenu sm;
    sm.show =[this](){
        mDropDown->show(((Spinner*)mSrc)->getTextDirection(),
        ((Spinner*)mSrc)->getTextAlignment());
    };
    sm.dismiss=[this](){
        mDropDown->dismiss();
    };
    sm.isShowing=[this](){
        return mDropDown->isShowing();
    };
    sm.getListView=[this]()->ListView*{
        return mDropDown->getListView();
    };
    return sm;
}

bool Spinner::SpinnerForwardingListener::onForwardingStarted(){
    if(!mDropDown->isShowing()){
        Spinner*spin=(Spinner*)mSrc;
        mDropDown->show(spin->getTextDirection(),spin->getTextAlignment());
    }
    return true;
}

Spinner::Spinner(int w,int h,int mode):AbsSpinner(w,h){
    mGravity = Gravity::CENTER;
    mDropDownWidth =0;
    mDisableChildrenWhenDisabled = true;
    mTempAdapter= nullptr;
    mPopup = new DropdownPopup(mContext,this,"cdroid:attr/spinnerStyle");
    mForwardingListener = new SpinnerForwardingListener(this,(DropdownPopup*)mPopup); 
}

Spinner::Spinner(Context*ctx,const AttributeSet&atts)
  :AbsSpinner(ctx,atts){
    mTempAdapter = nullptr;
    mForwardingListener = nullptr;
    mGravity = atts.getGravity("gravity",Gravity::CENTER);
    mDisableChildrenWhenDisabled = atts.getBoolean("disableChildrenWhenDisabled",false);
    const int mode = atts.getInt("spinnerMode",std::unordered_map<std::string,int>{
        {"dialog",(int)MODE_DIALOG},{"dropdown",(int)MODE_DROPDOWN}
    },MODE_DIALOG);

    Drawable*dr;
    DropdownPopup* popup;
    switch(mode){
    case MODE_DIALOG:
         mPopup = new DialogPopup(this);
         mPopup->setPromptText(atts.getString("propmt"));
         break;
    case MODE_DROPDOWN:
         popup = new DropdownPopup(ctx,this,"cdroid:attr/spinnerStyle");
         mDropDownWidth = atts.getLayoutDimension("dropDownWidth",LayoutParams::WRAP_CONTENT);
         dr = atts.getDrawable("dropDownSelector");
         if(dr)popup->setListSelector(dr);
         dr = mContext->getDrawable(atts.getString("popupBackground"));
         if(dr)popup->setBackgroundDrawable(dr);
         popup->setPromptText(atts.getString("propmt"));
         mPopup = popup;
         mForwardingListener = new SpinnerForwardingListener(this,popup); 
         break;
    } 
    // Base constructor can call setAdapter before we initialize mPopup.
    // Finish setting things up if this happened.
    if (mTempAdapter != nullptr) {
        setAdapter(mTempAdapter);
        mTempAdapter = nullptr;
    }
}

Spinner::~Spinner(){
    delete mPopup;
    delete mForwardingListener;
}

Context* Spinner::getPopupContext()const{
    return mContext;
}

void Spinner::setPopupBackgroundDrawable(Drawable* background){
    mPopup->setBackgroundDrawable(background);
}

void Spinner::setPopupBackgroundResource(const std::string& resId){
    setPopupBackgroundDrawable(getPopupContext()->getDrawable(resId));
}

Drawable* Spinner::getPopupBackground(){
    return mPopup->getBackground();
}

void Spinner::setDropDownVerticalOffset(int pixels) {
    mPopup->setVerticalOffset(pixels);
}

int Spinner::getDropDownVerticalOffset() {
    return mPopup->getVerticalOffset();
}

void Spinner::setDropDownHorizontalOffset(int pixels) {
    mPopup->setHorizontalOffset(pixels);
}

int Spinner::getDropDownHorizontalOffset() {
    return mPopup->getHorizontalOffset();
}

void Spinner::setDropDownWidth(int pixels) {
    if (dynamic_cast<DropdownPopup*>(mPopup)) {
        LOGE("Cannot set dropdown width for MODE_DIALOG, ignoring");
        return;
    }
    mDropDownWidth = pixels;
}

int Spinner::getDropDownWidth()const{
    return mDropDownWidth;
}

void Spinner::setEnabled(bool enabled) {
    AbsSpinner::setEnabled(enabled);
    if (mDisableChildrenWhenDisabled) {
        int count = getChildCount();
        for (int i = 0; i < count; i++) {
            getChildAt(i)->setEnabled(enabled);
        }
    }
}

void Spinner::setGravity(int gravity) {
    if (mGravity != gravity) {
        if ((gravity & Gravity::HORIZONTAL_GRAVITY_MASK) == 0) {
            gravity |= Gravity::START;
        }
        mGravity = gravity;
        requestLayout();
    }
}

int Spinner::getGravity()const{
    return mGravity;
}

void Spinner::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    AbsSpinner::onMeasure(widthMeasureSpec, heightMeasureSpec);
    if (mPopup && MeasureSpec::getMode(widthMeasureSpec) == MeasureSpec::AT_MOST) {
        int measuredWidth = getMeasuredWidth();
        setMeasuredDimension(std::min(std::max(measuredWidth,
            measureContentWidth(getAdapter(), getBackground())),
                    MeasureSpec::getSize(widthMeasureSpec)),
            getMeasuredHeight());
    }
}

void Spinner::onLayout(bool changed, int x, int y, int w, int h) {
    AbsSpinner::onLayout(changed, x, y, w, h);
    mInLayout = true;
    layout(0, false);
    mInLayout = false;
    LOGV("(%d,%d %d,%d)",x,y,w,h);
}

void Spinner::layout(int delta, bool animate){
    const int childrenLeft = mSpinnerPadding.left;
    const int childrenWidth = getWidth() - mSpinnerPadding.left - mSpinnerPadding.width;

    if (mDataChanged) {
        handleDataChanged();
    }
    
    // Handle the empty set by removing all views
    if (mItemCount == 0) {
        resetList();
        return;
    }

    if (mNextSelectedPosition >= 0) {
        setSelectedPositionInt(mNextSelectedPosition);
    }

    recycleAllViews();

    // Clear out old views
    removeAllViewsInLayout();

    // Make selected view and position it
    mFirstPosition = mSelectedPosition;

    if (mAdapter != nullptr) {
        View* sel = makeView(mSelectedPosition, true);
        const int width = sel->getMeasuredWidth();
        int selectedOffset = childrenLeft;
        const int layoutDirection = getLayoutDirection();
        const int absoluteGravity = Gravity::getAbsoluteGravity(mGravity, layoutDirection);
        switch (absoluteGravity & Gravity::HORIZONTAL_GRAVITY_MASK) {
        case Gravity::CENTER_HORIZONTAL:
            selectedOffset = childrenLeft + (childrenWidth / 2) - (width / 2);
            break;
        case Gravity::RIGHT:
            selectedOffset = childrenLeft + childrenWidth - width;
            break;
        }
        sel->offsetLeftAndRight(selectedOffset);
    }

    // Flush any cached views that did not get reused above
    mRecycler->clear();

    invalidate();

    checkSelectionChanged();

    mDataChanged = false;
    mNeedSync = false;
    setNextSelectedPositionInt(mSelectedPosition);
}

namespace{
    class DropDownAdapter:public Adapter {
    private:
        SpinnerAdapter* mAdapter;
    public:
        DropDownAdapter(SpinnerAdapter* adapter,Context*ctx) {
            mAdapter = adapter;
        }

        int getCount() const override{
            return (mAdapter == nullptr) ? 0 : mAdapter->getCount();
        }

        void* getItem(int position) const override{
            return (mAdapter == nullptr) ? nullptr : mAdapter->getItem(position);
        }

        long getItemId(int position) const override{
            return (mAdapter == nullptr) ? -1 : mAdapter->getItemId(position);
        }

        View* getView(int position, View* convertView, ViewGroup* parent) override{
            return getDropDownView(position, convertView, parent);
        }

        View* getDropDownView(int position, View* convertView, ViewGroup* parent) override{
            return (mAdapter == nullptr) ? nullptr : mAdapter->getDropDownView(position, convertView, parent);
        }

        bool hasStableIds() const override{
            return (mAdapter != nullptr) && mAdapter->hasStableIds();
        }

        void registerDataSetObserver(DataSetObserver* observer) override{
            if (mAdapter != nullptr) {
                mAdapter->registerDataSetObserver(observer);
            }
        }

        void unregisterDataSetObserver(DataSetObserver* observer) override{
            if (mAdapter != nullptr) {
                mAdapter->unregisterDataSetObserver(observer);
            }
        }

        bool areAllItemsEnabled() const override{
            return (mAdapter==nullptr)||mAdapter->areAllItemsEnabled();
        }

        bool isEnabled(int position) const override {
            return (mAdapter==nullptr)||mAdapter->isEnabled(position);
        }

        int getItemViewType(int position) const override{
            return 0;
        }

        int getViewTypeCount() const override{
            return 1;
        }

        bool isEmpty() const override{
            return getCount() == 0;
        }
    };
}

void Spinner::setAdapter(Adapter*adapter){
    // The super constructor may call setAdapter before we're prepared.
    // Postpone doing anything until we've finished construction.
    if (mPopup == nullptr) {
        mTempAdapter = adapter;
        return;
    }
    AbsSpinner::setAdapter(adapter);
    mRecycler->clear();
    mPopup->setAdapter(new DropDownAdapter(adapter,mContext));
}

int Spinner::getBaseline(){
    View* child = nullptr;

    if (getChildCount() > 0) {
        child = getChildAt(0);
    } else if (mAdapter && mAdapter->getCount() > 0) {
        child = makeView(0, false);
        mRecycler->put(0, child);
    }

    if (child != nullptr) {
        const int childBaseline = child->getBaseline();
        return childBaseline >= 0 ? child->getTop() + childBaseline : -1;
    } else {
        return -1;
    }
}

View* Spinner::makeView(int position, bool addChild) {
    View* child;
    if (!mDataChanged) {
        child = mRecycler->get(position);
        if (child != nullptr) {
            // Position the view
            setUpChild(child, addChild);
            return child;
        }
    }

    // Nothing found in the recycler -- ask the adapter for a view
    child = mAdapter->getView(position, nullptr, this);
    // Position the view
    setUpChild(child, addChild);
    return child;
}

void Spinner::setUpChild(View* child, bool addChild) {
    // Respect layout params that are already in the view. Otherwise
    // make some up...
    ViewGroup::LayoutParams* lp = child->getLayoutParams();
    if (lp == nullptr) lp = generateDefaultLayoutParams();

    addViewInLayout(child, 0, lp);

    child->setSelected(hasFocus());
    if (mDisableChildrenWhenDisabled) {
        child->setEnabled(isEnabled());
    }

    // Get measure specs
    const int childHeightSpec = ViewGroup::getChildMeasureSpec(mHeightMeasureSpec,
            mSpinnerPadding.top + mSpinnerPadding.height, lp->height);
    const int childWidthSpec = ViewGroup::getChildMeasureSpec(mWidthMeasureSpec,
            mSpinnerPadding.left + mSpinnerPadding.width, lp->width);

    // Measure child
    child->measure(childWidthSpec, childHeightSpec);

    int childLeft;
    // Position vertically based on gravity setting
    const int childTop = mSpinnerPadding.top  + ((getMeasuredHeight() - mSpinnerPadding.height -
                    mSpinnerPadding.top - child->getMeasuredHeight()) / 2);
    const int height= child->getMeasuredHeight();
    const int width = child->getMeasuredWidth();
    childLeft = 0;

    child->layout(childLeft, childTop,width,height );

    if (!addChild)removeViewInLayout(child);
}

bool Spinner::performClick() {
    bool handled = AbsSpinner::performClick();

    if (!handled) {
        handled = true;

        if (!mPopup->isShowing()) {
            mPopup->show(getTextDirection(), getTextAlignment());
        }
    }
    return handled;
}

void Spinner::onClick(DialogInterface& dialog, int which) {
    setSelection(which);
    dialog.dismiss();
}

void Spinner::onClick(int which) {
    setSelection(which);
    if ((mPopup != nullptr) && mPopup->isShowing()) {
        mPopup->dismiss();
    }
}

int Spinner::measureContentWidth(Adapter* adapter, Drawable* background){
    if (adapter == nullptr) return 0;

    int width = 0;
    View* itemView = nullptr;
    int itemType = 0;
    const int widthMeasureSpec =MeasureSpec::makeSafeMeasureSpec(getMeasuredWidth(), MeasureSpec::UNSPECIFIED);
    const int heightMeasureSpec = MeasureSpec::makeSafeMeasureSpec(getMeasuredHeight(), MeasureSpec::UNSPECIFIED);

    // Make sure the number of items we'll measure is capped. If it's a huge data set
    // with wildly varying sizes, oh well.
    int start = std::max(0, getSelectedItemPosition());
    const int end = std::min(adapter->getCount(), start + MAX_ITEMS_MEASURED);
    const int count = end - start;
    start = std::max(0, start - (MAX_ITEMS_MEASURED - count));
    for (int i = start; i < end; i++) {
        int positionType = adapter->getItemViewType(i);
        if (positionType != itemType) {
            itemType = positionType;
            itemView = nullptr;
        }
        itemView = adapter->getView(i, itemView, this);
        if (itemView->getLayoutParams() == nullptr) {
            itemView->setLayoutParams(new ViewGroup::LayoutParams(
                LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT));
        }
        itemView->measure(widthMeasureSpec, heightMeasureSpec);
        width = std::max(width, itemView->getMeasuredWidth());
    }

    // Add background padding to measured width
    if (background) {
        Rect rctmp;
        background->getPadding(rctmp);
        width += rctmp.left + rctmp.width;
    }
    return width;
}

bool Spinner::onTouchEvent(MotionEvent& event){
    if (mForwardingListener && mForwardingListener->onTouch(*this, event)) {
        return true;
    }
    return AbsSpinner::onTouchEvent(event);
}

PointerIcon* Spinner::onResolvePointerIcon(MotionEvent& event, int pointerIndex){
    if ((getPointerIcon() == nullptr) && isClickable() && isEnabled()) {
         return PointerIcon::getSystemIcon(getContext(), PointerIcon::TYPE_HAND);
    }
    return AbsSpinner::onResolvePointerIcon(event, pointerIndex);
}

/////////////////////////////////SpinnerPopup//////////////////////////////////////////
Spinner::DropdownPopup::DropdownPopup(Context*context,Spinner*sp,const std::string&defStyleAttr)
  :ListPopupWindow(context,AttributeSet(context,""),defStyleAttr){
    mSpinner = sp;
    mAdapter = nullptr;
    setAnchorView(mSpinner);
    setModal(true);
    setPromptPosition(POSITION_PROMPT_ABOVE);

    setOnItemClickListener([this](AdapterView& parent, View& v, int position, long id) {
         mSpinner->setSelection(position);
         if (mSpinner->mOnItemClickListener != nullptr) {
             mSpinner->performItemClick(v, position, mAdapter->getItemId(position));
         }
         dismiss();
    });

    mLayoutListener =[this](){
        if (!mSpinner->isVisibleToUser()) {
            dismiss();
        } else {
            computeContentWidth();
            // Use super.show here to update; we don't want to move the selected
            // position or adjust other things that would be reset otherwise.
            ListPopupWindow::show();
        }
    };
}

Spinner::DropdownPopup::~DropdownPopup(){
    delete mAdapter;
}

void Spinner::DropdownPopup::setAdapter(Adapter* adapter){
    mAdapter = adapter;
    ListPopupWindow::setAdapter(adapter);
}

void Spinner::DropdownPopup::dismiss(){
    mSpinner->mRecycler->clear();
    ListPopupWindow::dismiss();
}

bool Spinner::DropdownPopup::isShowing(){
    return ListPopupWindow::isShowing();
}

void Spinner::DropdownPopup::setPromptText(const std::string& hintText){
    // Hint text is ignored for dropdowns, but maintain it here.
    mHintText = hintText;
}

const std::string Spinner::DropdownPopup::getHintText(){
    return mHintText;
}

void Spinner::DropdownPopup::computeContentWidth() {
    Drawable* background = getBackground();
    int hOffset = 0;
    Rect mTempRect;
    if (background != nullptr) {
        background->getPadding(mTempRect);
        hOffset = mSpinner->isLayoutRtl() ? mTempRect.width : -mTempRect.left;
    } else {
        mTempRect.left = mTempRect.width = 0;
    }

    const int spinnerPaddingLeft = mSpinner->getPaddingLeft();
    const int spinnerPaddingRight= mSpinner->getPaddingRight();
    const int spinnerWidth = mSpinner->getWidth();

    if (mSpinner->mDropDownWidth  == LayoutParams::WRAP_CONTENT) {
        int contentWidth =  mSpinner->measureContentWidth(mSpinner->getAdapter(), mSpinner->getBackground());
        int contentWidthLimit = mSpinner->getContext()->getDisplayMetrics().widthPixels - mTempRect.width;
        if (contentWidth > contentWidthLimit) {
            contentWidth = contentWidthLimit;
        }
        setContentWidth(std::max( contentWidth, spinnerWidth - spinnerPaddingLeft - spinnerPaddingRight));
    } else if (mSpinner->mDropDownWidth == LayoutParams::MATCH_PARENT) {
        setContentWidth(spinnerWidth - spinnerPaddingLeft - spinnerPaddingRight);
    } else {
        setContentWidth(mSpinner->mDropDownWidth);
    }

    if (mSpinner->isLayoutRtl()) {
        hOffset += spinnerWidth - spinnerPaddingRight - mSpinner->getWidth();
    } else {
        hOffset += spinnerPaddingLeft;
    }
    setHorizontalOffset(hOffset);
}

int Spinner::DropdownPopup::getVerticalOffset(){
    int p=ListPopupWindow::getVerticalOffset();
    LOGD("VerticalOffset=%d",p);
    return p;
}

void Spinner::DropdownPopup::setVerticalOffset(int px){
    ListPopupWindow::setVerticalOffset(px);
}

int Spinner::DropdownPopup::getHorizontalOffset(){
    return ListPopupWindow::getHorizontalOffset();
}

void Spinner::DropdownPopup::setHorizontalOffset(int px){
    ListPopupWindow::setHorizontalOffset(px);
}

void Spinner::DropdownPopup::setBackgroundDrawable(Drawable* bg){
    ListPopupWindow::setBackgroundDrawable(bg);
}

Drawable* Spinner::DropdownPopup::getBackground(){
    return ListPopupWindow::getBackground();
}

void Spinner::DropdownPopup::setContentWidth(int width){
    Drawable* popupBackground = getBackground();
    if (popupBackground ) {
        Rect rect;
        popupBackground->getPadding(rect);
        mSpinner->mDropDownWidth = rect.left + rect.width + width;
    } else {
        mSpinner->mDropDownWidth=width;
    }
}

ListView*Spinner::DropdownPopup::getListView(){
    return ListPopupWindow::getListView();
}

void Spinner::DropdownPopup::show(int textDirection, int textAlignment) {
    const bool wasShowing = isShowing();

    computeContentWidth();

    setInputMethodMode(PopupWindow::INPUT_METHOD_NOT_NEEDED);
    ListPopupWindow::show();
    ListView* listView = getListView();

    LOGD("====mDropDownWidth=%d listView=%p",mSpinner->mDropDownWidth,listView);
    if(listView==nullptr)return;

    listView->setChoiceMode(ListView::CHOICE_MODE_SINGLE);
    listView->setTextDirection(textDirection);
    listView->setTextAlignment(textAlignment);
    mSpinner->setSelection(mSpinner->getSelectedItemPosition());

    if (wasShowing) {
        // Skip setting up the layout/dismiss listener below. If we were previously
        // showing it will still stick around.
        return;
    }

    // Make sure we hide if our anchor goes away.
    // TODO: This might be appropriate to push all the way down to PopupWindow,
    // but it may have other side effects to investigate first. (Text editing handles, etc.)
    ViewTreeObserver* vto = mSpinner->getViewTreeObserver();
    if (vto != nullptr) {
        vto->addOnGlobalLayoutListener(mLayoutListener);
        //OnDismissListener dl;
        setOnDismissListener([this] {
            ViewTreeObserver* vto = mSpinner->getViewTreeObserver();
            if (vto != nullptr) {
                vto->removeOnGlobalLayoutListener(mLayoutListener);
            }
        });
    }
}
//int SpinnerPopup::measureContentWidth(Adapter*,Drawable*){}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
Spinner::DialogPopup::DialogPopup(Spinner*spinner){
    mSpinner = spinner;
}

Spinner::DialogPopup::~DialogPopup(){
    delete mListAdapter;
}

void Spinner::DialogPopup::setAdapter(Adapter*adapter){
    mListAdapter = adapter;
}

void Spinner::DialogPopup::show(int textDirection, int textAlignment){
    if (mListAdapter == nullptr) {
        return;
    }
    DialogInterface::OnClickListener onDialogClick = std::bind(&Spinner::DialogPopup::onClick,this,std::placeholders::_1,std::placeholders::_2);
    mPopup =AlertDialog::Builder(mSpinner->getPopupContext())
        .setTitle(mPrompt)
        .setSingleChoiceItems(mListAdapter,mSpinner->getSelectedItemPosition(),onDialogClick).show();

    ListView* listView = mPopup->getListView();
    listView->setTextDirection(textDirection);
    listView->setTextAlignment(textAlignment);
    //mPopup->show();
}

void Spinner::DialogPopup::onClick(DialogInterface& dialog, int which) {
    mSpinner->setSelection(which);
    if (mSpinner->mOnItemClickListener != nullptr) {
        mSpinner->performItemClick(*mSpinner/*nullptr*/, which, mListAdapter->getItemId(which));
    }
    dismiss();
}

void Spinner::DialogPopup::dismiss(){
    mSpinner->mRecycler->clear();
    mPopup->dismiss();
    mPopup = nullptr;
}

bool Spinner::DialogPopup::isShowing(){
    return mPopup && mPopup->isShowing();
}

void Spinner::DialogPopup::setPromptText(const std::string& hintText){
    mPrompt = hintText;
}

const std::string Spinner::DialogPopup::getHintText(){
    return mPrompt;
}

int Spinner::DialogPopup::getVerticalOffset(){
    return 0;
}

void Spinner::DialogPopup::setVerticalOffset(int px){
    LOGE("Cannot set vertical offset for MODE_DIALOG, ignoring");
}

int Spinner::DialogPopup::getHorizontalOffset(){
    return 0;
}

void Spinner::DialogPopup::setHorizontalOffset(int px){
    LOGE("Cannot set horizontal offset for MODE_DIALOG, ignoring");
}

void Spinner::DialogPopup::setBackgroundDrawable(Drawable* bg){
    LOGE("Cannot set popup background for MODE_DIALOG, ignoring");
}

Drawable* Spinner::DialogPopup::getBackground(){
    return nullptr;
}

void Spinner::DialogPopup::computeContentWidth(){
}

void Spinner::DialogPopup::setContentWidth(int width){
}

}//endof namespace

