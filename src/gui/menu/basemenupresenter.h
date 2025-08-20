#ifndef __BASE_MENU_PRESENTER_H__
#define __BASE_MENU_PRESENTER_H__
#include <menu/menuview.h>
#include <menu/menupresenter.h>
namespace cdroid{
class BaseMenuPresenter:public MenuPresenter {
private:
    std::string mMenuLayoutRes;
    std::string mItemLayoutRes;
    int mId;
protected:
    Context* mSystemContext;
    Context* mContext;
    MenuBuilder* mMenu;
    LayoutInflater* mSystemInflater;
    LayoutInflater* mInflater;
    Callback mCallback;
    MenuView* mMenuView;
protected:
    virtual void addItemView(View* itemView, int childIndex);
    virtual bool filterLeftoverView(ViewGroup* parent, int childIndex);
public:
    BaseMenuPresenter(Context* context,const std::string& menuLayoutRes,const std::string& itemLayoutRes);

    void initForMenu(Context* context, MenuBuilder* menu)override;
    MenuView* getMenuView(ViewGroup* root)override;
    void updateMenuView(bool cleared);
    void setCallback(const Callback& cb)override;
    Callback getCallback();

    MenuView::ItemView* createItemView(ViewGroup* parent);

    virtual View* getItemView(MenuItemImpl* item, View* convertView, ViewGroup* parent);

    virtual void bindItemView(MenuItemImpl* item, MenuView::ItemView* itemView)=0;

    virtual bool shouldIncludeItem(int childIndex, MenuItemImpl* item);

    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)override;
    bool onSubMenuSelected(SubMenuBuilder* menu)override;
    bool flagActionItems();

    bool expandItemActionView(MenuBuilder& menu, MenuItemImpl& item);
    bool collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item);

    int getId() const override;
    void setId(int id);
};
}/*endof namespace*/
#endif/*__BASE_MENU_PRESENTER_H__*/
