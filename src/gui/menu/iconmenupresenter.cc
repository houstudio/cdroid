#include <menu/iconmenuitemview.h>
#include <menu/iconmenupresenter.h>
#include <menu/menudialoghelper.h>
#include <menu/menuitemimpl.h>
#include <menu/submenubuilder.h>
namespace cdroid{

IconMenuPresenter::IconMenuPresenter(Context* context)
    :BaseMenuPresenter(context,"",""){
            //com.android.internal.R.layout.icon_menu_layout,
            //com.android.internal.R.layout.icon_menu_item_layout);
}

void IconMenuPresenter::initForMenu(Context* context,MenuBuilder* menu){
    BaseMenuPresenter::initForMenu(context, menu);
    mMaxItems = -1;
}

void IconMenuPresenter::bindItemView(MenuItemImpl* item, MenuView::ItemView* itemView) {
    IconMenuItemView* view = (IconMenuItemView*) itemView;
    view->setItemData(item);

    view->initialize(item->getTitleForItemView(view), item->getIcon());

    view->setVisibility(item->isVisible() ? View::VISIBLE : View::GONE);
    view->setEnabled(view->isEnabled());
    view->setLayoutParams(view->getTextAppropriateLayoutParams());
}

bool IconMenuPresenter::shouldIncludeItem(int childIndex, MenuItemImpl* item){ 
    std::vector<MenuItemImpl*> itemsToShow = mMenu->getNonActionItems();
    const bool fits = (itemsToShow.size() == mMaxItems && childIndex < mMaxItems) ||
            childIndex < mMaxItems - 1;
    return fits && !item->isActionButton();
}

void IconMenuPresenter::addItemView(View* itemView, int childIndex) {
    IconMenuItemView* v = (IconMenuItemView*) itemView;
    IconMenuView* parent = (IconMenuView*) mMenuView;

    v->setIconMenuView(parent);
    v->setItemInvoker(std::bind(&IconMenuView::invokeItem,parent,std::placeholders::_1));
    v->setBackground(parent->getItemBackgroundDrawable());
    BaseMenuPresenter::addItemView(itemView, childIndex);
}

bool IconMenuPresenter::onSubMenuSelected(SubMenuBuilder* subMenu) {
    if (!subMenu->hasVisibleItems()) return false;

    // The window manager will give us a token.
    MenuDialogHelper* helper=new MenuDialogHelper(subMenu);
    helper->setPresenterCallback(mSubMenuPresenterCallback);
    helper->show(/*nullptr*/);
    mOpenSubMenu = helper;
    mOpenSubMenuId = subMenu->getInvokerItem()->getItemId();
    BaseMenuPresenter::onSubMenuSelected(subMenu);
    return true;
}

void IconMenuPresenter::updateMenuView(bool cleared) {
    IconMenuView* menuView = (IconMenuView*) mMenuView;
    if (mMaxItems < 0) mMaxItems = menuView->getMaxItems();
    std::vector<MenuItemImpl*> itemsToShow = mMenu->getNonActionItems();
    const bool needsMore = itemsToShow.size() > mMaxItems;
    BaseMenuPresenter::updateMenuView(cleared);

    if (needsMore && (mMoreView == nullptr || mMoreView->getParent() != menuView)) {
        if (mMoreView == nullptr) {
            mMoreView = menuView->createMoreItemView();
            mMoreView->setBackground(menuView->getItemBackgroundDrawable());
        }
        menuView->addView(mMoreView);
    } else if (!needsMore && mMoreView != nullptr) {
        menuView->removeView(mMoreView);
    }

    menuView->setNumActualItemsShown(needsMore ? mMaxItems - 1 : itemsToShow.size());
}

bool IconMenuPresenter::filterLeftoverView(ViewGroup* parent, int childIndex) {
    if (parent->getChildAt(childIndex) != mMoreView) {
        return BaseMenuPresenter::filterLeftoverView(parent, childIndex);
    }
    return false;
}

int IconMenuPresenter::getNumActualItemsShown() {
    return ((IconMenuView*) mMenuView)->getNumActualItemsShown();
}

void IconMenuPresenter::saveHierarchyState(Bundle& outState) {
    SparseArray<Parcelable*> viewStates;
    if (mMenuView != nullptr) {
        //((View*) mMenuView)->saveHierarchyState(viewStates);
    }
    //outState.putSparseParcelableArray(VIEWS_TAG, viewStates);
}

void IconMenuPresenter::restoreHierarchyState(Bundle& inState) {
    SparseArray<Parcelable*>* viewStates = nullptr;//inState.getSparseParcelableArray(VIEWS_TAG);
    if (viewStates != nullptr) {
        //((View*) mMenuView)->restoreHierarchyState(*viewStates);
    }
    const int subMenuId = 0;//inState.getInt(OPEN_SUBMENU_KEY, 0);
    if (subMenuId > 0 && mMenu != nullptr) {
        MenuItem* item = mMenu->findItem(subMenuId);
        if (item != nullptr) {
            onSubMenuSelected((SubMenuBuilder*)item->getSubMenu());
        }
    }
}

Parcelable* IconMenuPresenter::onSaveInstanceState() {
    if (mMenuView == nullptr||true) {
        return nullptr;
    }

    Bundle* state = new Bundle();
    saveHierarchyState(*state);
    if (mOpenSubMenuId > 0) {
        state->putInt(OPEN_SUBMENU_KEY, mOpenSubMenuId);
    }
    return nullptr;//state;
}

void IconMenuPresenter::onRestoreInstanceState(Parcelable& state) {
    //restoreHierarchyState(/*(Bundle)*/state);
}
}/*endof namespce*/
