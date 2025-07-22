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
