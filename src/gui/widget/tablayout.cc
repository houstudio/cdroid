#include <widget/tablayout.h>

namespace cdroid{
static constexpr int DEFAULT_HEIGHT_WITH_TEXT_ICON = 72;
static constexpr int DEFAULT_GAP_TEXT_ICON = 8;
static constexpr int DEFAULT_HEIGHT = 48;
static constexpr int TAB_MIN_WIDTH_MARGIN = 56;
static constexpr int MIN_INDICATOR_WIDTH  = 24;
//   static final int FIXED_WRAP_GUTTER_MIN = 16;
static constexpr int INVALID_WIDTH = -1;
static constexpr int ANIMATION_DURATION = 300;

TabLayout::TabLayout(int w,int h):HorizontalScrollView(w,h){
    initTabLayout();
}

TabLayout::TabLayout(Context*context,const AttributeSet&atts)
  :HorizontalScrollView(context,atts){
    initTabLayout();
}

void TabLayout::initTabLayout(){
    AttributeSet atts;
    mMode = MODE_SCROLLABLE;
    mTabPaddingStart= mTabPaddingTop = 0;
    mTabPaddingEnd = mTabPaddingBottom=0;
    mTabTextSize = 20;
    mTabTextColors  = nullptr;
    mSelectedTab    = nullptr;
    mScrollAnimator = nullptr;
    mViewPager = nullptr;
    mPagerAdapter =nullptr;
    mAdapterChangeListener = nullptr;
    mRequestedTabMinWidth = INVALID_WIDTH;
    mRequestedTabMaxWidth = INVALID_WIDTH;
    mTabTextMultiLineSize =2;
    mScrollableTabMinWidth =100;
    mTabStrip = new SlidingTabStrip(getContext(),atts,this);
    HorizontalScrollView::addView(mTabStrip, 0, new HorizontalScrollView::LayoutParams(
          LayoutParams::WRAP_CONTENT, LayoutParams::MATCH_PARENT));
    applyModeAndGravity();
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
    
    LOGD("item.Text=%s",item->mText.c_str());
    if (item->mIcon) {
        tab->setIcon(item->mIcon);
    }

    //tab->setCustomView(item->mCustomLayout);

    //tab->setContentDescription(item->getContentDescription());//getContentDescription inherited from View.
    addTab(tab);
}

void TabLayout::addOnTabSelectedListener(OnTabSelectedListener listener){
    mSelectedListeners.push_back(listener);
}

void TabLayout::removeOnTabSelectedListener(OnTabSelectedListener listener) {
    //mSelectedListeners.remove(listener);
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

    /*for (Iterator<Tab> i = mTabs.iterator(); i.hasNext();) {
        Tab tab = i.next();
        i.remove();
        tab.reset();
        //sTabPool.release(tab);
    }*/
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


void TabLayout::setTabTextColors(ColorStateList* textColor) {
    if (mTabTextColors != textColor) {
        mTabTextColors = textColor;
        updateAllTabs();
    }
}

ColorStateList* TabLayout::getTabTextColors()const{
    return mTabTextColors;
}

void TabLayout::setTabTextColors(int normalColor, int selectedColor){
    setTabTextColors(createColorStateList(normalColor, selectedColor));
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
            mViewPager->setCurrentItem(tab.getPosition());
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
        AttributeSet atts;
        tabView = new TabView(getContext(),atts,this);
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

View& TabLayout::addView(View* child){
    return addViewInternal(child);
}

View& TabLayout::addView(View* child, int index){
    return addViewInternal(child);
}

View& TabLayout::addView(View* child, ViewGroup::LayoutParams* params){
    return addViewInternal(child);
}

View& TabLayout::addView(View* child, int index, ViewGroup::LayoutParams* params){
    return addViewInternal(child);
}

View& TabLayout::addViewInternal(View* child){
    if (dynamic_cast<TabItem*>(child)){
        child->setId(getTabCount()); 
        addTabFromItemView((TabItem*) child);
    }else LOGE("Only TabItem instances can be added to TabLayout");
    return *child;
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

int dpToPx(int dps) {
    return dps;//Math.round(getResources().getDisplayMetrics().density * dps);
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
    int targetScrollX = calculateScrollXForTab(newPosition, 0);

    if (startScrollX != targetScrollX) {
        ensureScrollAnimator();
        mScrollAnimator->setIntValues({startScrollX, targetScrollX});
        mScrollAnimator->start();
    }

    // Now animate the indicator
    mTabStrip->animateIndicatorToPosition(newPosition, ANIMATION_DURATION);
}

void TabLayout::ensureScrollAnimator(){
     if (mScrollAnimator == nullptr) {
        mScrollAnimator = new ValueAnimator();
        mScrollAnimator->setInterpolator(new FastOutSlowInInterpolator());
        mScrollAnimator->setDuration(ANIMATION_DURATION);
        mScrollAnimator->addUpdateListener([this](ValueAnimator&anim) {
           IntPropertyValuesHolder*ip=(IntPropertyValuesHolder*)anim.getValues()[0]; 
           scrollTo(ip->getAnimatedValue(), 0);
        });
    }
}

void TabLayout::setScrollAnimatorListener(Animator::AnimatorListener listener){
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
    //ViewCompat.setPaddingRelative(mTabStrip, paddingStart, 0, 0, 0);

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
    LOGD("requestLayout=%d",requestLayout);
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
    std::vector<std::vector<int>>states;// = new int[2][];
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
    return hasIconAndText ? DEFAULT_HEIGHT_WITH_TEXT_ICON : DEFAULT_HEIGHT;
}

int TabLayout::getTabMinWidth() {
    if (mRequestedTabMinWidth != INVALID_WIDTH) {
        // If we have been given a min width, use it
        return mRequestedTabMinWidth;
    }
    // Else, we'll use the default value
    return mMode == MODE_SCROLLABLE ? mScrollableTabMinWidth : 0;
}

LayoutParams* TabLayout::generateLayoutParams(const AttributeSet& attrs){
    return generateDefaultLayoutParams();
}

int TabLayout::getTabMaxWidth() {
    return mTabMaxWidth;
}



///////////////////////////////////////////////////////////////////////////////////////////
TabLayout::TabItem::TabItem():View(0,0){
    mIcon=nullptr;
}
TabLayout::TabItem::TabItem(Context* context,const AttributeSet& attrs):View(context,attrs){
    mIcon=nullptr;
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
    /*if (mTabBackgroundResId != 0) {
        //ViewCompat.setBackground(this, AppCompatResources.getDrawable(context, mTabBackgroundResId));
    }*/
    //ViewCompat.setPaddingRelative(this, mTabPaddingStart, mTabPaddingTop, mTabPaddingEnd, mTabPaddingBottom);
    setGravity(Gravity::CENTER);
    setOrientation(VERTICAL);
    setClickable(true);
    //ViewCompat.setPointerIcon(this,PointerIconCompat.getSystemIcon(getContext(), PointerIconCompat.TYPE_HAND));
}

bool TabLayout::TabView::performClick(){
    bool handled = LinearLayout::performClick();

    if (mTab) {
        //if (!handled)playSoundEffect(SoundEffectConstants::CLICK);
        mTab->select();
        return true;
    } else {
        return handled;
    }
}

void TabLayout::TabView::setSelected(bool selected) {
    bool changed = isSelected() != selected;

    LinearLayout::setSelected(selected);

    /*if (changed && selected && Build.VERSION.SDK_INT < 16) {
        // Pre-JB we need to manually send the TYPE_VIEW_SELECTED event
        sendAccessibilityEvent(AccessibilityEvent.TYPE_VIEW_SELECTED);
    }*/

    // Always dispatch this to the child views, regardless of whether the value has
    // changed
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
        int curMaxLines  = mTextView->getMaxLines();// TextViewCompat.getMaxLines(mTextView);

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
                mTextView->setTextSize(textSize);//TypedValue.COMPLEX_UNIT_PX, textSize);
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

        mCustomTextView = (TextView*) custom->findViewById(CUSTOM_ID_TEXT);//android.R.id.text1);
        if (mCustomTextView != nullptr) {
            mDefaultMaxLines = mCustomTextView->getMaxLines();// TextViewCompat.getMaxLines(mCustomTextView);
        }
        mCustomIconView = (ImageView*) custom->findViewById(CUSTOM_ID_ICON);//android.R.id.icon);
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
        if (mIconView == nullptr) {
            ImageView* iconView = new ImageView(0,0);//(ImageView*) LayoutInflater.from(getContext()).inflate(R.layout.design_layout_tab_icon, this, false);
            addView(iconView, 0);
            mIconView = iconView;
        }
        if (mTextView == nullptr) {
            TextView* textView = new TextView("",0,0);//(TextView*) LayoutInflater.from(getContext()).inflate(R.layout.design_layout_tab_text, this, false);
            addView(textView);
            mTextView = textView;
            mDefaultMaxLines = mTextView->getMaxLines();
        }
        //TextViewCompat.setTextAppearance(mTextView, mTabTextAppearance);
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
        //iconView->setContentDescription(contentDesc);
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
        //textView->setContentDescription(contentDesc);
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
    mParent=parent;
    mIndicatorLeft =-1;
    mIndicatorRight=-1;
    mIndicatorAnimator=nullptr;
    mSelectedIndicatorHeight=4;
    mSelectedIndicatorColor=0x60FF0000;
    setWillNotDraw(false);
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
        int count = getChildCount();

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

void TabLayout::SlidingTabStrip::onLayout(bool changed, int l, int t, int r, int b) {
    LOGD("SlidingTabStrip::onLayout(%d,%d,%d,%d) childs=%d",l,t,r,b,getChildCount());
    LinearLayout::onLayout(changed, l, t, r, b);

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
        right = selectedTitle->getRight();

        if (mSelectionOffset > .0f && mSelectedPosition < getChildCount() - 1) {
            // Draw the selection partway between the tabs
            View* nextTitle = getChildAt(mSelectedPosition + 1);
            left = (int) (mSelectionOffset * nextTitle->getLeft() +
                    (1.0f - mSelectionOffset) * left);
            right = (int) (mSelectionOffset * nextTitle->getRight() +
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
        mIndicatorRight = right;
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
        return;
    }

    int targetLeft = targetView->getLeft();
    int targetRight= targetView->getRight();
    int startLeft;
    int startRight;

    if (std::abs(position - mSelectedPosition) <= 1) {
        // If the views are adjacent, we'll animate from edge-to-edge
        startLeft = mIndicatorLeft;
        startRight = mIndicatorRight;
    } else {
        // Else, we'll just grow from the nearest edge
        int offset = dpToPx(MOTION_NON_ADJACENT_OFFSET);
        if (position < mSelectedPosition) {
            // We're going end-to-start
            startLeft = startRight = isRtl ?(targetLeft - offset ):(targetRight + offset);
        } else {
            // We're going start-to-end
            startLeft = startRight = isRtl ?(targetRight + offset):(targetLeft - offset);
        }
    }

    if( mIndicatorAnimator==nullptr){
        mIndicatorAnimator = new ValueAnimator();
        mIndicatorAnimator->setInterpolator(new FastOutSlowInInterpolator());
        mIndicatorAnimator->setFloatValues({.0f,1.f});
    }
    if (startLeft != targetLeft || startRight != targetRight) {
        ValueAnimator* animator = mIndicatorAnimator;
        animator->setDuration(duration);
        animator->removeAllListeners();
        animator->addUpdateListener([this,startLeft,targetLeft,startRight,targetRight](ValueAnimator&anim) {
            const float fraction = anim.getAnimatedFraction();
            setIndicatorPosition(lerp(startLeft, targetLeft, fraction),lerp(startRight, targetRight, fraction));
        });
        Animator::AnimatorListener al;
        al.onAnimationEnd=[this,position](Animator&anim,bool reverse){
            mSelectedPosition = position;
            mSelectionOffset = .0f;
        };

        animator->addListener(al);
        animator->start();
    }

}

void TabLayout::SlidingTabStrip::draw(Canvas& canvas) {
    LinearLayout::draw(canvas);
    // Thick colored underline below the current selection
    if (mIndicatorLeft >= 0 && mIndicatorRight > mIndicatorLeft) {
        canvas.set_color(mSelectedIndicatorColor);        
        canvas.rectangle(mIndicatorLeft, getHeight() - mSelectedIndicatorHeight,
                mIndicatorRight-mIndicatorLeft, mSelectedIndicatorHeight);
        canvas.fill();
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
