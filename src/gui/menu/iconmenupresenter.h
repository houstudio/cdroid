#ifndef __ICON_MENUPRESENTER_H__
#define __ICON_MENUPRESENTER_H__
#include <menu/menuview.h>
#include <menu/basemenupresenter.h>
namespace cdroid{
class MenuDialogHelper;
class IconMenuPresenter:public BaseMenuPresenter {
private:
    static constexpr const char* VIEWS_TAG = "android:menu:icon";
    static constexpr const char* OPEN_SUBMENU_KEY = "android:menu:icon:submenu";
    IconMenuItemView* mMoreView;
    int mMaxItems = -1;
protected:
    int mOpenSubMenuId;
    MenuPresenter::Callback mSubMenuPresenterCallback;
    MenuDialogHelper* mOpenSubMenu;
protected:
    void addItemView(View* itemView, int childIndex)override;
    bool filterLeftoverView(ViewGroup* parent, int childIndex)override;
public:
    IconMenuPresenter(Context* context);

    void initForMenu(Context* context,MenuBuilder* menu)override;

    void bindItemView(MenuItemImpl* item, View* itemView)override;

    bool shouldIncludeItem(int childIndex, MenuItemImpl* item)override;

    bool onSubMenuSelected(SubMenuBuilder* subMenu)override;

    void updateMenuView(bool cleared)override;


    int getNumActualItemsShown();
    void saveHierarchyState(Bundle& outState);

    void restoreHierarchyState(Bundle& inState);

    Parcelable* onSaveInstanceState()override;

    void onRestoreInstanceState(Parcelable& state)override;

    /*class SubMenuPresenterCallback implements MenuPresenter.Callback {
    public:
        void onCloseMenu(MenuBuilder& menu, bool allMenusAreClosing) override{
            mOpenSubMenuId = 0;
            if (mOpenSubMenu != null) {
                mOpenSubMenu.dismiss();
                mOpenSubMenu = null;
            }
        }
        bool onOpenSubMenu(MenuBuilder& subMenu) override{
            if (subMenu != null) {
                mOpenSubMenuId = ((SubMenuBuilder) subMenu).getItem().getItemId();
            }
            return false;
        }

    };*/
};
}/*endof namespace*/
#endif/*__ICON_MENUPRESENTER_H__*/
