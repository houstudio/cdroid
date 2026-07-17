/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
 *
 * Line-by-line port of AOSP android-36
 *   com.android.internal.widget.floatingtoolbar.FloatingToolbar.
 *********************************************************************************/
#include <widget/floatingtoolbar.h>
#include <widget/floatingtoolbarpopup.h>
#include <widget/cdwindow.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <menu/submenu.h>
#include <widget/R.h>
#include <algorithm>

namespace cdroid{

FloatingToolbar::FloatingToolbar(Window* window)
    : mWindow(window)
    , mPopup(FloatingToolbarPopup::createInstance(window->getContext(), window->getRootView()))
    , mMenuItemClickListener([](MenuItem&){ return false; }) {   // NO_OP_MENUITEM_CLICK_LISTENER
    mOrientationChangeHandler = [this](View&, int nl, int nt, int nr, int nb,
                                              int ol, int ot, int orr, int ob) {
        // On orientation/size change, if the popup is showing and the layout changed,
        // mark width changed and re-layout.
        if (mPopup->isShowing() && (nl != ol || nt != ot || nr != orr || nb != ob)) {
            mPopup->setWidthChanged(true);
            updateLayout();
        }
    };
}

FloatingToolbar::~FloatingToolbar() {
    // AOSP relies on GC; in C++ the toolbar owns its popup. CDROID's FloatingActionMode
    // creates a fresh toolbar per ActionMode invocation, so this dtor must free the popup
    // (and its PopupWindow + view tree) on every finish() -- otherwise each invocation leaks
    // a PopupWindow + views, eventually corrupting state and crashing the next popup.
    // The layout-change listener is already removed in dismiss(); not touched here (the root
    // view may be tearing down at Window destruction).
    delete mPopup;
    mPopup = nullptr;
}

FloatingToolbar& FloatingToolbar::setMenu(Menu* menu) {
    mMenu = menu;
    return *this;
}

FloatingToolbar& FloatingToolbar::setOnMenuItemClickListener(
        const MenuItem::OnMenuItemClickListener& menuItemClickListener) {
    if (menuItemClickListener) {
        mMenuItemClickListener = menuItemClickListener;
    } else {
        mMenuItemClickListener = [](MenuItem&){ return false; };   // NO_OP
    }
    return *this;
}

FloatingToolbar& FloatingToolbar::setContentRect(const Rect& rect) {
    mContentRect = rect;
    return *this;
}

FloatingToolbar& FloatingToolbar::setSuggestedWidth(int suggestedWidth) {
    mPopup->setSuggestedWidth(suggestedWidth);
    return *this;
}

FloatingToolbar& FloatingToolbar::show() {
    registerOrientationHandler();
    doShow();
    return *this;
}

FloatingToolbar& FloatingToolbar::updateLayout() {
    if (mPopup->isShowing()) {
        doShow();
    }
    return *this;
}

void FloatingToolbar::dismiss() {
    unregisterOrientationHandler();
    mPopup->dismiss();
}

void FloatingToolbar::hide() {
    mPopup->hide();
}

bool FloatingToolbar::isShowing() {
    return mPopup->isShowing();
}

bool FloatingToolbar::isHidden() {
    return mPopup->isHidden();
}

void FloatingToolbar::setOutsideTouchable(bool outsideTouchable,
                                          const std::function<void()>& onDismiss) {
    mPopup->setOutsideTouchable(outsideTouchable, onDismiss);
}

void FloatingToolbar::doShow() {
    std::vector<MenuItem*> menuItems = getVisibleAndEnabledMenuItems(mMenu);
    // mMenuItemComparator: keep textAssist first; then requiresActionButton < requiresOverflow;
    // then by getOrder(). AOSP Comparator<MenuItem> is tri-state; converted to less-than here.
    auto tristate = [](MenuItem* a, MenuItem* b) -> int {
        if (a->getItemId() == R::id::textAssist)
            return (b->getItemId() == R::id::textAssist) ? 0 : -1;
        if (b->getItemId() == R::id::textAssist)
            return 1;
        if (a->requiresActionButton())
            return b->requiresActionButton() ? 0 : -1;
        if (b->requiresActionButton())
            return 1;
        if (a->requiresOverflow())
            return b->requiresOverflow() ? 0 : 1;
        if (b->requiresOverflow())
            return -1;
        return a->getOrder() - b->getOrder();
    };
    std::sort(menuItems.begin(), menuItems.end(),
              [&](MenuItem* a, MenuItem* b){ return tristate(a, b) < 0; });
    mPopup->show(menuItems, mMenuItemClickListener, mContentRect);
}

std::vector<MenuItem*> FloatingToolbar::getVisibleAndEnabledMenuItems(Menu* menu) {
    std::vector<MenuItem*> menuItems;
    if (menu != nullptr) {
        for (int i = 0; i < menu->size(); i++) {
            MenuItem* menuItem = menu->getItem(i);
            if (menuItem->isVisible() && menuItem->isEnabled()) {
                SubMenu* subMenu = menuItem->getSubMenu();
                if (subMenu != nullptr) {
                    std::vector<MenuItem*> sub = getVisibleAndEnabledMenuItems(subMenu);
                    menuItems.insert(menuItems.end(), sub.begin(), sub.end());
                } else {
                    menuItems.push_back(menuItem);
                }
            }
        }
    }
    return menuItems;
}

void FloatingToolbar::registerOrientationHandler() {
    unregisterOrientationHandler();
    mWindow->getRootView()->addOnLayoutChangeListener(mOrientationChangeHandler);
}

void FloatingToolbar::unregisterOrientationHandler() {
    mWindow->getRootView()->removeOnLayoutChangeListener(mOrientationChangeHandler);
}

}//namespace cdroid
