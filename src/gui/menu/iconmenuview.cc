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
#include <menu/menubuilder.h>
#include <menu/iconmenuview.h>
#include <menu/iconmenuitemview.h>
namespace cdroid{

DECLARE_WIDGET(IconMenuView)
IconMenuView::IconMenuView(Context* context,const AttributeSet& attrs)
  :ViewGroup(context, attrs){

    mRowHeight= attrs.getDimensionPixelSize("rowHeight", 64);
    mMaxRows  = attrs.getInt("maxRows", 2);
    mMaxItems = attrs.getInt("maxItems", 6);
    mMaxItemsPerRow = attrs.getInt("maxItemsPerRow", 3);
    mMoreIcon = attrs.getDrawable("moreIcon");

    mItemBackground = attrs.getDrawable("itemBackground");
    mHorizontalDivider = attrs.getDrawable("horizontalDivider");
    //mHorizontalDividerRects = new ArrayList<Rect>();
    mVerticalDivider =  attrs.getDrawable("verticalDivider");
    //mVerticalDividerRects = new ArrayList<Rect>();
    mAnimations = attrs.getResourceId("windowAnimationStyle", 0);

    if (mHorizontalDivider != nullptr) {
        mHorizontalDividerHeight = mHorizontalDivider->getIntrinsicHeight();
        // Make sure to have some height for the divider
        if (mHorizontalDividerHeight == -1) mHorizontalDividerHeight = 1;
    }

    if (mVerticalDivider != nullptr) {
        mVerticalDividerWidth = mVerticalDivider->getIntrinsicWidth();
        // Make sure to have some width for the divider
        if (mVerticalDividerWidth == -1) mVerticalDividerWidth = 1;
    }

    mLayout.resize(mMaxRows);

    // This view will be drawing the dividers
    setWillNotDraw(false);

    // This is so we'll receive the MENU key in touch mode
    setFocusableInTouchMode(true);
    // This is so our children can still be arrow-key focused
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    mRunnable = std::bind(&IconMenuView::run,this);
}

int IconMenuView::getMaxItems() const{
    return mMaxItems;
}

void IconMenuView::layoutItems(int width) {
    const int numItems = getChildCount();
    if (numItems == 0) {
        mLayoutNumRows = 0;
        return;
    }

    // Start with the least possible number of rows
    int curNumRows = std::min((int) std::ceil(numItems / (float) mMaxItemsPerRow), mMaxRows);

    /*
     * Increase the number of rows until we find a configuration that fits
     * all of the items' titles. Worst case, we use mMaxRows.
     */
    for (; curNumRows <= mMaxRows; curNumRows++) {
        layoutItemsUsingGravity(curNumRows, numItems);

        if (curNumRows >= numItems) {
            // Can't have more rows than items
            break;
        }

        if (doItemsFit()) {
            // All the items fit, so this is a good configuration
            break;
        }
    }
}

void IconMenuView::layoutItemsUsingGravity(int numRows, int numItems) {
    int numBaseItemsPerRow = numItems / numRows;
    int numLeftoverItems = numItems % numRows;
    /**
     * The bottom rows will each get a leftover item. Rows (indexed at 0)
     * that are >= this get a leftover item. Note: if there are 0 leftover
     * items, no rows will get them since this value will be greater than
     * the last row.
     */
    int rowsThatGetALeftoverItem = numRows - numLeftoverItems;

    std::vector<int>& layout = mLayout;
    for (int i = 0; i < numRows; i++) {
        layout[i] = numBaseItemsPerRow;

        // Fill the bottom rows with a leftover item each
        if (i >= rowsThatGetALeftoverItem) {
            layout[i]++;
        }
    }

    mLayoutNumRows = numRows;
}

bool IconMenuView::doItemsFit() {
    int itemPos = 0;

    std::vector<int>& layout = mLayout;
    int numRows = mLayoutNumRows;
    for (int row = 0; row < numRows; row++) {
        int numItemsOnRow = layout[row];

        if (numItemsOnRow == 1) {
            itemPos++;
            continue;
        }

        for (int itemsOnRowCounter = numItemsOnRow; itemsOnRowCounter > 0;
                itemsOnRowCounter--) {
            View* child = getChildAt(itemPos++);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            if (lp->maxNumItemsOnRow < numItemsOnRow) {
                return false;
            }
        }
    }

    return true;
}

Drawable* IconMenuView::getItemBackgroundDrawable() {
    return mItemBackground->getConstantState()->newDrawable();//getContext().getResources());
}

IconMenuItemView* IconMenuView::createMoreItemView() {
    Context* context = getContext();
    LayoutInflater* inflater = LayoutInflater::from(context);

    IconMenuItemView* itemView = (IconMenuItemView*) inflater->inflate("android:layout/icon_menu_item_layout", nullptr);

    itemView->initialize(""/*r.getText(com.android.internal.R.string.more_item_label)*/, mMoreIcon);

    // Set up a click listener on the view since there will be no invocation sequence
    // due to the lack of a MenuItemData this view
    itemView->setOnClickListener([this](View& v) {
            // Switches the menu to expanded mode. Requires support from
            // the menu's active callback.
            mMenu->changeMenuMode();
        });

    return itemView;
}


void IconMenuView::initialize(MenuBuilder* menu) {
    mMenu = menu;
}

void IconMenuView::positionChildren(int menuWidth, int menuHeight) {
    // Clear the containers for the positions where the dividers should be drawn
    if (mHorizontalDivider != nullptr) mHorizontalDividerRects.clear();
    if (mVerticalDivider != nullptr) mVerticalDividerRects.clear();

    // Get the minimum number of rows needed
    const int numRows = mLayoutNumRows;
    const int numRowsMinus1 = numRows - 1;
    const std::vector<int>& numItemsForRow = mLayout;

    // The item position across all rows
    int itemPos = 0;
    View* child;
    IconMenuView::LayoutParams* childLayoutParams = nullptr;

    // Use float for this to get precise positions (uniform item widths
    // instead of last one taking any slack), and then convert to ints at last opportunity
    float itemLeft;
    float itemTop = 0;
    // Since each row can have a different number of items, this will be computed per row
    float itemWidth;
    Rect rect;
    // Subtract the space needed for the horizontal dividers
    const float itemHeight = (menuHeight - mHorizontalDividerHeight * (numRows - 1))
            / (float)numRows;

    for (int row = 0; row < numRows; row++) {
        // Start at the left
        itemLeft = 0;

        // Subtract the space needed for the vertical dividers, and divide by the number of items
        itemWidth = (menuWidth - mVerticalDividerWidth * (numItemsForRow[row] - 1))
                / (float)numItemsForRow[row];

        for (int itemPosOnRow = 0; itemPosOnRow < numItemsForRow[row]; itemPosOnRow++) {
            // Tell the child to be exactly this size
            child = getChildAt(itemPos);
            child->measure(MeasureSpec::makeMeasureSpec((int) itemWidth, MeasureSpec::EXACTLY),
                    MeasureSpec::makeMeasureSpec((int) itemHeight, MeasureSpec::EXACTLY));

            // Remember the child's position for layout
            childLayoutParams = (IconMenuView::LayoutParams*) child->getLayoutParams();
            childLayoutParams->left = itemLeft;
            childLayoutParams->right= (itemLeft + itemWidth);
            childLayoutParams->top = itemTop;
            childLayoutParams->bottom = (int) (itemTop + itemHeight);

            // Increment by item width
            itemLeft += itemWidth;
            itemPos++;

            // Add a vertical divider to draw
            if (mVerticalDivider != nullptr) {
                rect.set(itemLeft,itemTop, mVerticalDividerWidth,itemHeight);
                mVerticalDividerRects.push_back(rect);
            }

            // Increment by divider width (even if we're not computing
            // dividers, since we need to leave room for them when
            // calculating item positions)
            itemLeft += mVerticalDividerWidth;
        }

        // Last child on each row should extend to very right edge
        if (childLayoutParams != nullptr) {
            childLayoutParams->right = menuWidth;
        }

        itemTop += itemHeight;

        // Add a horizontal divider to draw
        if ((mHorizontalDivider != nullptr) && (row < numRowsMinus1)) {
            rect.set(0, itemTop, menuWidth,mHorizontalDividerHeight);
            mHorizontalDividerRects.push_back(rect);

            itemTop += mHorizontalDividerHeight;
        }
    }
}

void IconMenuView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const int measuredWidth = resolveSize(INT_MAX, widthMeasureSpec);
    calculateItemFittingMetadata(measuredWidth);
    layoutItems(measuredWidth);

    // Get the desired height of the icon menu view (last row of items does
    // not have a divider below)
    const int desiredHeight = (mRowHeight + mHorizontalDividerHeight) *
            mLayoutNumRows - mHorizontalDividerHeight;

    // Maximum possible width and desired height
    setMeasuredDimension(measuredWidth, resolveSize(desiredHeight, heightMeasureSpec));

    // Position the children
    if (mLayoutNumRows > 0) {
        positionChildren(getMeasuredWidth(), getMeasuredHeight());
    }
}


void IconMenuView::onLayout(bool changed, int l, int t, int r, int b) {
    View* child;
    IconMenuView::LayoutParams* childLayoutParams;

    for (int i = getChildCount() - 1; i >= 0; i--) {
        child = getChildAt(i);
        childLayoutParams = (IconMenuView::LayoutParams*)child->getLayoutParams();

        // Layout children according to positions set during the measure
        child->layout(childLayoutParams->left, childLayoutParams->top,
                childLayoutParams->right - childLayoutParams->left,
                childLayoutParams->bottom - childLayoutParams->top);
    }
}

void IconMenuView::onDraw(Canvas& canvas) {
    Drawable* drawable = mHorizontalDivider;
    if (drawable != nullptr) {
        // If we have a horizontal divider to draw, draw it at the remembered positions
        const auto& rects = mHorizontalDividerRects;
        for (int i = rects.size() - 1; i >= 0; i--) {
            drawable->setBounds(rects.at(i));
            drawable->draw(canvas);
        }
    }

    drawable = mVerticalDivider;
    if (drawable != nullptr) {
        // If we have a vertical divider to draw, draw it at the remembered positions
        const auto & rects = mVerticalDividerRects;
        for (int i = rects.size() - 1; i >= 0; i--) {
            drawable->setBounds(rects.at(i));
            drawable->draw(canvas);
        }
    }
}

bool IconMenuView::invokeItem(MenuItemImpl& item) {
    return mMenu->performItemAction((MenuItem*)&item, 0);
}

ViewGroup::LayoutParams* IconMenuView::generateLayoutParams(const AttributeSet& attrs) const{
    return new IconMenuView::LayoutParams(getContext(), attrs);
}

bool IconMenuView::checkLayoutParams(const ViewGroup::LayoutParams* p) const{
    // Override to allow type-checking of LayoutParams.
    return dynamic_cast<const IconMenuView::LayoutParams*>(p)!=nullptr;
}

void IconMenuView::markStaleChildren() {
    if (!mHasStaleChildren) {
        mHasStaleChildren = true;
        requestLayout();
    }
}

int IconMenuView::getNumActualItemsShown() {
    return mNumActualItemsShown;
}

void IconMenuView::setNumActualItemsShown(int count) {
    mNumActualItemsShown = count;
}

int IconMenuView::getWindowAnimations() {
    return mAnimations;
}

std::vector<int> IconMenuView::getLayout() {
    return mLayout;
}

int IconMenuView::getLayoutNumRows() const{
    return mLayoutNumRows;
}

bool IconMenuView::dispatchKeyEvent(KeyEvent& event) {

    if (event.getKeyCode() == KeyEvent::KEYCODE_MENU) {
        if (event.getAction() == KeyEvent::ACTION_DOWN && event.getRepeatCount() == 0) {
            removeCallbacks(mRunnable);
            postDelayed(mRunnable, ViewConfiguration::getLongPressTimeout());
        } else if (event.getAction() == KeyEvent::ACTION_UP) {

            if (mMenuBeingLongpressed) {
                // It was in cycle mode, so reset it (will also remove us
                // from being called back)
                setCycleShortcutCaptionMode(false);
                return true;

            } else {
                // Just remove us from being called back
                removeCallbacks(mRunnable);
                // Fall through to normal processing too
            }
        }
    }

    return ViewGroup::dispatchKeyEvent(event);
}

void IconMenuView::onAttachedToWindow() {
    ViewGroup::onAttachedToWindow();
    requestFocus();
}

void IconMenuView::onDetachedFromWindow() {
    setCycleShortcutCaptionMode(false);
    ViewGroup::onDetachedFromWindow();
}

void IconMenuView::onWindowFocusChanged(bool hasWindowFocus) {
    if (!hasWindowFocus) {
        setCycleShortcutCaptionMode(false);
    }
    ViewGroup::onWindowFocusChanged(hasWindowFocus);
}

void IconMenuView::setCycleShortcutCaptionMode(bool cycleShortcutAndNormal) {
    if (!cycleShortcutAndNormal) {
        removeCallbacks(mRunnable);
        setChildrenCaptionMode(false);
        mMenuBeingLongpressed = false;
    } else {
        setChildrenCaptionMode(true);
    }
}

void IconMenuView::run() {
    if (mMenuBeingLongpressed) {
        // Cycle to other caption mode on the children
        setChildrenCaptionMode(!mLastChildrenCaptionMode);
    } else {
        // Switch ourselves to continuously cycle the items captions
        mMenuBeingLongpressed = true;
        setCycleShortcutCaptionMode(true);
    }

    // We should run again soon to cycle to the other caption mode
    postDelayed(mRunnable, ITEM_CAPTION_CYCLE_DELAY);
}

void IconMenuView::setChildrenCaptionMode(bool shortcut) {
    // Set the last caption mode pushed to children
    mLastChildrenCaptionMode = shortcut;
    for (int i = getChildCount() - 1; i >= 0; i--) {
        ((IconMenuItemView*) getChildAt(i))->setCaptionMode(shortcut);
    }
}

void IconMenuView::calculateItemFittingMetadata(int width) {
    const int maxNumItemsPerRow = mMaxItemsPerRow;
    const int numItems = getChildCount();
    for (int i = 0; i < numItems; i++) {
        LayoutParams* lp = (LayoutParams*) getChildAt(i)->getLayoutParams();
        // Start with 1, since that case does not get covered in the loop below
        lp->maxNumItemsOnRow = 1;
        for (int curNumItemsPerRow = maxNumItemsPerRow; curNumItemsPerRow > 0;
                curNumItemsPerRow--) {
            // Check whether this item can fit into a row containing curNumItemsPerRow
            if (lp->desiredWidth < width / curNumItemsPerRow) {
                // It can, mark this value as the most dense row it can fit into
                lp->maxNumItemsOnRow = curNumItemsPerRow;
                break;
            }
        }
    }
}

Parcelable* IconMenuView::onSaveInstanceState() {
    Parcelable* superState = ViewGroup::onSaveInstanceState();
    View* focusedView = getFocusedChild();
    for (int i = getChildCount() - 1; i >= 0; i--) {
        if (getChildAt(i) == focusedView) {
            return new SavedState(superState, i);
        }
    }
    return new SavedState(superState, -1);
}

void IconMenuView::onRestoreInstanceState(Parcelable& state) {
    SavedState& ss = (SavedState&) state;
    ViewGroup::onRestoreInstanceState(*ss.getSuperState());
    if (ss.focusedPosition >= getChildCount()) {
        return;
    }
    View* v = getChildAt(ss.focusedPosition);
    if (v != nullptr) {
        v->requestFocus();
    }
}

//////////////////////////////////////////////////////////////////////
IconMenuView::SavedState::SavedState(Parcelable* superState, int focusedPosition)
    :BaseSavedState(superState){
    this->focusedPosition = focusedPosition;
}

IconMenuView::SavedState::SavedState(Parcel& in)
    :BaseSavedState(in){
    focusedPosition = in.readInt();
}

void IconMenuView::SavedState::writeToParcel(Parcel& dest, int flags) {
    BaseSavedState::writeToParcel(dest, flags);
    dest.writeInt(focusedPosition);
}


IconMenuView::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs)
    :ViewGroup::MarginLayoutParams(c, attrs){
}

IconMenuView::LayoutParams::LayoutParams(int width, int height)
    :ViewGroup::MarginLayoutParams(width, height){
}
}/*endof namespace*/
