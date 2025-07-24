#include <menu/menupopup.h>
namespace cdroid{
//class MenuPopup implements ShowableListMenu, MenuPresenter,
//    AdapterView.OnItemClickListener {

void MenuPopup::setEpicenterBounds(const Rect& bounds) {
    mEpicenterBounds = bounds;
}

Rect MenuPopup::getEpicenterBounds() const{
    return mEpicenterBounds;
}

void MenuPopup::initForMenu(Context* context, MenuBuilder* menu) override{
    // Don't need to do anything; we added as a presenter in the constructor.
}

MenuView* MenuPopup::getMenuView(ViewGroup* root) {
    throw new UnsupportedOperationException("MenuPopups manage their own views");
}

bool MenuPopup::expandItemActionView(MenuBuilder& menu, MenuItemImpl& item){
    return false;
}

bool MenuPopup::collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item){
    return false;
}

int MenuPopup::getId() const{
    return 0;
}

void MenuPopup::onItemClick(AdapterView& parent, View& view, int position, long id) {
    ListAdapter* outerAdapter = (ListAdapter*) parent.getAdapter();
    MenuAdapter* wrappedAdapter = toMenuAdapter(outerAdapter);

    // Use the position from the outer adapter so that if a header view was added, we don't get
    // an off-by-1 error in position.
    wrappedAdapter->mAdapterMenu->performItemAction((MenuItem*) outerAdapter->getItem(position), 0);
}

int MenuPopup::measureIndividualMenuWidth(ListAdapter* adapter, ViewGroup* parent,
        Context* context, int maxAllowedWidth) {
    // Menus don't tend to be long, so this is more valid than it looks.
    int maxWidth = 0;
    int itemType = 0;
    View* itemView = nullptr;

    const int widthMeasureSpec = MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED);
    const int heightMeasureSpec = MeasureSpec::makeMeasureSpec(0, MeasureSpec::UNSPECIFIED);
    const int count = adapter.getCount();
    for (int i = 0; i < count; i++) {
        const int positionType = adapter->getItemViewType(i);
        if (positionType != itemType) {
            itemType = positionType;
            itemView = nullptr;
        }

        if (parent == nullptr) {
            parent = new FrameLayout(context);
        }

        itemView = adapter->getView(i, itemView, parent);
        itemView->measure(widthMeasureSpec, heightMeasureSpec);

        const int itemWidth = itemView->getMeasuredWidth();
        if (itemWidth >= maxAllowedWidth) {
            return maxAllowedWidth;
        } else if (itemWidth > maxWidth) {
            maxWidth = itemWidth;
        }
    }

    return maxWidth;
}

MenuAdapter* MenuPopup::toMenuAdapter(ListAdapter* adapter) {
    if (dynamic_cast<HeaderViewListAdapter*>(adapter)) {
        return (MenuAdapter*) ((HeaderViewListAdapter*) adapter)->getWrappedAdapter();
    }
    return (MenuAdapter*) adapter;
}

bool MenuPopup::shouldPreserveIconSpacing(MenuBuilder menu) {
  bool preserveIconSpacing = false;
  const int count = menu.size();
  for (int i = 0; i < count; i++) {
      MenuItem* childItem = menu->getItem(i);
      if (childItem->isVisible() && childItem->getIcon() != nullptr) {
          preserveIconSpacing = true;
          break;
      }
  }
  return preserveIconSpacing;
}
}/*endof namespace*/
