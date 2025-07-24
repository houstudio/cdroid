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
#ifndef __TABLAYOUT_H__
#define __TABLAYOUT_H__
#include <widget/horizontalscrollview.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <widget/imageview.h>
#include <widget/viewpager.h>
#include <animation/animator.h>
namespace cdroid{

class TabLayout:public HorizontalScrollView{
public:
    static constexpr int MODE_SCROLLABLE = 0;
    static constexpr int MODE_FIXED = 1;
    static constexpr int GRAVITY_FILL = 0;
    static constexpr int GRAVITY_CENTER = 1;
    static constexpr int INDICATOR_GRAVITY_BOTTOM = 0;
    static constexpr int INDICATOR_GRAVITY_CENTER = 1;
    static constexpr int INDICATOR_GRAVITY_TOP = 2;
    static constexpr int INDICATOR_GRAVITY_STRETCH = 3;
    class Tab{
    private:
        void* mTag;
        Drawable* mIcon;
        std::string mText;
        std::string mContentDesc;
        int  mPosition = INVALID_POSITION;
        View* mCustomView;
    public:
        static constexpr int INVALID_POSITION = -1;
        TabLayout* mParent;
        View* mView;/*TabView*/

        Tab();
        ~Tab(); 
        void*getTag()const;
        void setTag(void*tag);
        View* getCustomView()const;
        Tab& setCustomView(View*);
        Tab& setCustomView(const std::string&);
        Drawable* getIcon()const;
        Tab& setIcon(Drawable* icon);
        int  getPosition()const;
        void setPosition(int position);
        std::string getText()const;
        Tab& setText(const std::string&text);
        void select();
        bool isSelected()const;
        std::string getContentDescription()const;
        Tab& setContentDescription(const std::string&contentDesc);
        void updateView();
        void reset();
    };

    class TabView:public LinearLayout{
    private:
        static constexpr int CUSTOM_ID_TEXT =0;
        static constexpr int CUSTOM_ID_ICON =1;
        friend class TabLayout;
        TabLayout *mParent;
        Tab* mTab;
        TextView* mTextView;
        ImageView* mIconView;

        View* mCustomView;
        TextView* mCustomTextView;
        ImageView* mCustomIconView;
        Drawable* mBaseBackgroundDrawable;
        int mDefaultMaxLines = 2;
        int getContentWidth(); 
        void updateTextAndIcon(TextView* textView,ImageView* iconView);
        float approximateLineWidth(Layout* layout, int line, float textSize);
        void updateBackgroundDrawable(Context* context);
    public:
        TabView(Context* context,const AttributeSet&atts,TabLayout*parent);
        ~TabView()override;
        bool performClick()override;
        void setSelected(bool selected)override;
        void onMeasure(int origWidthMeasureSpec,int origHeightMeasureSpec)override;
        void setTab(Tab* tab);
        void reset();
        void update();
        Tab* getTab();
    };

    class  OnTabSelectedListener:public EventSet{
    public:
        std::function<void(Tab&)>onTabSelected;
        std::function<void(Tab&)>onTabUnselected;
        std::function<void(Tab&)>onTabReselected;
    };

    class TabItem:public View{
    public:
       std::string mText;
       Drawable* mIcon;
       std::string mCustomLayout;
       TabItem();
       TabItem(Context* context,const AttributeSet& attrs);
    };

    class TabLayoutOnPageChangeListener:public ViewPager::OnPageChangeListener{
    protected:
        friend class TabLayout;
        TabLayout*mTabLayout;
        int mPreviousScrollState;
        int mScrollState;
        void doPageScrollStateChanged(int);
        void doPageScrolled(int position,float positionOffset,int positionOffsetPixels);
        void doPageSelected(int position);
    public:
        TabLayoutOnPageChangeListener();
        void reset();
    };
private:
    class AdapterChangeListener:public ViewPager::OnAdapterChangeListener{

    };
    class PagerAdapterObserver:public DataSetObserver{
    protected:
        TabLayout*mTabLayout;
    public:
        PagerAdapterObserver(TabLayout*tab);
        void onChanged()override;
        void onInvalidated()override;
        void clearSavedState()override;
    };
    class SlidingTabStrip:public LinearLayout{
    private:
        int  mSelectedIndicatorHeight;
        int  mSelectedIndicatorColor;

        int  mSelectedPosition= -1;
        float mSelectionOffset;

        int  mLayoutDirection = -1;

        int  mIndicatorLeft  = -1;
        int  mIndicatorRight = -1;
        TabLayout*mParent;
        ValueAnimator* mIndicatorAnimator;
        void updateIndicatorPosition();
    protected:
        void calculateTabViewContentBounds(TabLayout::TabView* tabView, Rect& contentBounds);
        void onMeasure(int widthMeasureSpec,int heightMeasureSpec)override;
        void onLayout(bool changed, int l, int t, int r, int b)override;
    public:
        SlidingTabStrip(Context* context,const AttributeSet&atts,TabLayout*parent);
        ~SlidingTabStrip()override;
        void setSelectedIndicatorColor(int color);
        void setSelectedIndicatorHeight(int height);
        bool childrenNeedLayout();
        void setIndicatorPositionFromTabPosition(int position, float positionOffset);
        float getIndicatorPosition();
        void onRtlPropertiesChanged(int layoutDirection)override;
        void setIndicatorPosition(int left, int right);
        void animateIndicatorToPosition(int position, int duration);
        void draw(Canvas& canvas)override;
    };
    int  mRequestedTabMinWidth;
    int  mRequestedTabMaxWidth;
    int  mScrollableTabMinWidth;
    int  mContentInsetStart;
    Rect tabViewContentBounds;
    ValueAnimator* mScrollAnimator;
    Tab* mSelectedTab;
    SlidingTabStrip* mTabStrip;
    bool mSetupViewPagerImplicitly;

    void initTabLayout();
    void addTabFromItemView(TabItem* item);
    void setupWithViewPager(ViewPager* viewPager, bool autoRefresh, bool implicitSetup);
    int getTabScrollRange();
    void updateAllTabs();
    TabView*createTabView(Tab* tab);
    void configureTab(Tab* tab, int position);
    void addTabView(Tab* tab);
    void removeTabViewAt(int position);
    void animateToTab(int newPosition);
    void ensureScrollAnimator();
    void setScrollAnimatorListener(Animator::AnimatorListener listener);
    void setSelectedTabView(int position);
    void dispatchTabSelected(Tab* tab);
    void dispatchTabUnselected(Tab* tab);
    void dispatchTabReselected(Tab* tab);
    int  calculateScrollXForTab(int position, float positionOffset);
    void applyModeAndGravity();

    static ColorStateList* createColorStateList(int defaultColor, int selectedColor);
    int getDefaultHeight();
    int getTabMinWidth();
protected:
    static constexpr int FIXED_WRAP_GUTTER_MIN = 16; //dps
    static constexpr int MOTION_NON_ADJACENT_OFFSET = 24;
    int  mTabPaddingStart;
    int  mTabPaddingTop;
    int  mTabPaddingEnd;
    int  mTabPaddingBottom;
    int  mTabTextAppearance;
    bool mOwnedTabTextColors;
    const ColorStateList* mTabTextColors;
    const ColorStateList* mTabIconTint;
    const ColorStateList* mTabRippleColorStateList;
    Drawable* mTabSelectedIndicator;
    float mTabTextSize;
    float mTabTextMultiLineSize;
    std::string mTabBackgroundResId;
    int  mTabMaxWidth;

    int  mTabGravity;
    int  mTabIndicatorAnimationDuration;
    int  mTabIndicatorGravity;
    int  mMode;
    bool mSmoothScroll;/*used for viewpage item switch*/
    bool mInlineLabel;
    std::vector<OnTabSelectedListener> mSelectedListeners;
    OnTabSelectedListener mCurrentVpSelectedListener;
    bool inlineLabel;
    bool tabIndicatorFullWidth;
    bool unboundedRipple;
    ViewPager* mViewPager;
    std::vector<Tab*>mTabs;
    PagerAdapter* mPagerAdapter;
    DataSetObserver* mPagerAdapterObserver;
    AdapterChangeListener* mAdapterChangeListener;
    TabLayoutOnPageChangeListener mPageChangeListener;

    void setScrollPosition(int position, float positionOffset, bool updateSelectedText, bool updateIndicatorPosition);

    void setPagerAdapter(PagerAdapter* adapter,bool addObserver);
    void populateFromPagerAdapter();
    void addViewInternal(View* child,ViewGroup::LayoutParams*);
    LinearLayout::LayoutParams* createLayoutParamsForTabs();
    void updateTabViewLayoutParams(LinearLayout::LayoutParams* lp);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void selectTab(Tab* tab,bool updateIndicator);
    void updateTabViews(bool requestLayout);
    int  getTabMaxWidth();
public:
    TabLayout(int w,int h);
    TabLayout(Context*context,const AttributeSet&atts);
    ~TabLayout()override;
    void setSelectedTabIndicatorColor( int color);
    void setSelectedTabIndicatorHeight(int height);
    void setScrollPosition(int position, float positionOffset, bool updateSelectedText);
    float getScrollPosition()const;
    void addTab(Tab* tab);
    void addTab(Tab* tab, int position);
    void addTab(Tab* tab, bool setSelected);
    void addTab(Tab* tab, int position, bool setSelected);

    void addOnTabSelectedListener(const OnTabSelectedListener& listener);
    void removeOnTabSelectedListener(const OnTabSelectedListener& listener);
    void clearOnTabSelectedListeners();
    Tab* newTab();
    int  getTabCount()const;
    Tab* getTabAt(int index);
    int  getSelectedTabPosition()const;
    void removeTab(Tab* tab);
    void removeTabAt(int position);
    void removeAllTabs();
    void setTabMode(int mode);
    int  getTabMode()const;
    void setTabGravity(int gravity);
    int  getTabGravity()const;
    int  getTabIndicatorGravity()const;
    void setTabIndicatorGravity(int);
    Drawable* getSelectedTabIndicator()const;
    void setSelectedTabIndicator(Drawable*d);
    void setSelectedTabIndicator(const std::string&res);
    bool isTabIndicatorFullWidth()const;
    void setTabIndicatorFullWidth(bool tabIndicatorFullWidth);
    bool isInlineLabel()const;
    void setInlineLabel(bool);
    void setTabTextColors(const ColorStateList* textColor);
    const ColorStateList* getTabTextColors()const;
    void setTabTextColors(int normalColor, int selectedColor);
    void setupWithViewPager(ViewPager* viewPager);
    void setupWithViewPager(ViewPager* viewPager, bool autoRefresh);
    bool shouldDelayChildPressedState()override;

    void addView(View* child) override;
    void addView(View* child, int index)override;
    void addView(View* child, ViewGroup::LayoutParams* params)override;
    void addView(View* child, int index, ViewGroup::LayoutParams* params)override;

    FrameLayout::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
};

}/*endof namespace*/
#endif/*__TABLAYOUT_H__*/
