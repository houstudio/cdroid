#if 0
#include <menu/menuview.h>
#include <menu/menubuilder.h>
#include <menu/submenubuilder.h>
#include <menu/actionmenuview.h>
#include <menu/actionmenuitemview.h>
#include <menu/actionmenupresenter.h>
namespace cdroid{

ActionMenuPresenter::ActionMenuPresenter(Context* context)
    :BaseMenuPresenter(context, "cdroid:layout/action_menu_layout","cdroid:layout/action_menu_item_layout"){
    mItemAnimationPreDrawListener=[this]()->bool{
        computeMenuItemAnimationInfo(false);
         ((View*) mMenuView)->getViewTreeObserver()->removeOnPreDrawListener(mItemAnimationPreDrawListener);
         runItemAnimations();
         return true;
    };
    mAttachStateChangeListener.onViewAttachedToWindow=[](View&){};
    mAttachStateChangeListener.onViewDetachedFromWindow=[this](View&){
        ((View*) mMenuView)->getViewTreeObserver()->removeOnPreDrawListener( mItemAnimationPreDrawListener);
        mPreLayoutItems.clear();
        mPostLayoutItems.clear();
    };
}

void ActionMenuPresenter::initForMenu(Context* context, MenuBuilder* menu) {
    BaseMenuPresenter::initForMenu(context, menu);

    //Resources res = context.getResources();
    //ActionBarPolicy abp = ActionBarPolicy.get(context);
    if (!mReserveOverflowSet) {
        mReserveOverflow = true;//abp.showsOverflowMenuButton();
    }

    if (!mWidthLimitSet) {
        mWidthLimit = context->getDisplayMetrics().widthPixels/2 ;//abp.getEmbeddedMenuWidthLimit();
    }

    // Measure for initial configuration
    if (!mMaxItemsSet) {
        mMaxItems = context->getDisplayMetrics().widthPixels/120;//abp.getMaxActionButtons();
    }

    int width = mWidthLimit;
    if (mReserveOverflow) {
        if (mOverflowButton == nullptr) {
            mOverflowButton = new OverflowMenuButton(mSystemContext);
            if (mPendingOverflowIconSet) {
                mOverflowButton->setImageDrawable(mPendingOverflowIcon);
                mPendingOverflowIcon = nullptr;
                mPendingOverflowIconSet = false;
            }
            const int spec = MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED);
            mOverflowButton->measure(spec, spec);
        }
        width -= mOverflowButton->getMeasuredWidth();
    } else {
        mOverflowButton = nullptr;
    }

    mActionItemWidthLimit = width;

    mMinCellSize = (int) (ActionMenuView::MIN_CELL_SIZE * context->getDisplayMetrics().density);
}

/*void ActionMenuPresenter::onConfigurationChanged(Configuration newConfig) {
    if (!mMaxItemsSet) {
        mMaxItems = context->getDisplayMetrics().widthPixels/120;//ActionBarPolicy.get(mContext).getMaxActionButtons();
    }
    if (mMenu != nullptr) {
        mMenu->onItemsChanged(true);
    }
}*/

void ActionMenuPresenter::setWidthLimit(int width, bool strict) {
    mWidthLimit = width;
    mStrictWidthLimit = strict;
    mWidthLimitSet = true;
}

void ActionMenuPresenter::setReserveOverflow(bool reserveOverflow) {
    mReserveOverflow = reserveOverflow;
    mReserveOverflowSet = true;
}

void ActionMenuPresenter::setItemLimit(int itemCount) {
    mMaxItems = itemCount;
    mMaxItemsSet = true;
}

void ActionMenuPresenter::setExpandedActionViewsExclusive(bool isExclusive) {
    mExpandedActionViewsExclusive = isExclusive;
}

void ActionMenuPresenter::setOverflowIcon(Drawable* icon) {
    if (mOverflowButton != nullptr) {
        mOverflowButton->setImageDrawable(icon);
    } else {
        mPendingOverflowIconSet = true;
        mPendingOverflowIcon = icon;
    }
}

Drawable* ActionMenuPresenter::getOverflowIcon() {
    if (mOverflowButton != nullptr) {
        return mOverflowButton->getDrawable();
    } else if (mPendingOverflowIconSet) {
        return mPendingOverflowIcon;
    }
    return nullptr;
}

MenuView* ActionMenuPresenter::getMenuView(ViewGroup* root) {
    MenuView* oldMenuView = mMenuView;
    MenuView* result = BaseMenuPresenter::getMenuView(root);
    View::OnAttachStateChangeListener mAttachStateChangeListener;
    if (oldMenuView != result) {
        ((ActionMenuView*) result)->setPresenter(this);
        if (oldMenuView != nullptr) {
            ((View*) oldMenuView)->removeOnAttachStateChangeListener(mAttachStateChangeListener);
        }
        ((View*)result)->addOnAttachStateChangeListener(mAttachStateChangeListener);
    }
    return result;
}

View* ActionMenuPresenter::getItemView(MenuItemImpl* item, View* convertView, ViewGroup* parent) {
    View* actionView = item->getActionView();
    if ((actionView == nullptr) || item->hasCollapsibleActionView()) {
        actionView = BaseMenuPresenter::getItemView(item, convertView, parent);
    }
    actionView->setVisibility(item->isActionViewExpanded() ? View::GONE : View::VISIBLE);

    ActionMenuView* menuParent = (ActionMenuView*) parent;
    ViewGroup::LayoutParams* lp = actionView->getLayoutParams();
    if (!menuParent->checkLayoutParams(lp)) {
        actionView->setLayoutParams(menuParent->generateLayoutParams(lp));
    }
    return actionView;
}

void ActionMenuPresenter::bindItemView(MenuItemImpl* item, MenuView::ItemView* itemView) {
    itemView->initialize(item, 0);

    ActionMenuView* menuView = (ActionMenuView*) mMenuView;
    ActionMenuItemView* actionItemView = (ActionMenuItemView*) itemView;
    auto invoker=[this](MenuItemImpl&item)->bool{return ((ActionMenuView*)mMenuView)->invokeItem(item);};
    actionItemView->setItemInvoker(invoker);//menuView);
    //mPopupCallback=[this](){ return mActionButtonPopup != nullptr ? mActionButtonPopup->getPopup() : nullptr; }
    //if (mPopupCallback == nullptr) mPopupCallback = new ActionMenuPopupCallback();
    actionItemView->setPopupCallback(mPopupCallback);
}

bool ActionMenuPresenter::shouldIncludeItem(int childIndex, MenuItemImpl* item) {
    return item->isActionButton();
}

void ActionMenuPresenter::computeMenuItemAnimationInfo(bool preLayout) {
    ViewGroup* menuView = (ViewGroup*) mMenuView;
    const int count = menuView->getChildCount();
    auto& items = preLayout ? mPreLayoutItems : mPostLayoutItems;
    for (int i = 0; i < count; ++i) {
        View* child = menuView->getChildAt(i);
        const int id = child->getId();
        if (id > 0 && child->getWidth() != 0 && child->getHeight() != 0) {
            MenuItemLayoutInfo* info = new MenuItemLayoutInfo(child, preLayout);
            items.put(id, info);
        }
    }
}

void ActionMenuPresenter::runItemAnimations() {
    for (int i = 0; i < mPreLayoutItems.size(); ++i) {
        int id = mPreLayoutItems.keyAt(i);
        MenuItemLayoutInfo* menuItemLayoutInfoPre = mPreLayoutItems.get(id);
        const int postLayoutIndex = mPostLayoutItems.indexOfKey(id);
        if (postLayoutIndex >= 0) {
            // item exists pre and post: see if it's changed
            MenuItemLayoutInfo* menuItemLayoutInfoPost = mPostLayoutItems.valueAt(postLayoutIndex);
            PropertyValuesHolder* pvhX = nullptr;
            PropertyValuesHolder* pvhY = nullptr;
            if (menuItemLayoutInfoPre->left != menuItemLayoutInfoPost->left) {
                pvhX = PropertyValuesHolder::ofFloat("translationX",//View::TRANSLATION_X,
                        {float(menuItemLayoutInfoPre->left - menuItemLayoutInfoPost->left), 0.f});
            }
            if (menuItemLayoutInfoPre->top != menuItemLayoutInfoPost->top) {
                pvhY = PropertyValuesHolder::ofFloat("translationY",//"View::TRANSLATION_Y,
                        {float(menuItemLayoutInfoPre->top - menuItemLayoutInfoPost->top), 0.f});
            }
            if (pvhX != nullptr || pvhY != nullptr) {
                for (int j = 0; j < mRunningItemAnimations.size(); ++j) {
                    ItemAnimationInfo* oldInfo = mRunningItemAnimations.at(j);
                    if (oldInfo->id == id && oldInfo->animType == ItemAnimationInfo::MOVE) {
                        oldInfo->animator->cancel();
                    }
                }
                ObjectAnimator* anim;
                if (pvhX != nullptr) {
                    if (pvhY != nullptr) {
                        anim = ObjectAnimator::ofPropertyValuesHolder(menuItemLayoutInfoPost->view,{pvhX, pvhY});
                    } else {
                        anim = ObjectAnimator::ofPropertyValuesHolder(menuItemLayoutInfoPost->view,{pvhX});
                    }
                } else {
                    anim = ObjectAnimator::ofPropertyValuesHolder(menuItemLayoutInfoPost->view,{pvhY});
                }
                anim->setDuration(ITEM_ANIMATION_DURATION);
                anim->start();
                ItemAnimationInfo* info = new ItemAnimationInfo(id, menuItemLayoutInfoPost, anim,ItemAnimationInfo::MOVE);
                mRunningItemAnimations.push_back(info);
                Animator::AnimatorListener als;
                als.onAnimationEnd =[this](Animator&animation,bool isReverse){
                    for (auto it= mRunningItemAnimations.begin();it!=mRunningItemAnimations.end(); it++) {
                        if ((*it)->animator == &animation) {
                            mRunningItemAnimations.erase(it);
                            break;
                        }
                    }
                };
                anim->addListener(als);
            }
            mPostLayoutItems.remove(id);
        } else {
            // item used to be there, is now gone
            float oldAlpha = 1;
            for (int j = 0; j < mRunningItemAnimations.size(); ++j) {
                ItemAnimationInfo* oldInfo = mRunningItemAnimations.at(j);
                if (oldInfo->id == id && oldInfo->animType == ItemAnimationInfo::FADE_IN) {
                    oldAlpha = oldInfo->menuItemLayoutInfo->view->getAlpha();
                    oldInfo->animator->cancel();
                }
            }
            ObjectAnimator* anim = ObjectAnimator::ofFloat((void*)menuItemLayoutInfoPre->view, "alpha"/*View::ALPHA*/, {oldAlpha, 0.f});
            // Re-using the view from pre-layout assumes no view recycling
            ((ViewGroup*) mMenuView)->getOverlay()->add(menuItemLayoutInfoPre->view);
            anim->setDuration(ITEM_ANIMATION_DURATION);
            anim->start();
            ItemAnimationInfo* info = new ItemAnimationInfo(id, menuItemLayoutInfoPre, anim, ItemAnimationInfo::FADE_OUT);
            mRunningItemAnimations.push_back(info);
            Animator::AnimatorListener als;
            als.onAnimationEnd=[this,menuItemLayoutInfoPre](Animator&animation,bool isReverse){
                for (auto it=mRunningItemAnimations.begin();it!=mRunningItemAnimations.end(); it++) {
                    if ((*it)->animator == &animation) {
                        mRunningItemAnimations.erase(it);
                        break;
                    }
                }
                ((ViewGroup*) mMenuView)->getOverlay()->remove(menuItemLayoutInfoPre->view);
            };
            anim->addListener(als);
        }
    }
    for (int i = 0; i < mPostLayoutItems.size(); ++i) {
        int id = mPostLayoutItems.keyAt(i);
        const int postLayoutIndex = mPostLayoutItems.indexOfKey(id);
        if (postLayoutIndex >= 0) {
            // item is new
            MenuItemLayoutInfo* menuItemLayoutInfo = mPostLayoutItems.valueAt(postLayoutIndex);
            float oldAlpha = 0;
            for (int j = 0; j < mRunningItemAnimations.size(); ++j) {
                ItemAnimationInfo* oldInfo = mRunningItemAnimations.at(j);
                if (oldInfo->id == id && oldInfo->animType == ItemAnimationInfo::FADE_OUT) {
                    oldAlpha = oldInfo->menuItemLayoutInfo->view->getAlpha();
                    oldInfo->animator->cancel();
                }
            }
            ObjectAnimator* anim = ObjectAnimator::ofFloat(menuItemLayoutInfo->view, "alpha"/*View::ALPHA*/,{oldAlpha, 1.f});
            anim->start();
            anim->setDuration(ITEM_ANIMATION_DURATION);
            ItemAnimationInfo* info = new ItemAnimationInfo(id, menuItemLayoutInfo, anim, ItemAnimationInfo::FADE_IN);
            mRunningItemAnimations.push_back(info);
            Animator::AnimatorListener als;
            als.onAnimationEnd=[this](Animator&animation,bool isReverse){
                for (auto it=mRunningItemAnimations.begin(); it != mRunningItemAnimations.end(); it++) {
                    if ((*it)->animator == &animation) {
                        mRunningItemAnimations.erase(it);
                        break;
                    }
                }
            };
            anim->addListener(als);
        }
    }
    mPreLayoutItems.clear();
    mPostLayoutItems.clear();
}

void ActionMenuPresenter::setupItemAnimations() {
    computeMenuItemAnimationInfo(true);
    ((View*) mMenuView)->getViewTreeObserver()->addOnPreDrawListener(mItemAnimationPreDrawListener);
}

void ActionMenuPresenter::updateMenuView(bool cleared) {
    ViewGroup* menuViewParent = (ViewGroup*) ((View*) mMenuView)->getParent();
    if (menuViewParent != nullptr && ACTIONBAR_ANIMATIONS_ENABLED) {
        setupItemAnimations();
    }
    BaseMenuPresenter::updateMenuView(cleared);

    ((View*) mMenuView)->requestLayout();

    if (mMenu != nullptr) {
        std::vector<MenuItemImpl*> actionItems = mMenu->getActionItems();
        const int count = actionItems.size();
        for (int i = 0; i < count; i++) {
            ActionProvider* provider = actionItems.at(i)->getActionProvider();
            if (provider != nullptr) {
                provider->setSubUiVisibilityListener([this](bool isVisible){
                    onSubUiVisibilityChanged(isVisible);
                });//this);
            }
        }
    }

    std::vector<MenuItemImpl*> nonActionItems;// = mMenu != nullptr ? mMenu->getNonActionItems() : nullptr;
    if(mMenu)nonActionItems=mMenu->getNonActionItems();
    bool hasOverflow = false;
    if (mReserveOverflow && nonActionItems.size()){// != nullptr) {
        const int count = nonActionItems.size();
        if (count == 1) {
            hasOverflow = !nonActionItems.at(0)->isActionViewExpanded();
        } else {
            hasOverflow = count > 0;
        }
    }

    if (hasOverflow) {
        if (mOverflowButton == nullptr) {
            mOverflowButton = new OverflowMenuButton(mSystemContext);
        }
        ViewGroup* parent = (ViewGroup*) mOverflowButton->getParent();
        if ((void*)parent != (void*)mMenuView) {
            if (parent != nullptr) {
                parent->removeView(mOverflowButton);
            }
            ActionMenuView* menuView = (ActionMenuView*) mMenuView;
            menuView->addView(mOverflowButton, menuView->generateOverflowButtonLayoutParams());
        }
    } else if ((mOverflowButton != nullptr) && mOverflowButton->getParent() == (void*)mMenuView) {
        ((ViewGroup*) mMenuView)->removeView(mOverflowButton);
    }

    ((ActionMenuView*) mMenuView)->setOverflowReserved(mReserveOverflow);
}

bool ActionMenuPresenter::filterLeftoverView(ViewGroup* parent, int childIndex) {
    if (parent->getChildAt(childIndex) == mOverflowButton) return false;
    return BaseMenuPresenter::filterLeftoverView(parent, childIndex);
}

bool ActionMenuPresenter::onSubMenuSelected(SubMenuBuilder* subMenu) {
    if (!subMenu->hasVisibleItems()) return false;

    SubMenuBuilder* topSubMenu = subMenu;
    while (topSubMenu->getParentMenu() != mMenu) {
        topSubMenu = (SubMenuBuilder*)topSubMenu->getParentMenu();
    }
    View* anchor = findViewForItem(topSubMenu->getInvokerItem());
    if (anchor == nullptr) {
        // This means the submenu was opened from an overflow menu item, indicating the
        // MenuPopupHelper will handle opening the submenu via its MenuPopup. Return false to
        // ensure that the MenuPopup acts as presenter for the submenu, and acts on its
        // responsibility to display the new submenu.
        return false;
    }

    mOpenSubMenuId = subMenu->getInvokerItem()->getItemId();

    bool preserveIconSpacing = false;
    const int count = subMenu->size();
    for (int i = 0; i < count; i++) {
        MenuItem* childItem = subMenu->getItem(i);
        if (childItem->isVisible() && childItem->getIcon() != nullptr) {
            preserveIconSpacing = true;
            break;
        }
    }

    mActionButtonPopup = new ActionButtonSubmenu(mContext, subMenu, anchor);
    mActionButtonPopup->setForceShowIcon(preserveIconSpacing);
    mActionButtonPopup->show();

    BaseMenuPresenter::onSubMenuSelected(subMenu);
    return true;
}

View* ActionMenuPresenter::findViewForItem(MenuItem* item) {
    ViewGroup* parent = (ViewGroup*) mMenuView;
    if (parent == nullptr) return nullptr;

    const int count = parent->getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = parent->getChildAt(i);
        if (dynamic_cast<MenuView::ItemView*>(child) &&
                ((MenuView::ItemView*) child)->getItemData() == item) {
            return child;
        }
    }
    return nullptr;
}

/**
 * Display the overflow menu if one is present.
 * @return true if the overflow menu was shown, false otherwise.
 */
bool ActionMenuPresenter::showOverflowMenu() {
    if (mReserveOverflow && !isOverflowMenuShowing() && mMenu != nullptr && mMenuView != nullptr &&
            mPostedOpenRunnable == nullptr && !mMenu->getNonActionItems().empty()) {
        OverflowPopup* popup = new OverflowPopup(mContext, mMenu, mOverflowButton, true);
        mPostedOpenRunnable = [this,popup](){//new OpenOverflowRunnable(popup);
            if (mMenu != nullptr) {
                mMenu->changeMenuMode();
            }
            View* menuView = (View*) mMenuView;
            if (menuView != nullptr && /*menuView->getWindowToken() != nullptr &&*/ popup->tryShow()) {
                mOverflowPopup = popup;
            }
            mPostedOpenRunnable = nullptr;
        };
        // Post this for later; we might still need a layout for the anchor to be right.
        ((View*) mMenuView)->post(mPostedOpenRunnable);

        // ActionMenuPresenter uses null as a callback argument here
        // to indicate overflow is opening.
        BaseMenuPresenter::onSubMenuSelected(nullptr);

        return true;
    }
    return false;
}

/**
 * Hide the overflow menu if it is currently showing.
 *
 * @return true if the overflow menu was hidden, false otherwise.
 */
bool ActionMenuPresenter::hideOverflowMenu() {
    if ((mPostedOpenRunnable != nullptr) && (mMenuView != nullptr)) {
        ((View*) mMenuView)->removeCallbacks(mPostedOpenRunnable);
        mPostedOpenRunnable = nullptr;
        return true;
    }

    MenuPopupHelper* popup = mOverflowPopup;
    if (popup != nullptr) {
        popup->dismiss();
        return true;
    }
    return false;
}

bool ActionMenuPresenter::dismissPopupMenus() {
    bool result = hideOverflowMenu();
    result |= hideSubMenus();
    return result;
}

bool ActionMenuPresenter::hideSubMenus() {
    if (mActionButtonPopup != nullptr) {
        mActionButtonPopup->dismiss();
        return true;
    }
    return false;
}

bool ActionMenuPresenter::isOverflowMenuShowing() const{
    return (mOverflowPopup != nullptr) && mOverflowPopup->isShowing();
}

bool ActionMenuPresenter::isOverflowMenuShowPending() const{
    return (mPostedOpenRunnable != nullptr) || isOverflowMenuShowing();
}

bool ActionMenuPresenter::isOverflowReserved() const{
    return mReserveOverflow;
}

bool ActionMenuPresenter::flagActionItems() {
    std::vector<MenuItemImpl*> visibleItems;
    int itemsSize;
    if (mMenu != nullptr) {
        visibleItems = mMenu->getVisibleItems();
        itemsSize = visibleItems.size();
    } else {
        visibleItems.clear();//=nullptr;
        itemsSize = 0;
    }

    int maxActions = mMaxItems;
    int widthLimit = mActionItemWidthLimit;
    const int querySpec = MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED);
    ViewGroup* parent = (ViewGroup*) mMenuView;

    int requiredItems = 0;
    int requestedItems = 0;
    int firstActionWidth = 0;
    bool hasOverflow = false;
    for (int i = 0; i < itemsSize; i++) {
        MenuItemImpl* item = visibleItems.at(i);
        if (item->requiresActionButton()) {
            requiredItems++;
        } else if (item->requestsActionButton()) {
            requestedItems++;
        } else {
            hasOverflow = true;
        }
        if (mExpandedActionViewsExclusive && item->isActionViewExpanded()) {
            // Overflow everything if we have an expanded action view and we're
            // space constrained.
            maxActions = 0;
        }
    }

    // Reserve a spot for the overflow item if needed.
    if (mReserveOverflow &&
            (hasOverflow || requiredItems + requestedItems > maxActions)) {
        maxActions--;
    }
    maxActions -= requiredItems;

    SparseBooleanArray& seenGroups = mActionButtonGroups;
    seenGroups.clear();

    int cellSize = 0;
    int cellsRemaining = 0;
    if (mStrictWidthLimit) {
        cellsRemaining = widthLimit / mMinCellSize;
        const int cellSizeRemaining = widthLimit % mMinCellSize;
        cellSize = mMinCellSize + cellSizeRemaining / cellsRemaining;
    }

    // Flag as many more requested items as will fit.
    for (int i = 0; i < itemsSize; i++) {
        MenuItemImpl* item = visibleItems.at(i);

        if (item->requiresActionButton()) {
            View* v = getItemView(item, nullptr, parent);
            if (mStrictWidthLimit) {
                cellsRemaining -= ActionMenuView::measureChildForCells(v,
                        cellSize, cellsRemaining, querySpec, 0);
            } else {
                v->measure(querySpec, querySpec);
            }
            const int measuredWidth = v->getMeasuredWidth();
            widthLimit -= measuredWidth;
            if (firstActionWidth == 0) {
                firstActionWidth = measuredWidth;
            }
            const int groupId = item->getGroupId();
            if (groupId != 0) {
                seenGroups.put(groupId, true);
            }
            item->setIsActionButton(true);
        } else if (item->requestsActionButton()) {
            // Items in a group with other items that already have an action slot
            // can break the max actions rule, but not the width limit.
            const int groupId = item->getGroupId();
            const bool inGroup = seenGroups.get(groupId);
            bool isAction = (maxActions > 0 || inGroup) && widthLimit > 0 &&
                    (!mStrictWidthLimit || cellsRemaining > 0);

            if (isAction) {
                View* v = getItemView(item, nullptr, parent);
                if (mStrictWidthLimit) {
                    const int cells = ActionMenuView::measureChildForCells(v,
                            cellSize, cellsRemaining, querySpec, 0);
                    cellsRemaining -= cells;
                    if (cells == 0) {
                        isAction = false;
                    }
                } else {
                    v->measure(querySpec, querySpec);
                }
                const int measuredWidth = v->getMeasuredWidth();
                widthLimit -= measuredWidth;
                if (firstActionWidth == 0) {
                    firstActionWidth = measuredWidth;
                }

                if (mStrictWidthLimit) {
                    isAction &= widthLimit >= 0;
                } else {
                    // Did this push the entire first item past the limit?
                    isAction &= widthLimit + firstActionWidth > 0;
                }
            }

            if (isAction && groupId != 0) {
                seenGroups.put(groupId, true);
            } else if (inGroup) {
                // We broke the width limit. Demote the whole group, they all overflow now.
                seenGroups.put(groupId, false);
                for (int j = 0; j < i; j++) {
                    MenuItemImpl* areYouMyGroupie = visibleItems.at(j);
                    if (areYouMyGroupie->getGroupId() == groupId) {
                        // Give back the action slot
                        if (areYouMyGroupie->isActionButton()) maxActions++;
                        areYouMyGroupie->setIsActionButton(false);
                    }
                }
            }

            if (isAction) maxActions--;

            item->setIsActionButton(isAction);
        } else {
            // Neither requires nor requests an action button.
            item->setIsActionButton(false);
        }
    }
    return true;
}

void ActionMenuPresenter::onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing) {
    dismissPopupMenus();
    BaseMenuPresenter::onCloseMenu(menu, allMenusAreClosing);
}

Parcelable* ActionMenuPresenter::onSaveInstanceState() {
    //SavedState* state = new SavedState();
    //state->openSubMenuId = mOpenSubMenuId;
    return nullptr;//state;
}

void ActionMenuPresenter::onRestoreInstanceState(Parcelable& state) {
    /*SavedState& saved = (SavedState&) state;
    if (saved.openSubMenuId > 0) {
        MenuItem* item = mMenu->findItem(saved.openSubMenuId);
        if (item != nullptr) {
            SubMenuBuilder* subMenu = (SubMenuBuilder*) item->getSubMenu();
            onSubMenuSelected(subMenu);
        }
    }*/
}

void ActionMenuPresenter::onSubUiVisibilityChanged(bool isVisible) {
    if (isVisible) {
        // Not a submenu, but treat it like one.
        BaseMenuPresenter::onSubMenuSelected(nullptr);
    } else if (mMenu != nullptr) {
        mMenu->close(false /* closeAllMenus */);
    }
}

void ActionMenuPresenter::setMenuView(ActionMenuView* menuView) {
    if (menuView != mMenuView) {
        if (mMenuView != nullptr) {
            ((View*) mMenuView)->removeOnAttachStateChangeListener(mAttachStateChangeListener);
        }
        mMenuView = menuView;
        menuView->initialize(mMenu);
        menuView->addOnAttachStateChangeListener(mAttachStateChangeListener);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////

#if 0
static class SavedState implements Parcelable {
    public int openSubMenuId;

    SavedState() {
    }

    SavedState(Parcel in) {
        openSubMenuId = in.readInt();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(openSubMenuId);
    }

    public static final @android.annotation.NonNull Parcelable.Creator<SavedState> CREATOR
            = new Parcelable.Creator<SavedState>() {
        public SavedState createFromParcel(Parcel in) {
            return new SavedState(in);
        }

        public SavedState[] newArray(int size) {
            return new SavedState[size];
        }
    };
};

class OverflowMenuButton extends ImageButton implements ActionMenuView.ActionMenuChildView {
    public OverflowMenuButton(Context context) {
        super(context, null, com.android.internal.R.attr.actionOverflowButtonStyle);

        setClickable(true);
        setFocusable(true);
        setVisibility(VISIBLE);
        setEnabled(true);

        setOnTouchListener(new ForwardingListener(this) {
            @Override
            public ShowableListMenu getPopup() {
                if (mOverflowPopup == null) {
                    return null;
                }

                return mOverflowPopup.getPopup();
            }

            @Override
            public bool onForwardingStarted() {
                showOverflowMenu();
                return true;
            }

            @Override
            public bool onForwardingStopped() {
                // Displaying the popup occurs asynchronously, so wait for
                // the runnable to finish before deciding whether to stop
                // forwarding.
                if (mPostedOpenRunnable != null) {
                    return false;
                }

                hideOverflowMenu();
                return true;
            }
        });
    };

    @Override
    public bool performClick() {
        if (super.performClick()) {
            return true;
        }

        playSoundEffect(SoundEffectConstants.CLICK);
        showOverflowMenu();
        return true;
    }

    @Override
    public bool needsDividerBefore() {
        return false;
    }

    @Override
    public bool needsDividerAfter() {
        return false;
    }

    @Override
    public void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo info) {
        super.onInitializeAccessibilityNodeInfoInternal(info);
        info.setCanOpenPopup(true);
    }

    @Override
    protected bool setFrame(int l, int t, int r, int b) {
        final bool changed = super.setFrame(l, t, r, b);

        // Set up the hotspot bounds to square and centered on the image.
        final Drawable d = getDrawable();
        final Drawable bg = getBackground();
        if (d != null && bg != null) {
            final int width = getWidth();
            final int height = getHeight();
            final int halfEdge = Math.max(width, height) / 2;
            final int offsetX = getPaddingLeft() - getPaddingRight();
            final int offsetY = getPaddingTop() - getPaddingBottom();
            final int centerX = (width + offsetX) / 2;
            final int centerY = (height + offsetY) / 2;
            bg.setHotspotBounds(centerX - halfEdge, centerY - halfEdge,
                    centerX + halfEdge, centerY + halfEdge);
        }

        return changed;
    }
};

class OverflowPopup extends MenuPopupHelper {
    public OverflowPopup(Context context, MenuBuilder menu, View anchorView,
            bool overflowOnly) {
        super(context, menu, anchorView, overflowOnly,
                com.android.internal.R.attr.actionOverflowMenuStyle);
        setGravity(Gravity.END);
        setPresenterCallback(mPopupPresenterCallback);
    }

    @Override
    protected void onDismiss() {
        if (mMenu != null) {
            mMenu.close();
        }
        mOverflowPopup = null;

        super.onDismiss();
    }
};

class ActionButtonSubmenu extends MenuPopupHelper {
    public ActionButtonSubmenu(Context context, SubMenuBuilder subMenu, View anchorView) {
        super(context, subMenu, anchorView, false,
                com.android.internal.R.attr.actionOverflowMenuStyle);

        MenuItemImpl item = (MenuItemImpl) subMenu.getItem();
        if (!item.isActionButton()) {
            // Give a reasonable anchor to nested submenus.
            setAnchorView(mOverflowButton == null ? (View) mMenuView : mOverflowButton);
        }

        setPresenterCallback(mPopupPresenterCallback);
    }

    @Override
    protected void onDismiss() {
        mActionButtonPopup = null;
        mOpenSubMenuId = 0;

        super.onDismiss();
    }
};

class PopupPresenterCallback implements Callback {

    @Override
    public bool onOpenSubMenu(MenuBuilder subMenu) {
        if (subMenu == null) return false;

        mOpenSubMenuId = ((SubMenuBuilder) subMenu).getItem().getItemId();
        final Callback cb = getCallback();
        return cb != null ? cb.onOpenSubMenu(subMenu) : false;
    }

    @Override
    public void onCloseMenu(MenuBuilder& menu, bool allMenusAreClosing) {
        if (menu instanceof SubMenuBuilder) {
            menu.getRootMenu().close(false /* closeAllMenus */);
        }
        final Callback cb = getCallback();
        if (cb != null) {
            cb.onCloseMenu(menu, allMenusAreClosing);
        }
    }
};

class OpenOverflowRunnable implements Runnable {
    private OverflowPopup mPopup;

    public OpenOverflowRunnable(OverflowPopup popup) {
        mPopup = popup;
    }

    public void run() {
        if (mMenu != null) {
            mMenu.changeMenuMode();
        }
        final View menuView = (View) mMenuView;
        if (menuView != null && menuView.getWindowToken() != null && mPopup.tryShow()) {
            mOverflowPopup = mPopup;
        }
        mPostedOpenRunnable = null;
    }
};

class ActionMenuPopupCallback extends ActionMenuItemView.PopupCallback {
    @Override
    public ShowableListMenu getPopup() {
        return mActionButtonPopup != null ? mActionButtonPopup.getPopup() : null;
    }
};

static class MenuItemLayoutInfo {
    View* view;
    int left;
    int top;

    MenuItemLayoutInfo(View view, bool preLayout) {
        left = view.getLeft();
        top = view.getTop();
        if (preLayout) {
            // We track translation for pre-layout because a view might be mid-animation
            // and we need this information to know where to animate from
            left += view.getTranslationX();
            top += view.getTranslationY();
        }
        this.view = view;
    }
};

static class ItemAnimationInfo {
    int id;
    MenuItemLayoutInfo menuItemLayoutInfo;
    Animator* animator;
    int animType;
    static constexpr int MOVE = 0;
    static constexpr int FADE_IN = 1;
    static constexpr int FADE_OUT = 2;

    ItemAnimationInfo(int id, MenuItemLayoutInfo info, Animator anim, int animType) {
        this.id = id;
        menuItemLayoutInfo = info;
        animator = anim;
        this.animType = animType;
    }
};
#endif
}/*endof namespace*/
#endif
