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
#include <widgetEx/navigationview/navigationmenupresenter.h>
#include <widgetEx/navigationview/navigationmenuitemview.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widget/internal_R.h>
#include <widget/textview.h>
#include <widget/framelayout.h>
#include <view/layoutinflater.h>
#include <menu/submenu.h>
#include <drawable/colordrawable.h>
#include <content/typedarray.h>
#include <porting/cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

namespace{

int dp(Context* context,int dps){
    return (int)(context->getDisplayMetrics().density * dps + 0.5f);
}

// design_navigation_item_subheader.xml theme references.
// Terminating 0 is mandatory: obtainStyledAttributes counts attrs until 0.
const uint32_t SUBHEADER_THEME_ATTRS[] = {
    R::attr::listPreferredItemHeightSmall,
    R::attr::listPreferredItemPaddingStart,
    R::attr::listPreferredItemPaddingEnd,
    R::attr::textColorSecondary,
    0,
};
constexpr int IDX_HEIGHT_SMALL   = 0;
constexpr int IDX_PADDING_START  = 1;
constexpr int IDX_PADDING_END    = 2;
constexpr int IDX_TEXT_COLOR     = 3;

}//namespace

// --- unified data model (material NavigationMenuItem variants) ---------------

/** Unified data model for all sorts of navigation menu items. */
class NavigationMenuItem{
public:
    virtual ~NavigationMenuItem() = default;
};

/** Normal or subheader items. */
class NavigationMenuTextItem:public NavigationMenuItem{
private:
    MenuItemImpl* mMenuItem;
public:
    bool needsEmptyIcon;
    NavigationMenuTextItem(MenuItemImpl* item):mMenuItem(item),needsEmptyIcon(false){}
    MenuItemImpl* getMenuItem()const{ return mMenuItem; }
};

/** Separator items. */
class NavigationMenuSeparatorItem:public NavigationMenuItem{
private:
    int mPaddingTop;
    int mPaddingBottom;
public:
    NavigationMenuSeparatorItem(int paddingTop,int paddingBottom)
        :mPaddingTop(paddingTop), mPaddingBottom(paddingBottom){}
    int getPaddingTop()const{ return mPaddingTop; }
    int getPaddingBottom()const{ return mPaddingBottom; }
};

/** Header (not subheader) items. The actual content is held by
    NavigationMenuPresenter#mHeaderLayout. */
class NavigationMenuHeaderItem:public NavigationMenuItem{};

// --- view holders (material inflates the design_navigation_* layouts) --------

class NavigationMenuNormalViewHolder:public RecyclerView::ViewHolder{
public:
    NavigationMenuNormalViewHolder(NavigationMenuPresenter* presenter, ViewGroup* parent)
        : ViewHolder(new NavigationMenuItemView(parent->getContext())){
        itemView->setOnClickListener(presenter->itemClickListener());
    }
};

class NavigationMenuSubheaderViewHolder:public RecyclerView::ViewHolder{
public:
    NavigationMenuSubheaderViewHolder(Context* context, ViewGroup* parent)
        : ViewHolder(createSubheaderView(context, parent)){
    }
private:
    // design_navigation_item_subheader.xml
    static TextView* createSubheaderView(Context* context, ViewGroup* parent){
        TextView* subHeader = new TextView(context, nullptr, 0);
        auto ta = context->obtainStyledAttributes(SUBHEADER_THEME_ATTRS);
        const int heightSmall  = ta->getDimensionPixelSize(IDX_HEIGHT_SMALL,  dp(context, 48));
        const int paddingStart = ta->getDimensionPixelSize(IDX_PADDING_START, dp(context, 16));
        const int paddingEnd   = ta->getDimensionPixelSize(IDX_PADDING_END,   dp(context, 16));
        subHeader->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, heightSmall));
        subHeader->setGravity(Gravity::CENTER_VERTICAL | Gravity::START);
        subHeader->setMaxLines(1);
        subHeader->setTextAppearance(R::style::TextAppearance_Material_Body2);
        const RefPtr<ColorStateList> text_color = ta->getColorStateList(IDX_TEXT_COLOR);
        if (text_color != nullptr) {
            subHeader->setTextColor(text_color);
        }
        subHeader->setPaddingRelative(paddingStart, 0, paddingEnd, 0);
        return subHeader;
    }
};

class NavigationMenuSeparatorViewHolder:public RecyclerView::ViewHolder{
public:
    NavigationMenuSeparatorViewHolder(Context* context, ViewGroup* parent)
        : ViewHolder(createSeparatorView(context, parent)){
    }
private:
    // design_navigation_item_separator.xml
    static FrameLayout* createSeparatorView(Context* context, ViewGroup* /*parent*/){
        FrameLayout* frame = new FrameLayout(context, nullptr, 0);
        frame->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));
        View* divider = new View(context, nullptr, 0);
        divider->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, dp(context, 1)));
        // View::setBackground owns (and deletes) the drawable it is given, so
        // hand it a private copy — never the shared cached instance that
        // Resources::getDrawable hands out.
        Drawable* dividerDrawable = nullptr;
        TypedValue value;
        if (context->getTheme().resolveAttribute(R::attr::listDivider, &value, true)
                && value.resourceId != 0) {
            Drawable* cached = context->getDrawable(value.resourceId);
            if (cached != nullptr) {
                std::shared_ptr<Drawable::ConstantState> state = cached->getConstantState();
                dividerDrawable = state != nullptr ? state->newDrawable() : cached;
            }
        }
        divider->setBackground(dividerDrawable != nullptr ? dividerDrawable
                                                          : new ColorDrawable(0));
        frame->addView(divider);
        return frame;
    }
};

// --- adapter ------------------------------------------------------------------

class NavigationMenuPresenter::NavigationMenuAdapter:public RecyclerView::Adapter{
private:
    static constexpr const char* STATE_CHECKED_ITEM = "android:menu:checked";
    static constexpr const char* STATE_ACTION_VIEWS = "android:menu:action_views";
    static constexpr int VIEW_TYPE_NORMAL = 0;
    static constexpr int VIEW_TYPE_SUBHEADER = 1;
    static constexpr int VIEW_TYPE_SEPARATOR = 2;
    static constexpr int VIEW_TYPE_HEADER = 3;
private:
    NavigationMenuPresenter* mPresenter;
    std::vector<NavigationMenuItem*> mItems;
    MenuItemImpl* mCheckedItem = nullptr;
    bool mUpdateSuspended = false;
public:
    explicit NavigationMenuAdapter(NavigationMenuPresenter* presenter)
        : mPresenter(presenter){
        prepareMenuItems();
    }
    ~NavigationMenuAdapter()override{
        for (NavigationMenuItem* item : mItems) delete item;
    }

    long getItemId(int position)override{ return position; }

    int getItemCount()override{ return mItems.size(); }

    int getItemViewType(int position)override{
        NavigationMenuItem* item = mItems.at(position);
        if (dynamic_cast<NavigationMenuSeparatorItem*>(item)) {
            return VIEW_TYPE_SEPARATOR;
        } else if (dynamic_cast<NavigationMenuHeaderItem*>(item)) {
            return VIEW_TYPE_HEADER;
        } else if (dynamic_cast<NavigationMenuTextItem*>(item)) {
            NavigationMenuTextItem* textItem = (NavigationMenuTextItem*)item;
            if (textItem->getMenuItem()->hasSubMenu()) {
                return VIEW_TYPE_SUBHEADER;
            } else {
                return VIEW_TYPE_NORMAL;
            }
        }
        throw std::runtime_error("Unknown item type."); // material throws likewise
    }

    RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType)override{
        switch (viewType) {
            case VIEW_TYPE_NORMAL:
                return new NavigationMenuNormalViewHolder(mPresenter, parent);
            case VIEW_TYPE_SUBHEADER:
                return new NavigationMenuSubheaderViewHolder(parent->getContext(), parent);
            case VIEW_TYPE_SEPARATOR:
                return new NavigationMenuSeparatorViewHolder(parent->getContext(), parent);
            case VIEW_TYPE_HEADER:
                return new RecyclerView::ViewHolder(mPresenter->mHeaderLayout);
        }
        return nullptr;
    }

    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position)override{
        switch (getItemViewType(position)) {
            case VIEW_TYPE_NORMAL: {
                NavigationMenuItemView* itemView = (NavigationMenuItemView*)holder.itemView;
                itemView->setIconTintList(mPresenter->mIconTintList);
                itemView->setTextAppearance(mPresenter->mTextAppearance);
                if (mPresenter->mTextColor != nullptr) {
                    itemView->setTextColor(mPresenter->mTextColor);
                }
                itemView->setBackground(mPresenter->mItemBackground != nullptr
                        ? mPresenter->mItemBackground->getConstantState()->newDrawable()
                        : nullptr);
                if (mPresenter->mItemForeground != nullptr) {
                    itemView->setForeground(
                            mPresenter->mItemForeground->getConstantState()->newDrawable());
                }
                NavigationMenuTextItem* item = (NavigationMenuTextItem*)mItems.at(position);
                itemView->setNeedsEmptyIcon(item->needsEmptyIcon);
                itemView->setPadding(mPresenter->mItemHorizontalPadding,
                        mPresenter->mItemVerticalPadding,
                        mPresenter->mItemHorizontalPadding,
                        mPresenter->mItemVerticalPadding);
                itemView->setIconPadding(mPresenter->mItemIconPadding);
                if (mPresenter->mHasCustomItemIconSize) {
                    itemView->setIconSize(mPresenter->mItemIconSize);
                }
                itemView->setMaxLines(mPresenter->mItemMaxLines);
                itemView->initialize(item->getMenuItem(),
                        /* isBold= */ mPresenter->mTextAppearanceActiveBoldEnabled);
                break;
            }
            case VIEW_TYPE_SUBHEADER: {
                TextView* subHeader = (TextView*)holder.itemView;
                NavigationMenuTextItem* item = (NavigationMenuTextItem*)mItems.at(position);
                subHeader->setText(item->getMenuItem()->getTitle());
                if (mPresenter->mSubheaderTextAppearance != NO_TEXT_APPEARANCE_SET) {
                    subHeader->setTextAppearance(mPresenter->mSubheaderTextAppearance);
                }
                subHeader->setPaddingRelative(mPresenter->mSubheaderInsetStart,
                        subHeader->getPaddingTop(),
                        mPresenter->mSubheaderInsetEnd,
                        subHeader->getPaddingBottom());
                if (mPresenter->mSubheaderColor != nullptr) {
                    subHeader->setTextColor(mPresenter->mSubheaderColor);
                }
                break;
            }
            case VIEW_TYPE_SEPARATOR: {
                NavigationMenuSeparatorItem* item =
                        (NavigationMenuSeparatorItem*)mItems.at(position);
                holder.itemView->setPaddingRelative(mPresenter->mDividerInsetStart,
                        item->getPaddingTop(),
                        mPresenter->mDividerInsetEnd,
                        item->getPaddingBottom());
                break;
            }
        }
    }

    void onViewRecycled(RecyclerView::ViewHolder& holder)override{
        NavigationMenuItemView* itemView = dynamic_cast<NavigationMenuItemView*>(holder.itemView);
        if (itemView != nullptr) {
            itemView->recycle();
        }
    }

    void update(){
        const int prevItemSize = mItems.size();
        prepareMenuItems();
        notifyDataSetChanged();
        // If there were no structural changes, update the items due to the
        // adapter having stable ids.
        if (prevItemSize == mItems.size()) {
            notifyItemRangeChanged(0, mItems.size());
        }
    }

    /**
     * Flattens the visible menu items of the presenter's menu into mItems,
     * while inserting separators between items when necessary.
     */
    void prepareMenuItems(){
        if (mUpdateSuspended) {
            return;
        }
        mUpdateSuspended = true;
        for (NavigationMenuItem* item : mItems) delete item;
        mItems.clear();
        mItems.push_back(new NavigationMenuHeaderItem());

        int currentGroupId = -1;
        int currentGroupStart = 0;
        bool currentGroupHasIcon = false;
        const std::vector<MenuItemImpl*> visibleItems = mPresenter->mMenu->getVisibleItems();
        for (size_t i = 0, totalSize = visibleItems.size(); i < totalSize; i++) {
            MenuItemImpl* item = visibleItems.at(i);
            if (item->isChecked()) {
                setCheckedItem(item);
            }
            if (item->isCheckable()) {
                item->setExclusiveCheckable(false);
            }
            if (item->hasSubMenu()) {
                SubMenu* subMenu = item->getSubMenu();
                if (subMenu->hasVisibleItems()) {
                    if (i != 0) {
                        mItems.push_back(new NavigationMenuSeparatorItem(
                                mPresenter->mPaddingSeparator, 0));
                    }
                    mItems.push_back(new NavigationMenuTextItem(item));
                    bool subMenuHasIcon = false;
                    const int subMenuStart = mItems.size();
                    for (int j = 0, size = subMenu->size(); j < size; j++) {
                        MenuItemImpl* subMenuItem = (MenuItemImpl*)subMenu->getItem(j);
                        if (subMenuItem->isVisible()) {
                            if (!subMenuHasIcon && subMenuItem->getIcon() != nullptr) {
                                subMenuHasIcon = true;
                            }
                            if (subMenuItem->isCheckable()) {
                                subMenuItem->setExclusiveCheckable(false);
                            }
                            if (subMenuItem->isChecked()) {
                                setCheckedItem(subMenuItem);
                            }
                            mItems.push_back(new NavigationMenuTextItem(subMenuItem));
                        }
                    }
                    if (subMenuHasIcon) {
                        appendTransparentIconIfMissing(subMenuStart, mItems.size());
                    }
                }
            } else {
                const int groupId = item->getGroupId();
                if (groupId != currentGroupId) { // first item in group
                    currentGroupStart = mItems.size();
                    currentGroupHasIcon = item->getIcon() != nullptr;
                    if (i != 0) {
                        currentGroupStart++;
                        mItems.push_back(new NavigationMenuSeparatorItem(
                                mPresenter->mPaddingSeparator, mPresenter->mPaddingSeparator));
                    }
                } else if (!currentGroupHasIcon && item->getIcon() != nullptr) {
                    currentGroupHasIcon = true;
                    appendTransparentIconIfMissing(currentGroupStart, mItems.size());
                }
                NavigationMenuTextItem* textItem = new NavigationMenuTextItem(item);
                textItem->needsEmptyIcon = currentGroupHasIcon;
                mItems.push_back(textItem);
                currentGroupId = groupId;
            }
        }
        mUpdateSuspended = false;
    }

    void appendTransparentIconIfMissing(int startIndex, int endIndex){
        for (int i = startIndex; i < endIndex; i++) {
            NavigationMenuTextItem* textItem = (NavigationMenuTextItem*)mItems.at(i);
            textItem->needsEmptyIcon = true;
        }
    }

    void setCheckedItem(MenuItemImpl* checkedItem){
        if (mCheckedItem == checkedItem || !checkedItem->isCheckable()) {
            return;
        }
        if (mCheckedItem != nullptr) {
            mCheckedItem->setChecked(false);
        }
        mCheckedItem = checkedItem;
        checkedItem->setChecked(true);
    }

    MenuItemImpl* getCheckedItem(){
        return mCheckedItem;
    }

    Bundle* createInstanceState(){
        Bundle* state = new Bundle();
        if (mCheckedItem != nullptr) {
            state->putInt(STATE_CHECKED_ITEM, mCheckedItem->getItemId());
        }
        // AOSP also stores every item action view's hierarchy state here
        // (STATE_ACTION_VIEWS); CDROID views cannot save their hierarchy state.
        return state;
    }

    void restoreInstanceState(Bundle& state){
        const int checkedItem = state.containsKey(STATE_CHECKED_ITEM)
                ? state.getInt(STATE_CHECKED_ITEM) : 0;
        if (checkedItem != 0) {
            mUpdateSuspended = true;
            for (size_t i = 0, size = mItems.size(); i < size; i++) {
                NavigationMenuItem* item = mItems.at(i);
                NavigationMenuTextItem* textItem = dynamic_cast<NavigationMenuTextItem*>(item);
                if (textItem != nullptr) {
                    MenuItemImpl* menuItem = textItem->getMenuItem();
                    if (menuItem != nullptr && menuItem->getItemId() == checkedItem) {
                        setCheckedItem(menuItem);
                        break;
                    }
                }
            }
            mUpdateSuspended = false;
            prepareMenuItems();
        }
    }

    void setUpdateSuspended(bool updateSuspended){
        mUpdateSuspended = updateSuspended;
    }

    /** Returns the number of rows that will be used for accessibility. */
    int getRowCount(){
        int itemCount = 0;
        for (size_t i = 0; i < mItems.size(); i++) {
            const int type = getItemViewType(i);
            if (type == VIEW_TYPE_NORMAL || type == VIEW_TYPE_SUBHEADER) {
                itemCount++;
            }
        }
        return itemCount;
    }

    void updateAllTextMenuItems(){
        for (size_t i = 0; i < mItems.size(); i++) {
            if (dynamic_cast<NavigationMenuTextItem*>(mItems.at(i))
                    && getItemViewType(i) == VIEW_TYPE_NORMAL) {
                notifyItemChanged(i);
            }
        }
    }

    void updateAllSubHeaderMenuItems(){
        for (size_t i = 0; i < mItems.size(); i++) {
            if (dynamic_cast<NavigationMenuTextItem*>(mItems.at(i))
                    && getItemViewType(i) == VIEW_TYPE_SUBHEADER) {
                notifyItemChanged(i);
            }
        }
    }

    void updateAllDividerMenuItems(){
        for (size_t i = 0; i < mItems.size(); i++) {
            if (dynamic_cast<NavigationMenuSeparatorItem*>(mItems.at(i))) {
                notifyItemChanged(i);
            }
        }
    }
};

// --- presenter ----------------------------------------------------------------

NavigationMenuPresenter::~NavigationMenuPresenter(){
    delete mAdapter;
}

void NavigationMenuPresenter::initForMenu(Context* context, MenuBuilder* menu){
    mLayoutInflater = LayoutInflater::from(context);
    mMenu = menu;
    // design_navigation_separator_vertical_padding
    mPaddingSeparator = dp(context, 8);
}

NavigationMenuView* NavigationMenuPresenter::getMenuView(ViewGroup* root){
    (void)root;
    if (mMenuView == nullptr) {
        // design_navigation_menu.xml
        mMenuView = new NavigationMenuView(mLayoutInflater->getContext());
        mMenuView->setLayoutManager(
                new LinearLayoutManager(mMenuView->getContext(), LinearLayoutManager::VERTICAL, false));
        mMenuView->setPadding(0, 0, 0, dp(mMenuView->getContext(), 8)); // design_navigation_padding_bottom
        if (mAdapter == nullptr) {
            mAdapter = new NavigationMenuAdapter(this);
            // Prevent recreating all the Views when notifyDataSetChanged() is
            // called causing issues with the a11y reader (see b/112931425)
            mAdapter->setHasStableIds(true);
        }
        if (mOverScrollMode != -1) {
            mMenuView->setOverScrollMode(mOverScrollMode);
        }
        // design_navigation_item_header.xml
        mHeaderLayout = new LinearLayout(mMenuView->getContext(), nullptr, 0);
        mHeaderLayout->setOrientation(LinearLayout::VERTICAL);
        mHeaderLayout->setPadding(0, 0, 0, mPaddingSeparator);
        mMenuView->setAdapter(mAdapter);
    }
    return mMenuView;
}

void NavigationMenuPresenter::updateAllTextMenuItems(){
    if (mAdapter != nullptr) {
        mAdapter->updateAllTextMenuItems();
    }
}

void NavigationMenuPresenter::updateAllSubHeaderMenuItems(){
    if (mAdapter != nullptr) {
        mAdapter->updateAllSubHeaderMenuItems();
    }
}

void NavigationMenuPresenter::updateAllDividerMenuItems(){
    if (mAdapter != nullptr) {
        mAdapter->updateAllDividerMenuItems();
    }
}

void NavigationMenuPresenter::updateMenuView(bool cleared){
    (void)cleared;
    if (mAdapter != nullptr) {
        mAdapter->update();
    }
}

void NavigationMenuPresenter::setCallback(const Callback& cb){
    mCallback = cb;
}

bool NavigationMenuPresenter::onSubMenuSelected(SubMenuBuilder* subMenu){
    (void)subMenu;
    return false;
}

void NavigationMenuPresenter::onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing){
    if (mCallback.onCloseMenu) {
        mCallback.onCloseMenu(*menu, allMenusAreClosing);
    }
}

bool NavigationMenuPresenter::flagActionItems(){
    return false;
}

bool NavigationMenuPresenter::expandItemActionView(MenuBuilder& menu, MenuItemImpl& item){
    (void)menu; (void)item;
    return false;
}

bool NavigationMenuPresenter::collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item){
    (void)menu; (void)item;
    return false;
}

int NavigationMenuPresenter::getId()const{
    return mId;
}

void NavigationMenuPresenter::setId(int id){
    mId = id;
}

Parcelable* NavigationMenuPresenter::onSaveInstanceState(){
    NavigationMenuPresenterState* state = new NavigationMenuPresenterState();
    if (mAdapter != nullptr) {
        Bundle* adapterState = mAdapter->createInstanceState();
        state->menuState = std::move(*adapterState);
        delete adapterState;
    }
    return state;
}

void NavigationMenuPresenter::onRestoreInstanceState(Parcelable& parcelable){
    NavigationMenuPresenterState* state =
            dynamic_cast<NavigationMenuPresenterState*>(&parcelable);
    if (state != nullptr && mAdapter != nullptr) {
        mAdapter->restoreInstanceState(state->menuState);
    }
}

void NavigationMenuPresenter::setCheckedItem(MenuItemImpl* item){
    mAdapter->setCheckedItem(item);
}

MenuItemImpl* NavigationMenuPresenter::getCheckedItem(){
    return mAdapter->getCheckedItem();
}

View* NavigationMenuPresenter::inflateHeaderView(int res){
    View* view = mLayoutInflater->inflate(res, mHeaderLayout, false);
    addHeaderView(view);
    return view;
}

void NavigationMenuPresenter::addHeaderView(View* view){
    mHeaderLayout->addView(view);
    // The padding on top should be cleared.
    mMenuView->setPadding(0, 0, 0, mMenuView->getPaddingBottom());
}

void NavigationMenuPresenter::removeHeaderView(View* view){
    mHeaderLayout->removeView(view);
    if (!hasHeader()) {
        mMenuView->setPadding(0, mPaddingTopDefault, 0, mMenuView->getPaddingBottom());
    }
}

int NavigationMenuPresenter::getHeaderCount(){
    return mHeaderLayout->getChildCount();
}

bool NavigationMenuPresenter::hasHeader()const{
    return mHeaderLayout != nullptr && mHeaderLayout->getChildCount() > 0;
}

View* NavigationMenuPresenter::getHeaderView(int index){
    return mHeaderLayout->getChildAt(index);
}

void NavigationMenuPresenter::setSubheaderColor(const RefPtr<ColorStateList>& subheaderColor){
    mSubheaderColor = subheaderColor;
    updateAllSubHeaderMenuItems();
}

void NavigationMenuPresenter::setSubheaderTextAppearance(int resId){
    mSubheaderTextAppearance = resId;
    updateAllSubHeaderMenuItems();
}

const RefPtr<ColorStateList> NavigationMenuPresenter::getItemTintList()const{
    return mIconTintList;
}

void NavigationMenuPresenter::setItemIconTintList(const RefPtr<ColorStateList>& tint){
    mIconTintList = tint;
    updateAllTextMenuItems();
}

const RefPtr<ColorStateList> NavigationMenuPresenter::getItemTextColor()const{
    return mTextColor;
}

void NavigationMenuPresenter::setItemTextColor(const RefPtr<ColorStateList>& textColor){
    mTextColor = textColor;
    updateAllTextMenuItems();
}

void NavigationMenuPresenter::setItemTextAppearance(int resId){
    mTextAppearance = resId;
    updateAllTextMenuItems();
}

void NavigationMenuPresenter::setItemTextAppearanceActiveBoldEnabled(bool isBold){
    mTextAppearanceActiveBoldEnabled = isBold;
    updateAllTextMenuItems();
}

Drawable* NavigationMenuPresenter::getItemBackground()const{
    return mItemBackground;
}

void NavigationMenuPresenter::setItemBackground(Drawable* itemBackground){
    mItemBackground = itemBackground;
    updateAllTextMenuItems();
}

void NavigationMenuPresenter::setItemForeground(Drawable* itemForeground){
    mItemForeground = itemForeground;
    updateAllTextMenuItems();
}

int NavigationMenuPresenter::getItemHorizontalPadding()const{
    return mItemHorizontalPadding;
}

void NavigationMenuPresenter::setItemHorizontalPadding(int itemHorizontalPadding){
    mItemHorizontalPadding = itemHorizontalPadding;
    updateAllTextMenuItems();
}

int NavigationMenuPresenter::getItemVerticalPadding()const{
    return mItemVerticalPadding;
}

void NavigationMenuPresenter::setItemVerticalPadding(int itemVerticalPadding){
    mItemVerticalPadding = itemVerticalPadding;
    updateAllTextMenuItems();
}

int NavigationMenuPresenter::getDividerInsetStart()const{
    return mDividerInsetStart;
}

void NavigationMenuPresenter::setDividerInsetStart(int dividerInsetStart){
    mDividerInsetStart = dividerInsetStart;
    updateAllDividerMenuItems();
}

int NavigationMenuPresenter::getDividerInsetEnd()const{
    return mDividerInsetEnd;
}

void NavigationMenuPresenter::setDividerInsetEnd(int dividerInsetEnd){
    mDividerInsetEnd = dividerInsetEnd;
    updateAllDividerMenuItems();
}

int NavigationMenuPresenter::getSubheaderInsetStart()const{
    return mSubheaderInsetStart;
}

void NavigationMenuPresenter::setSubheaderInsetStart(int subheaderInsetStart){
    mSubheaderInsetStart = subheaderInsetStart;
    updateAllSubHeaderMenuItems();
}

int NavigationMenuPresenter::getSubheaderInsetEnd()const{
    return mSubheaderInsetEnd;
}

void NavigationMenuPresenter::setSubheaderInsetEnd(int subheaderInsetEnd){
    mSubheaderInsetEnd = subheaderInsetEnd;
    updateAllSubHeaderMenuItems();
}

int NavigationMenuPresenter::getItemIconPadding()const{
    return mItemIconPadding;
}

void NavigationMenuPresenter::setItemIconPadding(int itemIconPadding){
    mItemIconPadding = itemIconPadding;
    updateAllTextMenuItems();
}

void NavigationMenuPresenter::setItemMaxLines(int itemMaxLines){
    mItemMaxLines = itemMaxLines;
    updateAllTextMenuItems();
}

int NavigationMenuPresenter::getItemMaxLines()const{
    return mItemMaxLines;
}

void NavigationMenuPresenter::setItemIconSize(int itemIconSize){
    if (mItemIconSize != itemIconSize) {
        mItemIconSize = itemIconSize;
        mHasCustomItemIconSize = true;
        updateAllTextMenuItems();
    }
}

void NavigationMenuPresenter::setUpdateSuspended(bool updateSuspended){
    if (mAdapter != nullptr) {
        mAdapter->setUpdateSuspended(updateSuspended);
    }
}

void NavigationMenuPresenter::setBehindStatusBar(bool behindStatusBar){
    if (mIsBehindStatusBar != behindStatusBar) {
        mIsBehindStatusBar = behindStatusBar;
        updateTopPadding();
    }
}

bool NavigationMenuPresenter::isBehindStatusBar()const{
    return mIsBehindStatusBar;
}

void NavigationMenuPresenter::updateTopPadding(){
    if (mMenuView == nullptr) return;
    int topPadding = 0;
    // Set padding if there's no header and we are drawing behind the status bar.
    if (!hasHeader() && mIsBehindStatusBar) {
        topPadding = mPaddingTopDefault;
    }
    mMenuView->setPadding(0, topPadding, 0, mMenuView->getPaddingBottom());
}

void NavigationMenuPresenter::setOverScrollMode(int overScrollMode){
    mOverScrollMode = overScrollMode;
    if (mMenuView != nullptr) {
        mMenuView->setOverScrollMode(overScrollMode);
    }
}

// The material presenter holds this as a field; CDROID hands the lambda out to
// the item views (kept a method so the field stays one allocation).
const std::function<void(View&)>& NavigationMenuPresenter::itemClickListener(){
    if (!mOnClickListener) {
        mOnClickListener = [this](View& view){
            NavigationMenuItemView* itemView = (NavigationMenuItemView*)&view;
            setUpdateSuspended(true);
            MenuItemImpl* item = itemView->getItemData();
            const bool result = mMenu->performItemAction(item, this, 0);
            bool checkStateChanged = false;
            if (item != nullptr && item->isCheckable() && result) {
                mAdapter->setCheckedItem(item);
                checkStateChanged = true;
            }
            setUpdateSuspended(false);
            if (checkStateChanged) {
                updateMenuView(false);
            }
        };
    }
    return mOnClickListener;
}

}//namespace cdroid
