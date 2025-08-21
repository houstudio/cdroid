#ifndef __STANDARD_MENU_POPUP_H__
#define __STANDARD_MENU_POPUP_H__
#include <view/view.h>
#include <widget/listview.h>
#include <menu/menupopup.h>
namespace cdroid{
class MenuPopupWindow;
class StandardMenuPopup:public MenuPopup{//implements OnDismissListener, OnItemClickListener,MenuPresenter, OnKeyListener {
private:
    Context* mContext;
    MenuBuilder* mMenu;
    MenuAdapter* mAdapter;
    int mPopupMaxWidth;
    int mPopupStyleAttr;
    int mPopupStyleRes;

    MenuPopupWindow* mPopup;
    PopupWindow::OnDismissListener mOnDismissListener;

    View* mAnchorView;
    View* mShownAnchorView;
    Callback mPresenterCallback;
    ViewTreeObserver* mTreeObserver;

    /** Whether the popup has been dismissed. Once dismissed, it cannot be opened again. */
    bool mWasDismissed;
    /** Whether the cached content width value is valid. */
    bool mHasContentWidth;
    bool mShowTitle;
    bool mOverflowOnly;
    /** Cached content width. */
    int mContentWidth;
    int mDropDownGravity = Gravity::NO_GRAVITY;
private:
    bool tryShow();
public:
    StandardMenuPopup(Context* context, MenuBuilder* menu, View* anchorView, int popupStyleAttr,
            int popupStyleRes, bool overflowOnly);

    void setForceShowIcon(bool forceShow) override;

    void setGravity(int gravity) override;
    void show() override;
    void dismiss() override;

    void addMenu(MenuBuilder* menu) override;

    bool isShowing() override;
    virtual void onDismiss();

    void updateMenuView(bool cleared) override;
    void setCallback(const Callback& cb) override;

    bool onSubMenuSelected(SubMenuBuilder* subMenu)override;
    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)override;

    bool flagActionItems()override;

    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable& state)override;

    void setAnchorView(View* anchor) override;

    virtual bool onKey(View& v, int keyCode, KeyEvent& event);
    virtual void onItemClick(AdapterView&parent, View& view, int position, long id);
    void setOnDismissListener(const PopupWindow::OnDismissListener& listener) override;

    ListView* getListView() override;

    void setHorizontalOffset(int x) override;
    void setVerticalOffset(int y) override;

    void setShowTitle(bool showTitle)override;
};
}/*endof namespace*/
#endif/*__STANDARD_MENU_POPUP_H__*/
