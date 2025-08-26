#include <widget/menupopupwindow.h>
#include <menu/menubuilder.h>
#include <menu/menuitemimpl.h>
#include <menu/listmenuitemview.h>
#include <menu/listmenupresenter.h>
namespace cdroid{

MenuPopupWindow::MenuPopupWindow(Context* context,const AttributeSet& attrs,
        const std::string&defStyleAttr,const std::string&defStyleRes)
    :ListPopupWindow(context, attrs,defStyleAttr,defStyleRes){
}

DropDownListView* MenuPopupWindow::createDropDownListView(Context* context, bool hijackFocus){
    MenuDropDownListView* view = new MenuDropDownListView(context, hijackFocus);
    //view->setHoverListener(this);
    return view;
}

void MenuPopupWindow::setEnterTransition(Transition* enterTransition) {
    mPopup->setEnterTransition(enterTransition);
}

void MenuPopupWindow::setExitTransition(Transition* exitTransition) {
    mPopup->setExitTransition(exitTransition);
}

void MenuPopupWindow::setHoverListener(const MenuItemHoverListener& hoverListener) {
    mHoverListener = hoverListener;
}

void MenuPopupWindow::setTouchModal(bool touchModal) {
    mPopup->setTouchModal(touchModal);
}

void MenuPopupWindow::onItemHoverEnter(MenuBuilder& menu,MenuItem& item) {
    // Forward up the chain
    if (mHoverListener.onItemHoverEnter != nullptr) {
        mHoverListener.onItemHoverEnter(menu, item);
    }
}

void MenuPopupWindow::onItemHoverExit(MenuBuilder& menu, MenuItem& item) {
    // Forward up the chain
    if (mHoverListener.onItemHoverExit != nullptr) {
        mHoverListener.onItemHoverExit(menu, item);
    }
}

/////////////////////////////////////////////////////////////////////////////

MenuPopupWindow::MenuDropDownListView::MenuDropDownListView(Context* context, bool hijackFocus)
    :DropDownListView(context, hijackFocus){

    /*Configuration config = getConfiguration();
    if (config.getLayoutDirection() == View::LAYOUT_DIRECTION_RTL) {
        mAdvanceKey = KeyEvent::KEYCODE_DPAD_LEFT;
        mRetreatKey = KeyEvent::KEYCODE_DPAD_RIGHT;
    } else */{
        mAdvanceKey = KeyEvent::KEYCODE_DPAD_RIGHT;
        mRetreatKey = KeyEvent::KEYCODE_DPAD_LEFT;
    }
}

void MenuPopupWindow::MenuDropDownListView::setHoverListener(const MenuItemHoverListener& hoverListener) {
    mHoverListener = hoverListener;
}

void MenuPopupWindow::MenuDropDownListView::clearSelection() {
    setSelectedPositionInt(INVALID_POSITION);
    setNextSelectedPositionInt(INVALID_POSITION);
}

bool MenuPopupWindow::MenuDropDownListView::onKeyDown(int keyCode, KeyEvent& event){
    ListMenuItemView* selectedItem = (ListMenuItemView*) getSelectedView();
    if (selectedItem != nullptr && keyCode == mAdvanceKey) {
        if (selectedItem->isEnabled() && selectedItem->getItemData()->hasSubMenu()) {
            performItemClick(*selectedItem,getSelectedItemPosition(),getSelectedItemId());
        }
        return true;
    } else if (selectedItem != nullptr && keyCode == mRetreatKey) {
        setSelectedPositionInt(INVALID_POSITION);
        setNextSelectedPositionInt(INVALID_POSITION);

        // Close only the top-level menu.
        ((ListMenuPresenter::MenuAdapter*) getAdapter())->getAdapterMenu()->close(false /* closeAllMenus */);
        return true;
    }
    return DropDownListView::onKeyDown(keyCode, event);
}

bool MenuPopupWindow::MenuDropDownListView::onHoverEvent(MotionEvent& ev){
    // Dispatch any changes in hovered item index to the listener.
    if (mHoverListener.onItemHoverExit != nullptr||mHoverListener.onItemHoverEnter!=nullptr) {
        // The adapter may be wrapped. Adjust the index if necessary.
        int headersCount;
        ListMenuPresenter::MenuAdapter* menuAdapter;
        ListAdapter* adapter = getAdapter();
        if (dynamic_cast<HeaderViewListAdapter*>(adapter)) {
            HeaderViewListAdapter* headerAdapter = (HeaderViewListAdapter*) adapter;
            headersCount = headerAdapter->getHeadersCount();
            menuAdapter = (ListMenuPresenter::MenuAdapter*) headerAdapter->getWrappedAdapter();
        } else {
            headersCount = 0;
            menuAdapter = (ListMenuPresenter::MenuAdapter*) adapter;
        }

        // Find the menu item for the view at the event coordinates.
        MenuItem* menuItem = nullptr;
        if (ev.getAction() != MotionEvent::ACTION_HOVER_EXIT) {
            const int position = pointToPosition((int) ev.getX(), (int) ev.getY());
            if (position != INVALID_POSITION) {
                const int itemPosition = position - headersCount;
                if (itemPosition >= 0 && itemPosition < menuAdapter->getCount()) {
                    menuItem = (MenuItem*)menuAdapter->getItem(itemPosition);
                }
            }
        }

        MenuItem* oldMenuItem = mHoveredMenuItem;
        if (oldMenuItem != menuItem) {
            MenuBuilder* menu = menuAdapter->getAdapterMenu();
            if (oldMenuItem != nullptr) {
                mHoverListener.onItemHoverExit(*menu, *oldMenuItem);
            }
            mHoveredMenuItem = menuItem;
            if (menuItem != nullptr) {
                mHoverListener.onItemHoverEnter(*menu, *menuItem);
            }
        }
    }
    return DropDownListView::onHoverEvent(ev);
}

}
