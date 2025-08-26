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
#ifndef __POPUP_MENU_H__
#define __POPUP_MENU_H__
#include <widget/listview.h>
#include <menu/menubuilder.h>
#include <menu/menupopuphelper.h>
#include <widget/forwardinglistener.h>
namespace cdroid{

class PopupMenu{
public:
    DECLARE_UIEVENT(bool,OnMenuItemClickListener,MenuItem&);
    DECLARE_UIEVENT(void,OnDismissListener,PopupMenu&);
private:
    class MenuForwardingListener:public ForwardingListener{
    private:
        PopupMenu*mPopupMenu;
    public:
        MenuForwardingListener(PopupMenu*pm,View*v);
        bool onForwardingStarted()override;
        bool onForwardingStopped()override;
        ShowableListMenu getPopup()override;
    };

    Context* mContext;
    MenuBuilder* mMenu;
    View* mAnchor;
    MenuPopupHelper* mPopup;

    OnMenuItemClickListener mMenuItemClickListener;
    OnDismissListener mOnDismissListener;
    MenuForwardingListener *mMenuForwardingListener;
    View::OnTouchListener mDragListener;
public:
    PopupMenu(Context* context, View* anchor);
    PopupMenu(Context* context, View* anchor, int gravity);

    /**
     * Constructor a create a new popup menu with a specific style.
     *
     * @param context Context the popup menu is running in, through which it
     *        can access the current theme, resources, etc.
     * @param anchor Anchor view for this popup. The popup will appear below
     *        the anchor if there is room, or above it if there is not.
     * @param gravity The {@link Gravity} value for aligning the popup with its
     *        anchor.
     * @param popupStyleAttr An attribute in the current theme that contains a
     *        reference to a style resource that supplies default values for
     *        the popup window. Can be 0 to not look for defaults.
     * @param popupStyleRes A resource identifier of a style resource that
     *        supplies default values for the popup window, used only if
     *        popupStyleAttr is 0 or can not be found in the theme. Can be 0
     *        to not look for defaults.
     */
    PopupMenu(Context* context, View* anchor, int gravity, const std::string& popupStyleAttr,const std::string& popupStyleRes);
    virtual ~PopupMenu();

    /**
     * Sets the gravity used to align the popup window to its anchor view.
     * <p>
     * If the popup is showing, calling this method will take effect only
     * the next time the popup is shown.
     *
     * @param gravity the gravity used to align the popup window
     * @see #getGravity()
     */
    void setGravity(int gravity);
    int getGravity()const;

    /**
     * Returns an {@link OnTouchListener} that can be added to the anchor view
     * to implement drag-to-open behavior.
     * <p>
     * When the listener is set on a view, touching that view and dragging
     * outside of its bounds will open the popup window. Lifting will select
     * the currently touched list item.
     * <p>
     * Example usage:
     * <pre>
     * PopupMenu myPopup = new PopupMenu(context, myAnchor);
     * myAnchor.setOnTouchListener(myPopup.getDragToOpenListener());
     * </pre>
     *
     * @return a touch listener that controls drag-to-open behavior
     */
    View::OnTouchListener getDragToOpenListener();

    /**
     * Returns the {@link Menu} associated with this popup. Populate the
     * returned Menu with items before calling {@link #show()}.
     *
     * @return the {@link Menu} associated with this popup
     * @see #show()
     * @see #getMenuInflater()
     */
    Menu* getMenu() const;

    /**
     * @return a {@link MenuInflater} that can be used to inflate menu items
     *         from XML into the menu returned by {@link #getMenu()}
     * @see #getMenu()
     */
    MenuInflater* getMenuInflater();

    /**
     * Inflate a menu resource into this PopupMenu. This is equivalent to
     * calling {@code popupMenu.getMenuInflater().inflate(menuRes, popupMenu.getMenu())}.
     *
     * @param menuRes Menu resource to inflate
     */
    void inflate(const std::string& menuRes);

    void show();
    void dismiss();

    void setOnMenuItemClickListener(const OnMenuItemClickListener& listener);
    void setOnDismissListener(const OnDismissListener& listener);

    /**
     * Sets whether the popup menu's adapter is forced to show icons in the
     * menu item views.
     * <p>
     * Changes take effect on the next call to show().
     *
     * @param forceShowIcon {@code true} to force icons to be shown, or
     *                  {@code false} for icons to be optionally shown
     */
    void setForceShowIcon(bool forceShowIcon);

    /**
     * Returns the {@link ListView} representing the list of menu items in the currently showing
     * menu.
     *
     * @return The view representing the list of menu items.
     * @hide
     */
    ListView* getMenuListView();
};
}/*endof namespace*/
#endif/*__POPUP_MENU_H__*/
