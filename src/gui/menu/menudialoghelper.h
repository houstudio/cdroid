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
#ifndef __MENU_DIALOG_HELPER_H__
#define __MENU_DIALOG_HELPER_H__
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <menu/menupresenter.h>
namespace cdroid{
class ListMenuPresenter;
class MenuDialogHelper{/*public MenuHelper, DialogInterface.OnKeyListener,
        DialogInterface::OnClickListener, DialogInterface.OnDismissListener,
        MenuPresenter::Callback */
private:
    MenuBuilder* mMenu;
    AlertDialog* mDialog;
    MenuPresenter::Callback mPresenterCallback;
    ListMenuPresenter* mPresenter;
public:
    MenuDialogHelper(MenuBuilder* menu);

    void show();

    bool onKey(DialogInterface& dialog, int keyCode, KeyEvent& event);

    void setPresenterCallback(const MenuPresenter::Callback& cb);

    void dismiss();

    void onDismiss(DialogInterface& dialog);

    void onCloseMenu(MenuBuilder& menu, bool allMenusAreClosing);

    bool onOpenSubMenu(MenuBuilder& subMenu);

    void onClick(DialogInterface& dialog, int which);
};
}/*endof namespace*/
#endif/*__MENU_DIALOG_HELPER_H__*/
