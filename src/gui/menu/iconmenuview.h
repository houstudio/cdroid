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
#ifndef __ICON_MENU_VIEW_H__
#define __ICON_MENU_VIEW_H__
#include <view/viewgroup.h>
#include <menu/menuview.h>
namespace cdroid{
class MenuBuilder;
class IconMenuItemView;
class IconMenuView:public ViewGroup,public MenuView{// implements ItemInvoker,Runnable {
private:
    static constexpr int ITEM_CAPTION_CYCLE_DELAY = 1000;
    class SavedState;

    MenuBuilder* mMenu;

    /** Height of each row */
    int mRowHeight;
    /** Maximum number of rows to be shown */
    int mMaxRows;
    /** Maximum number of items to show in the icon menu. */
    int mMaxItems;
    /** Maximum number of items per row */
    int mMaxItemsPerRow;
    /** Actual number of items (the 'More' view does not count as an item) shown */
    int mNumActualItemsShown;

    Drawable* mHorizontalDivider;
    Drawable* mVerticalDivider;
    Runnable mRunnable;
    int mHorizontalDividerHeight;
    int mVerticalDividerWidth;
    std::vector<Rect> mHorizontalDividerRects;
    std::vector<Rect> mVerticalDividerRects;

    /** Icon for the 'More' button */
    Drawable* mMoreIcon;
    /** Background of each item (should contain the selected and focused states) */
    Drawable* mItemBackground;

    int mAnimations;
    int mLayoutNumRows;
    std::vector<int> mLayout;
    bool mHasStaleChildren;
    bool mMenuBeingLongpressed = false;
    bool mLastChildrenCaptionMode;
public:
    class LayoutParams;
private:
    void layoutItems(int width);
    void layoutItemsUsingGravity(int numRows, int numItems);
    bool doItemsFit();
    void positionChildren(int menuWidth, int menuHeight);
    void setCycleShortcutCaptionMode(bool cycleShortcutAndNormal);
    void setChildrenCaptionMode(bool shortcut);
    void calculateItemFittingMetadata(int width);
    void run();
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void onDraw(Canvas& canvas)override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    Parcelable* onSaveInstanceState()override;
    void onRestoreInstanceState(Parcelable& state)override;
public:
    IconMenuView(Context* context, const AttributeSet& attrs);
    int getMaxItems()const;

    Drawable* getItemBackgroundDrawable();

    IconMenuItemView* createMoreItemView();

    void initialize(MenuBuilder* menu);

    bool invokeItem(MenuItemImpl& item);

    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;

    void markStaleChildren();

    int getNumActualItemsShown();

    void setNumActualItemsShown(int count);

    int getWindowAnimations();
  
    std::vector<int> getLayout();

    int getLayoutNumRows()const;
    bool dispatchKeyEvent(KeyEvent& event)override;
    void onWindowFocusChanged(bool hasWindowFocus)override;
};

class IconMenuView::SavedState:public BaseSavedState {
public:
    int focusedPosition;
protected:
    SavedState(Parcel& in);
public:
    SavedState(Parcelable* superState, int focusedPosition);
    void writeToParcel(Parcel& dest, int flags)override;
};
class IconMenuView::LayoutParams:public ViewGroup::MarginLayoutParams{
protected:
    int left, top, right, bottom;
    int desiredWidth;
    int maxNumItemsOnRow;
    friend IconMenuView;
    friend IconMenuItemView;
public:
    LayoutParams(Context* c,const AttributeSet& attrs);
    LayoutParams(int width, int height);
};
}/*endof namespace*/
#endif/*__ICON_MENU_VIEW_H__*/
