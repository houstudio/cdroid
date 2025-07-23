#ifndef __LIST_MENUITEM_VIEW_H__
#define __LIST_MENUITEM_VIEW_H__
#include <menu/menuview.h>
#include <widget/linearlayout.h>
namespace cdroid{
class TextView;
class CheckBox;
class ImageView;
class RadioButton;
class ListMenuItemView:public LinearLayout,public MenuView::ItemView{//, AbsListView::SelectionBoundsAdjuster {
private:
    MenuItemImpl* mItemData;
    ImageView* mIconView;
    RadioButton* mRadioButton;
    TextView* mTitleView;
    CheckBox* mCheckBox;
    TextView* mShortcutView;
    ImageView* mSubMenuArrowView;
    ImageView* mGroupDivider;
    LinearLayout* mContent;

    Drawable* mBackground;
    std::string mTextAppearance;
    int mMenuType;
    Context* mTextAppearanceContext;
    Drawable* mSubMenuArrow;
    LayoutInflater* mInflater;
    bool mPreserveIconSpacing;
    bool mHasListDivider;
    bool mForceShowIcon;
private:
    void addContentView(View* v);
    void addContentView(View* v, int index);
    void setSubMenuArrowVisible(bool hasSubmenu);
    void insertIconView();
    void insertRadioButton();
    void insertCheckBox();
    LayoutInflater* getInflater();
protected:
    void onFinishInflate() override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
public:
    ListMenuItemView(Context* context,const AttributeSet& attrs);

    void initialize(MenuItemImpl* itemData, int menuType);

    void setForceShowIcon(bool forceShow);

    void setTitle(const std::string& title);

    MenuItemImpl* getItemData();
    void setEnabled(bool)override;
    void setCheckable(bool checkable);

    void setChecked(bool checked);

    void setShortcut(bool showShortcut, int shortcutKey);

    void setIcon(Drawable* icon);

    bool prefersCondensedTitle();

    bool showsIcon();

    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    /**
     * Enable or disable group dividers for this view.
     */
    void setGroupDividerEnabled(bool groupDividerEnabled);
    void adjustListItemSelectionBounds(Rect& rect);
};
}/*endof namespace*/
#endif/*__LIST_MENUITEM_VIEW_H__*/
