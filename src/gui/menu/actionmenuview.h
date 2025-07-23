#ifndef __ACTION_MENU_VIEW_H__
#define __ACTION_MENU_VIEW_H__
#include <menu/menuview.h>
#include <menu/menupresenter.h>
#include <widget/linearlayout.h>
namespace cdroid{
class ActionMenuPresenter;
class ActionMenuView :public LinearLayout,public MenuView{//MenuBuilder.ItemInvoker, MenuView {
public:
    static constexpr int MIN_CELL_SIZE = 56; // dips
    static constexpr int GENERATED_ITEM_PADDING = 4; // dips
    class LayoutParams;
    struct ActionMenuChildView {
        virtual bool needsDividerBefore()=0;
        virtual bool needsDividerAfter()=0;
    };
    DECLARE_UIEVENT(bool,OnMenuItemClickListener,MenuItem&);
private:
    friend ActionMenuPresenter;
    MenuBuilder* mMenu;
    /** Context against which to inflate popup menus. */
    Context* mPopupContext;

    /** Theme resource against which to inflate popup menus. */
    int mPopupTheme;

    bool mReserveOverflow;
    ActionMenuPresenter* mPresenter;
    MenuPresenter::Callback mActionMenuPresenterCallback;
    MenuBuilder::Callback mMenuBuilderCallback;
    bool mFormatItems;
    int mFormatItemsWidth;
    int mMinCellSize;
    int mGeneratedItemPadding;

    OnMenuItemClickListener mOnMenuItemClickListener;
private:
    void onMeasureExactFormat(int widthMeasureSpec, int heightMeasureSpec);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int left, int top, int right, int bottom);
    ViewGroup::LayoutParams* generateDefaultLayoutParams()const override;
    ViewGroup::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    bool hasDividerBeforeChildAt(int childIndex)override;
public:
    ActionMenuView(Context* context, AttributeSet& attrs);

    void setPopupTheme(int resId);
    int getPopupTheme();
    void setPresenter(ActionMenuPresenter* presenter);

    //void onConfigurationChanged(Configuration newConfig) override;

    void setOnMenuItemClickListener(const OnMenuItemClickListener& listener);

    static int measureChildForCells(View* child, int cellSize, int cellsRemaining,
            int parentHeightMeasureSpec, int parentHeightPadding);

    void onDetachedFromWindow() override;

    void setOverflowIcon(Drawable* icon);
    Drawable* getOverflowIcon();

    bool isOverflowReserved();
    void setOverflowReserved(bool reserveOverflow);

    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    LayoutParams* generateOverflowButtonLayoutParams();

    virtual bool invokeItem(MenuItemImpl& item);

    int getWindowAnimations();

    void initialize(MenuBuilder* menu);
    Menu* getMenu();

    void setMenuCallbacks(const MenuPresenter::Callback& pcb, const MenuBuilder::Callback& mcb);

    MenuBuilder* peekMenu();
    bool showOverflowMenu();
    bool hideOverflowMenu();

    bool isOverflowMenuShowing();
    bool isOverflowMenuShowPending();

    void dismissPopupMenus();

    bool dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event);

    void setExpandedActionViewsExclusive(bool exclusive);
#if 0
    private class MenuBuilderCallback implements MenuBuilder.Callback {
        @Override
        public bool onMenuItemSelected(MenuBuilder menu, MenuItem item) {
            return mOnMenuItemClickListener != null &&
                    mOnMenuItemClickListener.onMenuItemClick(item);
        }

        @Override
        public void onMenuModeChange(MenuBuilder menu) {
            if (mMenuBuilderCallback != null) {
                mMenuBuilderCallback.onMenuModeChange(menu);
            }
        }
    };

    private class ActionMenuPresenterCallback implements ActionMenuPresenter.Callback {
        @Override
        public void onCloseMenu(MenuBuilder menu, bool allMenusAreClosing) {
        }

        @Override
        public bool onOpenSubMenu(MenuBuilder subMenu) {
            return false;
        }
    };
#endif
};/*endof ActionMenuView*/
class ActionMenuView::LayoutParams:public LinearLayout::LayoutParams {
public:
    bool isOverflowButton;
    bool expandable;
    bool preventEdgeOffset;
    bool expanded;
    int cellsUsed;
    int extraPixels;
    LayoutParams(Context* c,const AttributeSet& attrs):LinearLayout::LayoutParams(c, attrs){
    }

    LayoutParams(const ViewGroup::LayoutParams& other):LinearLayout::LayoutParams(other){
    }

    LayoutParams(const LayoutParams& other)
        :LinearLayout::LayoutParams((const LinearLayout::LayoutParams&) other){
        isOverflowButton = other.isOverflowButton;
    }

    LayoutParams(int width, int height):LinearLayout::LayoutParams(width, height){
        isOverflowButton = false;
    }

    LayoutParams(int width, int height, bool isOverflowButton):LinearLayout::LayoutParams(width, height){
        this->isOverflowButton = isOverflowButton;
    }
};
}/*endof namespace*/
#endif/*__ACTION_MENU_VIEW_H__*/
