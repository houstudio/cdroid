#include <widget/spinner.h>
#include <widget/listview.h>
#include <widget/dropdownlistview.h>
#include <widget/forwardinglistener.h>
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <cdtypes.h>
#include <cdlog.h>
#define MAX_ITEMS_MEASURED  15
namespace cdroid{

/////////////////////////////////////////////////////////////////////////////////////////

DECLARE_WIDGET2(Spinner,"cdroid:attr/spinnerStyle")

Spinner::SpinnerForwardingListener::SpinnerForwardingListener(View*v,Spinner::DropdownPopup*d)
:ForwardingListener(v){
    mSpinner =(Spinner*)v;
    mDropDown = d;
}

ShowableListMenu Spinner::SpinnerForwardingListener::getPopup(){
    ShowableListMenu sm;
    sm.show =[this](){mDropDown->show(mSpinner->getTextDirection(),mSpinner->getTextAlignment());};
    sm.dismiss=[this](){mDropDown->dismiss();};
    sm.isShowing=[this]()->bool{return mDropDown->isShowing();};
    sm.getListView=[this]()->ListView*{return nullptr;};
    return sm;
}

bool Spinner::SpinnerForwardingListener::onForwardingStarted(){
    if(!mDropDown->isShowing()){
        mDropDown->show(mSpinner->getTextDirection(),mSpinner->getTextAlignment());
    }
    return true;
}

Spinner::Spinner(int w,int h,int mode):AbsSpinner(w,h){
    mPopupContext = mContext;
    mGravity = Gravity::CENTER;
    mDropDownWidth =0;
    mDisableChildrenWhenDisabled = true;
    mPopup = new DropdownPopup(mContext,this);
    mForwardingListener = new SpinnerForwardingListener(this,(DropdownPopup*)mPopup); 
}

Spinner::Spinner(Context*ctx,const AttributeSet&atts)
  :AbsSpinner(ctx,atts){
    mPopupContext = ctx;
    mForwardingListener = nullptr;
    mGravity = atts.getGravity("gravity",Gravity::CENTER);
    mDisableChildrenWhenDisabled = atts.getBoolean("disableChildrenWhenDisabled",false);
    const int mode = atts.getInt("spinnerMode",std::unordered_map<std::string,int>{
        {"dialog",(int)MODE_DIALOG},{"dropdown",(int)MODE_DROPDOWN}
    },MODE_DIALOG);

    DropdownPopup* popup;
    switch(mode){
    case MODE_DIALOG:
         mPopup = new DialogPopup(this);
         mPopup->setPromptText(atts.getString("propmt"));
         break;
    case MODE_DROPDOWN:
         popup = new DropdownPopup(ctx,this);
         mDropDownWidth = atts.getLayoutDimension("dropDownWidth",LayoutParams::WRAP_CONTENT);
         Drawable*d = mPopupContext->getDrawable(atts.getString("popupBackground"));
         if(d)popup->setBackgroundDrawable(d);
         popup->setPromptText(atts.getString("propmt"));
         mPopup = popup;
         mForwardingListener = new SpinnerForwardingListener(this,popup); 
         break;
    } 
}

Spinner::~Spinner(){
    delete mPopup;
    delete mForwardingListener;
}

Context* Spinner::getPopupContext()const{
    return mPopupContext;
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

View& Spinner::setEnabled(bool enabled) {
    AbsSpinner::setEnabled(enabled);
    if (mDisableChildrenWhenDisabled) {
        int count = getChildCount();
        for (int i = 0; i < count; i++) {
            getChildAt(i)->setEnabled(enabled);
        }
    }
    return *this;
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
    int childrenLeft = mSpinnerPadding.left;
    int childrenWidth = getWidth() - mSpinnerPadding.left - mSpinnerPadding.width;

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
        int width = sel->getMeasuredWidth();
        int selectedOffset = childrenLeft;
        int layoutDirection = getLayoutDirection();
        int absoluteGravity = Gravity::getAbsoluteGravity(mGravity, layoutDirection);
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

void Spinner::setAdapter(Adapter*adapter){
    AbsSpinner::setAdapter(adapter);
    mRecycler->clear();
    Context* popupContext = mPopupContext == nullptr ? mContext : mPopupContext;
    mPopup->setAdapter(adapter);//new DropDownAdapter(adapter, popupContext.getTheme()))
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
        int childBaseline = child->getBaseline();
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
    if (lp == nullptr)  lp = generateDefaultLayoutParams();

    addViewInLayout(child, 0, lp);

    child->setSelected(hasFocus());
    if (mDisableChildrenWhenDisabled) {
        child->setEnabled(isEnabled());
    }

    // Get measure specs
    int childHeightSpec = ViewGroup::getChildMeasureSpec(mHeightMeasureSpec,
            mSpinnerPadding.top + mSpinnerPadding.height, lp->height);
    int childWidthSpec = ViewGroup::getChildMeasureSpec(mWidthMeasureSpec,
            mSpinnerPadding.left + mSpinnerPadding.width, lp->width);

    // Measure child
    child->measure(childWidthSpec, childHeightSpec);

    int childLeft;
    // Position vertically based on gravity setting
    int childTop = mSpinnerPadding.top  + ((getMeasuredHeight() - mSpinnerPadding.height -
                    mSpinnerPadding.top - child->getMeasuredHeight()) / 2);
    int height =child->getMeasuredHeight();
    int width = child->getMeasuredWidth();
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

int Spinner::measureContentWidth(Adapter* adapter, Drawable* background){
    if (adapter == nullptr) return 0;

    int width = 0;
    View* itemView = nullptr;
    int itemType = 0;
    int widthMeasureSpec =MeasureSpec::makeSafeMeasureSpec(getMeasuredWidth(), MeasureSpec::UNSPECIFIED);
    int heightMeasureSpec = MeasureSpec::makeSafeMeasureSpec(getMeasuredHeight(), MeasureSpec::UNSPECIFIED);

    // Make sure the number of items we'll measure is capped. If it's a huge data set
    // with wildly varying sizes, oh well.
    int start = std::max(0, getSelectedItemPosition());
    int end = std::min(adapter->getCount(), start + MAX_ITEMS_MEASURED);
    int count = end - start;
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
Spinner::DropdownPopup::DropdownPopup(Context*context,Spinner*sp)
  :ListPopupWindow(context,AttributeSet(context,"")){
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

    int spinnerPaddingLeft = mSpinner->getPaddingLeft();
    int spinnerPaddingRight= mSpinner->getPaddingRight();
    int spinnerWidth = mSpinner->getWidth();

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

    //setInputMethodMode(ListPopupWindow::INPUT_METHOD_NOT_NEEDED);
    ListPopupWindow::show();
    ListView* listView = getListView();

    LOGV("====mDropDownWidth=%d listView=%p",mSpinner->mDropDownWidth,listView);
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

