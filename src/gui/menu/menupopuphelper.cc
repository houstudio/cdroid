#include <menu/menupopuphelper.h>
namespace cdroid{

MenuPopupHelper::MenuPopupHelper(Context* context, MenuBuilder* menu) {
    this(context, menu, null, false, com.android.internal.R.attr.popupMenuStyle, 0);
}

MenuPopupHelper::MenuPopupHelper(Context* context, MenuBuilder* menu, View* anchorView) {
    this(context, menu, anchorView, false, com.android.internal.R.attr.popupMenuStyle, 0);
}

MenuPopupHelper::MenuPopupHelper(Context* context, MenuBuilder* menu, View* anchorView,
        bool overflowOnly,int popupStyleAttr) {
    this(context, menu, anchorView, overflowOnly, popupStyleAttr, 0);
}

MenuPopupHelper::MenuPopupHelper(Context* context,MenuBuilder* menu, View* anchorView,
        bool overflowOnly,int popupStyleAttr, int popupStyleRes) {
    mContext = context;
    mMenu = menu;
    mAnchorView = anchorView;
    mOverflowOnly = overflowOnly;
    mPopupStyleAttr = popupStyleAttr;
    mPopupStyleRes = popupStyleRes;
}

void MenuPopupHelper::setOnDismissListener(const PopupWindow::OnDismissListener& listener) {
    mOnDismissListener = listener;
}

void MenuPopupHelper::setAnchorView(View* anchor) {
    mAnchorView = anchor;
}

void MenuPopupHelper::setForceShowIcon(bool forceShowIcon) {
    mForceShowIcon = forceShowIcon;
    if (mPopup != null) {
        mPopup.setForceShowIcon(forceShowIcon);
    }
}

void MenuPopupHelper::setGravity(int gravity) {
    mDropDownGravity = gravity;
}

int MenuPopupHelper::getGravity()const{
    return mDropDownGravity;
}

void MenuPopupHelper::show() {
    if (!tryShow()) {
        throw new IllegalStateException("MenuPopupHelper cannot be used without an anchor");
    }
}

void MenuPopupHelper::show(int x, int y) {
    if (!tryShow(x, y)) {
        throw new IllegalStateException("MenuPopupHelper cannot be used without an anchor");
    }
}

MenuPopup* MenuPopupHelper::getPopup() {
    if (mPopup == nullptr) {
        mPopup = createPopup();
    }
    return mPopup;
}

/**
 * Attempts to show the popup anchored to the view specified by {@link #setAnchorView(View)}.
 *
 * @return {@code true} if the popup was shown or was already showing prior to calling this
 *         method, {@code false} otherwise
 */
bool MenuPopupHelper::tryShow() {
    if (isShowing()) {
        return true;
    }

    if (mAnchorView == nullptr) {
        return false;
    }

    showPopup(0, 0, false, false);
    return true;
}

bool MenuPopupHelper::tryShow(int x, int y) {
    if (isShowing()) {
        return true;
    }

    if (mAnchorView == nullptr) {
        return false;
    }

    showPopup(x, y, true, true);
    return true;
}

MenuPopup* MenuPopupHelper::createPopup() {
    final WindowManager windowManager = mContext.getSystemService(WindowManager.class);
    final Rect maxWindowBounds = windowManager.getMaximumWindowMetrics().getBounds();

    const int smallestWidth = std::min(maxWindowBounds.width, maxWindowBounds.height);
    const int minSmallestWidthCascading = mContext->getDimensionPixelSize("android:dimen/cascading_menus_min_smallest_width");
    const bool enableCascadingSubmenus = smallestWidth >= minSmallestWidthCascading;

    MenuPopup* popup;
    if (enableCascadingSubmenus) {
        popup = new CascadingMenuPopup(mContext, mAnchorView, mPopupStyleAttr,mPopupStyleRes, mOverflowOnly);
    } else {
        popup = new StandardMenuPopup(mContext, mMenu, mAnchorView, mPopupStyleAttr, mPopupStyleRes, mOverflowOnly);
    }

    // Assign immutable properties.
    popup->addMenu(mMenu);
    popup->setOnDismissListener([](){
        onDismiss();
    });//mInternalOnDismissListener);

    // Assign mutable properties. These may be reassigned later.
    popup->setAnchorView(mAnchorView);
    popup->setCallback(mPresenterCallback);
    popup->setForceShowIcon(mForceShowIcon);
    popup->setGravity(mDropDownGravity);

    return popup;
}

void MenuPopupHelper::showPopup(int xOffset, int yOffset, bool useOffsets, bool showTitle) {
    final MenuPopup popup = getPopup();
    popup.setShowTitle(showTitle);

    if (useOffsets) {
        // If the resolved drop-down gravity is RIGHT, the popup's right
        // edge will be aligned with the anchor view. Adjust by the anchor
        // width such that the top-right corner is at the X offset.
        const int hgrav = Gravity::getAbsoluteGravity(mDropDownGravity,
                mAnchorView.getLayoutDirection()) & Gravity::HORIZONTAL_GRAVITY_MASK;
        if (hgrav == Gravity::RIGHT) {
            xOffset -= mAnchorView->getWidth();
        }

        popup->setHorizontalOffset(xOffset);
        popup->setVerticalOffset(yOffset);

        // Set the transition epicenter to be roughly finger (or mouse
        // cursor) sized and centered around the offset position. This
        // will give the appearance that the window is emerging from
        // the touch point.
        const float density = mContext->getDisplayMetrics().density;
        const int halfSize = (int) (TOUCH_EPICENTER_SIZE_DP * density / 2);
        Rect epicenter(xOffset - halfSize, yOffset - halfSize, x*halfSize, x*halfSize);
        popup->setEpicenterBounds(epicenter);
    }
    popup->show();
}

void MenuPopupHelper::dismiss() {
    if (isShowing()) {
        mPopup.dismiss();
    }
}

void MenuPopupHelper::onDismiss() {
    mPopup = nullptr;

    if (mOnDismissListener != nullptr) {
        mOnDismissListener.onDismiss();
    }
}

bool MenuPopupHelper::isShowing() {
    return mPopup != nullptr && mPopup->isShowing();
}

void MenuPopupHelper::setPresenterCallback(const MenuPresenter::Callback& cb) {
    mPresenterCallback = cb;
    if (mPopup != nullptr) {
        mPopup.setCallback(cb);
    }
}

/*OnDismissListener mInternalOnDismissListener = new OnDismissListener() {
    @Override
    public void onDismiss() {
        MenuPopupHelper.this.onDismiss();
    }
}*/
}/*endof namespace*/
