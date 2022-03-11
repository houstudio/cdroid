#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__
#include <widget/textview.h>
#include <widget/imageview.h>
#include <widget/imagebutton.h>
#include <view/viewgroup.h>
#include <widget/actionbar.h>
namespace cdroid{
typedef View ActionMenuView;
class Menu;
class MenuInflater;
class ToolBar:public ViewGroup{
public:
    class LayoutParams:public ActionBar::LayoutParams{
    public:
        static constexpr int CUSTOM = 0;
        static constexpr int SYSTEM = 1;
        static constexpr int EXPANDED = 2;
        int mViewType = CUSTOM;
    public:
        LayoutParams(Context* c,const AttributeSet& attrs);
        LayoutParams(int width, int height);
        LayoutParams(int width, int height, int gravity);
        LayoutParams(int gravity);
        LayoutParams(const LayoutParams& source);
        LayoutParams(const ActionBar::LayoutParams& source);
        LayoutParams(const MarginLayoutParams& source);
        LayoutParams(const ViewGroup::LayoutParams& source);
    };
private:
    ActionMenuView* mMenuView;
    TextView* mTitleTextView;
    TextView* mSubtitleTextView;
    ImageButton* mNavButtonView;
    ImageView* mLogoView;

    Drawable* mCollapseIcon;
    std::string mCollapseDescription;
    ImageButton* mCollapseButtonView;
    View*mExpandedActionView;

    int mPopupTheme;

    int mTitleTextAppearance;
    int mSubtitleTextAppearance;
    int mNavButtonStyle;

    int mButtonGravity;

    int mMaxButtonHeight;

    int mTitleMarginStart;
    int mTitleMarginEnd;
    int mTitleMarginTop;
    int mTitleMarginBottom;

    //RtlSpacingHelper mContentInsets;
    int mContentInsetStartWithNavigation;
    int mContentInsetEndWithActions;

    int mGravity = Gravity::START | Gravity::CENTER_VERTICAL;

    std::string mTitleText;
    std::string mSubtitleText;

    int mTitleTextColor;
    int mSubtitleTextColor;

    bool mEatingTouch;
    std::vector<View*> mHiddenViews;
    bool mCollapsible;
private:
    void ensureLogoView();
    void ensureNavButtonView();
    void ensureCollapseButtonView();
    void addSystemView(View* v, bool allowHide);
    void postShowOverflowMenu();
    void ensureMenu();
    void ensureMenuView();
    MenuInflater*getMenuInflater();
    void measureChildConstrained(View* child, int parentWidthSpec, int widthUsed,
         int parentHeightSpec, int heightUsed, int heightConstraint);
    int measureChildCollapseMargins(View* child,int parentWidthMeasureSpec, int widthUsed,
          int parentHeightMeasureSpec, int heightUsed, int*collapsingMargins);
    int getViewListMeasuredWidth(const std::vector<View*>& views, int*collapsingMargins);
    int layoutChildLeft(View* child, int left, int*collapsingMargins,int alignmentHeight);
    int layoutChildRight(View* child, int right, int*collapsingMargins,int alignmentHeight);
    int getChildTop(View* child, int alignmentHeight);
    int getChildVerticalGravity(int gravity);
    void addCustomViewsWithGravity(std::vector<View*>& views, int gravity);
    int getChildHorizontalGravity(int gravity);
    bool shouldCollapse();
    bool shouldLayout(View* view);
    int getHorizontalMargins(View* v);
    int getVerticalMargins(View* v);
    static bool isCustomView(View*);
protected:
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void onSetLayoutParams(View* child, ViewGroup::LayoutParams* lp);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    ViewGroup::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    ViewGroup::LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    void removeChildrenForExpandedActionView();
    void addChildrenForExpandedActionView();
    bool isChildOrHidden(View* child);
public:
    ToolBar(Context*,const AttributeSet&);
    void setTitleMargin(int start, int top, int end, int bottom);
    int getTitleMarginStart()const;
    void setTitleMarginStart(int margin);
    int getTitleMarginTop()const;
    void setTitleMarginTop(int margin);
    int getTitleMarginEnd()const;
    void setTitleMarginEnd(int margin);
    int getTitleMarginBottom()const;
    void setTitleMarginBottom(int margin);
    void setLogo(const std::string& resId);
    void setLogo(Drawable* drawable);
    Drawable* getLogo()const;
    void setLogoDescription(const std::string& description);
    std::string getLogoDescription()const;
    bool hasExpandedActionView()const;
    void collapseActionView();
    std::string getTitle()const;
    void setTitle(const std::string&);
    std::string getSubtitle()const;
    void setSubtitle(const std::string&);
    void setTitleTextColor(int);
    void setSubtitleTextColor(int);
    std::string getNavigationContentDescription()const;
    void setNavigationContentDescription(const std::string&);
    void setNavigationIcon(Drawable*);
    Drawable*getNavigationIcon()const;
    void setNavigationOnClickListener(View::OnClickListener);
    View*getNavigationView()const;
    Menu*getMenu();
    void setOverflowIcon(Drawable*);
    Drawable*getOverflowIcon();
    void inflateMenu(const std::string&);
    //void setOnMenuItemClickListener(OnMenuItemClickListener listener);
    void setContentInsetsRelative(int contentInsetStart, int contentInsetEnd);
    int getContentInsetStart()const;
    int getContentInsetEnd()const;
    void setContentInsetsAbsolute(int contentInsetLeft, int contentInsetRight);
    int getContentInsetLeft()const;
    int getContentInsetRight()const;
    int getContentInsetStartWithNavigation()const;
    void setContentInsetStartWithNavigation(int insetStartWithNavigation);
    int getContentInsetEndWithActions()const;
    void setContentInsetEndWithActions(int insetEndWithActions);
    int getCurrentContentInsetStart()const;
    int getCurrentContentInsetEnd()const;
    int getCurrentContentInsetLeft()const;
    int getCurrentContentInsetRight()const;
    bool onTouchEvent(MotionEvent&)override;
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
};

}//namespace
#endif
