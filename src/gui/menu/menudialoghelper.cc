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
#include <menu/menudialoghelper.h>
#include <menu/listmenupresenter.h>
namespace cdroid{

MenuDialogHelper::MenuDialogHelper(MenuBuilder* menu) {
    mMenu = menu;
}

void MenuDialogHelper::show() {
    // Many references to mMenu, create local reference
    MenuBuilder* menu = mMenu;

    // Get the builder for the dialog
    AlertDialog::Builder* builder = new AlertDialog::Builder(menu->getContext());

    mPresenter = new ListMenuPresenter(builder->getContext(),"android:layout/list_menu_item_layout");

    MenuPresenter::Callback mpc;
    mpc.onCloseMenu=[this](MenuBuilder& menu, bool allMenusAreClosing){
        onCloseMenu(menu,allMenusAreClosing);
    };
    mpc.onOpenSubMenu =[this](MenuBuilder& menu){
        return onOpenSubMenu(menu);
    };
    mPresenter->setCallback(mpc);
    mMenu->addMenuPresenter(mPresenter);
    builder->setAdapter(mPresenter->getAdapter(),[this](DialogInterface& dialog, int which){onClick(dialog,which);});

    // Set the title
    View* headerView = menu->getHeaderView();
    if (headerView != nullptr) {
        // Menu's client has given a custom header view, use it
        builder->setCustomTitle(headerView);
    } else {
        // Otherwise use the (text) title and icon
        builder->setIcon(menu->getHeaderIcon()).setTitle(menu->getHeaderTitle());
    }

    // Set the key listener
    builder->setOnKeyListener([this](DialogInterface& dialog, int keyCode, KeyEvent& event){
            return onKey(dialog,keyCode,event);
        });

    // Show the menu
    mDialog = builder->create();
    mDialog->setOnDismissListener([this](DialogInterface& dialog){
            onDismiss(dialog);
        });
#if 0
    WindowManager.LayoutParams lp = mDialog.getWindow().getAttributes();
    lp.type = WindowManager.LayoutParams.TYPE_APPLICATION_ATTACHED_DIALOG;
    if (windowToken != null) {
        lp.token = windowToken;
    }
    lp.flags |= WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM;
#endif

    mDialog->show();
}

bool MenuDialogHelper::onKey(DialogInterface& dialog, int keyCode, KeyEvent& event) {
    if (keyCode == KeyEvent::KEYCODE_MENU || keyCode == KeyEvent::KEYCODE_BACK) {
        if (event.getAction() == KeyEvent::ACTION_DOWN
                && event.getRepeatCount() == 0) {
            Window* win = mDialog->getWindow();
            if (win != nullptr) {
                View* decor = win->getRootView();//win->getDecorView();
                if (decor != nullptr) {
                    KeyEvent::DispatcherState* ds = decor->getKeyDispatcherState();
                    if (ds != nullptr) {
                        ds->startTracking(event, this);
                        return true;
                    }
                }
            }
        } else if (event.getAction() == KeyEvent::ACTION_UP && !event.isCanceled()) {
            Window* win = mDialog->getWindow();
            if (win != nullptr) {
                View* decor = win->getRootView();//win->getDecorView();
                if (decor != nullptr) {
                    KeyEvent::DispatcherState* ds = decor->getKeyDispatcherState();
                    if (ds != nullptr && ds->isTracking(event)) {
                        mMenu->close(true /* closeAllMenus */);
                        dialog.dismiss();
                        return true;
                    }
                }
            }
        }
    }

    // Menu shortcut matching
    return mMenu->performShortcut(keyCode, event, 0);

}

void MenuDialogHelper::setPresenterCallback(const MenuPresenter::Callback& cb) {
    mPresenterCallback = cb;
}

void MenuDialogHelper::dismiss() {
    if (mDialog != nullptr) {
        mDialog->dismiss();
    }
}

void MenuDialogHelper::onDismiss(DialogInterface& dialog) {
    mPresenter->onCloseMenu(mMenu, true);
}

void MenuDialogHelper::onCloseMenu(MenuBuilder& menu, bool allMenusAreClosing) {
    if (allMenusAreClosing || &menu == mMenu) {
        dismiss();
    }
    if (mPresenterCallback.onCloseMenu != nullptr) {
        mPresenterCallback.onCloseMenu(menu, allMenusAreClosing);
    }
}

bool MenuDialogHelper::onOpenSubMenu(MenuBuilder& subMenu) {
    if (mPresenterCallback.onOpenSubMenu != nullptr) {
        return mPresenterCallback.onOpenSubMenu(subMenu);
    }
    return false;
}

void MenuDialogHelper::onClick(DialogInterface& dialog, int which) {
    mMenu->performItemAction((MenuItem*) mPresenter->getAdapter()->getItem(which), 0);
}
}
