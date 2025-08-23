#ifndef __ACTION_MENU_VIEW_H__
#define __ACTION_MENU_VIEW_H__
#include <menu/menuview.h>
#include <menu/menubuilder.h>
#include <menu/menupresenter.h>
#include <widget/linearlayout.h>
namespace cdroid{
class ActionMenuPresenter;
class ActionMenuView :public LinearLayout,public MenuView{//MenuBuilder.ItemInvoker, MenuView {
public:
    static constexpr int MIN_CELL_SIZE = 56; // dips
    static constexpr int GENERATED_ITEM_PADDING = 4; // dips

    class LayoutParams:public LinearLayout::LayoutParams {
    public:
        bool isOverflowButton;
        bool expandable;
        bool preventEdgeOffset;
        bool expanded;
        int cellsUsed;
        int extraPixels;
        LayoutParams(Context* c,const AttributeSet& attrs);
        LayoutParams(const ViewGroup::LayoutParams& other);
        LayoutParams(const LayoutParams& other);
        LayoutParams(int width, int height);
        LayoutParams(int width, int height, bool isOverflowButton);
    };

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
    int mFormatItemsWidth;
    int mMinCellSize;
    int mGeneratedItemPadding;

    bool mReserveOverflow;
    bool mFormatItems;
    ActionMenuPresenter* mPresenter;
    MenuPresenter::Callback mActionMenuPresenterCallback;
    MenuBuilder::Callback mMenuBuilderCallback;

    OnMenuItemClickListener mOnMenuItemClickListener;
private:
    void onMeasureExactFormat(int widthMeasureSpec, int heightMeasureSpec);
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int left, int top, int right, int bottom);
    LayoutParams* generateDefaultLayoutParams()const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    bool hasDividerBeforeChildAt(int childIndex)override;
public:
    ActionMenuView(Context* context,const AttributeSet& attrs);
    ~ActionMenuView()override;
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

    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    LayoutParams* generateOverflowButtonLayoutParams()const;

    virtual bool invokeItem(MenuItemImpl& item);

    int getWindowAnimations();

    void initialize(MenuBuilder* menu);
    Menu* getMenu();

    void setMenuCallbacks(const MenuPresenter::Callback& pcb, const MenuBuilder::Callback& mcb);

    MenuBuilder* peekMenu();
    bool showOverflowMenu();
    bool hideOverflowMenu();

    bool isOverflowMenuShowing()const;
    bool isOverflowMenuShowPending()const;

    void dismissPopupMenus();

    bool dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event)override;

    void setExpandedActionViewsExclusive(bool exclusive);
};/*endof ActionMenuView*/
}/*endof namespace*/
#endif/*__ACTION_MENU_VIEW_H__*/
