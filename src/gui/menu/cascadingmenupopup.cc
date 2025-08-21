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
#include <widget/R.h>
#include <menu/menuadapter.h>
#include <menu/cascadingmenupopup.h>
#include <menu/submenubuilder.h>
#include <widget/menupopupwindow.h>
namespace cdroid{
//class CascadingMenuPopup extends MenuPopup implements MenuPresenter, OnKeyListener,PopupWindow.OnDismissListener {
//private final OnGlobalLayoutListener mGlobalLayoutListener = new OnGlobalLayoutListener() {
void CascadingMenuPopup::onGlobalLayout() {
    // Only move the popup if it's showing and non-modal. We don't want
    // to be moving around the only interactive window, since there's a
    // good chance the user is interacting with it.
    if (isShowing() && mShowingMenus.size() > 0
            && !mShowingMenus.at(0)->window->isModal()) {
        View* anchor = mShownAnchorView;
        if (anchor == nullptr || !anchor->isShown()) {
            dismiss();
        } else {
            // Recompute window sizes and positions.
            for (CascadingMenuInfo* info : mShowingMenus) {
                info->window->show();
            }
        }
    }
}

//OnAttachStateChangeListener mAttachStateChangeListener =new OnAttachStateChangeListener() {
void CascadingMenuPopup::onViewAttachedToWindow(View* v) {
}

void CascadingMenuPopup::onViewDetachedFromWindow(View* v) {
    if (mTreeObserver != nullptr) {
        if (!mTreeObserver->isAlive()) {
            mTreeObserver = v->getViewTreeObserver();
        }
        mTreeObserver->removeGlobalOnLayoutListener(mGlobalLayoutListener);
    }
    v->removeOnAttachStateChangeListener(mAttachStateChangeListener);
}

//MenuItemHoverListener mMenuItemHoverListener = new MenuItemHoverListener() {
void CascadingMenuPopup::onItemHoverExit(MenuBuilder& menu,MenuItem& item) {
    // If the mouse moves between two windows, hover enter/exit pairs
    // may be received out of order. So, instead of canceling all
    // pending runnables, only cancel runnables for the host menu.
    mSubMenuHoverHandler->removeCallbacksAndMessages(&menu);
}

void CascadingMenuPopup::onItemHoverEnter(MenuBuilder& menu,MenuItem& item) {
    // Something new was hovered, cancel all scheduled runnables.
    mSubMenuHoverHandler->removeCallbacksAndMessages(nullptr);

    // Find the position of the hovered menu within the added menus.
    int menuIndex = -1;
    for (int i = 0, count = mShowingMenus.size(); i < count; i++) {
        if (&menu == mShowingMenus.at(i)->menu) {
            menuIndex = i;
            break;
        }
    }

    if (menuIndex == -1) {
        return;
    }

    CascadingMenuInfo* nextInfo;
    const int nextIndex = menuIndex + 1;
    if (nextIndex < mShowingMenus.size()) {
        nextInfo = mShowingMenus.at(nextIndex);
    } else {
        nextInfo = nullptr;
    }

    Runnable runnable([&menu,&item,nextInfo,this](){
        // Close any other submenus that might be open at the
        // current or a deeper level.
        if (nextInfo != nullptr) {
            // Disable exit animations to prevent overlapping
            // fading out submenus.
            mShouldCloseImmediately = true;
            nextInfo->menu->close(false /* closeAllMenus */);
            mShouldCloseImmediately = false;
        }

        // Then open the selected submenu, if there is one.
        if (item.isEnabled() && item.hasSubMenu()) {
            menu.performItemAction(&item, 0);
        }
    });
    const int64_t uptimeMillis = SystemClock::uptimeMillis() + SUBMENU_TIMEOUT_MS;
    LOGE("TODO");//mSubMenuHoverHandler->postAtTime(runnable,menu, uptimeMillis);
}

CascadingMenuPopup::CascadingMenuPopup(Context* context, View* anchor,
        int popupStyleAttr, int popupStyleRes, bool overflowOnly) {
    mContext = context;//Preconditions.checkNotNull(context);
    mAnchorView = anchor;//Preconditions.checkNotNull(anchor);
    mPopupStyleAttr = popupStyleAttr;
    mPopupStyleRes = popupStyleRes;
    mOverflowOnly = overflowOnly;

    mForceShowIcon = false;
    mLastPosition = getInitialMenuPosition();

    //final Resources res = context.getResources();
    mMenuMaxWidth = std::max(context->getDisplayMetrics().widthPixels / 2,
            0);//res.getDimensionPixelSize(com.android.internal.R.dimen.config_prefDialogWidth));

    mSubMenuHoverHandler = new Handler();

    mItemLayout = "cdroid:layout/cascading_menu_item_layout_material";//com.android.internal.R.layout.cascading_menu_item_layout_material;
}

void CascadingMenuPopup::setForceShowIcon(bool forceShow) {
    mForceShowIcon = forceShow;
}

MenuPopupWindow* CascadingMenuPopup::createPopupWindow() {
    MenuPopupWindow* popupWindow = new MenuPopupWindow(mContext,AttributeSet(mContext,"cdroid"));// nullptr, mPopupStyleAttr, mPopupStyleRes);
    popupWindow->setHoverListener(mMenuItemHoverListener);
    popupWindow->setOnItemClickListener([this](AdapterView&parent, View& view, int position, long id){
        onItemClick(parent,view,position,id);
    });
    popupWindow->setOnDismissListener([this](){onDismiss();});
    popupWindow->setAnchorView(mAnchorView);
    popupWindow->setDropDownGravity(mDropDownGravity);
    popupWindow->setModal(true);
    popupWindow->setInputMethodMode(PopupWindow::INPUT_METHOD_NOT_NEEDED);
    return popupWindow;
}

void CascadingMenuPopup::show() {
    if (isShowing()) {
        return;
    }

    // Display all pending menus.
    for (MenuBuilder* menu : mPendingMenus) {
        showMenu(menu);
    }
    mPendingMenus.clear();

    mShownAnchorView = mAnchorView;

    if (mShownAnchorView != nullptr) {
        const bool addGlobalListener = mTreeObserver == nullptr;
        mTreeObserver = mShownAnchorView->getViewTreeObserver(); // Refresh to latest
        if (addGlobalListener) {
            mTreeObserver->addOnGlobalLayoutListener(mGlobalLayoutListener);
        }
        mShownAnchorView->addOnAttachStateChangeListener(mAttachStateChangeListener);
    }
}

void CascadingMenuPopup::dismiss() {
    // Need to make another list to avoid a concurrent modification
    // exception, as #onDismiss may clear mPopupWindows while we are
    // iterating. Remove from the last added menu so that the callbacks
    // are received in order from foreground to background.
    const int length = mShowingMenus.size();
    if (length > 0) {
        auto& addedMenus =mShowingMenus;// mShowingMenus.toArray(new CascadingMenuInfo[length]);
        for (int i = length - 1; i >= 0; i--) {
            CascadingMenuInfo* info = addedMenus[i];
            if (info->window->isShowing()) {
                info->window->dismiss();
            }
        }
    }
}

bool CascadingMenuPopup::onKey(View& v, int keyCode, KeyEvent& event) {
    if (event.getAction() == KeyEvent::ACTION_UP && keyCode == KeyEvent::KEYCODE_MENU) {
        dismiss();
        return true;
    }
    return false;
}

int CascadingMenuPopup::getInitialMenuPosition() {
    const int layoutDirection = mAnchorView->getLayoutDirection();
    return layoutDirection == View::LAYOUT_DIRECTION_RTL ? HORIZ_POSITION_LEFT :
            HORIZ_POSITION_RIGHT;
}

int CascadingMenuPopup::getNextMenuPosition(int nextMenuWidth) {
    ListView* lastListView = mShowingMenus.at(mShowingMenus.size() - 1)->getListView();

    int screenLocation[2];
    lastListView->getLocationOnScreen(screenLocation);

    Rect displayFrame;
    mShownAnchorView->getWindowVisibleDisplayFrame(displayFrame);

    if (mLastPosition == HORIZ_POSITION_RIGHT) {
        const int right = screenLocation[0] + lastListView->getWidth() + nextMenuWidth;
        if (right > displayFrame.right()) {
            return HORIZ_POSITION_LEFT;
        }
        return HORIZ_POSITION_RIGHT;
    } else { // LEFT
        const int left = screenLocation[0] - nextMenuWidth;
        if (left < 0) {
            return HORIZ_POSITION_RIGHT;
        }
        return HORIZ_POSITION_LEFT;
    }
}

void CascadingMenuPopup::addMenu(MenuBuilder* menu) {
    menu->addMenuPresenter(this, mContext);

    if (isShowing()) {
        showMenu(menu);
    } else {
        mPendingMenus.push_back(menu);
    }
}

void CascadingMenuPopup::showMenu(MenuBuilder* menu) {
    LayoutInflater* inflater = LayoutInflater::from(mContext);
    MenuAdapter* adapter = new MenuAdapter(menu, inflater, mOverflowOnly, mItemLayout);

    // Apply "force show icon" setting. There are 3 cases:
    // (1) This is the top level menu and icon spacing is forced. Add spacing.
    // (2) This is a submenu. Add spacing if any of the visible menu items has an icon.
    // (3) This is the top level menu and icon spacing isn't forced. Do not add spacing.
    if (!isShowing() && mForceShowIcon) {
      // Case 1
      adapter->setForceShowIcon(true);
    } else if (isShowing()) {
      // Case 2
      adapter->setForceShowIcon(MenuPopup::shouldPreserveIconSpacing(menu));
    }
    // Case 3: Else, don't allow spacing for icons (default behavior; do nothing).

    const int menuWidth = measureIndividualMenuWidth(adapter, nullptr, mContext, mMenuMaxWidth);
    MenuPopupWindow* popupWindow = createPopupWindow();
    popupWindow->setAdapter(adapter);
    popupWindow->setContentWidth(menuWidth);
    popupWindow->setDropDownGravity(mDropDownGravity);

    CascadingMenuInfo* parentInfo;
    View* parentView;
    if (mShowingMenus.size() > 0) {
        parentInfo = mShowingMenus.at(mShowingMenus.size() - 1);
        parentView = findParentViewForSubmenu(parentInfo, menu);
    } else {
        parentInfo = nullptr;
        parentView = nullptr;
    }

    if (parentView != nullptr) {
        // This menu is a cascading submenu anchored to a parent view.
        popupWindow->setAnchorView(parentView);
        popupWindow->setTouchModal(false);
        popupWindow->setEnterTransition(nullptr);

        const int nextMenuPosition = getNextMenuPosition(menuWidth);
        const bool showOnRight = nextMenuPosition == HORIZ_POSITION_RIGHT;
        mLastPosition = nextMenuPosition;

        // Compute the horizontal offset to display the submenu to the right or to the left
        // of the parent item.
        // By now, mDropDownGravity is the resolved absolute gravity, so
        // this should work in both LTR and RTL.
        int x;
        if ((mDropDownGravity & Gravity::RIGHT) == Gravity::RIGHT) {
            if (showOnRight) {
                x = menuWidth;
            } else {
                x = -parentView->getWidth();
            }
        } else {
            if (showOnRight) {
                x = parentView->getWidth();
            } else {
                x = -menuWidth;
            }
        }
        popupWindow->setHorizontalOffset(x);

        // Align with the top edge of the parent view (or the bottom edge when the submenu is
        // flipped vertically).
        popupWindow->setOverlapAnchor(true);
        popupWindow->setVerticalOffset(0);
    } else {
        if (mHasXOffset) {
            popupWindow->setHorizontalOffset(mXOffset);
        }
        if (mHasYOffset) {
            popupWindow->setVerticalOffset(mYOffset);
        }
        Rect epicenterBounds = getEpicenterBounds();
        popupWindow->setEpicenterBounds(epicenterBounds);
    }


    CascadingMenuInfo* menuInfo = new CascadingMenuInfo(popupWindow, menu, mLastPosition);
    mShowingMenus.push_back(menuInfo);

    popupWindow->show();

    ListView* listView = popupWindow->getListView();
    listView->setOnKeyListener([this](View&v,int keyCode,KeyEvent&event){
        return onKey(v,keyCode,event);
    });

    // If this is the root menu, show the title if requested.
    if (parentInfo == nullptr && mShowTitle && menu->getHeaderTitle().size()) {
        FrameLayout* titleItemView = (FrameLayout*) inflater->inflate(
            "cdroid:layout/popup_menu_header_item_layout", listView, false);
        TextView* titleView = (TextView*) titleItemView->findViewById(R::id::title);
        titleItemView->setEnabled(false);
        titleView->setText(menu->getHeaderTitle());
        listView->addHeaderView(titleItemView, nullptr, false);

        // Show again to update the title.
        popupWindow->show();
    }
}

MenuItem* CascadingMenuPopup::findMenuItemForSubmenu(MenuBuilder* parent, MenuBuilder* submenu) {
    for (int i = 0, count = parent->size(); i < count; i++) {
        MenuItem* item = parent->getItem(i);
        if (item->hasSubMenu() && submenu == (MenuBuilder*)item->getSubMenu()) {
            return item;
        }
    }

    return nullptr;
}

View* CascadingMenuPopup::findParentViewForSubmenu(CascadingMenuInfo* parentInfo, MenuBuilder* submenu) {
    MenuItem* owner = findMenuItemForSubmenu(parentInfo->menu, submenu);
    if (owner == nullptr) {
        // Couldn't find the submenu owner.
        return nullptr;
    }

    // The adapter may be wrapped. Adjust the index if necessary.
    int headersCount;
    MenuAdapter* menuAdapter;
    ListView* listView = parentInfo->getListView();
    ListAdapter* listAdapter = listView->getAdapter();
    if (dynamic_cast<HeaderViewListAdapter*>(listAdapter)) {
        HeaderViewListAdapter* headerAdapter = (HeaderViewListAdapter*) listAdapter;
        headersCount = headerAdapter->getHeadersCount();
        menuAdapter = (MenuAdapter*) headerAdapter->getWrappedAdapter();
    } else {
        headersCount = 0;
        menuAdapter = (MenuAdapter*) listAdapter;
    }

    // Find the index within the menu adapter's data set of the menu item.
    int ownerPosition = AbsListView::INVALID_POSITION;
    for (int i = 0, count = menuAdapter->getCount(); i < count; i++) {
        if (owner == menuAdapter->getItem(i)) {
            ownerPosition = i;
            break;
        }
    }
    if (ownerPosition == AbsListView::INVALID_POSITION) {
        // Couldn't find the owner within the menu adapter.
        return nullptr;
    }

    // Adjust the index for the adapter used to display views.
    ownerPosition += headersCount;

    // Adjust the index for the visible views.
    const int ownerViewPosition = ownerPosition - listView->getFirstVisiblePosition();
    if (ownerViewPosition < 0 || ownerViewPosition >= listView->getChildCount()) {
        // Not visible on screen.
        return nullptr;
    }

    return listView->getChildAt(ownerViewPosition);
}

bool CascadingMenuPopup::isShowing() {
    return mShowingMenus.size() > 0 && mShowingMenus.at(0)->window->isShowing();
}

void CascadingMenuPopup::onDismiss() {
    // The dismiss listener doesn't pass the calling window, so walk
    // through the stack to figure out which one was just dismissed.
    CascadingMenuInfo* dismissedInfo = nullptr;
    for (int i = 0, count = mShowingMenus.size(); i < count; i++) {
        CascadingMenuInfo* info = mShowingMenus.at(i);
        if (!info->window->isShowing()) {
            dismissedInfo = info;
            break;
        }
    }

    // Close all menus starting from the dismissed menu, passing false
    // since we are manually closing only a subset of windows.
    if (dismissedInfo != nullptr) {
        dismissedInfo->menu->close(false);
    }
}

void CascadingMenuPopup::onItemClick(AdapterView&parent, View& view, int position, long id){
}

void CascadingMenuPopup::updateMenuView(bool cleared) {
    for (CascadingMenuInfo* info : mShowingMenus) {
        toMenuAdapter(info->getListView()->getAdapter())->notifyDataSetChanged();
    }
}

void CascadingMenuPopup::setCallback(const Callback& cb) {
    mPresenterCallback = cb;
}

bool CascadingMenuPopup::onSubMenuSelected(SubMenuBuilder* subMenu) {
    // Don't allow double-opening of the same submenu.
    for (CascadingMenuInfo* info : mShowingMenus) {
        if (subMenu == info->menu) {
            // Just re-focus that one.
            info->getListView()->requestFocus();
            return true;
        }
    }

    if (subMenu->hasVisibleItems()) {
        addMenu(subMenu);

        if (mPresenterCallback.onOpenSubMenu != nullptr) {
            mPresenterCallback.onOpenSubMenu(*subMenu);
        }
        return true;
    }
    return false;
}

int CascadingMenuPopup::findIndexOfAddedMenu(MenuBuilder* menu) {
    for (int i = 0, count = mShowingMenus.size(); i < count; i++) {
        CascadingMenuInfo* info  = mShowingMenus.at(i);
        if (menu == info->menu) {
            return i;
        }
    }

    return -1;
}

void CascadingMenuPopup::onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing) {
    const int menuIndex = findIndexOfAddedMenu(menu);
    if (menuIndex < 0) {
        return;
    }

    // Recursively close descendant menus.
    const int nextMenuIndex = menuIndex + 1;
    if (nextMenuIndex < mShowingMenus.size()) {
        CascadingMenuInfo* childInfo = mShowingMenus.at(nextMenuIndex);
        childInfo->menu->close(false /* closeAllMenus */);
    }

    // Close the target menu.
    CascadingMenuInfo* info = mShowingMenus.at(menuIndex);
    mShowingMenus.erase(mShowingMenus.begin()+menuIndex);
    info->menu->removeMenuPresenter(this);
    if (mShouldCloseImmediately) {
        // Disable all exit animations.
        info->window->setExitTransition(nullptr);
        info->window->setAnimationStyle(0);
    }
    info->window->dismiss();

    const int count = mShowingMenus.size();
    if (count > 0) {
        mLastPosition = mShowingMenus.at(count - 1)->position;
    } else {
        mLastPosition = getInitialMenuPosition();
    }

    if (count == 0) {
        // This was the last window. Clean up.
        dismiss();

        if (mPresenterCallback.onCloseMenu != nullptr) {
            mPresenterCallback.onCloseMenu(*menu, true);
        }

        if (mTreeObserver != nullptr) {
            if (mTreeObserver->isAlive()) {
                mTreeObserver->removeGlobalOnLayoutListener(mGlobalLayoutListener);
            }
            mTreeObserver = nullptr;
        }
        mShownAnchorView->removeOnAttachStateChangeListener(mAttachStateChangeListener);

        // If every [sub]menu was dismissed, that means the whole thing was
        // dismissed, so notify the owner.
        mOnDismissListener();//.onDismiss();
    } else if (allMenusAreClosing) {
        // Close all menus starting from the root. This will recursively
        // close any remaining menus, so we don't need to propagate the
        // "closeAllMenus" flag. The last window will clean up.
        CascadingMenuInfo* rootInfo = mShowingMenus.at(0);
        rootInfo->menu->close(false /* closeAllMenus */);
    }
}

bool CascadingMenuPopup::flagActionItems() {
    return false;
}

Parcelable* CascadingMenuPopup::onSaveInstanceState() {
    return nullptr;
}

void CascadingMenuPopup::onRestoreInstanceState(Parcelable& state) {
}

void CascadingMenuPopup::setGravity(int dropDownGravity) {
    if (mRawDropDownGravity != dropDownGravity) {
        mRawDropDownGravity = dropDownGravity;
        mDropDownGravity = Gravity::getAbsoluteGravity(
                dropDownGravity, mAnchorView->getLayoutDirection());
    }
}

void CascadingMenuPopup::setAnchorView(View* anchor) {
    if (mAnchorView != anchor) {
        mAnchorView = anchor;

        // Gravity resolution may have changed, update from raw gravity.
        mDropDownGravity = Gravity::getAbsoluteGravity(
                mRawDropDownGravity, mAnchorView->getLayoutDirection());
    }
}

void CascadingMenuPopup::setOnDismissListener(const PopupWindow::OnDismissListener& listener) {
    mOnDismissListener = listener;
}

ListView* CascadingMenuPopup::getListView() {
    return mShowingMenus.empty() ? nullptr : mShowingMenus.at(mShowingMenus.size() - 1)->getListView();
}

void CascadingMenuPopup::setHorizontalOffset(int x) {
    mHasXOffset = true;
    mXOffset = x;
}

void CascadingMenuPopup::setVerticalOffset(int y) {
    mHasYOffset = true;
    mYOffset = y;
}

void CascadingMenuPopup::setShowTitle(bool showTitle) {
    mShowTitle = showTitle;
}

///private static class CascadingMenuInfo {
CascadingMenuPopup::CascadingMenuInfo::CascadingMenuInfo(MenuPopupWindow* window, MenuBuilder* menu,
        int position) {
    this->window = window;
    this->menu = menu;
    this->position = position;
}

ListView* CascadingMenuPopup::CascadingMenuInfo::getListView() {
    return window->getListView();
}
}/*endof namespace*/

