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
#include <widget/internal_R.h>
#include <core/context.h>
#include <widget/spinner.h>
#include <widget/framework_styleable.h>
#include <content/contextthemewrapper.h>
#include <widget/listview.h>
#include <widget/dropdownlistview.h>
#include <widget/forwardinglistener.h>
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>
#define MAX_ITEMS_MEASURED  15
namespace cdroid{
using namespace cdroid::internal;

/////////////////////////////////////////////////////////////////////////////////////////

DECLARE_WIDGET2(Spinner, "android.widget.Spinner");

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

Spinner::Spinner(Context*ctx)
    :Spinner(ctx,nullptr){}

Spinner::Spinner(Context*ctx,const AttributeSet* atts):Spinner(ctx,atts,R::attr::spinnerStyle){}

Spinner::Spinner(Context*ctx,const AttributeSet* pAttrs,int defStyleAttr)
  :AbsSpinner(ctx,pAttrs, defStyleAttr){
    mTempAdapter = nullptr;
    mForwardingListener = nullptr;
    // Phase 2: TypedArray (binary AXML typed resolution). ta=null → text XML fallback.
    auto ta = ctx->obtainStyledAttributes(pAttrs, R::styleable::Spinner, defStyleAttr);


mGravity = ta->getInt(R::styleable::Spinner_gravity,Gravity::CENTER);
mDisableChildrenWhenDisabled = ta->getBoolean(R::styleable::Spinner_disableChildrenWhenDisabled, false);
const int mode = ta->getInt(R::styleable::Spinner_spinnerMode,MODE_DIALOG);

// AOSP Spinner: android:popupTheme wraps the popup context so the dropdown
// inflates with a different theme. The wrapper is owned by this Spinner.
const int popupThemeResId = ta->getResourceId(R::styleable::Spinner_popupTheme, 0);
if (popupThemeResId != 0) {
    mPopupContext = new ContextThemeWrapper(ctx, popupThemeResId);
    mOwnsPopupContext = true;
} else {
    mPopupContext = ctx;
}

Drawable*dr;
DropdownPopup* popup;
switch(mode){
case MODE_DIALOG:
     mPopup = new DialogPopup(this);
     mPopup->setPromptText(ta->getString(R::styleable::Spinner_prompt));
     break;
case MODE_DROPDOWN:
     popup = new DropdownPopup(getPopupContext(),this,defStyleAttr);
     mDropDownWidth = ta->getLayoutDimension(R::styleable::Spinner_dropDownWidth,LayoutParams::WRAP_CONTENT);
     dr = ta->getDrawable(R::styleable::Spinner_dropDownSelector);
     if(dr)popup->setListSelector(dr);
     dr = ta->getDrawable(R::styleable::Spinner_popupBackground);
     if(dr)popup->setBackgroundDrawable(dr);
     popup->setPromptText(ta->getString(R::styleable::Spinner_prompt));
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
    if (mOwnsPopupContext) delete mPopupContext;
}

Context* Spinner::getPopupContext()const{
    return mPopupContext;
}

void Spinner::setPopupBackgroundDrawable(Drawable* background){
    mPopup->setBackgroundDrawable(background);
}

void Spinner::setPopupBackgroundResource(int resId){
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

// AOSP Spinner.onInitializeAccessibilityNodeInfoInternal: a Spinner with an
// adapter opens a popup (dropdown or dialog) — announce that capability.
void Spinner::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    AbsSpinner::onInitializeAccessibilityNodeInfoInternal(info);
    if (mAdapter != nullptr) {
        info.setCanOpenPopup(true);
    }
}

View* Spinner::makeView(int position, bool addChild) {
    View* child;
    if (!mDataChanged) {
        child = mRecycler->get(position);
        if (child != nullptr) {
            setUpChild(child, addChild);
            return child;
        }
    }

    // Nothing found in the recycler -- ask the adapter for a view
    child = mAdapter->getView(position, nullptr, this);
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
            delete itemView;   // AOSP drops it for GC; we own the measure tree
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
    // AOSP drops the measure tree here (GC reclaims it). The bin is NOT the
    // place for it though: put(end-1) collides with AbsSpinner::onMeasure's
    // put(selectedPosition) on the same key and used to silently overwrite a
    // live tree — free the measure tree directly instead.
    delete itemView;

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
Spinner::DropdownPopup::DropdownPopup(Context*context,Spinner*sp,int defStyleAttr)
  :ListPopupWindow(context,nullptr,defStyleAttr){
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
    // The ListPopupWindow base (popup window + dropdown ListView) destructs
    // AFTER this body, and the ListView dereferences mAdapter until its tree
    // dies — deleting the wrap here was the Spinner::~Spinner 0x0 virtual
    // call during an active show. With the tree up, hand both wraps to the
    // spinner's looper so they die once this object (and the base's window
    // teardown) has completed; otherwise free them now. Teardown-time posts
    // can be dropped by a dying looper — same accepted edge as
    // DialogPopup::dismiss's owner deletion.
    Adapter* current = mAdapter;
    Adapter* pending = mPendingAdapterDelete;
    mAdapter = nullptr;
    mPendingAdapterDelete = nullptr;
    if (current == nullptr && pending == nullptr) return;
    if (isShowing()) {
        mSpinner->post([current, pending]() { delete current; delete pending; });
    } else {
        delete current;
        delete pending;
    }
}

void Spinner::DropdownPopup::setAdapter(Adapter* adapter){
    // Spinner::setAdapter wraps the data adapter in a fresh DropDownAdapter
    // per call; the replaced wrap is ours to retire. ListPopupWindow::
    // setAdapter unregisters the observer from (and swaps the live list off)
    // the OLD wrap, so it must stay alive for that call — retire it right
    // after, immediately when no tree is up, deferred while it is (the
    // popup ListView keeps calling into a retired wrap until teardown).
    Adapter* retired = (mAdapter != nullptr && mAdapter != adapter) ? mAdapter : nullptr;
    mAdapter = adapter;
    ListPopupWindow::setAdapter(adapter);
    if (retired == nullptr) return;
    if (isShowing()) {
        delete mPendingAdapterDelete;
        mPendingAdapterDelete = retired;
    } else {
        delete retired;
    }
}

void Spinner::DropdownPopup::dismiss(){
    mSpinner->mRecycler->clear();
    // AOSP removes the layout listener from its OnDismissListener, which fires
    // synchronously at dismiss; CDROID defers the dismiss listener to the
    // decor's teardown-complete, so remove it here instead — otherwise the
    // selection's layout pass re-fires the listener and re-shows the popup
    // right after the pick (and before the list was released, crashed on it).
    ViewTreeObserver* vto = mSpinner->getViewTreeObserver();
    if (vto != nullptr) {
        vto->removeOnGlobalLayoutListener(mLayoutListener);
    }
    ListPopupWindow::dismiss();
    // Flush a wrap retired during a show now that the dismiss has queued the
    // decor teardown — posted AFTER it, the looper order frees the wrap only
    // once the popup ListView has stopped touching it (DialogPopup pattern).
    if (mPendingAdapterDelete != nullptr) {
        Adapter* pending = mPendingAdapterDelete;
        mPendingAdapterDelete = nullptr;
        mSpinner->post([pending]() { delete pending; });
    }
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

int Spinner::DropdownPopup::getVerticalOffset()const{
    int p=ListPopupWindow::getVerticalOffset();
    return p;
}

void Spinner::DropdownPopup::setVerticalOffset(int px){
    ListPopupWindow::setVerticalOffset(px);
}

int Spinner::DropdownPopup::getHorizontalOffset()const{
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

    LOGD("mDropDownWidth=%d listView=%p",mSpinner->mDropDownWidth,listView);
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
Spinner::DialogPopup::DialogPopup(Spinner*spinner){
    mSpinner = spinner;
}

Spinner::DialogPopup::~DialogPopup(){
    delete mListAdapter;
    // Not dismissed (never opened, or still showing): we own the dialog —
    // ~Dialog removes a live window itself. ~AlertDialog stays protected
    // ("use dismiss()"), so delete through the public base dtor; virtual
    // dispatch still runs the full ~AlertDialog chain.
    Dialog* owner = mPopup;
    delete owner;
}

void Spinner::DialogPopup::setAdapter(Adapter*adapter){
    // Spinner::setAdapter wraps the data adapter in a fresh DropDownAdapter
    // per call; the replaced wrap is ours. A live dialog's ListView still
    // references the old one, so free it immediately only when no popup is
    // up — during the dismiss's posted teardown (selection rebinds the row
    // while isShowing() is still true) hand it to the spinner's looper to
    // die after the dialog does; dropping it there leaked one wrap per
    // selection (valgrind: 48B x selections at spinner.cc setAdapter).
    if (mListAdapter != nullptr && mListAdapter != adapter) {
        if (mPopup == nullptr || !mPopup->isShowing()) {
            delete mListAdapter;
        } else {
            // Defer past the dialog's OWN death, not just off this stack: the
            // popup ListView keeps calling mAdapter (layoutChildren via the
            // touchMode dispatch) until the dialog tree is destroyed, and a
            // looper-order delete of the wrap crashed it (UAF in
            // ListView::layoutChildren reading freed mAdapter). The dialog's
            // posted teardown frees this wrap after its tree.
            delete mPendingAdapterDelete;
            mPendingAdapterDelete = mListAdapter;
        }
    }
    mListAdapter = adapter;
}

void Spinner::DialogPopup::show(int textDirection, int textAlignment){
    if (mListAdapter == nullptr) {
        return;
    }
    DialogInterface::OnClickListener onDialogClick = [this](DialogInterface& dialog, int which){
        onClick(dialog,which);
    };
    mPopup =AlertDialog::Builder(mSpinner->getPopupContext())
        .setTitle(mPrompt)
        .setSingleChoiceItems(mListAdapter,mSpinner->getSelectedItemPosition(),onDialogClick).show();

    ListView* listView = mPopup->getListView();
    listView->setTextDirection(textDirection);
    listView->setTextAlignment(textAlignment);
    mPopup->show();
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
    // The owner frees the shell after teardown (see dialog.h); via the public
    // base dtor — ~AlertDialog itself stays protected. Deferred to the looper:
    // this dismiss runs INSIDE the dialog's item-click callback (AlertController's
    // lambda touches the dialog after the listener returns — a synchronous
    // delete was a use-after-free, valgrind: invalid read at alertcontroller.cc:790,
    // and the pointer-corruption fallout showed up as the 239K/632K "leak"
    // clusters). AOSP survives the same reentry on GC.
    Dialog* owner = mPopup;
    Adapter* pendingWrap = mPendingAdapterDelete;
    mPendingAdapterDelete = nullptr;
    mSpinner->post([owner, pendingWrap]() {
        delete owner;          // dialog dies first (its ListView stops touching mAdapter)
        delete pendingWrap;    // then the retired wrap is safe to free
    });
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

int Spinner::DialogPopup::getVerticalOffset()const{
    return 0;
}

void Spinner::DialogPopup::setVerticalOffset(int px){
    LOGE("Cannot set vertical offset for MODE_DIALOG, ignoring");
}

int Spinner::DialogPopup::getHorizontalOffset()const{
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

