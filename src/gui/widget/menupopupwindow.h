#ifndef __MENU_POPUP_WINDOW_H__
#define __MENU_POPUP_WINDOW_H__
#include <widget/listpopupwindow.h>
#include <widget/dropdownlistview.h>
#include <menu/menubuilder.h>
#include <menu/menuitemhoverlistener.h>
namespace cdroid{
class Transition;
class MenuPopupWindow:public ListPopupWindow{// implements MenuItemHoverListener {
private:
    MenuItemHoverListener mHoverListener;
public:
    class MenuDropDownListView;
    MenuPopupWindow(Context* context,const AttributeSet& attrs);

    DropDownListView* createDropDownListView(Context* context, bool hijackFocus);

    void setEnterTransition(Transition* enterTransition);
    void setExitTransition(Transition* exitTransition);

    void setHoverListener(const MenuItemHoverListener& hoverListener);

    void setTouchModal(bool touchModal);

    void onItemHoverEnter(MenuBuilder& menu,MenuItem& item);
    void onItemHoverExit(MenuBuilder& menu, MenuItem& item);

    class MenuDropDownListView:public DropDownListView {
        int mAdvanceKey;
        int mRetreatKey;
    private:
        MenuItemHoverListener mHoverListener;
        MenuItem* mHoveredMenuItem;
    public:
        MenuDropDownListView(Context* context, bool hijackFocus);
        void setHoverListener(const MenuItemHoverListener& hoverListener);
        void clearSelection();
        bool onKeyDown(int keyCode, KeyEvent& event) override;
        bool onHoverEvent(MotionEvent& ev) override;
    };
};
}/*endof namespace*/
#endif/*__MENU_POPUP_WINDOW_H__*/
