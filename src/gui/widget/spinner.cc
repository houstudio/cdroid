#include <widget/spinner.h>
#include <widget/measurespec.h>
#include <widget/listview.h>
#include <cdtypes.h>
#include <cdlog.h>
#define MAX_ITEMS_MEASURED  15
namespace cdroid{

/////////////////////////////////////////////////////////////////////////////////////////

DECLARE_WIDGET2(Spinner,"cdroid:attr/spinnerStyle")

Spinner::Spinner(int w,int h,int mode):AbsSpinner(w,h){
    mPopupContext=nullptr;
    mGravity= Gravity::CENTER;
    mDisableChildrenWhenDisabled=true;
    mPopup=new DialogPopup(this);
}

Spinner::Spinner(Context*ctx,const AttributeSet&atts)
  :AbsSpinner(ctx,atts){
    mPopupContext =ctx;
    mGravity= atts.getGravity("gravity",Gravity::CENTER);
    mDisableChildrenWhenDisabled=atts.getBoolean("disableChildrenWhenDisabled",false);
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
    /*if (dynamic_cast<DropdownPopup*>(mPopup)) {
        LOGE("Cannot set dropdown width for MODE_DIALOG, ignoring");
        return;
    }*/
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
    if (mPopup != nullptr && MeasureSpec::getMode(widthMeasureSpec) == MeasureSpec::AT_MOST) {
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
    mPopup->show(0,0);
    return AbsSpinner::onTouchEvent(event);
}

/////////////////////////////////SpinnerPopup//////////////////////////////////////////
Spinner::DropdownPopup::DropdownPopup(Spinner*sp){
    mDropDownWidth = LayoutParams::WRAP_CONTENT;
    mSpinner=sp;
    mAdapter=nullptr;
    mListView=nullptr;
}

Spinner::DropdownPopup::~DropdownPopup(){
}

void Spinner::DropdownPopup::setAdapter(Adapter* adapter){
    mAdapter = adapter;
}

void Spinner::DropdownPopup::dismiss(){
}

bool Spinner::DropdownPopup::isShowing(){
    return true;
}

void Spinner::DropdownPopup::setPromptText(const std::string& hintText){
}

const std::string Spinner::DropdownPopup::getHintText(){
    return "";
}

int Spinner::DropdownPopup::getVerticalOffset(){
    return 0;
}

void Spinner::DropdownPopup::setVerticalOffset(int px){
}

int Spinner::DropdownPopup::getHorizontalOffset(){
    return 0;
}

void Spinner::DropdownPopup::setHorizontalOffset(int px){
}

void Spinner::DropdownPopup::setBackgroundDrawable(Drawable* bg){
    
}

Drawable* Spinner::DropdownPopup::getBackground(){
    return nullptr;
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

    if (mDropDownWidth  == LayoutParams::WRAP_CONTENT) {
        int contentWidth =  mSpinner->measureContentWidth(mSpinner->getAdapter(), mSpinner->getBackground());
        int contentWidthLimit = mSpinner->getContext()->getDisplayMetrics().widthPixels - mTempRect.width;
        if (contentWidth > contentWidthLimit) {
            contentWidth = contentWidthLimit;
        }
        setContentWidth(std::max( contentWidth, spinnerWidth - spinnerPaddingLeft - spinnerPaddingRight));
    } else if (mDropDownWidth == LayoutParams::MATCH_PARENT) {
        setContentWidth(spinnerWidth - spinnerPaddingLeft - spinnerPaddingRight);
    } else {
        setContentWidth(mDropDownWidth);
    }

    if (mSpinner->isLayoutRtl()) {
        hOffset += spinnerWidth - spinnerPaddingRight - mSpinner->getWidth();
    } else {
        hOffset += spinnerPaddingLeft;
    }
    setHorizontalOffset(hOffset);
}

void Spinner::DropdownPopup::setContentWidth(int width){
    Drawable* popupBackground = getBackground();
    if (popupBackground ) {
	Rect rect;
        popupBackground->getPadding(rect);
        mDropDownWidth = rect.left + rect.width + width;
    } else {
        mDropDownWidth=width;
    }
}

ListView*Spinner::DropdownPopup::getListView()const{
    return mListView;
}

void Spinner::DropdownPopup::show(int textDirection, int textAlignment) {
    bool wasShowing = isShowing();

    computeContentWidth();

    //setInputMethodMode(ListPopupWindow.INPUT_METHOD_NOT_NEEDED);
    //show();
    LOGD("====mDropDownWidth=%d",mDropDownWidth);
    ListView* listView = getListView();

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
    /*final ViewTreeObserver vto = getViewTreeObserver();
    if (vto != null) {
        OnGlobalLayoutListener layoutListener = new OnGlobalLayoutListener() {
        @Override
        public void onGlobalLayout() {
            if (!Spinner.this.isVisibleToUser()) {
                dismiss();
            } else {
                computeContentWidth();
                // Use super.show here to update; we don't want to move the selected
                // position or adjust other things that would be reset otherwise.
                DropdownPopup.super.show();
                        }
            }
        };
        vto.addOnGlobalLayoutListener(layoutListener);
        setOnDismissListener(new OnDismissListener() {
            @Override public void onDismiss() {
                final ViewTreeObserver vto = getViewTreeObserver();
                if (vto != null) {
                    vto.removeOnGlobalLayoutListener(layoutListener);
                }
            }
        });
    }*/
}
//int SpinnerPopup::measureContentWidth(Adapter*,Drawable*){}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
Spinner::DialogPopup::DialogPopup(Spinner*spinner){
    mSpinner=spinner;
}

void Spinner::DialogPopup::setAdapter(Adapter*adapter){
    mAdapter=adapter;
}

void Spinner::DialogPopup::show(int textDirection, int textAlignment){
}

void Spinner::DialogPopup::dismiss(){
    mPopup->dismiss();
    mPopup=nullptr;
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
}

int Spinner::DialogPopup::getHorizontalOffset(){
    return 0;
}

void Spinner::DialogPopup::setHorizontalOffset(int px){
}

void Spinner::DialogPopup::setBackgroundDrawable(Drawable* bg){
}

Drawable* Spinner::DialogPopup::getBackground(){
    return nullptr;
}

void Spinner::DialogPopup::computeContentWidth(){
}

void Spinner::DialogPopup::setContentWidth(int width){
}

}//endof namespace

