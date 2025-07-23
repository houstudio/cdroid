#ifndef __MENU_POPUP_HELPER_H__
#define __MENU_POPUP_HELPER_H__
#include <menu/menuhelper.h>
#include <widget/popupwindow.h>
namespace cdroid{
class MenuPopup;
class MenuPopupHelper:public MenuHelper {
private:
    static constexpr int TOUCH_EPICENTER_SIZE_DP = 48;
    Context* mContext;
    // Immutable cached popup menu properties.
    MenuBuilder* mMenu;
    bool mOverflowOnly;
    int mPopupStyleAttr;
    int mPopupStyleRes;

    // Mutable cached popup menu properties.
    View* mAnchorView;
    int mDropDownGravity = Gravity::START;
    bool mForceShowIcon;
    MenuPresenter::Callback mPresenterCallback;

    MenuPopup* mPopup;
    PopupWindow::OnDismissListener mOnDismissListener;
private:
    MenuPopup* createPopup();
    void showPopup(int xOffset, int yOffset, bool useOffsets, bool showTitle);
protected:
    /**
     * Called after the popup has been dismissed.
     * <p>
     * <strong>Note:</strong> Subclasses should call the super implementation
     * last to ensure that any necessary tear down has occurred before the
     * listener specified by {@link #setOnDismissListener(OnDismissListener)}
     * is called.
     */
    virtual void onDismiss();
public:
    MenuPopupHelper(Context* context, MenuBuilder* menu);
    MenuPopupHelper(Context* context, MenuBuilder* menu, View* anchorView);
    MenuPopupHelper(Context* context, MenuBuilder* menu,
     View* anchorView, bool overflowOnly,int popupStyleAttr);

    MenuPopupHelper(Context* context,MenuBuilder* menu, View* anchorView,
     bool overflowOnly,int popupStyleAttr,int popupStyleRes);

    void setOnDismissListener(const PopupWindow::OnDismissListener& listener);

    /**
      * Sets the view to which the popup window is anchored.
      * <p>
      * Changes take effect on the next call to show().
      *
      * @param anchor the view to which the popup window should be anchored
      */
    void setAnchorView(View* anchor);

    /**
     * Sets whether the popup menu's adapter is forced to show icons in the
     * menu item views.
     * <p>
     * Changes take effect on the next call to show().
     *
     * This method should not be accessed directly outside the framework, please use
     * {@link android.widget.PopupMenu#setForceShowIcon(boolean)} instead.
     *
     * @param forceShowIcon {@code true} to force icons to be shown, or
     *                  {@code false} for icons to be optionally shown
     */
    void setForceShowIcon(bool forceShowIcon);

    /**
      * Sets the alignment of the popup window relative to the anchor view.
      * <p>
      * Changes take effect on the next call to show().
      *
      * @param gravity alignment of the popup relative to the anchor
      */
    void setGravity(int gravity);
    int getGravity() const;

    void show();
    void show(int x, int y);
    MenuPopup* getPopup();

    /**
     * Attempts to show the popup anchored to the view specified by {@link #setAnchorView(View)}.
     *
     * @return {@code true} if the popup was shown or was already showing prior to calling this
     *         method, {@code false} otherwise
     */
    bool tryShow();

    /**
     * Shows the popup menu and makes a best-effort to anchor it to the
     * specified (x,y) coordinate relative to the anchor view.
     * <p>
     * Additionally, the popup's transition epicenter (see
     * {@link android.widget.PopupWindow#setEpicenterBounds(Rect)} will be
     * centered on the specified coordinate, rather than using the bounds of
     * the anchor view.
     * <p>
     * If the popup's resolved gravity is {@link Gravity#LEFT}, this will
     * display the popup with its top-left corner at (x,y) relative to the
     * anchor view. If the resolved gravity is {@link Gravity#RIGHT}, the
     * popup's top-right corner will be at (x,y).
     * <p>
     * If the popup cannot be displayed fully on-screen, this method will
     * attempt to scroll the anchor view's ancestors and/or offset the popup
     * such that it may be displayed fully on-screen.
     *
     * @param x x coordinate relative to the anchor view
     * @param y y coordinate relative to the anchor view
     * @return {@code true} if the popup was shown or was already showing prior
     *         to calling this method, {@code false} otherwise
     */
    bool tryShow(int x, int y);

    /**
     * Dismisses the popup, if showing.
     */
    void dismiss();
    bool isShowing();
    void setPresenterCallback(const MenuPresenter::Callback& cb);
};
}/*endof namespace*/
#endif/*__MENU_POPUP_HELPER_H__*/
