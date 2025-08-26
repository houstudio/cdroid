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
#include <widget/listpopupwindow.h>
#include <widget/linearlayout.h>
#include <cdlog.h>
namespace cdroid{

class PopupDataSetObserver:public DataSetObserver{
private:
    ListPopupWindow*mListPopupWindow;
public:
    PopupDataSetObserver(ListPopupWindow*p){
        mListPopupWindow = p;
    }
    void onChanged()override{
        if(mListPopupWindow->isShowing())
            mListPopupWindow->show();
    }
    void onInvalidated()override{
        mListPopupWindow->dismiss();
    }
    void clearSavedState()override{
    }
};

ListPopupWindow::ListPopupWindow(Context*context,const AttributeSet&atts)
    :ListPopupWindow(context,atts,"android:attr/listPopupWindowStyle",""){
}

ListPopupWindow::ListPopupWindow(Context* context,const AttributeSet& atts, const std::string&defStyleAttr)
    :ListPopupWindow(context,atts,defStyleAttr,""){
}

ListPopupWindow::ListPopupWindow(Context* context,const AttributeSet& atts, const std::string&defStyleAttr, const std::string&defStyleRes){
    mContext = context;
    initPopupWindow();
    mDropDownHorizontalOffset = atts.getDimensionPixelOffset("dropDownHorizontalOffet",0);
    mDropDownVerticalOffset   = atts.getDimensionPixelOffset("dropDownVerticalOffet",0);
    mPopup = new PopupWindow(mContext,atts,defStyleAttr,defStyleRes);
}

ListPopupWindow::~ListPopupWindow(){
    delete mHandler;
    delete mPopup;
    delete mObserver;
    delete mDropDownListHighlight;
}

void ListPopupWindow::initPopupWindow(){
    mObserver = nullptr;
    mOverlapAnchor = 0xFF;
    mDropDownAlwaysVisible  = false;
    mForceIgnoreOutsideTouch= false;
    mListItemExpandMaximum = INT_MAX;
    mDropDownHorizontalOffset = 0;
    mDropDownVerticalOffset = 0;
    mDropDownList = nullptr;
    mPromptView = nullptr;
    mDropDownAnchorView = nullptr;
    mDropDownListHighlight = nullptr;
    mDropDownWidth  = LayoutParams::WRAP_CONTENT;
    mDropDownHeight = LayoutParams::WRAP_CONTENT;
    mDropDownGravity= Gravity::NO_GRAVITY;
    mPromptPosition = POSITION_PROMPT_ABOVE;
    mHandler = new Handler();
    mPopup = nullptr;
    mHideSelector = [this](){
        clearListSelection();
    };
    mTouchInterceptor = [this] (View& v, MotionEvent& event){
        const int action = event.getAction();
        const int x = (int) event.getX();
        const int y = (int) event.getY();
        if (action == MotionEvent::ACTION_DOWN && mPopup && mPopup->isShowing() &&
            (x >= 0 && x < mPopup->getWidth() && y >= 0 && y < mPopup->getHeight())) {
                mHandler->postDelayed(mResizePopupRunnable, EXPAND_LIST_TIMEOUT);
        } else if (action == MotionEvent::ACTION_UP) {
                mHandler->removeCallbacks(mResizePopupRunnable);
        }
        return false;
    };
}

void ListPopupWindow::setAdapter(Adapter*adapter){
    if( mObserver == nullptr){
        mObserver = new PopupDataSetObserver(this);
    } else if( mAdapter!=nullptr){
        mAdapter->unregisterDataSetObserver(mObserver);
    }
    mAdapter = adapter;
    if(adapter) adapter->registerDataSetObserver(mObserver);
    if(mDropDownList)
        mDropDownList->setAdapter(mAdapter);
}

void ListPopupWindow::setPromptPosition(int position) {
    mPromptPosition = position;
}

int ListPopupWindow::getPromptPosition(){
    return mPromptPosition;
}

void ListPopupWindow::setModal(bool modal) {
     mModal = modal;
     mPopup->setFocusable(modal);
}

bool ListPopupWindow::isModal(){
    return mModal;
}

void ListPopupWindow::setForceIgnoreOutsideTouch(bool forceIgnoreOutsideTouch) {
    mForceIgnoreOutsideTouch = forceIgnoreOutsideTouch;
}

void ListPopupWindow::setDropDownAlwaysVisible(bool dropDownAlwaysVisible) {
    mDropDownAlwaysVisible = dropDownAlwaysVisible;
}

bool ListPopupWindow::isDropDownAlwaysVisible() {
    return mDropDownAlwaysVisible;
}

void ListPopupWindow::setSoftInputMode(int mode) {
    //mPopup->setSoftInputMode(mode);
}

int ListPopupWindow::getSoftInputMode() {
    return 0;//mPopup->getSoftInputMode();
}

void ListPopupWindow::setListSelector(Drawable* selector) {
    if(selector!=mDropDownListHighlight){
        delete mDropDownListHighlight;
        mDropDownListHighlight = selector;
    }
}

Drawable* ListPopupWindow::getBackground() {
    return mPopup->getBackground();
}

void ListPopupWindow::setBackgroundDrawable(Drawable* d) {
    mPopup->setBackgroundDrawable(d);
}

void ListPopupWindow::setAnimationStyle(const std::string&animationStyle){
    //mPopup->setAnimationStyle(animationStyle);
}


std::string ListPopupWindow::getAnimationStyle(){
    return std::string();//mPopup->getAnimationStyle();
}

View* ListPopupWindow::getAnchorView() {
    return mDropDownAnchorView;
}

void ListPopupWindow::setAnchorView(View* anchor) {
    mDropDownAnchorView = anchor;
}


int ListPopupWindow::getHorizontalOffset() {
    return mDropDownHorizontalOffset;
}


void ListPopupWindow::setHorizontalOffset(int offset) {
    mDropDownHorizontalOffset = offset;
}

int ListPopupWindow::getVerticalOffset() {
    return mDropDownVerticalOffset;
}

void ListPopupWindow::setVerticalOffset(int offset) {
    mDropDownVerticalOffset = offset;
}

void ListPopupWindow::setEpicenterBounds(Rect bounds) {
    mEpicenterBounds = bounds;
}


void ListPopupWindow::setDropDownGravity(int gravity) {
    mDropDownGravity = gravity;
}

int ListPopupWindow::getWidth() {
    return mDropDownWidth;
}


void ListPopupWindow::setWidth(int width) {
    mDropDownWidth = width;
}


void ListPopupWindow::setContentWidth(int width) {
    Drawable* popupBackground = mPopup->getBackground();
    if (popupBackground != nullptr) {
        Rect mTempRect;
        popupBackground->getPadding(mTempRect);
        mDropDownWidth = mTempRect.left + mTempRect.right() + width;
    } else {
        setWidth(width);
    }
}

int ListPopupWindow::getHeight() {
    return mDropDownHeight;
}


void ListPopupWindow::setHeight(int height) {
    if (height < 0 && LayoutParams::WRAP_CONTENT != height
            && LayoutParams::MATCH_PARENT != height) {
        throw "Invalid height. Must be a positive value, MATCH_PARENT, or WRAP_CONTENT.";
    }
    mDropDownHeight = height;
}

void ListPopupWindow::setWindowLayoutType(int layoutType) {
    mDropDownWindowLayoutType = layoutType;
}


void ListPopupWindow::setOnItemClickListener(AdapterView::OnItemClickListener clickListener) {
    mItemClickListener = clickListener;
}


void ListPopupWindow::setOnItemSelectedListener(AdapterView::OnItemSelectedListener selectedListener) {
    mItemSelectedListener = selectedListener;
}


void ListPopupWindow::setPromptView( View* prompt) {
    bool showing = isShowing();
    if (showing) {
        removePromptView();
    }
    mPromptView = prompt;
    if (showing) {
        show();
    }
}

void ListPopupWindow::postShow(){
    mHandler->post(mShowDropDownRunnable);
}

void ListPopupWindow::show() {
     int height = buildDropDown();

     bool noInputMethod = false;//isInputMethodNotNeeded();
     mPopup->setWindowLayoutType(mDropDownWindowLayoutType);

     if (mPopup->isShowing()) {
         if (!getAnchorView()->isAttachedToWindow()) {
             //Don't update position if the anchor view is detached from window.
             return;
         }
         int widthSpec;
         if (mDropDownWidth == LayoutParams::MATCH_PARENT) {
             // The call to PopupWindow's update method below can accept -1 for any
             // value you do not want to update.
             widthSpec = -1;
         } else if (mDropDownWidth == LayoutParams::WRAP_CONTENT) {
             widthSpec = getAnchorView()->getWidth();
         } else {
             widthSpec = mDropDownWidth;
         }

         int heightSpec;
         if (mDropDownHeight == LayoutParams::MATCH_PARENT) {
             // The call to PopupWindow's update method below can accept -1 for any
             // value you do not want to update.
             heightSpec = noInputMethod ? height : LayoutParams::MATCH_PARENT;
             if (noInputMethod) {
                 mPopup->setWidth(mDropDownWidth == LayoutParams::MATCH_PARENT ? LayoutParams::MATCH_PARENT : 0);
                 mPopup->setHeight(0);
             } else {
                 mPopup->setWidth(mDropDownWidth == LayoutParams::MATCH_PARENT ? LayoutParams::MATCH_PARENT : 0);
                 mPopup->setHeight(LayoutParams::MATCH_PARENT);
             }
         } else if (mDropDownHeight == LayoutParams::WRAP_CONTENT) {
             heightSpec = height;
         } else {
             heightSpec = mDropDownHeight;
         }

         mPopup->setOutsideTouchable(!mForceIgnoreOutsideTouch && !mDropDownAlwaysVisible);

         mPopup->update(getAnchorView(), mDropDownHorizontalOffset, mDropDownVerticalOffset,
             (widthSpec < 0)? -1 : widthSpec, (heightSpec < 0)? -1 : heightSpec);
     } else {
         int widthSpec;
         if (mDropDownWidth == LayoutParams::MATCH_PARENT) {
             widthSpec = LayoutParams::MATCH_PARENT;
         } else {
             if (mDropDownWidth == LayoutParams::WRAP_CONTENT) {
                 widthSpec = getAnchorView()->getWidth();
             } else {
                 widthSpec = mDropDownWidth;
             }
         }

         int heightSpec;
         if (mDropDownHeight == LayoutParams::MATCH_PARENT) {
             heightSpec = LayoutParams::MATCH_PARENT;
         } else {
             if (mDropDownHeight == LayoutParams::WRAP_CONTENT) {
                 heightSpec = height;
             } else {
                 heightSpec = mDropDownHeight;
             }
         }

         mPopup->setWidth(widthSpec);
         mPopup->setHeight(heightSpec);
         //setPopupClipToScreenEnabled(true);

         // use outside touchable to dismiss drop down when touching outside of it, so
         // only set this if the dropdown is not always visible
         mPopup->setOutsideTouchable(!mForceIgnoreOutsideTouch && !mDropDownAlwaysVisible);
         mPopup->setTouchInterceptor(mTouchInterceptor);
         if (mOverlapAnchor!=0xFF){
             mPopup->setOverlapAnchor(mOverlapAnchor);
         }
         mPopup->setEpicenterBounds(mEpicenterBounds);
         mPopup->showAsDropDown(getAnchorView(), mDropDownHorizontalOffset,
                 mDropDownVerticalOffset, mDropDownGravity);
         mDropDownList->setSelection(ListView::INVALID_POSITION);

         if (!mModal || mDropDownList->isInTouchMode()) {
             clearListSelection();
         }
         if (!mModal) {
             mHandler->post(mHideSelector);
         }
     }
}

void ListPopupWindow::dismiss() {
    mPopup->dismiss();
    removePromptView();
    mPopup->setContentView(nullptr);
    //mDropDownList->setAdapter(nullptr);
    //delete mDropDownList;
    mDropDownList = nullptr;
    mHandler->removeCallbacks(mResizePopupRunnable);
}

void ListPopupWindow::setOnDismissListener(PopupWindow::OnDismissListener listener) {
     mPopup->setOnDismissListener(listener);
}

void ListPopupWindow::removePromptView() {
    if (mPromptView != nullptr) {
        ViewGroup* parent = mPromptView->getParent();
        parent->removeView(mPromptView);
    }
}

void ListPopupWindow::setInputMethodMode(int mode) {
    mPopup->setInputMethodMode(mode);
}


int ListPopupWindow::getInputMethodMode() {
    return mPopup->getInputMethodMode();
}

void ListPopupWindow::setSelection(int position) {
    DropDownListView* list = mDropDownList;
    if (isShowing() && list != nullptr) {
        list->setListSelectionHidden(false);
        list->setSelection(position);

        if (list->getChoiceMode() != ListView::CHOICE_MODE_NONE) {
            list->setItemChecked(position, true);
        }
    }
}


void ListPopupWindow::clearListSelection() {
    DropDownListView* list = mDropDownList;
    if (list != nullptr) {
        // WARNING: Please read the comment where mListSelectionHidden is declared
        list->setListSelectionHidden(true);
        //list.hideSelector();
        list->requestLayout();
    }
}


bool ListPopupWindow::isShowing() {
    return mPopup->isShowing();
}


bool ListPopupWindow::isInputMethodNotNeeded() {
    return false;//mPopup->getInputMethodMode() == INPUT_METHOD_NOT_NEEDED;
}

bool ListPopupWindow::performItemClick(int position) {
    if (isShowing()) {
        if (mItemClickListener != nullptr) {
            DropDownListView* list = mDropDownList;
            View* child = list->getChildAt(position - list->getFirstVisiblePosition());
            ListAdapter* adapter = list->getAdapter();
            mItemClickListener(*list,*child, position, adapter->getItemId(position));
        }
        return true;
    }
    return false;
}


void* ListPopupWindow::getSelectedItem() {
    if (!isShowing()) {
        return nullptr;
    }
    return mDropDownList->getSelectedItem();
}


int ListPopupWindow::getSelectedItemPosition() {
    if (!isShowing()) {
        return ListView::INVALID_POSITION;
    }
    return mDropDownList->getSelectedItemPosition();
}


long ListPopupWindow::getSelectedItemId() {
    if (!isShowing()) {
        return ListView::INVALID_ROW_ID;
    }
    return mDropDownList->getSelectedItemId();
}


View* ListPopupWindow::getSelectedView() {
    if (!isShowing()) {
        return nullptr;
    }
    return mDropDownList->getSelectedView();
}

ListView* ListPopupWindow::getListView() const{
    return mDropDownList;
}

DropDownListView* ListPopupWindow::createDropDownListView(Context* context, bool hijackFocus){
    return new DropDownListView(context, hijackFocus);
}

void ListPopupWindow::setListItemExpandMax(int max){
    mListItemExpandMaximum = max;
}

bool ListPopupWindow::onKeyDown(int keyCode,KeyEvent& event){
    // when the drop down is shown, we drive it directly
    if (isShowing()) {
        // the key events are forwarded to the list in the drop down view
        // note that ListView handles space but we don't want that to happen
        // also if selection is not currently in the drop down, then don't
        // let center or enter presses go there since that would cause it
        // to select one of its items
        if (keyCode != KeyEvent::KEYCODE_SPACE
                && (mDropDownList->getSelectedItemPosition() >= 0
                || !KeyEvent::isConfirmKey(keyCode))) {
            int curIndex = mDropDownList->getSelectedItemPosition();
            bool consumed;

            bool below = !mPopup->isAboveAnchor();

            ListAdapter* adapter = mAdapter;

            bool allEnabled;
            int firstItem = INT_MAX;//Integer.MAX_VALUE;
            int lastItem = INT_MIN;//Integer.MIN_VALUE;

            if (adapter != nullptr) {
                allEnabled = adapter->areAllItemsEnabled();
                firstItem = allEnabled ? 0 :
                       mDropDownList->lookForSelectablePosition(0, true);
                lastItem = allEnabled ? adapter->getCount() - 1 :
                       mDropDownList->lookForSelectablePosition(adapter->getCount() - 1, false);
            }

            if ((below && keyCode == KeyEvent::KEYCODE_DPAD_UP && curIndex <= firstItem) ||
                   (!below && keyCode == KeyEvent::KEYCODE_DPAD_DOWN && curIndex >= lastItem)) {
                // When the selection is at the top, we block the key
                // event to prevent focus from moving.
                clearListSelection();
                mPopup->setInputMethodMode(PopupWindow::INPUT_METHOD_NEEDED);
                show();
                return true;
            } else {
                // WARNING: Please read the comment where mListSelectionHidden
                //          is declared
                mDropDownList->setListSelectionHidden(false);
            } 

            consumed = mDropDownList->onKeyDown(keyCode, event);
            LOGV("Key down: code=%d list consumed=%d",keyCode,consumed);

            if (consumed) {
                // If it handled the key event, then the user is
                // navigating in the list, so we should put it in front.
                mPopup->setInputMethodMode(PopupWindow::INPUT_METHOD_NOT_NEEDED);
                // Here's a little trick we need to do to make sure that
                // the list view is actually showing its focus indicator,
                // by ensuring it has focus and getting its window out
                // of touch mode.
                mDropDownList->requestFocusFromTouch();
                show();

                switch (keyCode) {
                // avoid passing the focus from the text view to the
                // next component
                case KeyEvent::KEYCODE_ENTER:
                case KeyEvent::KEYCODE_DPAD_CENTER:
                case KeyEvent::KEYCODE_DPAD_DOWN:
                case KeyEvent::KEYCODE_DPAD_UP:
                     return true;
                }
            } else {
                if (below && keyCode == KeyEvent::KEYCODE_DPAD_DOWN) {
                    // when the selection is at the bottom, we block the
                    // event to avoid going to the next focusable widget
                    if (curIndex == lastItem) {
                        return true;
                    }
                } else if (!below && keyCode == KeyEvent::KEYCODE_DPAD_UP &&
                        curIndex == firstItem) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool ListPopupWindow::onKeyUp(int keyCode,KeyEvent& event) {
    if (isShowing() && mDropDownList->getSelectedItemPosition() >= 0) {
        bool consumed = mDropDownList->onKeyUp(keyCode, event);
        if (consumed && KeyEvent::isConfirmKey(keyCode)) {
            // if the list accepts the key events and the key event was a click, the text view
            // gets the selected item from the drop down as its content
            dismiss();
        }
        return consumed;
    }
    return false;
}

int ListPopupWindow::buildDropDown() {
    ViewGroup* dropDownView;
    int otherHeights = 0;

    if (mDropDownList == nullptr) {
        Context* context = mContext;

        /**
         * This Runnable exists for the sole purpose of checking if the view layout has got
         * completed and if so call showDropDown to display the drop down. This is used to show
         * the drop down as soon as possible after user opens up the search dialog, without
         * waiting for the normal UI pipeline to do its job which is slower than this method.
         */
        mShowDropDownRunnable =[this](){
            // View layout should be all done before displaying the drop down.
            View* view = getAnchorView();
            if (view != nullptr){// && view.getWindowToken() != null) {
                show();
            }
        };

        mDropDownList = createDropDownListView(context, !mModal);
        if (mDropDownListHighlight != nullptr) {
            mDropDownList->setSelector(mDropDownListHighlight);
        }
        mDropDownList->setAdapter(mAdapter);
        mDropDownList->setOnItemClickListener(mItemClickListener);
        mDropDownList->setFocusable(true);
        mDropDownList->setFocusableInTouchMode(true);
        AdapterView::OnItemSelectedListener listener;
        listener.onItemSelected = [this](AdapterView&parent,View&view,int position,long id){
            if (position != -1) {
                if (mDropDownList != nullptr) {
                    mDropDownList->setListSelectionHidden(false);
                }
            }
        };
        listener.onNothingSelected=[](AdapterView&parent){};
        mDropDownList->setOnItemSelectedListener(listener);
        mDropDownList->setOnScrollListener(mScrollListener);

        if (mItemSelectedListener.onItemSelected != nullptr||mItemSelectedListener.onNothingSelected!=nullptr) {
            mDropDownList->setOnItemSelectedListener(mItemSelectedListener);
        }

        dropDownView = mDropDownList;

        View* hintView = mPromptView;
        if (hintView != nullptr) {
            // if a hint has been specified, we accommodate more space for it and
            // add a text view in the drop down menu, at the bottom of the list
            LinearLayout* hintContainer = new LinearLayout(context,AttributeSet());
            hintContainer->setOrientation(LinearLayout::VERTICAL);

            LinearLayout::LayoutParams* hintParams = new LinearLayout::LayoutParams(
                    LayoutParams::MATCH_PARENT, 0, 1.0f
            );

            switch (mPromptPosition) {
            case POSITION_PROMPT_BELOW:
                hintContainer->addView(dropDownView, hintParams);
                hintContainer->addView(hintView);
                break;

            case POSITION_PROMPT_ABOVE:
                hintContainer->addView(hintView);
                hintContainer->addView(dropDownView, hintParams);
                break;
            default:
                LOGE("Invalid hint position %d" ,mPromptPosition);
                break;
            }

            // Measure the hint's height to find how much more vertical
            // space we need to add to the drop down's height.
            int widthSize;
            int widthMode;
            if (mDropDownWidth >= 0) {
                widthMode = MeasureSpec::AT_MOST;
                widthSize = mDropDownWidth;
            } else {
                widthMode = MeasureSpec::UNSPECIFIED;
                widthSize = 0;
            }
            int widthSpec = MeasureSpec::makeMeasureSpec(widthSize, widthMode);
            int heightSpec = MeasureSpec::UNSPECIFIED;
            hintView->measure(widthSpec, heightSpec);

            hintParams = (LinearLayout::LayoutParams*) hintView->getLayoutParams();
            otherHeights = hintView->getMeasuredHeight() + hintParams->topMargin
                    + hintParams->bottomMargin;

            dropDownView = hintContainer;
        }

        mPopup->setContentView(dropDownView);
    } else {
        dropDownView = (ViewGroup*) mPopup->getContentView();
        View* view = mPromptView;
        if (view != nullptr) {
            LinearLayout::LayoutParams* hintParams =
                    (LinearLayout::LayoutParams*) view->getLayoutParams();
            otherHeights = view->getMeasuredHeight() + hintParams->topMargin
                    + hintParams->bottomMargin;
        }
    }

    // getMaxAvailableHeight() subtracts the padding, so we put it back
    // to get the available height for the whole window.
    int padding;
    Drawable* background = mPopup->getBackground();
    Rect mTempRect;
    if (background != nullptr) {
        background->getPadding(mTempRect);
        padding = mTempRect.top + mTempRect.height;

        // If we don't have an explicit vertical offset, determine one from
        // the window background so that content will line up.
        if (mDropDownVerticalOffset!=0){
            mDropDownVerticalOffset = -mTempRect.top;
        }
    } else {
        mTempRect.setEmpty();
        padding = 0;
    }

    // Max height available on the screen for a popup.
    bool ignoreBottomDecorations =
            mPopup->getInputMethodMode() == PopupWindow::INPUT_METHOD_NOT_NEEDED;
    int maxHeight = mPopup->getMaxAvailableHeight(getAnchorView(), mDropDownVerticalOffset,
            ignoreBottomDecorations);
    if (mDropDownAlwaysVisible || mDropDownHeight == LayoutParams::MATCH_PARENT) {
        return maxHeight + padding;
    }

    int childWidthSpec;
    switch (mDropDownWidth) {
    case LayoutParams::WRAP_CONTENT:
        childWidthSpec = MeasureSpec::makeMeasureSpec(
                mContext->getDisplayMetrics().widthPixels
                       - (mTempRect.left + mTempRect.width),
                MeasureSpec::AT_MOST);
        break;
    case LayoutParams::MATCH_PARENT:
        childWidthSpec = MeasureSpec::makeMeasureSpec(
                mContext->getDisplayMetrics().widthPixels
                        - (mTempRect.left + mTempRect.width),
                MeasureSpec::EXACTLY);
        break;
    default:
        childWidthSpec = MeasureSpec::makeMeasureSpec(mDropDownWidth, MeasureSpec::EXACTLY);
        break;
    }

    // Add padding only if the list has items in it, that way we don't show
    // the popup if it is not needed.
    int listContent = mDropDownList->measureHeightOfChildren(childWidthSpec,
            0, DropDownListView::NO_POSITION, maxHeight - otherHeights, -1);
    if (listContent > 0) {
        int listPadding = mDropDownList->getPaddingTop()
                + mDropDownList->getPaddingBottom();
        otherHeights += padding + listPadding;
    }

    return listContent + otherHeights;
}

void ListPopupWindow::setOverlapAnchor(bool overlap) {
    mOverlapAnchor = overlap;
}
}
