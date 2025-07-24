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
#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__
#include <widget/textview.h>
#include <widget/imageview.h>
#include <widget/imagebutton.h>
#include <view/viewgroup.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <widget/actionbar.h>
#include <widget/rtlspacinghelper.h>

namespace cdroid{
typedef View ActionMenuView;
class Menu;
class MenuInflater;
class Toolbar:public ViewGroup{
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

    Context* mPopupContext;
    Drawable* mCollapseIcon;
    std::string mCollapseDescription;
    ImageButton* mCollapseButtonView;
    View*mExpandedActionView;

    int mPopupTheme;

    std::string mTitleTextAppearance;
    std::string mSubtitleTextAppearance;
    std::string mNavButtonStyle;

    int mButtonGravity;

    int mMaxButtonHeight;

    int mTitleMarginStart;
    int mTitleMarginEnd;
    int mTitleMarginTop;
    int mTitleMarginBottom;

    RtlSpacingHelper* mContentInsets;
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
    Runnable mShowOverflowMenuRunnable;
    MenuItem::OnMenuItemClickListener mOnMenuItemClickListener;
private:
    void initToolbar();
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
    void ensureContentInsets();
protected:
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void onSetLayoutParams(View* child, ViewGroup::LayoutParams* lp);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    void removeChildrenForExpandedActionView();
    void addChildrenForExpandedActionView();
    bool isChildOrHidden(View* child);
    Context*getPopupContext();
public:
    Toolbar(Context*,const AttributeSet&);
    ~Toolbar()override;
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
    void setOnMenuItemClickListener(MenuItem::OnMenuItemClickListener listener);
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
    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    void setCollapsible(bool);
    //void setMenuCallbacks(MenuPresenter.Callback pcb, MenuBuilder.Callback mcb);
};
}/*endof namespace*/
#endif
