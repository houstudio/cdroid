#ifndef __ACTION_MENU_PRESENTER_H__
#define __ACTION_MENU_PRESENTER_H__
#include <widget/imagebutton.h>
#include <menu/menupopuphelper.h>
#include <menu/basemenupresenter.h>
#include <menu/menubuilder.h>
namespace cdroid{
class ActionMenuView;
class ActionMenuItemView;
class ActionMenuPresenter:public BaseMenuPresenter{
        //implements ActionProvider.SubUiVisibilityListener {
private:
    static constexpr int ITEM_ANIMATION_DURATION = 150;
    static constexpr bool ACTIONBAR_ANIMATIONS_ENABLED = false;
    class SavedState;
    class OverflowMenuButton;
    class OverflowPopup;
    class ActionButtonSubmenu;
    class ItemAnimationInfo;
    class MenuItemLayoutInfo;

    OverflowMenuButton* mOverflowButton;
    Drawable* mPendingOverflowIcon;
    bool mPendingOverflowIconSet;
    bool mReserveOverflow;
    bool mReserveOverflowSet;
    bool mMaxItemsSet;
    bool mStrictWidthLimit;
    bool mWidthLimitSet;
    bool mExpandedActionViewsExclusive;
    int mWidthLimit;
    int mActionItemWidthLimit;
    int mMaxItems;
    int mMinCellSize;
    int mOpenSubMenuId;

    // Group IDs that have been added as actions - used temporarily, allocated here for reuse.
    SparseBooleanArray mActionButtonGroups;

    OverflowPopup* mOverflowPopup;
    ActionButtonSubmenu* mActionButtonPopup;


    Runnable mPostedOpenRunnable;
    //ActionMenuPopupCallback mPopupCallback;
    ActionMenuItemView::PopupCallback mPopupCallback;

    //PopupPresenterCallback mPopupPresenterCallback = new PopupPresenterCallback();
    MenuPresenter::Callback mPopupPresenterCallback;

    // These collections are used to store pre- and post-layout information for menu items,
    // which is used to determine appropriate animations to run for changed items.
    SparseArray<MenuItemLayoutInfo*> mPreLayoutItems;
    SparseArray<MenuItemLayoutInfo*> mPostLayoutItems;
    // The list of currently running animations on menu items.
    std::vector<ItemAnimationInfo*> mRunningItemAnimations;
    ViewTreeObserver::OnPreDrawListener mItemAnimationPreDrawListener;
    View::OnAttachStateChangeListener mAttachStateChangeListener;
private:
    void computeMenuItemAnimationInfo(bool preLayout);
    void runItemAnimations();
    void setupItemAnimations();
    View* findViewForItem(MenuItem* item);
public:
    ActionMenuPresenter(Context* context);
    void initForMenu(Context* context, MenuBuilder* menu)override;
    //void onConfigurationChanged(Configuration newConfig)override;
    void setWidthLimit(int width, bool strict);
    void setReserveOverflow(bool reserveOverflow);

    void setItemLimit(int itemCount);

    void setExpandedActionViewsExclusive(bool isExclusive);

    void setOverflowIcon(Drawable* icon);
    Drawable* getOverflowIcon();

    MenuView* getMenuView(ViewGroup* root) override;

    View* getItemView(MenuItemImpl* item, View* convertView, ViewGroup* parent)override;

    void bindItemView(MenuItemImpl* item, MenuView::ItemView* itemView)override;

    bool shouldIncludeItem(int childIndex, MenuItemImpl* item)override;

    void updateMenuView(bool cleared) override;

    bool filterLeftoverView(ViewGroup* parent, int childIndex) override;

    bool onSubMenuSelected(SubMenuBuilder* subMenu)override;

    /**
     * Display the overflow menu if one is present.
     * @return true if the overflow menu was shown, false otherwise.
     */
    bool showOverflowMenu();
    bool hideOverflowMenu();

    /**
     * Dismiss all popup menus - overflow and submenus.
     * @return true if popups were dismissed, false otherwise. (This can be because none were open.)
     */
    bool dismissPopupMenus();
    /**
     * Dismiss all submenu popups.
     *
     * @return true if popups were dismissed, false otherwise. (This can be because none were open.)
     */
    bool hideSubMenus();

    /**
     * @return true if the overflow menu is currently showing
     */
    bool isOverflowMenuShowing()const;

    bool isOverflowMenuShowPending()const;

    /**
     * @return true if space has been reserved in the action menu for an overflow item.
     */
    bool isOverflowReserved() const;

    bool flagActionItems();

    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)override;

    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable& state) override;
    virtual void onSubUiVisibilityChanged(bool isVisible);

    void setMenuView(ActionMenuView* menuView);
};/*endof ActionMenuPresenter*/

/*class ActionMenuPresenter::SavedState:public BaseSavedState{// implements Parcelable {
public:
    int openSubMenuId;
public:
    SavedState() {
    }

    SavedState(Parcel& in) override{
        openSubMenuId = in.readInt();
    }

    int describeContents() override{
        return 0;
    }

    void writeToParcel(Parcel& dest, int flags) {
        dest.writeInt(openSubMenuId);
    }
};*/

class ActionMenuPresenter::OverflowMenuButton:public ImageButton,ActionMenuView::ActionMenuChildView {
protected:
    bool setFrame(int l, int t, int r, int b)override;
public:
    OverflowMenuButton(Context* context);

    bool performClick() override;

    bool needsDividerBefore() override{
        return false;
    }

    bool needsDividerAfter() override{
        return false;
    }
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info);
};

class ActionMenuPresenter::OverflowPopup:public MenuPopupHelper {
protected:
    void onDismiss() override;
public:
    OverflowPopup(Context* context, MenuBuilder* menu, View* anchorView,bool overflowOnly);
};

class ActionMenuPresenter::ActionButtonSubmenu:public MenuPopupHelper {
protected:
    void onDismiss()override;
public:
    ActionButtonSubmenu(Context* context, SubMenuBuilder* subMenu, View* anchorView);
};

/*class ActionMenuPresenter::PopupPresenterCallback:public MenuPresenter::Callback {
public:
    bool onOpenSubMenu(MenuBuilder& subMenu) override;
    void onCloseMenu(MenuBuilder& menu, bool allMenusAreClosing) override;
};

class ActionMenuPresenter::OpenOverflowRunnable:public Runnable {
private:
    OverflowPopup* mPopup;
public:
    OpenOverflowRunnable(OverflowPopup* popup) {
        mPopup = popup;
    }
    void run();
};

class ActionMenuPresenter::ActionMenuPopupCallback:public ActionMenuItemView::PopupCallback {
public:
    ShowableListMenu* getPopup() override{
        return mActionButtonPopup != nullptr ? mActionButtonPopup->getPopup() : nullptr;
    }
};*/

class ActionMenuPresenter::MenuItemLayoutInfo {
public:
    View* view;
    int left;
    int top;
public:
    MenuItemLayoutInfo(View* view, bool preLayout);
};

class ActionMenuPresenter::ItemAnimationInfo {
public:
    int id;
    MenuItemLayoutInfo* menuItemLayoutInfo;
    Animator* animator;
    int animType;
    static constexpr int MOVE = 0;
    static constexpr int FADE_IN = 1;
    static constexpr int FADE_OUT = 2;
public:
    ItemAnimationInfo(int id, MenuItemLayoutInfo* info, Animator* anim, int animType) {
        this->id = id;
        menuItemLayoutInfo = info;
        animator = anim;
        this->animType = animType;
    }
};
}/*endof namespace*/
#endif/*__ACTION_MENU_PRESENTER_H__*/
