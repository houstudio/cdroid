#ifndef __MENU_POP_H__
#define __MENU_POP_H__
#include <widget/popupwindow.h>
namespace cdroid{
class MenuPopup implements ShowableListMenu, MenuPresenter,
        AdapterView.OnItemClickListener {
private:
    Rect mEpicenterBounds;
protected:
    static int measureIndividualMenuWidth(ListAdapter* adapter, ViewGroup* parent,
            Context* context, int maxAllowedWidth);
    static MenuAdapter* toMenuAdapter(ListAdapter* adapter);
    static bool shouldPreserveIconSpacing(MenuBuilder* menu);
public:
    virtual void setForceShowIcon(bool forceShow)=0;

    virtual void addMenu(MenuBuilder menu)=0;

    virtual void setGravity(int dropDownGravity)=0;

    virtual void setAnchorView(View anchor)=0;

    virtual void setHorizontalOffset(int x)=0;
    virtual void setVerticalOffset(int y)=0;

    /**
     * Specifies the anchor-relative bounds of the popup's transition
     * epicenter.
     *
     * @param bounds anchor-relative bounds
     */
    void setEpicenterBounds(const Rect& bounds);
    Rect getEpicenterBounds() const;

    /**
     * Set whether a title entry should be shown in the popup menu (if a title exists for the
     * menu).
     *
     * @param showTitle
     */
    virtual void setShowTitle(bool showTitle)=0;

    /**
     * Set a listener to receive a callback when the popup is dismissed.
     *
     * @param listener Listener that will be notified when the popup is dismissed.
     */
    void setOnDismissListener(const PopupWindow::OnDismissListener& listener)=0;

    void initForMenu(Context* context, MenuBuilder* menu) override;
    MenuView* getMenuView(ViewGroup* root);

    bool expandItemActionView(MenuBuilder& menu, MenuItemImpl& item) override;
    bool collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item) override;

    int getId()const override;

    void onItemClick(AdapterView&parent, View& view, int position, long id);
};
}/*endof namespace*/
#endif/*__MENU_POP_H__*/
