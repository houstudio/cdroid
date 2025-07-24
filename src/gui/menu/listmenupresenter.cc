#include <view/layoutinflater.h>
#include <menu/menubuilder.h>
#include <menu/submenubuilder.h>
#include <menu/listmenupresenter.h>
#include <menu/expandedmenuview.h>
namespace cdroid{

ListMenuPresenter::ListMenuPresenter(Context* context, const std::string& itemLayoutRes)
    :ListMenuPresenter(itemLayoutRes, 0){
    mContext = context;
    mInflater = LayoutInflater::from(mContext);
}

ListMenuPresenter::ListMenuPresenter(const std::string& itemLayoutRes, int themeRes) {
    mItemLayoutRes = itemLayoutRes;
    mThemeRes = themeRes;
}

void ListMenuPresenter::initForMenu(Context* context,MenuBuilder* menu) {
    if (mThemeRes != 0) {
        mContext = context;//new ContextThemeWrapper(context, mThemeRes);
        mInflater = LayoutInflater::from(mContext);
    } else if (mContext != nullptr) {
        mContext = context;
        if (mInflater == nullptr) {
            mInflater = LayoutInflater::from(mContext);
        }
    }
    mMenu = menu;
    if (mAdapter != nullptr) {
        mAdapter->notifyDataSetChanged();
    }
}

MenuView* ListMenuPresenter::getMenuView(ViewGroup* root) {
    if (mMenuView == nullptr) {
        mMenuView = (ExpandedMenuView*) mInflater->inflate("cdrroid:layout/expanded_menu_layout", root, false);
        if (mAdapter == nullptr) {
            mAdapter = new MenuAdapter(this);
        }
        mMenuView->setAdapter(mAdapter);
        auto onClick = std::bind(&ListMenuPresenter::onItemClick,this, std::placeholders::_1,
                std::placeholders::_2,std::placeholders::_3,std::placeholders::_4);
        mMenuView->setOnItemClickListener(onClick);
    }
    return mMenuView;
}

ListAdapter* ListMenuPresenter::getAdapter() {
    if (mAdapter == nullptr) {
        mAdapter = new MenuAdapter(this);
    }
    return mAdapter;
}

void ListMenuPresenter::updateMenuView(bool cleared) {
    if (mAdapter != nullptr) mAdapter->notifyDataSetChanged();
}

void ListMenuPresenter::setCallback(const Callback& cb) {
    mCallback = cb;
}

bool ListMenuPresenter::onSubMenuSelected(SubMenuBuilder* subMenu) {
    if (!subMenu->hasVisibleItems()) return false;

    // The window manager will give us a token.
    //new MenuDialogHelper(subMenu).show(nullptr);
    if (mCallback.onOpenSubMenu != nullptr) {
        mCallback.onOpenSubMenu(*subMenu);
    }
    return true;
}

void ListMenuPresenter::onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing) {
    if (mCallback.onCloseMenu != nullptr) {
        mCallback.onCloseMenu(*menu, allMenusAreClosing);
    }
}

int ListMenuPresenter::getItemIndexOffset() {
    return mItemIndexOffset;
}

void ListMenuPresenter::setItemIndexOffset(int offset) {
    mItemIndexOffset = offset;
    if (mMenuView != nullptr) {
        updateMenuView(false);
    }
}

void ListMenuPresenter::onItemClick(AdapterView& parent, View& view, int position, long id) {
    mMenu->performItemAction((MenuItemImpl*)mAdapter->getItem(position), this, 0);
}

bool ListMenuPresenter::flagActionItems() {
    return false;
}

bool ListMenuPresenter::expandItemActionView(MenuBuilder& menu, MenuItemImpl& item) {
    return false;
}

bool ListMenuPresenter::collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item) {
    return false;
}

void ListMenuPresenter::saveHierarchyState(Bundle& outState) {
    SparseArray<Parcelable*> viewStates;// = new SparseArray<Parcelable>();
    if (mMenuView != nullptr) {
        //((View*) mMenuView)->saveHierarchyState(viewStates);
    }
    //outState.putSparseParcelableArray(VIEWS_TAG, viewStates);
}

void ListMenuPresenter::restoreHierarchyState(Bundle& inState) {
    SparseArray<Parcelable*>* viewStates = nullptr;//inState.getSparseParcelableArray(VIEWS_TAG);
    if (viewStates != nullptr) {
        //((View*) mMenuView)->restoreHierarchyState(viewStates);
    }
}

void ListMenuPresenter::setId(int id) {
    mId = id;
}

int ListMenuPresenter::getId() const{
    return mId;
}

Parcelable* ListMenuPresenter::onSaveInstanceState() {
    if (mMenuView == nullptr) {
        return nullptr;
    }

    Bundle* state = new Bundle();
    saveHierarchyState(*state);
    return nullptr;//state;
}

void ListMenuPresenter::onRestoreInstanceState(Parcelable& state) {
    restoreHierarchyState((Bundle&) state);
}

///////////////////////////////////////////////////////////////////////////////////////
ListMenuPresenter::MenuAdapter::MenuAdapter(ListMenuPresenter*presenter)
    :mPresenter(presenter) {
    findExpandedIndex();
}

int ListMenuPresenter::MenuAdapter::getCount() const{
    std::vector<MenuItemImpl*> items = mPresenter->mMenu->getNonActionItems();
    int count = items.size() - mPresenter->mItemIndexOffset;
    if (mExpandedIndex < 0) {
        return count;
    }
    return count - 1;
}

MenuBuilder*ListMenuPresenter::MenuAdapter::getAdapterMenu(){
    return nullptr;
}

void* ListMenuPresenter::MenuAdapter::getItem(int position)const{
    std::vector<MenuItemImpl*> items = mPresenter->mMenu->getNonActionItems();
    position += mPresenter->mItemIndexOffset;
    if (mExpandedIndex >= 0 && position >= mExpandedIndex) {
        position++;
    }
    return items.at(position);
}

long ListMenuPresenter::MenuAdapter::getItemId(int position) const{
    // Since a menu item's ID is optional, we'll use the position as an
    // ID for the item in the AdapterView
    return position;
}

View* ListMenuPresenter::MenuAdapter::getView(int position, View* convertView, ViewGroup* parent) {
    if (convertView == nullptr) {
        convertView = mPresenter->mInflater->inflate(mPresenter->mItemLayoutRes, parent, false);
    }

    MenuView::ItemView* itemView = (MenuView::ItemView*) convertView;
    itemView->initialize((MenuItemImpl*)getItem(position), 0);
    return convertView;
}

void ListMenuPresenter::MenuAdapter::findExpandedIndex() {
    const MenuItemImpl* expandedItem = mPresenter->mMenu->getExpandedItem();
    if (expandedItem != nullptr) {
        std::vector<MenuItemImpl*> items = mPresenter->mMenu->getNonActionItems();
        const int count = items.size();
        for (int i = 0; i < count; i++) {
            MenuItemImpl* item = items.at(i);
            if (item == expandedItem) {
                mExpandedIndex = i;
                return;
            }
        }
    }
    mExpandedIndex = -1;
}

void ListMenuPresenter::MenuAdapter::notifyDataSetChanged() {
    findExpandedIndex();
    BaseAdapter::notifyDataSetChanged();
}
}
