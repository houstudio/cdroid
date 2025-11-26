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
#include <widget/tablayout.h>
#include <widget/R.h>
#include <core/build.h>
#include <utils/mathutils.h>
#include <porting/cdlog.h>

namespace cdroid{
static constexpr int DEFAULT_HEIGHT_WITH_TEXT_ICON = 72;
static constexpr int DEFAULT_GAP_TEXT_ICON = 8;
static constexpr int DEFAULT_HEIGHT = 48;
static constexpr int TAB_MIN_WIDTH_MARGIN = 56;
static constexpr int MIN_INDICATOR_WIDTH  = 24;
//   static final int FIXED_WRAP_GUTTER_MIN = 16;
static constexpr int INVALID_WIDTH = -1;
static constexpr int ANIMATION_DURATION = 300;

DECLARE_WIDGET(TabLayout)

TabLayout::TabLayout(int w,int h):HorizontalScrollView(w,h){
    initTabLayout();
    applyModeAndGravity();
}

TabLayout::TabLayout(Context*context,const AttributeSet&atts)
  :HorizontalScrollView(context,atts){
    initTabLayout();

    mTabStrip->setSelectedIndicatorHeight(atts.getDimensionPixelSize("tabIndicatorHeight",-1));
    mTabStrip->setSelectedIndicatorColor(atts.getColor("tabIndicatorColor",0));

    setSelectedTabIndicator(context->getDrawable(atts.getString("tabIndicator"))); 
    setTabIndicatorGravity(atts.getGravity("tabIndicatorGravity",0));
    setTabIndicatorFullWidth(atts.getBoolean("tabIndicatorFullWidth",true));

    mTabPaddingStart = mTabPaddingTop=mTabPaddingEnd=mPaddingBottom=atts.getDimensionPixelSize("tabPadding",0);
    mTabPaddingStart = atts.getDimensionPixelSize("tabPaddingStart",mTabPaddingStart);
    mTabPaddingEnd   = atts.getDimensionPixelSize("tabPaddingEnd",mTabPaddingEnd);
    mTabPaddingTop   = atts.getDimensionPixelSize("tabPaddingTop",mTabPaddingTop);
    mTabPaddingBottom= atts.getDimensionPixelSize("tabPaddingBottom",mTabPaddingBottom); 
    mTabTextSize  = atts.getDimensionPixelSize("tabTextSize",mTabTextSize);

    if(atts.hasAttribute("tabTextColor"))
        mTabTextColors = context->getColorStateList(atts.getString("tabTextColor"));
    else{ 
        mTabTextColors = new ColorStateList(0xFFFFFFFF);
        mOwnedTabTextColors = true;
    }

    if(atts.hasAttribute("tabSelectedTextColor")){
        const int selected = atts.getColor("tabSelectedTextColor",0);
        const int defColor = mTabTextColors->getDefaultColor();
        if(mOwnedTabTextColors){
            delete mTabTextColors;
        }
        mTabTextColors = createColorStateList(defColor, selected);
        mOwnedTabTextColors =true;
    }

    mTabIndicatorAnimationDuration = atts.getInt("tabIndicatorAnimationDuration", 300);
    mRequestedTabMinWidth = atts.getDimensionPixelSize("tabMinWidth", -1);
    mRequestedTabMaxWidth = atts.getDimensionPixelSize("tabMaxWidth", -1);

    mTabBackgroundResId= atts.getString("tabBackground");
    mContentInsetStart  = atts.getDimensionPixelSize("tabContentStart", 0);
    mMode = atts.getInt("tabMode",std::unordered_map<std::string,int>{{"scrollable",0},{"fixed",1}},mMode);
    mSmoothScroll = atts.getBoolean("smoothScroll",true);
    mTabGravity =atts.getGravity("tabGravity",0);
    mInlineLabel=atts.getBoolean("tabInlineLabel",false);
    applyModeAndGravity();
}

TabLayout::~TabLayout(){
    for(Tab*tab:mTabs){
        delete tab;
    }
    if(mOwnedTabTextColors){
        delete mTabTextColors;/*ColorStateList is global holded by keymap ,cant destroied*/
    }
    delete mScrollAnimator;
    delete mAdapterChangeListener;
    delete mPagerAdapterObserver;
}

void TabLayout::initTabLayout(){
    AttributeSet atts(getContext(),"cdroid");
    mMode = MODE_SCROLLABLE;
    mInlineLabel =false;
    mTabPaddingStart= mTabPaddingTop   = 0;
    mTabPaddingEnd  = mTabPaddingBottom= 0;
    mTabTextSize = 20;
    mContentInsetStart = 0;
    mOwnedTabTextColors= false;
    mTabTextColors  = nullptr;
    mSelectedTab    = nullptr;
    mScrollAnimator = nullptr;
    mViewPager    = nullptr;
    mPagerAdapter = nullptr;
    mTabIndicatorGravity  = Gravity::BOTTOM;
    mPagerAdapterObserver = nullptr;
    mAdapterChangeListener= nullptr;
    mTabSelectedIndicator = nullptr;
    mRequestedTabMinWidth = INVALID_WIDTH;
    mRequestedTabMaxWidth = INVALID_WIDTH;
    mTabTextMultiLineSize = 2;
    mScrollableTabMinWidth= 100;
    tabIndicatorFullWidth = true;
    mTabStrip = new SlidingTabStrip(getContext(),atts,this);
    HorizontalScrollView::addView(mTabStrip, 0, new HorizontalScrollView::LayoutParams(
          LayoutParams::WRAP_CONTENT, LayoutParams::MATCH_PARENT));
}

void TabLayout::setSelectedTabIndicatorColor( int color){
    mTabStrip->setSelectedIndicatorColor(color);
}

void TabLayout::setSelectedTabIndicatorHeight(int height) {
    mTabStrip->setSelectedIndicatorHeight(height);
}

void TabLayout::setScrollPosition(int position, float positionOffset, bool updateSelectedText, bool updateIndicatorPosition){
    int roundedPosition = std::round(position + positionOffset);
    if (roundedPosition < 0 || roundedPosition >= mTabStrip->getChildCount()) {
        return;
    }

    // Set the indicator position, if enabled
    if (updateIndicatorPosition) {
        mTabStrip->setIndicatorPositionFromTabPosition(position, positionOffset);
    }

    // Now update the scroll position, canceling any running animation
    if (mScrollAnimator && mScrollAnimator->isRunning()) {
        mScrollAnimator->cancel();
    }
    scrollTo(calculateScrollXForTab(position, positionOffset), 0);

    // Update the 'selected state' view as we scroll, if enabled
    if (updateSelectedText) {
        setSelectedTabView(roundedPosition);
    }
}

void TabLayout::setScrollPosition(int position, float positionOffset, bool updateSelectedText){
    setScrollPosition(position, positionOffset, updateSelectedText, true);
}

float TabLayout::getScrollPosition()const{
    return mTabStrip->getIndicatorPosition();
}

void TabLayout::addTab(Tab* tab) {
    addTab(tab, mTabs.empty());
}

void TabLayout::addTab(Tab* tab, int position) {
    addTab(tab, position, mTabs.empty());
}

void TabLayout::addTab(Tab* tab, bool setSelected){
    addTab(tab,mTabs.size(), setSelected);
}

void TabLayout::addTab(Tab* tab, int position, bool setSelected){
    LOGE_IF(tab->mParent != this,"Tab belongs to a different TabLayout.");
    configureTab(tab, position);
    addTabView(tab);

    if (setSelected) {
        tab->select();
    }
}

void TabLayout::addTabFromItemView(TabItem* item){
    Tab* tab = newTab();
    tab->setText(item->mText);
    
    if (item->mIcon) {
        tab->setIcon(item->mIcon);
    }
    if (item->mCustomLayout.size()){
        tab->setCustomView(item->mCustomLayout);
    }
    tab->setContentDescription(item->getContentDescription());//getContentDescription inherited from View.
    addTab(tab);
    delete item;//added by zhhou
}

void TabLayout::addOnTabSelectedListener(const OnTabSelectedListener& listener){
    mSelectedListeners.push_back(listener);
}

void TabLayout::removeOnTabSelectedListener(const OnTabSelectedListener& listener) {
    auto it = std::find(mSelectedListeners.begin(),mSelectedListeners.end(),listener);
    if(it!=mSelectedListeners.end()){
        mSelectedListeners.erase(it);
    }
}

void TabLayout::clearOnTabSelectedListeners() {
    mSelectedListeners.clear();
}

TabLayout::Tab* TabLayout::newTab(){
    Tab* tab =new Tab();
    tab->mParent = this;
    tab->mView = createTabView(tab);
    return tab;    
}

int TabLayout::getTabCount()const{
    return mTabs.size();
} 

TabLayout::Tab* TabLayout::getTabAt(int index){
    return (index < 0 || index >= getTabCount()) ? nullptr : mTabs.at(index);
}

int TabLayout::getSelectedTabPosition()const{
    return mSelectedTab ? mSelectedTab->getPosition() : -1;
}

void TabLayout::removeTab(Tab* tab) {
    LOGE_IF(tab->mParent != this,"Tab does not belong to this TabLayout.");
    removeTabAt(tab->getPosition());
}

void TabLayout::removeTabAt(int position){
    int selectedTabPosition = mSelectedTab ? mSelectedTab->getPosition() : 0;
    removeTabViewAt(position);

    auto itr=mTabs.erase(mTabs.begin()+position);
    Tab* removedTab =(*itr); 
    if (removedTab) {
        removedTab->reset();
    }

    int newTabCount = mTabs.size();
    for (int i = position; i < newTabCount; i++) {
        mTabs.at(i)->setPosition(i);
    }

    if (selectedTabPosition == position) {
        selectTab(mTabs.empty() ? nullptr : mTabs.at(std::max(0, position - 1)),true);
    }
}

void TabLayout::removeAllTabs(){
    for (int i = mTabStrip->getChildCount() - 1; i >= 0; i--) {
        removeTabViewAt(i);
    }

    while (!mTabs.empty()){
        auto it= mTabs.begin();
        Tab* tab = (*it);
        delete tab;
        mTabs.erase(it);
    }
    mSelectedTab = nullptr;
}

void TabLayout::setTabMode(int mode){
    if (mode != mMode) {
        mMode = mode;
        applyModeAndGravity();
    }
}

int TabLayout::getTabMode()const{
    return mMode;
}

void TabLayout::setTabGravity(int gravity) {
    if (mTabGravity != gravity) {
        mTabGravity = gravity;
        applyModeAndGravity();
    }
}

int TabLayout::getTabGravity()const{
    return mTabGravity;
}

Drawable*TabLayout::getSelectedTabIndicator()const{
    return mTabSelectedIndicator;
}

void TabLayout::setSelectedTabIndicator(Drawable*d){
    if(mTabSelectedIndicator!=d){
        delete mTabSelectedIndicator;
        mTabSelectedIndicator=d;
        mTabStrip->postInvalidateOnAnimation();
    }
}

void TabLayout::setSelectedTabIndicator(const std::string&res){
    setSelectedTabIndicator(getContext()->getDrawable(res));
}

int TabLayout::getTabIndicatorGravity()const{
    return mTabIndicatorGravity;
}

void TabLayout::setTabIndicatorGravity(int gravity){
    if(mTabIndicatorGravity!=gravity){
         mTabIndicatorGravity =gravity;
         mTabStrip->postInvalidateOnAnimation();
    }
}

bool TabLayout::isTabIndicatorFullWidth()const{
    return tabIndicatorFullWidth;
}

void TabLayout::setTabIndicatorFullWidth(bool tabIndicatorFullWidth) {
    this->tabIndicatorFullWidth = tabIndicatorFullWidth;
    mTabStrip->postInvalidateOnAnimation();//slidingTabIndicator);
}

bool TabLayout::isInlineLabel()const{
    return mInlineLabel;
}

void TabLayout::setInlineLabel(bool v){
    if(mInlineLabel==v)return;
    mInlineLabel=v;
    for(int i=0;i<mTabStrip->getChildCount();i++){
        View*child=mTabStrip->getChildAt(i);
        if(dynamic_cast<TabLayout::TabView*>(child)){
            TabLayout::TabView*tv=(TabLayout::TabView*)child;
            //tv->updateBackgroundDrawable(getContext());
        }  
    }
}

void TabLayout::setTabTextColors(const ColorStateList* textColor) {
    if (mTabTextColors!=textColor){
        mTabTextColors = textColor;
        updateAllTabs();
    }
}

const ColorStateList* TabLayout::getTabTextColors()const{
    return mTabTextColors;
}

void TabLayout::setTabTextColors(int normalColor, int selectedColor){
    ColorStateList *cls = createColorStateList(normalColor, selectedColor);
    mOwnedTabTextColors =true;
    setTabTextColors(cls);
}

void TabLayout::setupWithViewPager(ViewPager* viewPager, bool autoRefresh, bool implicitSetup){

    if (mViewPager) {
        // If we've already been setup with a ViewPager, remove us from it
        //if (mPageChangeListener != nullptr) 
            mViewPager->removeOnPageChangeListener(mPageChangeListener);
        
        if (mAdapterChangeListener != nullptr)
            mViewPager->removeOnAdapterChangeListener(*mAdapterChangeListener);
    
    }

    // If we already have a tab selected listener for the ViewPager, remove it
    removeOnTabSelectedListener(mCurrentVpSelectedListener);

    if (viewPager) {
        mViewPager = viewPager;

        // Add our custom OnPageChangeListener to the ViewPager
        //if (mPageChangeListener == nullptr)
            mPageChangeListener.mTabLayout=this;// = new TabLayoutOnPageChangeListener(this);
        
        mPageChangeListener.reset();
        viewPager->addOnPageChangeListener(mPageChangeListener);

        // Now we'll add a tab selected listener to set ViewPager's current item
        mCurrentVpSelectedListener.onTabSelected=[this](Tab&tab){
            LOGV("selectTab %d/%d",tab.getPosition(),getTabCount());
            mViewPager->setCurrentItem(tab.getPosition(),mSmoothScroll);
        };
        addOnTabSelectedListener(mCurrentVpSelectedListener);

        
        PagerAdapter* adapter = viewPager->getAdapter();
        if (adapter != nullptr) {
            // Now we'll populate ourselves from the pager adapter, adding an observer if
            // autoRefresh is enabled
            setPagerAdapter(adapter, autoRefresh);
        }

        // Add a listener so that we're notified of any adapter changes
        if (mAdapterChangeListener == nullptr) {
            mAdapterChangeListener = new AdapterChangeListener();
        }
        //mAdapterChangeListener.setAutoRefresh(autoRefresh);
        viewPager->addOnAdapterChangeListener(*mAdapterChangeListener);

        // Now update the scroll position to match the ViewPager's current item
        setScrollPosition(viewPager->getCurrentItem(), .0f, true);
    } else {
        // We've been given a null ViewPager so we need to clear out the internal state,
        // listeners and observers
        mViewPager = nullptr;
        setPagerAdapter(nullptr, false);
    }

    mSetupViewPagerImplicitly = implicitSetup;
}

void TabLayout::setupWithViewPager(ViewPager* viewPager,bool autoRefresh){
    setupWithViewPager(viewPager, autoRefresh, false);
}
void TabLayout::setupWithViewPager(ViewPager* viewPager){
    setupWithViewPager(viewPager, true);
}

bool TabLayout::shouldDelayChildPressedState(){
     return getTabScrollRange() > 0;
}

int TabLayout::getTabScrollRange(){
    return std::max(0, mTabStrip->getWidth() - getWidth() - getPaddingLeft()
            - getPaddingRight());
}

void TabLayout::setPagerAdapter(PagerAdapter* adapter,bool addObserver){
    if (mPagerAdapter  && mPagerAdapterObserver) {
        // If we already have a PagerAdapter, unregister our observer
        mPagerAdapter->unregisterDataSetObserver(mPagerAdapterObserver);
    }

    mPagerAdapter = adapter;

    if (addObserver && adapter != nullptr) {
        // Register our observer on the new adapter
        if (mPagerAdapterObserver == nullptr) {
            mPagerAdapterObserver = new PagerAdapterObserver(this);
        }
        adapter->registerDataSetObserver(mPagerAdapterObserver);
    }

    // Finally make sure we reflect the new adapter
    populateFromPagerAdapter();
}

void TabLayout::populateFromPagerAdapter(){
    removeAllTabs();

    if (mPagerAdapter) {
        int adapterCount = mPagerAdapter->getCount();
        for (int i = 0; i < adapterCount; i++) {
            Tab*tab=newTab();
            tab->setText(mPagerAdapter->getPageTitle(i));
            addTab(tab, false);
        }

        // Make sure we reflect the currently set ViewPager item
        if (mViewPager && adapterCount > 0) {
            int curItem = mViewPager->getCurrentItem();
            if (curItem != getSelectedTabPosition() && curItem < getTabCount()) {
                selectTab(getTabAt(curItem),true);
            }
        }
    }
}

void TabLayout::updateAllTabs(){
    for (int i = 0, z = mTabs.size(); i < z; i++) {
        mTabs.at(i)->updateView();
    }
}

TabLayout::TabView*TabLayout::createTabView(TabLayout::Tab* tab){
    TabView* tabView =nullptr;// mTabViewPool != null ? mTabViewPool.acquire() : null;
    if (tabView == nullptr) {
        tabView = new TabView(getContext(),AttributeSet(getContext(),"cdroid"),this);
    }
    tabView->setTab(tab);
    tabView->setFocusable(true);
    tabView->setMinimumWidth(getTabMinWidth());
    return tabView;
}

void TabLayout::configureTab(TabLayout::Tab* tab, int position){
    tab->setPosition(position);
    mTabs.insert(mTabs.begin()+position, tab);
    int count = mTabs.size();
    for (int i = position + 1; i < count; i++) {
        mTabs[i]->setPosition(i);
    }
}

void TabLayout::addTabView(TabLayout::Tab* tab){
    TabView* tabView = (TabView*)tab->mView;
    mTabStrip->addView(tabView, tab->getPosition(), createLayoutParamsForTabs());
}

void TabLayout::addView(View* child){
    addViewInternal(child,nullptr);
}

void TabLayout::addView(View* child, int index){
    addViewInternal(child,nullptr);
}

void TabLayout::addView(View* child, ViewGroup::LayoutParams* params){
    addViewInternal(child,params);
}

void TabLayout::addView(View* child, int index, ViewGroup::LayoutParams* params){
    addViewInternal(child,params);
}

void TabLayout::addViewInternal(View* child,ViewGroup::LayoutParams*params){
    delete params;//addTabFromItemView will create an other LayoutParams 
    if (dynamic_cast<TabItem*>(child)){
        child->setId(getTabCount()); 
        addTabFromItemView((TabItem*) child);
    }else LOGE("Only TabItem instances can be added to TabLayout");
}

LinearLayout::LayoutParams* TabLayout::createLayoutParamsForTabs(){
    LinearLayout::LayoutParams* lp = new LinearLayout::LayoutParams(
            LayoutParams::WRAP_CONTENT, LayoutParams::MATCH_PARENT);
    updateTabViewLayoutParams(lp);
    return lp;
}

void TabLayout::updateTabViewLayoutParams(LinearLayout::LayoutParams* lp){
    if (mMode == MODE_FIXED && mTabGravity == GRAVITY_FILL) {
       lp->width = 0;
       lp->weight = 1;
    } else {
       lp->width = LinearLayout::LayoutParams::WRAP_CONTENT;
       lp->weight = 0;
    }
}

static int dpToPx(int dps) {
    return dps;//Math.round(getDisplayMetrics().density * dps);
}

void TabLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int idealHeight = dpToPx(getDefaultHeight()) + getPaddingTop() + getPaddingBottom();
    switch (MeasureSpec::getMode(heightMeasureSpec)) {
    case MeasureSpec::AT_MOST:
        heightMeasureSpec = MeasureSpec::makeMeasureSpec(
                std::min(idealHeight, MeasureSpec::getSize(heightMeasureSpec)),
                MeasureSpec::EXACTLY);
        break;
    case MeasureSpec::UNSPECIFIED:
        heightMeasureSpec = MeasureSpec::makeMeasureSpec(idealHeight, MeasureSpec::EXACTLY);
        break;
    }

    int specWidth = MeasureSpec::getSize(widthMeasureSpec);
    if (MeasureSpec::getMode(widthMeasureSpec) != MeasureSpec::UNSPECIFIED) {
        // If we don't have an unspecified width spec, use the given size to calculate
        // the max tab width
        mTabMaxWidth = mRequestedTabMaxWidth > 0  ? mRequestedTabMaxWidth
                : specWidth - dpToPx(TAB_MIN_WIDTH_MARGIN);
    }

    // Now super measure itself using the (possibly) modified height spec
    HorizontalScrollView::onMeasure(widthMeasureSpec, heightMeasureSpec);

    if (getChildCount() == 1) {
        // If we're in fixed mode then we need to make the tab strip is the same width as us
        // so we don't scroll
        View* child = getChildAt(0);
        bool remeasure = false;

        switch (mMode) {
        case MODE_SCROLLABLE:
            // We only need to resize the child if it's smaller than us. This is similar
            // to fillViewport
            remeasure = child->getMeasuredWidth() < getMeasuredWidth();
            break;
        case MODE_FIXED:
            // Resize the child so that it doesn't scroll
            remeasure = child->getMeasuredWidth() != getMeasuredWidth();
            break;
        }

        if (remeasure) {
            // Re-measure the child with a widthSpec set to be exactly our measure width
            int childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec, getPaddingTop()
                    + getPaddingBottom(), child->getLayoutParams()->height);
            int childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(
                    getMeasuredWidth(), MeasureSpec::EXACTLY);
            child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
        }
    }
}

void TabLayout::removeTabViewAt(int position){
    TabView* view = (TabView*) mTabStrip->getChildAt(position);
    mTabStrip->removeViewAt(position);
    if (view) {
        view->reset();
        //mTabViewPool.release(view);
    }
    requestLayout();
}

void TabLayout::animateToTab(int newPosition){
    if (newPosition == Tab::INVALID_POSITION) {
        return;
    }

    if ( false==isLaidOut()  || mTabStrip->childrenNeedLayout()) {
        // If we don't have a window token, or we haven't been laid out yet just draw the new
        // position now
        setScrollPosition(newPosition, .0f, true);
        return;
    }

    int startScrollX = getScrollX();
    int targetScrollX= calculateScrollXForTab(newPosition, 0);

    if (startScrollX != targetScrollX) {
        ensureScrollAnimator();
        mScrollAnimator->setFloatValues({(float)startScrollX, (float)targetScrollX});
        mScrollAnimator->start();
    }

    // Now animate the indicator
    mTabStrip->animateIndicatorToPosition(newPosition, ANIMATION_DURATION);
}

void TabLayout::ensureScrollAnimator(){
     if (mScrollAnimator == nullptr) {
        mScrollAnimator = new ValueAnimator();
        mScrollAnimator->setInterpolator(FastOutSlowInInterpolator::Instance);
        mScrollAnimator->setDuration(ANIMATION_DURATION);
        mScrollAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim) {
           PropertyValuesHolder*ip=anim.getValues()[0]; 
           scrollTo(GET_VARIANT(ip->getAnimatedValue(),float),0);
        }));
    }
}

void TabLayout::setScrollAnimatorListener(const Animator::AnimatorListener& listener){
    ensureScrollAnimator();
    mScrollAnimator->addListener(listener);
}

void TabLayout::setSelectedTabView(int position){
    int tabCount = mTabStrip->getChildCount();
    if (position < tabCount) {
        for (int i = 0; i < tabCount; i++) {
            View* child = mTabStrip->getChildAt(i);
            child->setSelected(i == position);
        }
    }
}

void TabLayout::selectTab(TabLayout::Tab* tab,bool updateIndicator){
    Tab* currentTab = mSelectedTab;
    if (currentTab == tab) {
        if (currentTab) {
            dispatchTabReselected(tab);
            animateToTab(tab->getPosition());
        }
    } else {
        int newPosition = tab ? tab->getPosition() : Tab::INVALID_POSITION;
        if (updateIndicator) {
            if ((currentTab == nullptr || currentTab->getPosition() == Tab::INVALID_POSITION)
                    && newPosition != Tab::INVALID_POSITION) {
                // If we don't currently have a tab, just draw the indicator
                setScrollPosition(newPosition, .0f, true);
            } else {
                animateToTab(newPosition);
            }
            if (newPosition != Tab::INVALID_POSITION) {
                setSelectedTabView(newPosition);
            }
        }
        if (currentTab) dispatchTabUnselected(currentTab);

        mSelectedTab = tab;
        if (tab) dispatchTabSelected(tab);
    }    
}

void TabLayout::dispatchTabSelected(Tab* tab) {
    for (auto ls:mSelectedListeners) {
        if(ls.onTabSelected)
            ls.onTabSelected(*tab);
    }
}

void TabLayout::dispatchTabUnselected(Tab* tab) {
    for (auto ls:mSelectedListeners) {
        if(ls.onTabUnselected)
            ls.onTabUnselected(*tab);
    }
}

void TabLayout::dispatchTabReselected(Tab* tab) {
    for (auto ls:mSelectedListeners) {
        if(ls.onTabReselected)
            ls.onTabReselected(*tab);
    }
}

int TabLayout::calculateScrollXForTab(int position, float positionOffset){
    if (mMode == MODE_SCROLLABLE) {
        View* selectedChild = mTabStrip->getChildAt(position);
        View* nextChild = position + 1 < mTabStrip->getChildCount()
             ? mTabStrip->getChildAt(position + 1): nullptr;
        int selectedWidth = selectedChild ? selectedChild->getWidth() : 0;
        int nextWidth = nextChild  ? nextChild->getWidth() : 0;

        // base scroll amount: places center of tab in center of parent
        int scrollBase = selectedChild->getLeft() + (selectedWidth / 2) - (getWidth() / 2);
        // offset amount: fraction of the distance between centers of tabs
        int scrollOffset = (int) ((selectedWidth + nextWidth) * 0.5f * positionOffset);

        return (getLayoutDirection() == LAYOUT_DIRECTION_LTR)
                ? scrollBase + scrollOffset
                    : scrollBase - scrollOffset;
    }
    return 0;
}

void TabLayout::applyModeAndGravity(){
    int paddingStart = 0;
    if (mMode == MODE_SCROLLABLE) {
        // If we're scrollable, or fixed at start, inset using padding
        paddingStart = std::max(0, mContentInsetStart - mTabPaddingStart);
    }
    mTabStrip->setPaddingRelative(paddingStart, 0, 0, 0);

    switch (mMode) {
    case MODE_FIXED:
        mTabStrip->setGravity(Gravity::CENTER_HORIZONTAL);
        break;
    case MODE_SCROLLABLE:
        mTabStrip->setGravity(Gravity::START);
        break;
    }
    updateTabViews(true);
}

void TabLayout::updateTabViews(bool requestLayout){
    LOGD("requestLayout=%d mintabwidth=%d %d children",requestLayout,getTabMinWidth(),mTabStrip->getChildCount());
    for (int i = 0; i < mTabStrip->getChildCount(); i++) {
        View* child = mTabStrip->getChildAt(i);
        child->setMinimumWidth(getTabMinWidth());
        updateTabViewLayoutParams((LinearLayout::LayoutParams*) child->getLayoutParams());
        if (requestLayout) {
            child->requestLayout();
        }
    }
}

ColorStateList* TabLayout::createColorStateList(int defaultColor, int selectedColor){
    std::vector<std::vector<int>>states;
    std::vector<int>colors;

    states.push_back(StateSet::get(StateSet::VIEW_STATE_SELECTED));
    colors.push_back(selectedColor);

    // Default enabled state
    states.push_back(StateSet::NOTHING);
    colors.push_back(defaultColor);
    LOGD("createColorStateList %x,%x",defaultColor,selectedColor);
    return new ColorStateList(states, colors);
}

int TabLayout::getDefaultHeight() {
    bool hasIconAndText = false;
    for (int i = 0, count = mTabs.size(); i < count; i++) {
        Tab* tab = mTabs.at(i);
        if (tab && tab->getIcon() && !tab->getText().empty()) {
            hasIconAndText = true;
            break;
        }
    }
    return hasIconAndText&&!isInlineLabel() ? DEFAULT_HEIGHT_WITH_TEXT_ICON : DEFAULT_HEIGHT;
}

int TabLayout::getTabMinWidth() {
    if (mRequestedTabMinWidth != INVALID_WIDTH) {
        // If we have been given a min width, use it
        return mRequestedTabMinWidth;
    }
    // Else, we'll use the default value
    return mMode == MODE_SCROLLABLE ? mScrollableTabMinWidth : 0;
}

FrameLayout::LayoutParams* TabLayout::generateLayoutParams(const AttributeSet& attrs)const{
    return generateDefaultLayoutParams();
}

int TabLayout::getTabMaxWidth() {
    return mTabMaxWidth;
}



///////////////////////////////////////////////////////////////////////////////////////////
DECLARE_WIDGET3(TabLayout::TabItem,TabItem,"")

TabLayout::TabItem::TabItem():View(0,0){
    mIcon=nullptr;
}
TabLayout::TabItem::TabItem(Context* context,const AttributeSet& attrs):View(context,attrs){
    mText=attrs.getString("text");
    mIcon=context->getDrawable(attrs.getString("icon"));
    LOGV("%s,%p",mText.c_str(),mIcon);
}

/*-------------------------------------------------------------------------------------------*/
TabLayout::Tab::Tab(){
    mTag   = nullptr;
    mParent= nullptr;
    mView  = nullptr;
    mIcon  = nullptr;
    mCustomView=nullptr;
}

TabLayout::Tab::~Tab(){
}

void TabLayout::Tab::setTag(void*tag){
    mTag=tag;
}

void*TabLayout::Tab::getTag()const{
    return mTag;
}

View*TabLayout::Tab::getCustomView()const{
    return mCustomView;
}

TabLayout::Tab& TabLayout::Tab::setCustomView(View*v){
    mCustomView=v;
    updateView();
    return *this;
}

TabLayout::Tab& TabLayout::Tab::setCustomView(const std::string&resid){
    View*v=LayoutInflater::from(mParent->getContext())->inflate(resid,nullptr,false);
    return setCustomView(v);
}

Drawable* TabLayout::Tab::getIcon()const{
    return mIcon;
}

TabLayout::Tab& TabLayout::Tab::setIcon(Drawable* icon){
    mIcon=icon;
    updateView();
    return *this;
}

int TabLayout::Tab::getPosition()const{
    return mPosition;
}

void TabLayout::Tab::setPosition(int position){
    mPosition=position;
}

std::string TabLayout::Tab::getText()const{
    return mText;
}

TabLayout::Tab& TabLayout::Tab::setText(const std::string&text){
    mText=text;
    updateView();
    return *this;
}

void TabLayout::Tab::select() {
    LOGE_IF(mParent==nullptr,"Tab not attached to a TabLayout");
    mParent->selectTab((TabLayout::Tab*)this,true);
}

bool TabLayout::Tab::isSelected()const{
    LOGE_IF(mParent==nullptr,"Tab not attached to a TabLayout");
    return mParent->getSelectedTabPosition() == mPosition;
}

TabLayout::Tab& TabLayout::Tab::setContentDescription(const std::string&contentDesc) {
    mContentDesc = contentDesc;
    updateView();
    return *this;
}

std::string TabLayout::Tab::getContentDescription()const{
    return mContentDesc;
}

void TabLayout::Tab::updateView() {
    if (mView) ((TabView*)mView)->update();
}

void TabLayout::Tab::reset() {
    mParent = nullptr;
    mView   = nullptr;
    mTag    = nullptr;
    mIcon   = nullptr;
    mText   = nullptr;
    mContentDesc = nullptr;
    mPosition = INVALID_POSITION;
    mCustomView = nullptr;
}

/*----------------------------------------------------------------------------------*/
TabLayout::TabView::TabView(Context* context,const AttributeSet&atts,TabLayout*parent)
  :LinearLayout(context,atts){
    mParent = parent;
    mTab    = nullptr;
    mTextView = nullptr;
    mIconView = nullptr;
    mCustomView = nullptr;
    mCustomTextView = nullptr;
    mCustomIconView = nullptr;
    if(parent->mTabBackgroundResId.length())
        setBackgroundResource(parent->mTabBackgroundResId);
    setPaddingRelative(parent->mTabPaddingStart, parent->mTabPaddingTop, parent->mTabPaddingEnd, parent->mTabPaddingBottom);
    setGravity(Gravity::CENTER);
    setOrientation(parent->mInlineLabel?HORIZONTAL:VERTICAL);
    setClickable(true);
    //ViewCompat.setPointerIcon(this,PointerIconCompat.getSystemIcon(getContext(), PointerIconCompat.TYPE_HAND));
}

TabLayout::TabView::~TabView(){
    delete mTab;
}

void TabLayout::TabView::updateBackgroundDrawable(Context* context) {
    if (mParent->mTabBackgroundResId.size()) {
        mBaseBackgroundDrawable = context->getDrawable(mParent->mTabBackgroundResId);
        if (mBaseBackgroundDrawable && mBaseBackgroundDrawable->isStateful()) {
          mBaseBackgroundDrawable->setState(getDrawableState());
        }
    } else {
        mBaseBackgroundDrawable = nullptr;
    }

    Drawable* background;
    Drawable* contentDrawable = new GradientDrawable();
    ((GradientDrawable*) contentDrawable)->setColor(Color::TRANSPARENT);

    if (0/*tabRippleColorStateList*/) {
        GradientDrawable* maskDrawable = new GradientDrawable();
        // TODO: Find a workaround for this. Currently on certain devices/versions,
        // LayerDrawable will draw a black background underneath any layer with a non-opaque color,
        // (e.g. ripple) unless we set the shape to be something that's not a perfect rectangle.
        maskDrawable->setCornerRadius(0.00001F);
        maskDrawable->setColor(Color::WHITE);

        //ColorStateList* rippleColor =  RippleUtils.convertToRippleDrawableColor(tabRippleColorStateList);

        // TODO: Add support to RippleUtils.compositeRippleColorStateList for different ripple color
        // for selected items vs non-selected items
        /*if (Build::VERSION::SDK_INT >= Build::VERSION_CODES::LOLLIPOP) {
            background =new RippleDrawable(
                  rippleColor,
                  unboundedRipple ? null : contentDrawable,
                  unboundedRipple ? null : maskDrawable);
        } else {
           Drawable* rippleDrawable = DrawableCompat.wrap(maskDrawable);
           rippleDrawable->setTintList(rippleColor);
           background = new LayerDrawable(new Drawable[] {contentDrawable, rippleDrawable});
        }*/
    } else {
        background = contentDrawable;
    }
    setBackground(background);
    mParent->invalidate();
}

bool TabLayout::TabView::performClick(){
    bool handled = LinearLayout::performClick();

    if (mTab) {
        if (!handled)playSoundEffect(SoundEffectConstants::CLICK);
        mTab->select();
        return true;
    } else {
        return handled;
    }
}

void TabLayout::TabView::setSelected(bool selected) {
    const bool changed = isSelected() != selected;

    LinearLayout::setSelected(selected);

    if (changed && selected && Build::VERSION::SDK_INT < 16) {
        // Pre-JB we need to manually send the TYPE_VIEW_SELECTED event
        sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_SELECTED);
    }

    // Always dispatch this to the child views, regardless of whether the value has changed
    if (mTextView)  mTextView->setSelected(selected);

    if (mIconView)  mIconView->setSelected(selected);

    if (mCustomView)mCustomView->setSelected(selected);

}


void TabLayout::TabView::onMeasure(int origWidthMeasureSpec,int origHeightMeasureSpec) {
    int specWidthSize = MeasureSpec::getSize(origWidthMeasureSpec);
    int specWidthMode = MeasureSpec::getMode(origWidthMeasureSpec);
    int maxWidth = mParent->getTabMaxWidth();

    int widthMeasureSpec;
    int heightMeasureSpec = origHeightMeasureSpec;

    if (maxWidth > 0 && (specWidthMode == MeasureSpec::UNSPECIFIED || specWidthSize > maxWidth)) {
        // If we have a max width and a given spec which is either unspecified or
        // larger than the max width, update the width spec using the same mode
        widthMeasureSpec = MeasureSpec::makeMeasureSpec(mParent->mTabMaxWidth, MeasureSpec::AT_MOST);
    } else {
        // Else, use the original width spec
        widthMeasureSpec = origWidthMeasureSpec;
    }

    // Now lets measure
    LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);

    // We need to switch the text size based on whether the text is spanning 2 lines or not
    if (mTextView != nullptr) {
        //Resources res = getResources();
        float textSize = mParent->mTabTextSize;
        int maxLines = mDefaultMaxLines;

        if (mIconView && mIconView->getVisibility() == VISIBLE) {
            // If the icon view is being displayed, we limit the text to 1 line
            maxLines = 1;
        } else if (mTextView && mTextView->getLineCount() > 1) {
            // Otherwise when we have text which wraps we reduce the text size
            textSize = mParent->mTabTextMultiLineSize;
        }

        float curTextSize= mTextView->getTextSize();
        int curLineCount = mTextView->getLineCount();
        int curMaxLines  = mTextView->getMaxLines();

        if (textSize != curTextSize || (curMaxLines >= 0 && maxLines != curMaxLines)) {
             // We've got a new text size and/or max lines...
            bool updateTextView = true;

            if (mParent->mMode == MODE_FIXED && textSize > curTextSize && curLineCount == 1) {
                // If we're in fixed mode, going up in text size and currently have 1 line
                // then it's very easy to get into an infinite recursion.
                // To combat that we check to see if the change in text size
                // will cause a line count change. If so, abort the size change and stick
                // to the smaller size.
                Layout* layout =mTextView->getLayout();
                if (layout == nullptr || approximateLineWidth(layout, 0, textSize)
                        > getMeasuredWidth() - getPaddingLeft() - getPaddingRight()) {
                    updateTextView = false;
                }
            }

            if (updateTextView) {
                mTextView->setTextSize(TypedValue::COMPLEX_UNIT_PX, textSize);
                mTextView->setMaxLines(maxLines);
                LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
            }
        }
    }
}

void TabLayout::TabView::setTab(Tab* tab) {
    if (tab != mTab) {
        mTab = tab;
        update();
    }
}

void TabLayout::TabView::reset() {
    setTab(nullptr);
    setSelected(false);
}

void TabLayout::TabView::update() {
    Tab* tab = mTab;
    View* custom = tab ? tab->getCustomView() : nullptr;
    if (custom != nullptr) {
        ViewGroup* customParent = custom->getParent();
        if (customParent != this) {
            if (customParent != nullptr) {
                ((ViewGroup*) customParent)->removeView(custom);
            }
            addView(custom);
        }
        mCustomView = custom;
        if (mTextView != nullptr) {
            mTextView->setVisibility(GONE);
        }
        if (mIconView != nullptr) {
            mIconView->setVisibility(GONE);
            mIconView->setImageDrawable(nullptr);
        }

        mCustomTextView = (TextView*) custom->findViewById(cdroid::R::id::text1);
        if (mCustomTextView != nullptr) {
            mDefaultMaxLines = mCustomTextView->getMaxLines();
        }
        mCustomIconView = (ImageView*) custom->findViewById(cdroid::R::id::icon);
    } else {
        // We do not have a custom view. Remove one if it already exists
        if (mCustomView != nullptr) {
            removeView(mCustomView);
            mCustomView = nullptr;
        }
        mCustomTextView = nullptr;
        mCustomIconView = nullptr;
    }

    if (mCustomView == nullptr) {
        // If there isn't a custom view, we'll us our own in-built layouts
        LayoutInflater* inflater =LayoutInflater::from(getContext());
        if (mIconView == nullptr) {
            ImageView* iconView = (ImageView*)inflater->inflate("cdroid:layout/design_layout_tab_icon.xml", this, true);
            mIconView = iconView;
        }
        if (mTextView == nullptr) {
            TextView* textView = (TextView*)inflater->inflate("cdroid:layout/design_layout_tab_text.xml", this, true);
            mTextView = textView;
            mDefaultMaxLines = mTextView->getMaxLines();
        }
        //mTextView->setTextAppearance(mTabTextAppearance);
        if (mParent->mTabTextColors) {
            mTextView->setTextColor(mParent->mTabTextColors);
        }
        updateTextAndIcon(mTextView, mIconView);
    } else {
        // Else, we'll see if there is a TextView or ImageView present and update them
        if (mCustomTextView || mCustomIconView ) {
            updateTextAndIcon(mCustomTextView, mCustomIconView);
        }
    }

    // Finally update our selected state
    setSelected(tab && tab->isSelected());
}

int TabLayout::TabView::getContentWidth(){
    bool initialized = false;
    int left = 0;
    int right = 0;
    View*var4[] = {mTextView, mIconView, mCustomView};
    int var5 = sizeof(var4)/sizeof(View*);

    for(int var6 = 0; var6 < var5; ++var6) {
        View* view = var4[var6];
        if (view && view->getVisibility() == View::INVISIBLE) {
            left = initialized ? std::min(left, view->getLeft()) : view->getLeft();
            right = initialized ? std::max(right, view->getRight()) : view->getRight();
            initialized = true;
        }
    }
    return right - left;
}

void TabLayout::TabView::updateTextAndIcon(TextView* textView,ImageView* iconView) {
    Drawable* icon = mTab ? mTab->getIcon() : nullptr;
    std::string text = mTab ? mTab->getText() : "";
    std::string contentDesc = mTab ? mTab->getContentDescription() : "";

    if (iconView != nullptr) {
        if (icon != nullptr) {
            iconView->setImageDrawable(icon);
            iconView->setVisibility(VISIBLE);
            setVisibility(VISIBLE);
        } else {
            iconView->setVisibility(GONE);
            iconView->setImageDrawable(nullptr);
        }
        iconView->setContentDescription(contentDesc);
    }

    bool hasText = !text.empty();//!TextUtils.isEmpty(text);
    if (textView != nullptr) {
        if (hasText) {
            textView->setText(text);
            textView->setVisibility(VISIBLE);
            setVisibility(VISIBLE);
        } else {
            textView->setVisibility(GONE);
            textView->setText("");
        }
        textView->setContentDescription(contentDesc);
    }

    if (iconView != nullptr) {
        MarginLayoutParams* lp = ((MarginLayoutParams*) iconView->getLayoutParams());
        int bottomMargin = 0;
        if (hasText && iconView->getVisibility() == VISIBLE) {
            // If we're showing both text and icon, add some margin bottom to the icon
            bottomMargin = dpToPx(DEFAULT_GAP_TEXT_ICON);
        }
        if (bottomMargin != lp->bottomMargin) {
            lp->bottomMargin = bottomMargin;
            iconView->requestLayout();
        }
    }
    //TooltipCompat.setTooltipText(this, hasText ? null : contentDesc);
}

TabLayout::Tab* TabLayout::TabView::getTab() {
    return mTab;
}

/**
 * Approximates a given lines width with the new provided text size.
 */
float TabLayout::TabView::approximateLineWidth(Layout* layout, int line, float textSize) {
    return layout->getLineWidth(line) ;//* (textSize / layout.getPaint().getTextSize());
}

/*-------------------------------------------------------------------------------------------------------*/
TabLayout::SlidingTabStrip::SlidingTabStrip(Context* context,const AttributeSet&atts,TabLayout*parent)
 :LinearLayout(context,atts){
    mParent = parent;
    mIndicatorLeft  = -1;
    mIndicatorRight = -1;
    mIndicatorAnimator = nullptr;
    mSelectedIndicatorHeight= 4;
    mSelectedIndicatorColor = 0x60FF0000;
    setWillNotDraw(false);
}

TabLayout::SlidingTabStrip::~SlidingTabStrip(){
    delete mIndicatorAnimator;
}

void TabLayout::SlidingTabStrip::setSelectedIndicatorColor(int color) {
    if (mSelectedIndicatorColor != color) {
        mSelectedIndicatorColor=color;
        postInvalidateOnAnimation();
    }
}

void TabLayout::SlidingTabStrip::setSelectedIndicatorHeight(int height) {
    if (mSelectedIndicatorHeight != height) {
        mSelectedIndicatorHeight = height;
        postInvalidateOnAnimation();
    }
}

bool TabLayout::SlidingTabStrip::childrenNeedLayout() {
    for (int i = 0, z = getChildCount(); i < z; i++) {
        View* child = getChildAt(i);
        if (child->getWidth() <= 0) {
            return true;
        }
    }
    return false;
}

void TabLayout::SlidingTabStrip::setIndicatorPositionFromTabPosition(int position, float positionOffset) {
    if (mIndicatorAnimator && mIndicatorAnimator->isRunning()) {
        mIndicatorAnimator->cancel();
    }

    mSelectedPosition = position;
    mSelectionOffset = positionOffset;
    updateIndicatorPosition();
}

float TabLayout::SlidingTabStrip::getIndicatorPosition() {
    return mSelectedPosition + mSelectionOffset;
}

void TabLayout::SlidingTabStrip::onRtlPropertiesChanged(int layoutDirection) {
    LinearLayout::onRtlPropertiesChanged(layoutDirection);

    // Workaround for a bug before Android M where LinearLayout did not relayout itself when
    // layout direction changed.
    if (true){//Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
        //noinspection WrongConstant
        if (mLayoutDirection != layoutDirection) {
            requestLayout();
            mLayoutDirection = layoutDirection;
        }
    }
}

void TabLayout::SlidingTabStrip::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);

    if (MeasureSpec::getMode(widthMeasureSpec) != MeasureSpec::EXACTLY) {
        // HorizontalScrollView will first measure use with UNSPECIFIED, and then with
        // EXACTLY. Ignore the first call since anything we do will be overwritten anyway
        return;
    }

    if (mParent->mMode == MODE_FIXED && mParent->mTabGravity == GRAVITY_CENTER) {
        const int count = getChildCount();

        // First we'll find the widest tab
        int largestTabWidth = 0;
        for (int i = 0, z = count; i < z; i++) {
            View* child = getChildAt(i);
            if (child->getVisibility() == VISIBLE) {
                largestTabWidth = std::max(largestTabWidth, child->getMeasuredWidth());
            }
        }

        if (largestTabWidth <= 0) {
            // If we don't have a largest child yet, skip until the next measure pass
            return;
        }

        int gutter = dpToPx(TabLayout::FIXED_WRAP_GUTTER_MIN);
        bool remeasure = false;

        if (largestTabWidth * count <= getMeasuredWidth() - gutter * 2) {
            // If the tabs fit within our width minus gutters, we will set all tabs to have
            // the same width
            for (int i = 0; i < count; i++) {
                LinearLayout::LayoutParams* lp =(LayoutParams*) getChildAt(i)->getLayoutParams();
                if (lp->width != largestTabWidth || lp->weight != 0) {
                    lp->width = largestTabWidth;
                    lp->weight = 0;
                    remeasure = true;
                }
            }
        } else {
            // If the tabs will wrap to be larger than the width minus gutters, we need
            // to switch to GRAVITY_FILL
            mParent->mTabGravity = GRAVITY_FILL;
            mParent->updateTabViews(false);
            remeasure = true;
        }

        if (remeasure) {
            // Now re-measure after our changes
            LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
        }
    }
}

void TabLayout::SlidingTabStrip::onLayout(bool changed, int l, int t, int w, int h) {
    LinearLayout::onLayout(changed, l, t, w, h);

    if (mIndicatorAnimator && mIndicatorAnimator->isRunning()) {
        // If we're currently running an animation, lets cancel it and start a
        // new animation with the remaining duration
        mIndicatorAnimator->cancel();
        long duration = mIndicatorAnimator->getDuration();
        animateIndicatorToPosition(mSelectedPosition,
                std::round((1.f - mIndicatorAnimator->getAnimatedFraction()) * duration));
    } else {
        // If we've been layed out, update the indicator position
        updateIndicatorPosition();
    }
}

void TabLayout::SlidingTabStrip::updateIndicatorPosition() {
    View* selectedTitle = getChildAt(mSelectedPosition);
    int left, right;

    if (selectedTitle && selectedTitle->getWidth() > 0) {
        left = selectedTitle->getLeft();
        right= selectedTitle->getRight();
        if(!mParent->tabIndicatorFullWidth && dynamic_cast<TabLayout::TabView*>(selectedTitle)){
            calculateTabViewContentBounds((TabLayout::TabView*)selectedTitle,mParent->tabViewContentBounds);
            left = mParent->tabViewContentBounds.left;
            right= mParent->tabViewContentBounds.right();
        }
        if (mSelectionOffset > .0f && mSelectedPosition < getChildCount() - 1) {
            // Draw the selection partway between the tabs
            View* nextTitle = getChildAt(mSelectedPosition + 1);
            int nextTitleLeft = nextTitle->getLeft();
            int nextTitleRight= nextTitle->getRight(); 
            if(!mParent->tabIndicatorFullWidth && dynamic_cast<TabLayout::TabView*>(nextTitle)){
                calculateTabViewContentBounds((TabLayout::TabView*)selectedTitle,mParent->tabViewContentBounds);
                nextTitleLeft = mParent->tabViewContentBounds.left;
                nextTitleRight= mParent->tabViewContentBounds.right();
            }
            left = (int) (mSelectionOffset * nextTitle->getLeft() +
                    (1.0f - mSelectionOffset) * left);
            right= (int) (mSelectionOffset * nextTitle->getRight() +
                    (1.0f - mSelectionOffset) * right);
        }
    } else {
        left = right = -1;
    }

    setIndicatorPosition(left, right);
}

void TabLayout::SlidingTabStrip::setIndicatorPosition(int left, int right) {
    if (left != mIndicatorLeft || right != mIndicatorRight) {
        // If the indicator's left/right has changed, invalidate
        mIndicatorLeft = left;
        mIndicatorRight= right;
        postInvalidateOnAnimation();
    }
}

void TabLayout::SlidingTabStrip::animateIndicatorToPosition(int position, int duration) {
    if (mIndicatorAnimator && mIndicatorAnimator->isRunning()) {
        mIndicatorAnimator->cancel();
    }

    const bool isRtl = getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;

    View* targetView = getChildAt(position);
    if (targetView == nullptr) {
        // If we don't have a view, just update the position now and return
        updateIndicatorPosition();
    }else{

        int targetLeft = targetView->getLeft();
        int targetRight= targetView->getRight();

        if (!mParent->tabIndicatorFullWidth && dynamic_cast<TabLayout::TabView*>(targetView)){
            // If the views are adjacent, we'll animate from edge-to-edge
            calculateTabViewContentBounds((TabLayout::TabView*)targetView,mParent->tabViewContentBounds);
            targetLeft = mParent->tabViewContentBounds.left;
            targetRight= mParent->tabViewContentBounds.right();
        } 
        int startLeft = mIndicatorLeft;
        int startRight= mIndicatorRight;

        if( mIndicatorAnimator==nullptr){ 
            mIndicatorAnimator = new ValueAnimator();
            mIndicatorAnimator->setInterpolator(FastOutSlowInInterpolator::Instance);
            mIndicatorAnimator->setFloatValues({.0f,1.f});
        }
        if (startLeft != targetLeft || startRight != targetRight) {
            ValueAnimator* animator = mIndicatorAnimator;
            animator->setDuration(duration);
            animator->removeAllListeners();
            animator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this,startLeft,targetLeft,startRight,targetRight](ValueAnimator&anim) {
                const float fraction = anim.getAnimatedFraction();
                setIndicatorPosition((int)MathUtils::lerp(float(startLeft), float(targetLeft), fraction),
                                     (int)MathUtils::lerp(float(startRight), float(targetRight), fraction));
            }));
            Animator::AnimatorListener al;
            al.onAnimationEnd=[this,position](Animator&anim,bool reverse){
                mSelectedPosition = position;
                mSelectionOffset = .0f;
            };

            animator->addListener(al);
            animator->start();
        }
    }

}

void TabLayout::SlidingTabStrip::calculateTabViewContentBounds(TabLayout::TabView* tabView, Rect& contentBounds){
    int tabViewContentWidth = tabView->getContentWidth();
    if (tabViewContentWidth <  dpToPx(24)) {
         tabViewContentWidth = dpToPx(24);
    }
    int tabViewCenter = (tabView->getLeft() + tabView->getRight()) / 2;
    int contentLeftBounds = tabViewCenter - tabViewContentWidth / 2;
    int contentRightBounds = tabViewCenter + tabViewContentWidth / 2;
    contentBounds.set((float)contentLeftBounds, 0.0F, tabViewContentWidth, 0.0F);
}

void TabLayout::SlidingTabStrip::draw(Canvas& canvas) {
    LinearLayout::draw(canvas);
    // Thick colored underline below the current selection
    int indicatorHeight= mSelectedIndicatorHeight;
    int indicatorTop   = 0;
    int indicatorBottom= 0;
    switch(mParent->getTabIndicatorGravity()&Gravity::VERTICAL_GRAVITY_MASK){
    case Gravity::BOTTOM: 
        indicatorTop   = getHeight()-indicatorHeight;
        indicatorBottom= getHeight();
        break;
    case Gravity::CENTER_VERTICAL:
        indicatorTop    = (getHeight()-indicatorHeight)/2;
        indicatorBottom = (getHeight()+indicatorHeight)/2; 
        break;
    case Gravity::TOP:
        indicatorTop = 0;
        indicatorBottom = indicatorHeight;
        break;
    case Gravity::FILL_VERTICAL:
        indicatorTop = 0;
        indicatorBottom = getHeight();
        break;
    }
    if (mIndicatorLeft >= 0 && mIndicatorRight > mIndicatorLeft) {
        Drawable*d =mParent->mTabSelectedIndicator;
        if(d){
            d->setBounds(mIndicatorLeft,indicatorTop, mIndicatorRight-mIndicatorLeft,indicatorBottom-indicatorTop);
            d->setColorFilter(mSelectedIndicatorColor,PorterDuffMode::SRC_IN);
            d->draw(canvas);
        }else{
            canvas.set_color(mSelectedIndicatorColor);        
            canvas.rectangle(mIndicatorLeft,indicatorTop, mIndicatorRight-mIndicatorLeft,indicatorBottom-indicatorTop);
            canvas.fill();
        }
    }
}

/*----------------------------------------------------------------*/

TabLayout::TabLayoutOnPageChangeListener::TabLayoutOnPageChangeListener(){
    mTabLayout = nullptr;
    onPageSelected=std::bind(&TabLayoutOnPageChangeListener::doPageSelected,this,std::placeholders::_1);
    onPageScrolled=std::bind(&TabLayoutOnPageChangeListener::doPageScrolled,this,std::placeholders::_1,
                            std::placeholders::_2,std::placeholders::_3);
    onPageScrollStateChanged=std::bind(&TabLayoutOnPageChangeListener::doPageScrollStateChanged,
          this,std::placeholders::_1);
}


void TabLayout::TabLayoutOnPageChangeListener::reset(){
    mPreviousScrollState = mScrollState = ViewPager::SCROLL_STATE_IDLE;
}

void TabLayout::TabLayoutOnPageChangeListener::doPageScrollStateChanged(int state){
    mPreviousScrollState = mScrollState;
    mScrollState = state;
}

void TabLayout::TabLayoutOnPageChangeListener::doPageScrolled(int position,float positionOffset,int positionOffsetPixels){
    if (mTabLayout) {
        // Only update the text selection if we're not settling, or we are settling after
        // being dragged
        bool updateText = mScrollState != ViewPager::SCROLL_STATE_SETTLING ||
               mPreviousScrollState == ViewPager::SCROLL_STATE_DRAGGING;
        // Update the indicator if we're not settling after being idle. This is caused
        // from a setCurrentItem() call and will be handled by an animation from
        // onPageSelected() instead.
        bool updateIndicator = !(mScrollState == ViewPager::SCROLL_STATE_SETTLING
                && mPreviousScrollState == ViewPager::SCROLL_STATE_IDLE);
        mTabLayout->setScrollPosition(position, positionOffset, updateText, updateIndicator);
    }
}

void TabLayout::TabLayoutOnPageChangeListener::doPageSelected(int position){
    if (mTabLayout  && mTabLayout->getSelectedTabPosition() != position
                    && position < mTabLayout->getTabCount()) {
        // Select the tab, only updating the indicator if we're not being dragged/settled
        // (since onPageScrolled will handle that).
        const bool updateIndicator = mScrollState == ViewPager::SCROLL_STATE_IDLE
                || (mScrollState == ViewPager::SCROLL_STATE_SETTLING
                && mPreviousScrollState == ViewPager::SCROLL_STATE_IDLE);
        mTabLayout->selectTab(mTabLayout->getTabAt(position), updateIndicator);
    }
}

TabLayout::PagerAdapterObserver::PagerAdapterObserver(TabLayout*tab){
    mTabLayout = tab;
}

void TabLayout::PagerAdapterObserver::onChanged(){
    mTabLayout->populateFromPagerAdapter();
}

void TabLayout::PagerAdapterObserver::onInvalidated(){
    mTabLayout->populateFromPagerAdapter();
}

void TabLayout::PagerAdapterObserver::clearSavedState(){
    mTabLayout->populateFromPagerAdapter();
}

/*------------------------------------------------------------------------------*/
}//endof namespace
