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
#include <menu/popupmenu.h>
#include <menu/menupopup.h>
#include <menu/menuinflater.h>
#include <widget/forwardinglistener.h>
namespace cdroid{
    
PopupMenu::PopupMenu(Context* context, View* anchor)
    :PopupMenu(context, anchor, Gravity::NO_GRAVITY){
}

PopupMenu::PopupMenu(Context* context, View* anchor, int gravity)
    :PopupMenu(context, anchor, gravity, 0/*R.attr.popupMenuStyle*/, 0){
}

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
PopupMenu::PopupMenu(Context* context, View* anchor, int gravity, int popupStyleAttr,int popupStyleRes) {
    mContext = context;
    mAnchor = anchor;
    mMenuForwardingListener = nullptr;
    mMenu = new MenuBuilder(context);
    MenuBuilder::Callback cbk;
    cbk.onMenuItemSelected=[this](MenuBuilder& menu, MenuItem& item){
        if (mMenuItemClickListener != nullptr) {
            return mMenuItemClickListener/*.onMenuItemClick*/(item);
        }
        return false;
    };

    cbk.onMenuModeChange=[](MenuBuilder&menu){};
    mMenu->setCallback(cbk);

    mPopup = new MenuPopupHelper(context, mMenu, anchor, false, popupStyleAttr, popupStyleRes);
    mPopup->setGravity(gravity);
    mPopup->setOnDismissListener([this](){
        if(mOnDismissListener!=nullptr){
            mOnDismissListener(*this);
        }
    });
}

/**
 * Sets the gravity used to align the popup window to its anchor view.
 * <p>
 * If the popup is showing, calling this method will take effect only
 * the next time the popup is shown.
 *
 * @param gravity the gravity used to align the popup window
 * @see #getGravity()
 */
void PopupMenu::setGravity(int gravity) {
    mPopup->setGravity(gravity);
}

/**
 * @return the gravity used to align the popup window to its anchor view
 * @see #setGravity(int)
 */
int PopupMenu::getGravity() const{
    return mPopup->getGravity();
}

class PopupMenu::MenuForwardingListener:public ForwardingListener{
private:
    PopupMenu*mPopupMenu;
public:
    MenuForwardingListener(PopupMenu*pm,View*v):ForwardingListener(v),mPopupMenu(pm){}
    bool onForwardingStarted()override{
        mPopupMenu->show();
        return true;
    }
    bool onForwardingStopped()override{
        mPopupMenu->dismiss();
        return true;
    }
    ShowableListMenu getPopup()override{
        ShowableListMenu lm;
        auto p = mPopupMenu->mPopup;
        lm.show=[p](){p->show();};
        lm.dismiss=[p](){p->dismiss();};
        lm.isShowing=[p](){return p->isShowing();};
        lm.getListView=[this](){return mPopupMenu->getMenuListView();};
        return lm;
    }
};

View::OnTouchListener PopupMenu::getDragToOpenListener() {
    if (mMenuForwardingListener == nullptr) {
        mMenuForwardingListener = new MenuForwardingListener(this,mAnchor);
        mDragListener=[this](View&view,MotionEvent&event){
            return mMenuForwardingListener->onTouch(view,event);
        };
    }
    return mDragListener;
}
/**
 * Returns the {@link Menu} associated with this popup. Populate the
 * returned Menu with items before calling {@link #show()}.
 *
 * @return the {@link Menu} associated with this popup
 * @see #show()
 * @see #getMenuInflater()
 */
Menu* PopupMenu::getMenu() const{
    return mMenu;
}

/**
 * @return a {@link MenuInflater} that can be used to inflate menu items
 *         from XML into the menu returned by {@link #getMenu()}
 * @see #getMenu()
 */
MenuInflater* PopupMenu::getMenuInflater() {
    return new MenuInflater(mContext);
}

/**
 * Inflate a menu resource into this PopupMenu. This is equivalent to
 * calling {@code popupMenu.getMenuInflater().inflate(menuRes, popupMenu.getMenu())}.
 *
 * @param menuRes Menu resource to inflate
 */
void PopupMenu::inflate(const std::string& menuRes) {
    getMenuInflater()->inflate(menuRes, mMenu);
}

void PopupMenu::show() {
    mPopup->show();
}

void PopupMenu::dismiss() {
    mPopup->dismiss();
}

void PopupMenu::setOnMenuItemClickListener(const OnMenuItemClickListener& listener) {
    mMenuItemClickListener = listener;
}

void PopupMenu::setOnDismissListener(const OnDismissListener& listener) {
    mOnDismissListener = listener;
}

void PopupMenu::setForceShowIcon(bool forceShowIcon) {
    mPopup->setForceShowIcon(forceShowIcon);
}

ListView* PopupMenu::getMenuListView() {
    if (!mPopup->isShowing()) {
        return nullptr;
    }
    return mPopup->getPopup()->getListView();
}
}/*endof namespace*/
