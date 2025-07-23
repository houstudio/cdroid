#ifndef __LIST_MENU_PRESENTER_H__
#define __LIST_MENU_PRESENTER_H__
#include <menu/menupresenter.h>
#include <widget/adapter.h>
#include <widget/adapterview.h>
namespace cdroid{
class MenuBuilder;
class ExpandedMenuView;
class ListMenuPresenter:public MenuPresenter{//, AdapterView.OnItemClickListener {
protected:
    class MenuAdapter;
    friend MenuAdapter;
    friend class MenuPopupWindow;
    Context* mContext;
    LayoutInflater* mInflater;
    MenuBuilder* mMenu;
    ExpandedMenuView* mMenuView;
    int mThemeRes;
    std::string mItemLayoutRes;
    MenuAdapter* mAdapter;
private:
    int mItemIndexOffset;
    Callback mCallback;
    int mId;
public:
    static constexpr const char* VIEWS_TAG = "android:menu:list";

    ListMenuPresenter(Context* context,const std::string& itemLayoutRes);
    ListMenuPresenter(const std::string& itemLayoutRes, int themeRes);

    void initForMenu(Context* context,MenuBuilder* menu)override;

    MenuView* getMenuView(ViewGroup* root)override;

    ListAdapter* getAdapter();
    void updateMenuView(bool cleared)override;

    void setCallback(const Callback& cb)override;

    bool onSubMenuSelected(SubMenuBuilder* subMenu)override;
    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)override;

    int getItemIndexOffset();
    void setItemIndexOffset(int offset);

    void onItemClick(AdapterView& parent, View& view, int position, long id);

    bool flagActionItems() override;

    bool expandItemActionView(MenuBuilder& menu, MenuItemImpl& item);
    bool collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item);

    void saveHierarchyState(Bundle& outState);
    void restoreHierarchyState(Bundle& inState);

    void setId(int id);
    int getId()const override;

    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable& state)override;
};
class ListMenuPresenter::MenuAdapter:public BaseAdapter {
private:
    ListMenuPresenter*mPresenter;
    int mExpandedIndex = -1;
public:
    MenuAdapter(ListMenuPresenter*presenter);
    int getCount()const override;
    MenuBuilder*getAdapterMenu();
    void* getItem(int position)const override;
    long getItemId(int position)const override;
    View* getView(int position, View* convertView, ViewGroup* parent)override;
    void findExpandedIndex();
    void notifyDataSetChanged() override;
};
}/*endof namespace*/
#endif/*__LIST_MENU_PRESENTER_H__*/
