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
#include <widget/internal_R.h>
#include <menu/popupmenu.h>
#include <menu/menupopup.h>
#include <menu/menuinflater.h>
#include <porting/cdlog.h>
namespace cdroid{
using namespace cdroid::internal;

PopupMenu::PopupMenu(Context* context, View* anchor)
    :PopupMenu(context, anchor, Gravity::NO_GRAVITY){
}

PopupMenu::~PopupMenu(){
    // A dismiss cascade may have posted the self-delete. When this destructor
    // runs first (a legacy owner deleting the menu itself), dropping the
    // handler PURGES the pending post (~Handler clears its queued messages),
    // so the posted delete cannot double-free. The posted path itself nulls
    // the member before `delete this`, so this purge never runs from it.
    if (mDeleteHandler != nullptr) {
        delete mDeleteHandler;
        mDeleteHandler = nullptr;
    }
    if (mAliveFlag != nullptr) {
        *mAliveFlag = false;  // drag-to-open / ShowableListMenu become no-ops
    }
    // mPopup (~MenuPopupHelper -> ~CascadingMenuPopup) unregisters itself from
    // mMenu's presenter list on teardown, so the helper chain must die while
    // mMenu is still alive -- delete it first.
    delete mPopup;
    delete mMenu;
    delete mMenuForwardingListener;
}

PopupMenu::PopupMenu(Context* context, View* anchor, int gravity)
    :PopupMenu(context, anchor, gravity, R::attr::popupMenuStyle, 0){
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
PopupMenu::PopupMenu(Context* context, View* anchor, int gravity, int popupStyleAttr, int popupStyleRes) {
    mContext = context;
    mAnchor = anchor;
    mMenuForwardingListener = nullptr;
    mAliveFlag = std::make_shared<bool>(true);
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
        // Fire-and-forget: this is the OUTERMOST listener of the dismiss
        // cascade, but the cascade's frames below it (PopupWindow::dismiss ->
        // ListPopupWindow -> CascadingMenuPopup::onCloseMenu ->
        // MenuBuilder::close -> MenuPopupHelper::onDismiss) are still on the
        // stack, which is why the deletion must be POSTED - it runs on the
        // next looper drain, after everything has unwound. Same pattern as
        // Window::finishClose and ActionMenuPresenter::OverflowPopup.
        // The post is STAGED before the app listener (which may take any
        // action; nothing after it may touch members): if the destructor runs
        // first it purges the staged post (~Handler clears its queue); if the
        // post runs first it nulls mDeleteHandler before `delete this`, so the
        // destructor never deletes the handler it is dispatching from.
        if (mDeleteHandler == nullptr) {
            Handler* h = new Handler();
            mDeleteHandler = h;
            PopupMenu* self = this;
            h->post([this, h, self](){
                mDeleteHandler = nullptr;
                delete self;   // ~PopupMenu sees a null handler, frees the rest
                delete h;      // the established AMP/Window idiom
            });
        }
        if(mOnDismissListener!=nullptr){
            mOnDismissListener(*this);   // app listener: nothing follows it
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

View::OnTouchListener PopupMenu::getDragToOpenListener() {
    if (mMenuForwardingListener == nullptr) {
        mMenuForwardingListener = new MenuForwardingListener(this,mAnchor);
        // The app's anchor view holds this touch listener longer than the
        // (one-shot) menu lives - gate every entry on the alive-flag so the
        // forwarding becomes a no-op after the menu self-destructs.
        const auto alive = mAliveFlag;
        PopupMenu* pm = this;
        mDragListener=[pm, alive](View&view,MotionEvent&event){
            if (!*alive) return false;
            return pm->mMenuForwardingListener->onTouch(view,event);
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
void PopupMenu::inflate(int menuRes) {
    getMenuInflater()->inflate(menuRes, mMenu);
}

void PopupMenu::show() {
    // One-shot: once the dismiss cascade posted the self-delete this object is
    // logically dead (or already freed), and re-showing would re-enter
    // MenuPopupHelper::getPopup's reclaim at a non-safe point. Create a new
    // PopupMenu to show again (AOSP allows re-show only because GC keeps the
    // object reachable).
    if (mDeleteHandler != nullptr) {
        LOGW("PopupMenu is one-shot; create a new instance to show again");
        return;
    }
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

////////////////////////////////////////////////////////////////////////////////////////

PopupMenu::MenuForwardingListener::MenuForwardingListener(PopupMenu*pm,View*v)
    :ForwardingListener(v),mPopupMenu(pm),mAlive(pm->mAliveFlag){
}

bool PopupMenu::MenuForwardingListener::onForwardingStarted(){
    if (!*mAlive) return false;
    mPopupMenu->show();
    return true;
}
bool PopupMenu::MenuForwardingListener::onForwardingStopped(){
    if (!*mAlive) return true;
    mPopupMenu->dismiss();
    return true;
}
ShowableListMenu PopupMenu::MenuForwardingListener::getPopup(){
    ShowableListMenu lm;
    // Every closure outlives the (one-shot) menu: gate on the alive-flag so a
    // call after the menu self-destructed never dereferences the raw pointers.
    const auto alive = mAlive;
    PopupMenu* pm = mPopupMenu;
    lm.show=[alive, pm](){ if (*alive) pm->mPopup->show(); };
    lm.dismiss=[alive, pm](){ if (*alive) pm->mPopup->dismiss(); };
    lm.isShowing=[alive, pm](){ return *alive && pm->mPopup->isShowing(); };
    lm.getListView=[alive, pm](){ return *alive ? pm->getMenuListView() : nullptr; };
    return lm;
}

}/*endof namespace*/
