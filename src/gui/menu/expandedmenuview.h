#ifndef __EXPANDED_MENU_VIEW_H__
#define __EXPANDED_MENU_VIEW_H__
#include <menu/menuview.h>
#include <widget/listview.h>
namespace cdroid{
class ExpandedMenuView:public ListView,public MenuView{// implements ItemInvoker, MenuView, OnItemClickListener {
private:
    int mAnimations;
    MenuBuilder* mMenu;
protected:
    void onDetachedFromWindow() override;
public:
    ExpandedMenuView(Context* context,const AttributeSet& attrs);
    void initialize(MenuBuilder* menu)override;
    bool invokeItem(MenuItemImpl* item);

    void onItemClick(AdapterView& parent, View& v, int position, long id);
    int getWindowAnimations();
};
}/*endof namespace*/
#endif/*__EXPANDED_MENU_VIEW_H__*/
