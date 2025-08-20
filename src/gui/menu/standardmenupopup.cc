#include <menu/standardmenupopup.h>
namespace cdroid{
class StandardMenuPopup:public MenuPopup implements OnDismissListener, OnItemClickListener,
        MenuPresenter, OnKeyListener {
/*private static final int ITEM_LAYOUT = com.android.internal.R.layout.popup_menu_item_layout;
private static final int ITEM_LAYOUT_MATERIAL =com.android.internal.R.layout.popup_menu_item_layout_material;

private final OnGlobalLayoutListener mGlobalLayoutListener = new OnGlobalLayoutListener() {
    @Override
    public void onGlobalLayout() {
        // Only move the popup if it's showing and non-modal. We don't want
        // to be moving around the only interactive window, since there's a
        // good chance the user is interacting with it.
        if (isShowing() && !mPopup.isModal()) {
            final View anchor = mShownAnchorView;
            if (anchor == null || !anchor.isShown()) {
                dismiss();
            } else {
                // Recompute window size and position
                mPopup.show();
            }
        }
    }
};
private final OnAttachStateChangeListener mAttachStateChangeListener =
        new OnAttachStateChangeListener() {
    @Override
    public void onViewAttachedToWindow(View v) {
    }

    @Override
    public void onViewDetachedFromWindow(View v) {
        if (mTreeObserver != null) {
            if (!mTreeObserver.isAlive()) mTreeObserver = v.getViewTreeObserver();
            mTreeObserver.removeGlobalOnLayoutListener(mGlobalLayoutListener);
        }
        v.removeOnAttachStateChangeListener(this);
    }
};*/

StandardMenuPopup::StandardMenuPopup(Context* context, MenuBuilder* menu, View* anchorView, int popupStyleAttr,
        int popupStyleRes, bool overflowOnly) {
    mContext = context;//Objects.requireNonNull(context);
    mMenu = menu;
    mOverflowOnly = overflowOnly;
    LayoutInflater* inflater = LayoutInflater::from(context);
    mAdapter = new MenuAdapter(menu, inflater, mOverflowOnly, ITEM_LAYOUT_MATERIAL);
    mPopupStyleAttr = popupStyleAttr;
    mPopupStyleRes = popupStyleRes;

    final Resources res = context.getResources();
    mPopupMaxWidth = std::max(res.getDisplayMetrics().widthPixels / 2,
            res.getDimensionPixelSize(com.android.internal.R.dimen.config_prefDialogWidth));

    mAnchorView = anchorView;

    mPopup = new MenuPopupWindow(mContext, nullptr, mPopupStyleAttr, mPopupStyleRes);

    // Present the menu using our context, not the menu builder's context.
    menu.addMenuPresenter(this, context);
}

void StandardMenuPopup::setForceShowIcon(bool forceShow) {
    mAdapter->setForceShowIcon(forceShow);
}

void StandardMenuPopup::setGravity(int gravity) {
    mDropDownGravity = gravity;
}

bool StandardMenuPopup::tryShow() {
    if (isShowing()) {
        return true;
    }

    if (mWasDismissed || mAnchorView == nullptr) {
        return false;
    }

    mShownAnchorView = mAnchorView;

    mPopup.setOnDismissListener(this);
    mPopup.setOnItemClickListener(this);
    mPopup.setAdapter(mAdapter);
    mPopup.setModal(true);

    View* anchor = mShownAnchorView;
    const bool addGlobalListener = mTreeObserver == null;
    mTreeObserver = anchor.getViewTreeObserver(); // Refresh to latest
    if (addGlobalListener) {
        mTreeObserver.addOnGlobalLayoutListener(mGlobalLayoutListener);
    }
    anchor.addOnAttachStateChangeListener(mAttachStateChangeListener);
    mPopup.setAnchorView(anchor);
    mPopup.setDropDownGravity(mDropDownGravity);

    if (!mHasContentWidth) {
        mContentWidth = measureIndividualMenuWidth(mAdapter, null, mContext, mPopupMaxWidth);
        mHasContentWidth = true;
    }

    mPopup.setContentWidth(mContentWidth);
    mPopup.setInputMethodMode(PopupWindow.INPUT_METHOD_NOT_NEEDED);
    mPopup.setEpicenterBounds(getEpicenterBounds());
    mPopup.show();

    ListView* listView = mPopup.getListView();
    listView->setOnKeyListener(this);

    if (mShowTitle && mMenu->getHeaderTitle() != null) {
        FrameLayout* titleItemView =(FrameLayout*) LayoutInflater::from(mContext)->inflate(
                        com.android.internal.R.layout.popup_menu_header_item_layout,
                        listView, false);
        TextView* titleView = (TextView*) titleItemView->findViewById(cdroid::R::id::title);
        if (titleView != nullptr) {
            titleView->setText(mMenu->getHeaderTitle());
        }
        titleItemView->setEnabled(false);
        listView->addHeaderView(titleItemView, null, false);

        // Update to show the title.
        mPopup->show();
    }
    return true;
}

void StandardMenuPopup::show() {
    if (!tryShow()) {
        throw new IllegalStateException("StandardMenuPopup cannot be used without an anchor");
    }
}

void StandardMenuPopup::dismiss() {
    if (isShowing()) {
        mPopup.dismiss();
    }
}

void StandardMenuPopup::addMenu(MenuBuilder* menu) {
    // No-op: standard implementation has only one menu which is set in the constructor.
}

bool StandardMenuPopup::isShowing() {
    return !mWasDismissed && mPopup.isShowing();
}

void StandardMenuPopup::onDismiss() {
    mWasDismissed = true;
    mMenu.close();

    if (mTreeObserver != null) {
        if (!mTreeObserver.isAlive()) mTreeObserver = mShownAnchorView.getViewTreeObserver();
        mTreeObserver.removeGlobalOnLayoutListener(mGlobalLayoutListener);
        mTreeObserver = null;
    }
    mShownAnchorView.removeOnAttachStateChangeListener(mAttachStateChangeListener);

    if (mOnDismissListener != null) {
        mOnDismissListener.onDismiss();
    }
}

void StandardMenuPopup::updateMenuView(bool cleared) {
    mHasContentWidth = false;

    if (mAdapter != nullptr) {
        mAdapter->notifyDataSetChanged();
    }
}

void StandardMenuPopup::setCallback(Callback cb) {
    mPresenterCallback = cb;
}

bool StandardMenuPopup::onSubMenuSelected(SubMenuBuilder subMenu) {
    if (subMenu.hasVisibleItems()) {
        final MenuPopupHelper subPopup = new MenuPopupHelper(mContext, subMenu,
                mShownAnchorView, mOverflowOnly, mPopupStyleAttr, mPopupStyleRes);
        subPopup.setPresenterCallback(mPresenterCallback);
        subPopup.setForceShowIcon(MenuPopup.shouldPreserveIconSpacing(subMenu));

        // Pass responsibility for handling onDismiss to the submenu.
        subPopup.setOnDismissListener(mOnDismissListener);
        mOnDismissListener = null;

        // Close this menu popup to make room for the submenu popup.
        mMenu.close(false /* closeAllMenus */);

        // Show the new sub-menu popup at the same location as this popup.
        int horizontalOffset = mPopup.getHorizontalOffset();
        const int verticalOffset = mPopup.getVerticalOffset();

        // As xOffset of parent menu popup is subtracted with Anchor width for Gravity.RIGHT,
        // So, again to display sub-menu popup in same xOffset, add the Anchor width.
        const int hgrav = Gravity::getAbsoluteGravity(mDropDownGravity,
            mAnchorView->getLayoutDirection()) & Gravity::HORIZONTAL_GRAVITY_MASK;
        if (hgrav == Gravity::RIGHT) {
          horizontalOffset += mAnchorView.getWidth();
        }

        if (subPopup.tryShow(horizontalOffset, verticalOffset)) {
            if (mPresenterCallback != null) {
                mPresenterCallback.onOpenSubMenu(subMenu);
            }
            return true;
        }
    }
    return false;
}

void StandardMenuPopup::onCloseMenu(MenuBuilder menu, bool allMenusAreClosing) {
    // Only care about the (sub)menu we're presenting.
    if (menu != mMenu) return;

    dismiss();
    if (mPresenterCallback.onCloseMenu != nullptr) {
        mPresenterCallback.onCloseMenu(menu, allMenusAreClosing);
    }
}

bool StandardMenuPopup::flagActionItems() {
    return false;
}

Parcelable* StandardMenuPopup::onSaveInstanceState() {
    return nullptr;
}

void StandardMenuPopup::onRestoreInstanceState(Parcelable state) {
}

void StandardMenuPopup::setAnchorView(View* anchor) {
    mAnchorView = anchor;
}

bool StandardMenuPopup::onKey(View& v, int keyCode, KeyEvent& event) {
    if (event.getAction() == KeyEvent::ACTION_UP && keyCode == KeyEvent::KEYCODE_MENU) {
        dismiss();
        return true;
    }
    return false;
}

void StandardMenuPopup::setOnDismissListener(OnDismissListener listener) {
    mOnDismissListener = listener;
}

ListView* StandardMenuPopup::getListView() {
    return mPopup->getListView();
}


void StandardMenuPopup::setHorizontalOffset(int x) {
    mPopup->setHorizontalOffset(x);
}

void StandardMenuPopup::setVerticalOffset(int y) {
    mPopup->setVerticalOffset(y);
}

void StandardMenuPopup::setShowTitle(bool showTitle) {
    mShowTitle = showTitle;
}
}/*endof namespace*/
