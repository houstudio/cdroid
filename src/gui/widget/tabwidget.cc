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
#include <widget/tabwidget.h>
#include <cdlog.h>

namespace cdroid{

DECLARE_WIDGET2(TabWidget,"cdroid:attr/tabWidgetStyle")

TabWidget::TabWidget(int w,int h):LinearLayout(w,h){
    initTab();
}

TabWidget::TabWidget(Context*ctx,const AttributeSet&atts)
  :LinearLayout(ctx,atts){
    initTab();
    const bool hasExplicitLeft = atts.hasAttribute("tabStripLeft");
    if(hasExplicitLeft)
        mLeftStrip = atts.getDrawable("tabStripLeft");
    else
        mLeftStrip = atts.getDrawable("tab_bottom_left");

    const bool hasExplicitRight = atts.hasAttribute("tabStripRight");
    if(hasExplicitRight)
        mRightStrip = atts.getDrawable("tabStripRight");
    else
        mRightStrip = atts.getDrawable("tab_bottom_right");
    setChildrenDrawingOrderEnabled(true);
}

TabWidget::~TabWidget(){
    delete mLeftStrip;
    delete mRightStrip;
}

void TabWidget::initTab(){
    mSelectedTab = -1;
    mDrawBottomStrips = true;
    mImposedTabsHeight= -1;
    mLeftStrip = nullptr;
    mRightStrip= nullptr;
    mSelectionChangedListener=nullptr;
}

void TabWidget::onSizeChanged(int w, int h, int oldw, int oldh){
    mStripMoved = true;
    LinearLayout::onSizeChanged(w, h, oldw, oldh);
}

int TabWidget::getChildDrawingOrder(int childCount, int i){
    if (mSelectedTab == -1) {
        return i;
    } else {
        // Always draw the selected tab last, so that drop shadows are drawn
        // in the correct z-order.
        if (i == childCount - 1) {
            return mSelectedTab;
        } else if (i >= mSelectedTab) {
            return i + 1;
        } else {
            return i;
        }
    }
}

void TabWidget::measureChildBeforeLayout(View* child, int childIndex, int widthMeasureSpec, int totalWidth,
        int heightMeasureSpec, int totalHeight) {
    if (!isMeasureWithLargestChildEnabled() && mImposedTabsHeight >= 0) {
        widthMeasureSpec = MeasureSpec::makeMeasureSpec(
            totalWidth + mImposedTabWidths[childIndex], MeasureSpec::EXACTLY);
        heightMeasureSpec= MeasureSpec::makeMeasureSpec(mImposedTabsHeight,MeasureSpec::EXACTLY);
    }
    LinearLayout::measureChildBeforeLayout(child, childIndex,
                widthMeasureSpec, totalWidth, heightMeasureSpec, totalHeight);
}

void TabWidget::measureHorizontal(int widthMeasureSpec, int heightMeasureSpec) {
    if (MeasureSpec::getMode(widthMeasureSpec) == MeasureSpec::UNSPECIFIED) {
        LinearLayout::measureHorizontal(widthMeasureSpec, heightMeasureSpec);
        return;
    }

    // First, measure with no constraint
    int width = MeasureSpec::getSize(widthMeasureSpec);
    int unspecifiedWidth = MeasureSpec::makeSafeMeasureSpec(width,MeasureSpec::UNSPECIFIED);
    mImposedTabsHeight = -1;
    LinearLayout::measureHorizontal(unspecifiedWidth, heightMeasureSpec);

    int extraWidth = getMeasuredWidth() - width;
    if (extraWidth > 0) {
        int count = getChildCount();

        int childCount = 0;
        for (int i = 0; i < count; i++) {
            View* child = getChildAt(i);
            if (child->getVisibility() == GONE) continue;
            childCount++;
        }

        if (childCount > 0) {
            if (mImposedTabWidths.size() != count) {
                mImposedTabWidths.resize(count);
            }
            for (int i = 0; i < count; i++) {
                View* child = getChildAt(i);
                if (child->getVisibility() == GONE) continue;
                int childWidth = child->getMeasuredWidth();
                int delta = extraWidth / childCount;
                int newWidth = std::max(0, childWidth - delta);
                mImposedTabWidths[i] = newWidth;
                // Make sure the extra width is evenly distributed, no int division remainder
                extraWidth -= childWidth - newWidth; // delta may have been clamped
                childCount--;
                mImposedTabsHeight = std::max(mImposedTabsHeight, child->getMeasuredHeight());
            }
        }
    }

    // Measure again, this time with imposed tab widths and respecting
    // initial spec request.
    LinearLayout::measureHorizontal(widthMeasureSpec, heightMeasureSpec);
}

View* TabWidget::getChildTabViewAt(int index) {
    return getChildAt(index);
}

int TabWidget::getTabCount()const {
    return getChildCount();
}

void TabWidget::setDividerDrawable(Drawable* drawable) {
    LinearLayout::setDividerDrawable(drawable);
}

void TabWidget::setDividerDrawable(const std::string& resId) {
    setDividerDrawable(mContext->getDrawable(resId));
}
void TabWidget::setLeftStripDrawable(Drawable* drawable) {
    mLeftStrip = drawable;
    requestLayout();
    invalidate();
}
void TabWidget::setLeftStripDrawable(const std::string&resId) {
    setLeftStripDrawable(mContext->getDrawable(resId));
}

Drawable* TabWidget::getLeftStripDrawable() {
    return mLeftStrip;
}

void TabWidget::setRightStripDrawable(Drawable* drawable) {
    mRightStrip = drawable;
    requestLayout();
    invalidate();
}

void TabWidget::setRightStripDrawable(const std::string& resId) {
    setRightStripDrawable(mContext->getDrawable(resId));
}

Drawable* TabWidget::getRightStripDrawable() {
    return mRightStrip;
}
void TabWidget::setStripEnabled(bool stripEnabled) {
    mDrawBottomStrips = stripEnabled;
    invalidate();
}

bool TabWidget::isStripEnabled()const{
    return mDrawBottomStrips;
}

void TabWidget::childDrawableStateChanged(View* child) {
    if (getTabCount() > 0 && child == getChildTabViewAt(mSelectedTab)) {
        // To make sure that the bottom strip is redrawn
        invalidate();
    }
    LinearLayout::childDrawableStateChanged(child);
}

void TabWidget::dispatchDraw(Canvas& canvas){
    LinearLayout::dispatchDraw(canvas);

    // Do nothing if there are no tabs.
    if (getTabCount() == 0||mSelectedTab<0) return;

    // If the user specified a custom view for the tab indicators, then
    // do not draw the bottom strips.
    if (!mDrawBottomStrips) {
        // Skip drawing the bottom strips.
        return;
    }

    View* selectedChild = getChildTabViewAt(mSelectedTab);

    if(mLeftStrip)
        mLeftStrip->setState(selectedChild->getDrawableState());
    if(mRightStrip)
        mRightStrip->setState(selectedChild->getDrawableState());

    if (mStripMoved) {
        Rect bounds = mBounds;
        bounds.left = selectedChild->getLeft();
        bounds.width = selectedChild->getWidth();
        int myHeight = getHeight();
        if(mLeftStrip)
            mLeftStrip->setBounds(std::min(0, bounds.left - mLeftStrip->getIntrinsicWidth()),
                myHeight - mLeftStrip->getIntrinsicHeight(), bounds.left, myHeight);
        if(mRightStrip)mRightStrip->setBounds(bounds.right(), myHeight - mRightStrip->getIntrinsicHeight(),
                std::max(getWidth(), bounds.width + mRightStrip->getIntrinsicWidth()), myHeight);
        mStripMoved = false;
    }
    if(mLeftStrip )mLeftStrip->draw(canvas);
    if(mRightStrip)mRightStrip->draw(canvas);    
}

void TabWidget::setCurrentTab(int index) {
    if (index < 0 || index >= getTabCount() || index == mSelectedTab) {
        return;
    }

    if (mSelectedTab != -1) {
        getChildTabViewAt(mSelectedTab)->setSelected(false);
    }
    mSelectedTab = index;
    getChildTabViewAt(mSelectedTab)->setSelected(true);
    mStripMoved = true;
}
void TabWidget::focusCurrentTab(int index) {
    int oldTab = mSelectedTab;

    // set the tab
    setCurrentTab(index);

    // change the focus if applicable.
    if (oldTab != index) {
        getChildTabViewAt(index)->requestFocus();
    }
}

void TabWidget::setEnabled(bool enabled){
    LinearLayout::setEnabled(enabled);

    int count = getTabCount();
    for (int i = 0; i < count; i++) {
        View* child = getChildTabViewAt(i);
        child->setEnabled(enabled);
    }
}

void TabWidget::onClickChild(View&v,int idx){
    if(mSelectionChangedListener)
        mSelectionChangedListener(idx,true);
}

void TabWidget::addView(View* child) {
    if (child->getLayoutParams() == nullptr) {
        LinearLayout::LayoutParams* lp = new LayoutParams(0, LayoutParams::MATCH_PARENT, 1.0f);
        lp->setMargins(0, 0, 0, 0);
        child->setLayoutParams(lp);
    }

    // Ensure you can navigate to the tab with the keyboard, and you can touch it
    child->setFocusable(true);
    child->setClickable(true);

    if (child->getPointerIcon() == nullptr) {
        child->setPointerIcon(PointerIcon::getSystemIcon(getContext(), PointerIcon::TYPE_HAND));
    }

    LinearLayout::addView(child);

    // TODO: detect this via geometry with a tabwidget listener rather
    // than potentially interfere with the view's listener
    child->setOnClickListener(std::bind(&TabWidget::onClickChild,this,std::placeholders::_1,getTabCount() - 1));
}

void TabWidget::removeAllViews() {
    LinearLayout::removeAllViews();
    mSelectedTab = -1;
}

PointerIcon* TabWidget::onResolvePointerIcon(MotionEvent& event, int pointerIndex) {
    if (!isEnabled()) {
        return nullptr;
    }
    return LinearLayout::onResolvePointerIcon(event, pointerIndex);
}

void TabWidget::setTabSelectionListener(const OnTabSelectionChanged& listener) {
    mSelectionChangedListener = listener;
}

void TabWidget::onFocusChange(View* v, bool hasFocus) {
    // No-op. Tab selection is separate from keyboard focus.
}

}
