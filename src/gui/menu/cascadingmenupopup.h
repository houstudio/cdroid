#ifndef __CASCADING_MENU_POPUP_H__
#define __CASCADING_MENU_POPUP_H__
#include <widget/listview.h>
#include <widget/popupwindow.h>
#include <menu/menupopup.h>
namespace cdroid{
class MenuPopupWindow;
class CascadingMenuPopup:public MenuPopup{// implements MenuPresenter, OnKeyListener,PopupWindow.OnDismissListener {
private:
    static constexpr int HORIZ_POSITION_LEFT = 0;
    static constexpr int HORIZ_POSITION_RIGHT = 1;
    static constexpr int SUBMENU_TIMEOUT_MS = 200;
    class CascadingMenuInfo;
private:
    Context* mContext;
    int mMenuMaxWidth;
    int mPopupStyleAttr;
    int mPopupStyleRes;
    bool mOverflowOnly;
    Handler* mSubMenuHoverHandler;
    std::vector<MenuBuilder*> mPendingMenus;
    std::vector<CascadingMenuInfo*> mShowingMenus;
    ViewTreeObserver::OnGlobalLayoutListener mGlobalLayoutListener;
    View::OnAttachStateChangeListener mAttachStateChangeListener;
    MenuItemHoverListener mMenuItemHoverListener;

    int mRawDropDownGravity = Gravity::NO_GRAVITY;
    int mDropDownGravity = Gravity::NO_GRAVITY;
    View* mAnchorView;
    View* mShownAnchorView;
    int mLastPosition;
    bool mHasXOffset;
    bool mHasYOffset;
    int mXOffset;
    int mYOffset;
    bool mForceShowIcon;
    bool mShowTitle;
    Callback mPresenterCallback;
    ViewTreeObserver* mTreeObserver;
    PopupWindow::OnDismissListener mOnDismissListener;
    int mItemLayout;

    /** Whether popup menus should disable exit animations when closing. */
    bool mShouldCloseImmediately;
private:
    void onGlobalLayout();
    void onViewAttachedToWindow(View*);
    void onViewDetachedFromWindow(View*);
    void onItemHoverExit(MenuBuilder& menu,MenuItem& item);
    void onItemHoverEnter(MenuBuilder& menu,MenuItem& item);
    MenuPopupWindow* createPopupWindow();
    int getInitialMenuPosition();
    int getNextMenuPosition(int nextMenuWidth);
    void showMenu(MenuBuilder* menu);
    MenuItem* findMenuItemForSubmenu(MenuBuilder* parent, MenuBuilder* submenu);
    View* findParentViewForSubmenu(CascadingMenuInfo* parentInfo, MenuBuilder* submenu);
    int findIndexOfAddedMenu(MenuBuilder* menu);
public:
    CascadingMenuPopup(Context* context,View* anchor, int popupStyleAttr, int popupStyleRes, bool overflowOnly);

    void setForceShowIcon(bool forceShow)override;

    void show() override;

    void dismiss() override;
    bool onKey(View& v, int keyCode, KeyEvent& event);

    void addMenu(MenuBuilder* menu)override;

    bool isShowing() override;

    void onDismiss();

    void updateMenuView(bool cleared)override;

    void setCallback(const Callback& cb) override;
    bool onSubMenuSelected(SubMenuBuilder* subMenu)override;
    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)override;

    bool flagActionItems()override;

    Parcelable* onSaveInstanceState()override;
    void onRestoreInstanceState(Parcelable& state)override;

    void setGravity(int dropDownGravity)override;

    void setAnchorView(View* anchor)override;

    void setOnDismissListener(const PopupWindow::OnDismissListener& listener)override;

    ListView* getListView()override;

    void setHorizontalOffset(int x)override;
    void setVerticalOffset(int y)override;
    void setShowTitle(bool showTitle) override;
};
class CascadingMenuPopup::CascadingMenuInfo {
public:
    MenuPopupWindow* window;
    MenuBuilder* menu;
    int position;
public:
    CascadingMenuInfo(MenuPopupWindow* window,MenuBuilder* menu,int position);
    ListView* getListView();
};
}/*endof namespace*/
#endif/*__CASCADING_MENU_POPUP_H__*/
