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

DECLARE_WIDGET(TabLayout)

TabLayout::TabLayout(int w,int h):HorizontalScrollView(w,h){
    initTabLayout();
    applyModeAndGravity();
}

TabLayout::TabLayout(Context*context,const AttributeSet&atts)
  :HorizontalScrollView(context,atts){
    initTabLayout();

    const int animationMode = atts.getInt("tabIndicatorAnimationMode",std::unordered_map<std::string,int>{
            {"linear",(int)INDICATOR_ANIMATION_MODE_LINEAR},
            {"elastic",(int)INDICATOR_ANIMATION_MODE_ELASTIC},
            {"fade",(int)INDICATOR_ANIMATION_MODE_FADE},
            },INDICATOR_ANIMATION_MODE_LINEAR);
    setTabIndicatorAnimationMode(animationMode);

    setSelectedTabIndicator(atts.getDrawable("tabIndicator"));
    setSelectedTabIndicatorColor(atts.getColor("tabIndicatorColor",0));
    mSlidingTabIndicator->setSelectedIndicatorHeight(atts.getDimensionPixelSize("tabIndicatorHeight",2));
    setSelectedTabIndicatorGravity(atts.getGravity("tabIndicatorGravity",0));
    setTabIndicatorFullWidth(atts.getBoolean("tabIndicatorFullWidth",true));


    mTabPaddingStart = mTabPaddingTop = mTabPaddingEnd =
        mPaddingBottom = atts.getDimensionPixelSize("tabPadding",0);
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

    mTabIndicatorAnimationDuration = atts.getInt("tabIndicatorAnimationDuration",ANIMATION_DURATION);
    mRequestedTabMinWidth = atts.getDimensionPixelSize("tabMinWidth", -1);
    mRequestedTabMaxWidth = atts.getDimensionPixelSize("tabMaxWidth", -1);

    mTabBackgroundResId= atts.getString("tabBackground");
    mContentInsetStart  = atts.getDimensionPixelSize("tabContentStart", 0);
    mMode = atts.getInt("tabMode",std::unordered_map<std::string,int>{
            {"scrollable",(int)MODE_SCROLLABLE},
            {"fixed",(int)MODE_FIXED},
            {"auto",(int)MODE_AUTO}},mMode);
    mSmoothScroll = atts.getBoolean("smoothScroll",true);
    mTabGravity = atts.getInt("tabGravity",std::unordered_map<std::string,int>{
            {"fill",(int)GRAVITY_FILL},
            {"center",(int)GRAVITY_CENTER},
            {"start",(int)GRAVITY_START}},GRAVITY_FILL);
    mInlineLabel= atts.getBoolean("tabInlineLabel",false);
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
    delete mTabIndicatorInterpolator;
}

void TabLayout::initTabLayout(){
    AttributeSet atts(getContext(),"cdroid");
    mMode = MODE_SCROLLABLE;
    mInlineLabel =false;
    mTabPaddingStart= mTabPaddingTop   = 0;
    mTabPaddingEnd  = mTabPaddingBottom= 0;
    mTabTextSize = 20;
    mContentInsetStart = 0;
    mTabIndicatorHeight =-1;
    mOwnedTabTextColors= false;
    mTabTextColors  = nullptr;
    mSelectedTab    = nullptr;
    mScrollAnimator = nullptr;
    mViewPager    = nullptr;
    mPagerAdapter = nullptr;
    mTabSelectedIndicatorColor =0;
    mTabIndicatorTimeInterpolator = nullptr;
    mTabIndicatorInterpolator = nullptr;
    mTabIndicatorGravity  = Gravity::BOTTOM;
    mPagerAdapterObserver = nullptr;
    mAdapterChangeListener= nullptr;
    mTabSelectedIndicator = nullptr;
    mRequestedTabMinWidth = INVALID_WIDTH;
    mRequestedTabMaxWidth = INVALID_WIDTH;
    mTabTextMultiLineSize = 2;
    mScrollableTabMinWidth= 100;
    mTabIndicatorFullWidth = true;
    mViewPagerScrollState = ViewPager::SCROLL_STATE_IDLE;
    mTabIndicatorAnimationMode = INDICATOR_ANIMATION_MODE_LINEAR;
    mSlidingTabIndicator = new SlidingTabIndicator(getContext(),atts,this);
    HorizontalScrollView::addView(mSlidingTabIndicator, 0, new HorizontalScrollView::LayoutParams(
          LayoutParams::WRAP_CONTENT, LayoutParams::MATCH_PARENT));
}

static void setTint(Drawable* drawable,int color) {
    const bool hasTint = color != 0;
    if (hasTint) {
        drawable->setTint( color);
    } else {
        drawable->setTintList((ColorStateList*)nullptr);
    }

}

void TabLayout::setSelectedTabIndicatorColor( int color){
    mTabSelectedIndicatorColor = color;//TODO mTabSelectedIndicatorColor
    setTint(mTabSelectedIndicator,mTabSelectedIndicatorColor);
    updateTabViews(false);
}

void TabLayout::setSelectedTabIndicatorHeight(int height) {
    mTabIndicatorHeight = height;
    mSlidingTabIndicator->setSelectedIndicatorHeight(height);
}

void TabLayout::setScrollPosition(int position, float positionOffset, bool updateSelectedText){
    setScrollPosition(position, positionOffset, updateSelectedText, true);
}

void TabLayout::setScrollPosition(int position, float positionOffset, bool updateSelectedTabView, bool updateIndicatorPosition) {
    setScrollPosition(position, positionOffset, updateSelectedTabView, updateIndicatorPosition, true);
}

void TabLayout::setScrollPosition(int position, float positionOffset, bool updateSelectedText, bool updateIndicatorPosition,bool alwaysScroll){
    const int roundedPosition = std::round(position + positionOffset);
    if (roundedPosition < 0 || roundedPosition >= mSlidingTabIndicator->getChildCount()) {
        return;
    }

    // Set the indicator position, if enabled
    if (updateIndicatorPosition) {
        mSlidingTabIndicator->setIndicatorPositionFromTabPosition(position, positionOffset);
    }

    // Now update the scroll position, canceling any running animation
    if (mScrollAnimator && mScrollAnimator->isRunning()) {
        mScrollAnimator->cancel();
    }

    const int scrollXForPosition = calculateScrollXForTab(position, positionOffset);
    const int scrollX = getScrollX();
    bool toMove = ((position < getSelectedTabPosition()) && (scrollXForPosition >= scrollX))
        || ((position > getSelectedTabPosition()) && (scrollXForPosition <= scrollX)) || (position == getSelectedTabPosition());
    if(getLayoutDirection()==View::LAYOUT_DIRECTION_RTL){
        toMove = ((position < getSelectedTabPosition()) && (scrollXForPosition <= scrollX))
            || ((position > getSelectedTabPosition()) && (scrollXForPosition >= scrollX))
            || (position == getSelectedTabPosition());
    }
    if(toMove||(mViewPagerScrollState==ViewPager::SCROLL_STATE_DRAGGING)||alwaysScroll){
        scrollTo(calculateScrollXForTab(position, positionOffset), 0);
    }

    // Update the 'selected state' view as we scroll, if enabled
    if (updateSelectedText) {
        setSelectedTabView(roundedPosition);
    }
}

float TabLayout::getScrollPosition()const{
    return mSlidingTabIndicator->getIndicatorPosition();
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

bool TabLayout::isScrollingEnabled() const{
    return getTabMode() == MODE_SCROLLABLE || getTabMode() == MODE_AUTO;
}

bool TabLayout::onInterceptTouchEvent(MotionEvent& event) {
    return isScrollingEnabled() && HorizontalScrollView::onInterceptTouchEvent(event);
}

bool TabLayout::onTouchEvent(MotionEvent& event) {
    return event.getActionMasked() == MotionEvent::ACTION_SCROLL && !isScrollingEnabled() ? false : HorizontalScrollView::onTouchEvent(event);
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
    const int selectedTabPosition = mSelectedTab ? mSelectedTab->getPosition() : 0;
    removeTabViewAt(position);

    Tab* removedTab =(position<mTabs.size()&&position>=0)?mTabs.at(position):nullptr;
    if (removedTab) {
        removedTab->reset();
        delete removedTab;
        mTabs.erase(mTabs.begin()+position);
    }

    const int newTabCount = mTabs.size();
    for (int i = position; i < newTabCount; i++) {
        mTabs.at(i)->setPosition(i);
    }

    if (selectedTabPosition == position) {
        selectTab(mTabs.empty() ? nullptr : mTabs.at(std::max(0, position - 1)),true);
    }
}

void TabLayout::removeAllTabs(){
    for (int i = mSlidingTabIndicator->getChildCount() - 1; i >= 0; i--) {
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

Drawable*TabLayout::getTabSelectedIndicator()const{
    return mTabSelectedIndicator;
}

void TabLayout::setSelectedTabIndicator(Drawable*tabSelectedIndicator){
    if(tabSelectedIndicator==nullptr){
        tabSelectedIndicator = new GradientDrawable();
        ((GradientDrawable*)tabSelectedIndicator)->setColor(0xFFFFFFFF);
    }
    if(mTabSelectedIndicator!=tabSelectedIndicator){
        delete mTabSelectedIndicator;
        mTabSelectedIndicator = tabSelectedIndicator->mutate();
        setTint(mTabSelectedIndicator,mTabSelectedIndicatorColor);
        const int indicatorHeight = mTabIndicatorHeight == -1 ? mTabSelectedIndicator->getIntrinsicHeight() : mTabIndicatorHeight;
        mSlidingTabIndicator->setSelectedIndicatorHeight(indicatorHeight);
        mSlidingTabIndicator->postInvalidateOnAnimation();
    }
}

void TabLayout::setSelectedTabIndicator(const std::string&res){
    setSelectedTabIndicator(getContext()->getDrawable(res));
}

int TabLayout::getTabIndicatorGravity()const{
    return mTabIndicatorGravity;
}

void TabLayout::setSelectedTabIndicatorGravity(int gravity){
    if(mTabIndicatorGravity!=gravity){
         mTabIndicatorGravity =gravity;
         mSlidingTabIndicator->postInvalidateOnAnimation();
    }
}

void TabLayout::setTabIndicatorAnimationMode(int tabIndicatorAnimationMode) {
    mTabIndicatorAnimationMode = tabIndicatorAnimationMode;
    delete mTabIndicatorInterpolator;
    mTabIndicatorInterpolator = nullptr;
    switch (tabIndicatorAnimationMode) {
    case INDICATOR_ANIMATION_MODE_LINEAR:
        mTabIndicatorInterpolator = new TabIndicatorInterpolator();
        break;
    case INDICATOR_ANIMATION_MODE_ELASTIC:
        mTabIndicatorInterpolator = new ElasticTabIndicatorInterpolator();
        break;
    case INDICATOR_ANIMATION_MODE_FADE:
        mTabIndicatorInterpolator = new FadeTabIndicatorInterpolator();
        break;
    default:
        throw std::invalid_argument(std::string("invalid TabIndicatorAnimationMode:")+std::to_string(tabIndicatorAnimationMode));
    }
}

int TabLayout::getTabIndicatorAnimationMode() const{
    return mTabIndicatorAnimationMode;
}

bool TabLayout::isTabIndicatorFullWidth()const{
    return mTabIndicatorFullWidth;
}

void TabLayout::setTabIndicatorFullWidth(bool tabIndicatorFullWidth) {
    mTabIndicatorFullWidth = tabIndicatorFullWidth;
    mSlidingTabIndicator->jumpIndicatorToSelectedPosition();
    mSlidingTabIndicator->postInvalidateOnAnimation();
}

bool TabLayout::isInlineLabel()const{
    return mInlineLabel;
}

void TabLayout::setInlineLabel(bool v){
    if(mInlineLabel==v)return;
    mInlineLabel=v;
    for(int i=0;i<mSlidingTabIndicator->getChildCount();i++){
        View*child=mSlidingTabIndicator->getChildAt(i);
        if(dynamic_cast<TabLayout::TabView*>(child)){
            TabLayout::TabView*tv=(TabLayout::TabView*)child;
            //tv->updateOrientation();
        }  
    }
    applyModeAndGravity();
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
            LOGD("selectTab %d/%d",tab.getPosition(),getTabCount());
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

void TabLayout::updateViewPagerScrollState(int scrollState) {
    mViewPagerScrollState = scrollState;
}

bool TabLayout::shouldDelayChildPressedState(){
     return getTabScrollRange() > 0;
}

void TabLayout::onAttachedToWindow() {
    HorizontalScrollView::onAttachedToWindow();
    //MaterialShapeUtils.setParentAbsoluteElevation(this);
    if (mViewPager == nullptr) {
        ViewGroup* vp = getParent();
        if (dynamic_cast<ViewPager*>(vp)) {
            setupWithViewPager((ViewPager*)vp, true, true);
        }
    }

}

void TabLayout::onDetachedFromWindow() {
    HorizontalScrollView::onDetachedFromWindow();
    if (mSetupViewPagerImplicitly) {
        setupWithViewPager(nullptr);
        mSetupViewPagerImplicitly = false;
    }
}

int TabLayout::getTabScrollRange() {
    return std::max(0, mSlidingTabIndicator->getWidth() - getWidth() - getPaddingLeft()
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
    const int count = mTabs.size();
    for (int i = position + 1; i < count; i++) {
        mTabs[i]->setPosition(i);
    }
}

void TabLayout::addTabView(TabLayout::Tab* tab){
    TabView* tabView = (TabView*)tab->mView;
    mSlidingTabIndicator->addView(tabView, tab->getPosition(), createLayoutParamsForTabs());
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
       lp->weight = 1.0f;
    } else {
       lp->width = LinearLayout::LayoutParams::WRAP_CONTENT;
       lp->weight = 0.0f;
    }
}

static int dpToPx(int dps) {
    return dps;//Math.round(getDisplayMetrics().density * dps);
}

void TabLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    const int idealHeight = dpToPx(getDefaultHeight()) + getPaddingTop() + getPaddingBottom();
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
            const int childHeightMeasureSpec = getChildMeasureSpec(heightMeasureSpec, getPaddingTop()
                    + getPaddingBottom(), child->getLayoutParams()->height);
            const int childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(
                    getMeasuredWidth(), MeasureSpec::EXACTLY);
            child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
        }
    }
}

void TabLayout::removeTabViewAt(int position){
    TabView* view = (TabView*) mSlidingTabIndicator->getChildAt(position);
    mSlidingTabIndicator->removeViewAt(position);
    if (view) {
        view->reset();
        delete view;
        //mTabViewPool.release(view);
    }
    requestLayout();
}

void TabLayout::animateToTab(int newPosition){
    if (newPosition == Tab::INVALID_POSITION) {
        return;
    }

    if ( false==isLaidOut()  || mSlidingTabIndicator->childrenNeedLayout()) {
        // If we don't have a window token, or we haven't been laid out yet just draw the new
        // position now
        setScrollPosition(newPosition, 0.0f, true);
        return;
    }

    const int startScrollX = getScrollX();
    const int targetScrollX= calculateScrollXForTab(newPosition, 0);

    if (startScrollX != targetScrollX) {
        ensureScrollAnimator();
        mScrollAnimator->setFloatValues({(float)startScrollX, (float)targetScrollX});
        mScrollAnimator->start();
    }

    // Now animate the indicator
    mSlidingTabIndicator->animateIndicatorToPosition(newPosition,mTabIndicatorAnimationDuration);//ANIMATION_DURATION);
}

void TabLayout::ensureScrollAnimator(){
     if (mScrollAnimator == nullptr) {
        mScrollAnimator = new ValueAnimator();
        mScrollAnimator->setInterpolator(FastOutSlowInInterpolator::Instance);
        mScrollAnimator->setDuration(mTabIndicatorAnimationDuration);//ANIMATION_DURATION);
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
    const int tabCount = mSlidingTabIndicator->getChildCount();
    if (position < tabCount) {
        for (int i = 0; i < tabCount; i++) {
            View* child = mSlidingTabIndicator->getChildAt(i);
            child->setSelected(i == position);
            child->setActivated(i == position);
            if(dynamic_cast<TabView*>(child)){
                ((TabView*)child)->updateTab();
            }
        }
    }
}

void TabLayout::selectTab(TabLayout::Tab*tab){
    selectTab(tab,true);
}

void TabLayout::selectTab(TabLayout::Tab* tab,bool updateIndicator){
    Tab* currentTab = mSelectedTab;
    if (currentTab == tab) {
        if (currentTab!=nullptr) {
            dispatchTabReselected(tab);
            animateToTab(tab->getPosition());
        }
    } else {
        const int newPosition = tab ? tab->getPosition() : Tab::INVALID_POSITION;
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

        mSelectedTab = tab;
        if ((currentTab!=nullptr)&&(currentTab->mParent!=nullptr)){
            dispatchTabUnselected(currentTab);
        }
        if (tab){
            dispatchTabSelected(tab);
        }
    }    
}

void TabLayout::dispatchTabSelected(Tab* tab) {
    for (auto ls:mSelectedListeners) {
        if(ls.onTabSelected){
            ls.onTabSelected(*tab);
        }
    }
}

void TabLayout::dispatchTabUnselected(Tab* tab) {
    for (auto ls:mSelectedListeners) {
        if(ls.onTabUnselected){
            ls.onTabUnselected(*tab);
        }
    }
}

void TabLayout::dispatchTabReselected(Tab* tab) {
    for (auto ls:mSelectedListeners) {
        if(ls.onTabReselected){
            ls.onTabReselected(*tab);
        }
    }
}

int TabLayout::calculateScrollXForTab(int position, float positionOffset){
    if (mMode == MODE_SCROLLABLE) {
        View* selectedChild = mSlidingTabIndicator->getChildAt(position);
        if(selectedChild==nullptr){
            return 0;
        } else {
            View* nextChild = position + 1 < mSlidingTabIndicator->getChildCount()
                 ? mSlidingTabIndicator->getChildAt(position + 1): nullptr;
            const int selectedWidth = selectedChild ? selectedChild->getWidth() : 0;
            const int nextWidth = nextChild  ? nextChild->getWidth() : 0;

            // base scroll amount: places center of tab in center of parent
            const int scrollBase = selectedChild->getLeft() + (selectedWidth / 2) - (getWidth() / 2);
            // offset amount: fraction of the distance between centers of tabs
            const int scrollOffset = (int) ((selectedWidth + nextWidth) * 0.5f * positionOffset);

            return (getLayoutDirection() == LAYOUT_DIRECTION_LTR)
                    ? scrollBase + scrollOffset : scrollBase - scrollOffset;
        }
    }
    return 0;
}

void TabLayout::applyModeAndGravity(){
    int paddingStart = 0;
    if (mMode == MODE_SCROLLABLE) {
        // If we're scrollable, or fixed at start, inset using padding
        paddingStart = std::max(0, mContentInsetStart - mTabPaddingStart);
    }
    mSlidingTabIndicator->setPaddingRelative(paddingStart, 0, 0, 0);

    switch (mMode) {
    case MODE_SCROLLABLE:
        applyGravityForModeScrollable(mTabGravity);
        break;
    case MODE_FIXED:
    case MODE_AUTO:
        LOGW_IF(mTabGravity==2,"GRAVITY_START is not supported with the current tab mode, GRAVITY_CENTER will be used instead");
        mSlidingTabIndicator->setGravity(Gravity::CENTER_HORIZONTAL);
        break;
    }
    updateTabViews(true);
}

void TabLayout::applyGravityForModeScrollable(int tabGravity) {
    switch (tabGravity) {
    case GRAVITY_FILL:
        LOGW("MODE_SCROLLABLE + GRAVITY_FILL is not supported, GRAVITY_START will be used instead");
    case GRAVITY_START:
        mSlidingTabIndicator->setGravity(Gravity::LEFT|Gravity::CENTER_VERTICAL);//0x800003
        break;
    case GRAVITY_CENTER:
        mSlidingTabIndicator->setGravity(Gravity::CENTER_HORIZONTAL);
        break;
    }

}

void TabLayout::updateTabViews(bool requestLayout){
    LOGD("requestLayout=%d mintabwidth=%d %d children",requestLayout,getTabMinWidth(),mSlidingTabIndicator->getChildCount());
    for (int i = 0; i < mSlidingTabIndicator->getChildCount(); i++) {
        View* child = mSlidingTabIndicator->getChildAt(i);
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

int TabLayout::getDefaultHeight() const{
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

int TabLayout::getTabMinWidth() const{
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

int TabLayout::getTabMaxWidth() const{
    return mTabMaxWidth;
}

///////////////////////////////////////////////////////////////////////////////////////////
DECLARE_WIDGET3(TabLayout::TabItem,TabItem,"")

TabLayout::TabItem::TabItem():View(0,0){
    mIcon = nullptr;
}

TabLayout::TabItem::TabItem(Context* context,const AttributeSet& attrs):View(context,attrs){
    mText = attrs.getString("text");
    mIcon = attrs.getDrawable("icon");
    LOGV("%s,%p",mText.c_str(),mIcon);
}

TabLayout::TabItem::~TabItem(){
    //delete mIcon;/*cant destroy mIcon duetoTabLayout::addTabFromItemView*/
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
    mTag = tag;
}

void*TabLayout::Tab::getTag()const{
    return mTag;
}

View*TabLayout::Tab::getCustomView()const{
    return mCustomView;
}

TabLayout::Tab& TabLayout::Tab::setCustomView(View*v){
    mCustomView = v;
    updateView();
    return *this;
}

TabLayout::Tab& TabLayout::Tab::setCustomView(const std::string&resid){
    View*v = LayoutInflater::from(mParent->getContext())->inflate(resid,nullptr,false);
    return setCustomView(v);
}

Drawable* TabLayout::Tab::getIcon()const{
    return mIcon;
}

TabLayout::Tab& TabLayout::Tab::setIcon(Drawable* icon){
    mIcon = icon;
    if((mParent->mTabGravity==INDICATOR_GRAVITY_CENTER)||(mParent->mMode==MODE_AUTO)){
        mParent->updateTabViews(true);
    }
    updateView();
    return *this;
}

int TabLayout::Tab::getPosition()const{
    return mPosition;
}

void TabLayout::Tab::setPosition(int position){
    mPosition = position;
}

std::string TabLayout::Tab::getText()const{
    return mText;
}

TabLayout::Tab& TabLayout::Tab::setText(const std::string&text){
    mText = text;
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
    mText.clear();
    mContentDesc.clear();
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
    //delete mTab;cant destroied here ,it is owned by TabLayout::mTabs
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
    const bool handled = LinearLayout::performClick();

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

void TabLayout::TabView::updateTab() {
#if 0
    Tab* tab = mTab;
    View* custom = tab != nullptr ? tab->getCustomView() : nullptr;
    if (custom != nullptr) {
        ViewGroup* customParent = custom->getParent();
        if (customParent != this) {
            if (customParent != nullptr) {
                ((ViewGroup*)customParent)->removeView(custom);
            }

            if (mCustomView != nullptr) {
                ViewGroup* customViewParent = mCustomView->getParent();
                if (customViewParent != nullptr) {
                    ((ViewGroup*)customViewParent)->removeView(mCustomView);
                }
            }

            addView(custom);
        }

        mCustomView = custom;
        if (mTextView != nullptr) {
            mTextView->setVisibility(View::GONE);
        }

        if (mIconView != nullptr) {
            mIconView->setVisibility(View::GONE);
            mIconView->setImageDrawable(nullptr);
        }

        mCustomTextView = (TextView*)custom->findViewById(16908308);
        if (mCustomTextView != nullptr) {
            mDefaultMaxLines = mCustomTextView->getMaxLines();
        }

        mCustomIconView = (ImageView*)custom->findViewById(16908294);
    } else {
        if (mCustomView != nullptr) {
            removeView(mCustomView);
            mCustomView = nullptr;
        }

        mCustomTextView = nullptr;
        mCustomIconView = nullptr;
    }

    if (mCustomView == nullptr) {
        if (mIconView == nullptr) {
            inflateAndAddDefaultIconView();
        }

        if (mTextView == nullptr) {
            inflateAndAddDefaultTextView();
            mDefaultMaxLines = mTextView->getMaxLines();
        }

        mTextView->setTextAppearance(mParent->defaultTabTextAppearance);
        if (isSelected() && mParent->selectedTabTextAppearance != -1) {
            mTextView->setTextAppearance(mParent->selectedTabTextAppearance);
        } else {
            mTextView->setTextAppearance(mParent->mTabTextAppearance);
        }

        if (mParent->mTabTextColors != nullptr) {
            mTextView->setTextColor(mParent->mTabTextColors);
        }

        updateTextAndIcon(mTextView, mIconView/*, true*/);
        tryUpdateBadgeAnchor();
        //addOnLayoutChangeListener(this->mIconView);
        //addOnLayoutChangeListener(this->mTextView);
    } else if (mCustomTextView != nullptr || mCustomIconView != nullptr) {
        updateTextAndIcon(mCustomTextView, mCustomIconView/*, false*/);
    }

    /*if (tab != nullptr && !TextUtils::isEmpty(tab->mContentDesc)) {
        setContentDescription(tab->mContentDesc);
    }*/
#endif
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

int TabLayout::TabView::getContentWidth()const{
    bool initialized = false;
    int left = 0;
    int right = 0;
    View*views[] = {mTextView, mIconView, mCustomView};
    for(View*view:views) {
        if (view && view->getVisibility() == View::VISIBLE) {
            left = initialized ? std::min(left, view->getLeft()) : view->getLeft();
            right = initialized ? std::max(right, view->getRight()) : view->getRight();
            initialized = true;
        }
    }
    return right - left;
}

int TabLayout::TabView::getContentHeight() const{
    bool initialized = false;
    int top = 0;
    int bottom = 0;
    View*views[] = {mTextView, mIconView, mCustomView};
    for(View* view : views) {
        if (view != nullptr && view->getVisibility() == View::VISIBLE) {
            top = initialized ? std::min(top, view->getTop()) : view->getTop();
            bottom = initialized ? std::max(bottom, view->getBottom()) : view->getBottom();
            initialized = true;
        }
    }

    return bottom - top;
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
TabLayout::SlidingTabIndicator::SlidingTabIndicator(Context* context,const AttributeSet&atts,TabLayout*parent)
 :LinearLayout(context,atts){
    mParent = parent;
    mIndicatorLeft  = -1;
    mIndicatorRight = -1;
    mIndicatorAnimator = nullptr;
    mSelectedIndicatorHeight= 4;
    mSelectedIndicatorColor = 0x60FF0000;
    setWillNotDraw(false);
}

TabLayout::SlidingTabIndicator::~SlidingTabIndicator(){
    delete mIndicatorAnimator;
}

void TabLayout::SlidingTabIndicator::setSelectedIndicatorColor(int color) {
    if (mSelectedIndicatorColor != color) {
        mSelectedIndicatorColor=color;
        postInvalidateOnAnimation();
    }
}

void TabLayout::SlidingTabIndicator::setSelectedIndicatorHeight(int height) {
    Rect bounds = mParent->mTabSelectedIndicator->getBounds();
    bounds.height = height;
    mParent->mTabSelectedIndicator->setBounds(bounds.left,0,bounds.width,height);
    requestLayout();
}

bool TabLayout::SlidingTabIndicator::childrenNeedLayout() const{
    for (int i = 0, z = getChildCount(); i < z; i++) {
        View* child = getChildAt(i);
        if (child->getWidth() <= 0) {
            return true;
        }
    }
    return false;
}

void TabLayout::SlidingTabIndicator::setIndicatorPositionFromTabPosition(int position, float positionOffset) {
    if (mIndicatorAnimator && mIndicatorAnimator->isRunning()) {
        mIndicatorAnimator->cancel();
    }

    mSelectedPosition = position;
    mSelectionOffset = positionOffset;
    View* firstTitle = getChildAt(position);
    View* nextTitle = getChildAt(position + 1);
    tweenIndicatorPosition(firstTitle, nextTitle, positionOffset);
}

float TabLayout::SlidingTabIndicator::getIndicatorPosition() {
    return mSelectedPosition + mSelectionOffset;
}

void TabLayout::SlidingTabIndicator::onRtlPropertiesChanged(int layoutDirection) {
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

void TabLayout::SlidingTabIndicator::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
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

        const int gutter = dpToPx(TabLayout::FIXED_WRAP_GUTTER_MIN);
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

void TabLayout::SlidingTabIndicator::onLayout(bool changed, int l, int t, int w, int h) {
    LinearLayout::onLayout(changed, l, t, w, h);

    if (mIndicatorAnimator && mIndicatorAnimator->isRunning()) {
        updateOrRecreateIndicatorAnimation(false,mParent->getSelectedTabPosition(),-1);
    } else {
        jumpIndicatorToIndicatorPosition();
    }
}

void TabLayout::SlidingTabIndicator::jumpIndicatorToPosition(int position) {
    if (mParent->mViewPagerScrollState == ViewPager::SCROLL_STATE_IDLE || mParent->getTabSelectedIndicator()->getBounds().left == -1
            && mParent->getTabSelectedIndicator()->getBounds().width == 0) {
        View* currentView = this->getChildAt(position);
        mParent->mTabIndicatorInterpolator->setIndicatorBoundsForTab(mParent, currentView, mParent->mTabSelectedIndicator);
        mParent->mIndicatorPosition = position;
    }
}

void TabLayout::SlidingTabIndicator::jumpIndicatorToSelectedPosition() {
    jumpIndicatorToPosition(mParent->getSelectedTabPosition());
}

void TabLayout::SlidingTabIndicator::jumpIndicatorToIndicatorPosition() {
    if (mParent->mIndicatorPosition == -1) {
        mParent->mIndicatorPosition = mParent->getSelectedTabPosition();
    }

    jumpIndicatorToPosition(mParent->mIndicatorPosition);
}

void TabLayout::SlidingTabIndicator::tweenIndicatorPosition(View* startTitle, View* endTitle, float fraction) {
    const bool hasVisibleTitle = (startTitle != nullptr) && (startTitle->getWidth() > 0);
    if (hasVisibleTitle) {
        mParent->mTabIndicatorInterpolator->updateIndicatorForOffset(mParent, startTitle, endTitle, fraction, mParent->mTabSelectedIndicator);
    } else {
        mParent->mTabSelectedIndicator->setBounds(-1, mParent->mTabSelectedIndicator->getBounds().top,
                0, mParent->mTabSelectedIndicator->getBounds().height);
    }

    postInvalidateOnAnimation();
}

void TabLayout::SlidingTabIndicator::setIndicatorPosition(int left, int right) {
    if (left != mIndicatorLeft || right != mIndicatorRight) {
        // If the indicator's left/right has changed, invalidate
        mIndicatorLeft = left;
        mIndicatorRight= right;
        postInvalidateOnAnimation();
    }
}

void TabLayout::SlidingTabIndicator::animateIndicatorToPosition(int position, int duration) {
    if (mIndicatorAnimator && mIndicatorAnimator->isRunning()) {
        mIndicatorAnimator->cancel();
    }

    updateOrRecreateIndicatorAnimation(true,position,duration);
}

void TabLayout::SlidingTabIndicator::updateOrRecreateIndicatorAnimation(bool recreateAnimation, int position, int duration) {
    if (mParent->mIndicatorPosition != position) {
        View* currentView = getChildAt(mParent->getSelectedTabPosition());
        View* targetView = getChildAt(position);
        if (targetView == nullptr) {
            jumpIndicatorToSelectedPosition();
        } else {
            mParent->mIndicatorPosition = position;
            ValueAnimator::AnimatorUpdateListener updateListener
                =[this,currentView,targetView](ValueAnimator& valueAnimator) {
                 tweenIndicatorPosition(currentView, targetView, valueAnimator.getAnimatedFraction());
            };
            if (recreateAnimation) {
                ValueAnimator* animator = mIndicatorAnimator = new ValueAnimator();
                animator->setInterpolator(mParent->mTabIndicatorTimeInterpolator);
                animator->setDuration((long)duration);
                animator->setFloatValues({0.0F, 1.0F});
                animator->addUpdateListener(updateListener);
                animator->start();
            } else {
                mIndicatorAnimator->removeAllUpdateListeners();
                mIndicatorAnimator->addUpdateListener(updateListener);
            }

        }
    }
}

void TabLayout::SlidingTabIndicator::calculateTabViewContentBounds(TabLayout::TabView* tabView, Rect& contentBounds){
    int tabViewContentWidth = tabView->getContentWidth();
    if (tabViewContentWidth <  dpToPx(24)) {
         tabViewContentWidth = dpToPx(24);
    }
    const int tabViewCenter = (tabView->getLeft() + tabView->getRight()) / 2;
    const int contentLeftBounds = tabViewCenter - tabViewContentWidth / 2;
    const int contentRightBounds = tabViewCenter + tabViewContentWidth / 2;
    contentBounds.set((float)contentLeftBounds, 0.0F, tabViewContentWidth, 0.0F);
}

void TabLayout::SlidingTabIndicator::draw(Canvas& canvas) {
    int indicatorHeight = mParent->mTabSelectedIndicator->getBounds().height;
    // Thick colored underline below the current selection
    if(indicatorHeight<0){
        indicatorHeight= mParent->mTabSelectedIndicator->getIntrinsicHeight();
    }
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

    if (mParent->mTabSelectedIndicator->getBounds().width> 0) {
        Rect indicatorBounds = mParent->mTabSelectedIndicator->getBounds();
        mParent->mTabSelectedIndicator->setBounds(indicatorBounds.left, indicatorTop, indicatorBounds.width, indicatorBottom-indicatorTop);
        mParent->mTabSelectedIndicator->draw(canvas);
    }

    LinearLayout::draw(canvas);
}

/*----------------------------------------------------------------*/

TabLayout::TabLayoutOnPageChangeListener::TabLayoutOnPageChangeListener(){
    mTabLayout = nullptr;
    onPageSelected = std::bind(&TabLayoutOnPageChangeListener::doPageSelected,this,std::placeholders::_1);
    onPageScrolled = std::bind(&TabLayoutOnPageChangeListener::doPageScrolled,this,std::placeholders::_1,
                            std::placeholders::_2,std::placeholders::_3);
    onPageScrollStateChanged = std::bind(&TabLayoutOnPageChangeListener::doPageScrollStateChanged,
          this,std::placeholders::_1);
}


void TabLayout::TabLayoutOnPageChangeListener::reset(){
    mPreviousScrollState = mScrollState = ViewPager::SCROLL_STATE_IDLE;
}

void TabLayout::TabLayoutOnPageChangeListener::doPageScrollStateChanged(int state){
    mPreviousScrollState = mScrollState;
    mScrollState = state;
    if(mTabLayout!=nullptr){
        mTabLayout->updateViewPagerScrollState(mScrollState);
    }
}

void TabLayout::TabLayoutOnPageChangeListener::doPageScrolled(int position,float positionOffset,int positionOffsetPixels){
    if (mTabLayout) {
        // Only update the text selection if we're not settling, or we are settling after
        // being dragged
        const bool updateText = mScrollState != ViewPager::SCROLL_STATE_SETTLING ||
               mPreviousScrollState == ViewPager::SCROLL_STATE_DRAGGING;
        // Update the indicator if we're not settling after being idle. This is caused
        // from a setCurrentItem() call and will be handled by an animation from
        // onPageSelected() instead.
        const bool updateIndicator = !(mScrollState == ViewPager::SCROLL_STATE_SETTLING
                && mPreviousScrollState == ViewPager::SCROLL_STATE_IDLE);
        mTabLayout->setScrollPosition(position, positionOffset, updateText, updateIndicator);
    }
}

void TabLayout::TabLayoutOnPageChangeListener::doPageSelected(int position){
    if (mTabLayout && mTabLayout->getSelectedTabPosition() != position
                    && position < mTabLayout->getTabCount()) {
        // Select the tab, only updating the indicator if we're not being dragged/settled
        // (since onPageScrolled will handle that).
        const bool updateIndicator = ((mScrollState == ViewPager::SCROLL_STATE_IDLE)
                || (mScrollState == ViewPager::SCROLL_STATE_SETTLING))
                && (mPreviousScrollState == ViewPager::SCROLL_STATE_IDLE);
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
//class TabLayout::TabIndicatorInterpolator{
RectF TabLayout::TabIndicatorInterpolator::calculateTabViewContentBounds(TabLayout::TabView* tabView,int minWidth) {
    int tabViewContentWidth = tabView->getContentWidth();
    int tabViewContentHeight = tabView->getContentHeight();
    const int minWidthPx = minWidth;//(int)ViewUtils.dpToPx(tabView->getContext(), minWidth);
    if (tabViewContentWidth < minWidthPx) {
        tabViewContentWidth = minWidthPx;
    }

    const int tabViewCenterX = (tabView->getLeft() + tabView->getRight()) / 2;
    const int tabViewCenterY = (tabView->getTop() + tabView->getBottom()) / 2;
    const int contentLeftBounds = tabViewCenterX - tabViewContentWidth / 2;
    const int contentTopBounds = tabViewCenterY - tabViewContentHeight / 2;
    const int contentRightBounds = tabViewCenterX + tabViewContentWidth / 2;
    const int contentBottomBounds = tabViewCenterY + tabViewCenterX / 2;
    return {(float)contentLeftBounds, (float)contentTopBounds, 
        (float)contentRightBounds-contentLeftBounds, (float)(contentBottomBounds-contentTopBounds)};
}

RectF TabLayout::TabIndicatorInterpolator::calculateIndicatorWidthForTab(TabLayout* tabLayout, View* tab) {
    if (tab == nullptr) {
        return RectF();
    } else {
        return !tabLayout->isTabIndicatorFullWidth() && dynamic_cast<TabLayout::TabView*>(tab) ?
            calculateTabViewContentBounds((TabLayout::TabView*)tab, 24)
            :RectF::MakeLTRB((float)tab->getLeft(), (float)tab->getTop(), (float)tab->getRight(), (float)tab->getBottom());
    }
}

void TabLayout::TabIndicatorInterpolator::setIndicatorBoundsForTab(TabLayout* tabLayout, View* tab,Drawable* indicator){
    RectF startIndicator = calculateIndicatorWidthForTab(tabLayout, tab);
    indicator->setBounds((int)startIndicator.left, indicator->getBounds().top, (int)startIndicator.width, indicator->getBounds().height);
}

void TabLayout::TabIndicatorInterpolator::updateIndicatorForOffset(TabLayout* tabLayout, View* startTitle, View* endTitle,float offset, Drawable* indicator){
    RectF startIndicator = calculateIndicatorWidthForTab(tabLayout, startTitle);
    RectF endIndicator = calculateIndicatorWidthForTab(tabLayout, endTitle);
    const int left = AnimationUtils::lerp((int)startIndicator.left, (int)endIndicator.left, offset);
    const int right= AnimationUtils::lerp((int)startIndicator.right(), (int)endIndicator.right(), offset);
    indicator->setBounds(left, indicator->getBounds().top,right - left, indicator->getBounds().height);
}

//class TabLayout::FadeTabIndicatorInterpolator
void TabLayout::FadeTabIndicatorInterpolator::updateIndicatorForOffset(TabLayout* tabLayout, View* startTitle, View* endTitle, float offset,Drawable* indicator){
    View* tab = offset < 0.5F ? startTitle : endTitle;
    RectF bounds = calculateIndicatorWidthForTab(tabLayout, tab);
    const float alpha = offset < 0.5F ? AnimationUtils::lerp(1.0F, 0.0F, 0.0F, 0.5F, offset) : AnimationUtils::lerp(0.0F, 1.0F, 0.5F, 1.0F, offset);
    indicator->setBounds((int)bounds.left, indicator->getBounds().top, (int)bounds.width, indicator->getBounds().height);
    indicator->setAlpha((int)(alpha * 255.0F));
}

//class TabLayout::ElasticTabIndicatorInterpolator
void TabLayout::ElasticTabIndicatorInterpolator::updateIndicatorForOffset(TabLayout* tabLayout, View* startTitle, View* endTitle, float offset,Drawable* indicator) {
    RectF startIndicator = calculateIndicatorWidthForTab(tabLayout, startTitle);
    RectF endIndicator = calculateIndicatorWidthForTab(tabLayout, endTitle);
    const bool isMovingRight = startIndicator.left < endIndicator.left;
    float leftFraction;
    float rightFraction;
    if (isMovingRight) {
        leftFraction = accInterp(offset);
        rightFraction = decInterp(offset);
    } else {
        leftFraction = decInterp(offset);
        rightFraction = accInterp(offset);
    }
    const int left =AnimationUtils::lerp((int)startIndicator.left, (int)endIndicator.left, leftFraction);
    const int right=AnimationUtils::lerp((int)startIndicator.right(), (int)endIndicator.right(), rightFraction);
    indicator->setBounds(left, indicator->getBounds().top, right-left, indicator->getBounds().height);
}

}//endof namespace
