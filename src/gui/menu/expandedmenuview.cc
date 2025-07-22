#include <menu/expandedmenuview.h>
#include <menu/menubuilder.h>
namespace cdroid{

ExpandedMenuView::ExpandedMenuView(Context* context,const AttributeSet& attrs)
    :ListView(context, attrs){

    //TypedArray a = context.obtainStyledAttributes(attrs, com.android.internal.R.styleable.MenuView, 0, 0);
    //mAnimations = attrs.getResourceId(com.android.internal.R.styleable.MenuView_windowAnimationStyle, 0);
    auto onItemClock = std::bind(&ExpandedMenuView::onItemClick,this,std::placeholders::_1,
            std::placeholders::_2,std::placeholders::_3,std::placeholders::_4);
    setOnItemClickListener(onItemClock);
}

void ExpandedMenuView::initialize(MenuBuilder* menu) {
    mMenu = menu;
}

void ExpandedMenuView::onDetachedFromWindow(){
    ListView::onDetachedFromWindow();

    // Clear the cached bitmaps of children
    setChildrenDrawingCacheEnabled(false);
}

bool ExpandedMenuView::invokeItem(MenuItemImpl* item) {
    return mMenu->performItemAction((MenuItem*)item, 0);
}

void ExpandedMenuView::onItemClick(AdapterView& parent, View& v, int position, long id) {
    invokeItem((MenuItemImpl*) getAdapter()->getItem(position));
}

int ExpandedMenuView::getWindowAnimations() {
    return mAnimations;
}

}
