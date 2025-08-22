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
#include <core/textutils.h>
#include <widget/toolbar.h>
#include <menu/actionmenupresenter.h>
#include <gui_features.h>

namespace cdroid{

DECLARE_WIDGET(Toolbar)

Toolbar::Toolbar(Context*ctx,const AttributeSet&atts):ViewGroup(ctx,atts){
    initToolbar();
    
    mTitleTextAppearance = atts.getString("titleTextAppearance");
    mSubtitleTextAppearance = atts.getString("subtitleTextAppearance");
    mNavButtonStyle= atts.getString("navigationButtonStyle");
    mGravity = atts.getGravity("gravity",mGravity);
    mButtonGravity = atts.getGravity("buttonGravity",Gravity::TOP);
    mTitleMarginStart = mTitleMarginEnd = mTitleMarginTop = mTitleMarginBottom =
              atts.getDimensionPixelOffset("titleMargin", 0);
    const int marginStart = atts.getDimensionPixelOffset("titleMarginStart", -1);
    if (marginStart >= 0) {
        mTitleMarginStart = marginStart;
    }

    const int marginEnd = atts.getDimensionPixelOffset("titleMarginEnd", -1);
    if (marginEnd >= 0) {
        mTitleMarginEnd = marginEnd;
    }

    const int marginTop = atts.getDimensionPixelOffset("titleMarginTop", -1);
    if (marginTop >= 0) {
        mTitleMarginTop = marginTop;
    }

    const int marginBottom = atts.getDimensionPixelOffset("titleMarginBottom", -1);
    if (marginBottom >= 0) {
        mTitleMarginBottom = marginBottom;
    }

    mMaxButtonHeight = atts.getDimensionPixelSize("maxButtonHeight", -1);

    const int contentInsetStart= atts.getDimensionPixelOffset("contentInsetStart",RtlSpacingHelper::UNDEFINED);
    const int contentInsetEnd  = atts.getDimensionPixelOffset("contentInsetEnd", RtlSpacingHelper::UNDEFINED);
    const int contentInsetLeft = atts.getDimensionPixelSize("contentInsetLeft", 0);
    const int contentInsetRight= atts.getDimensionPixelSize("contentInsetRight", 0);

    ensureContentInsets();
    mContentInsets->setAbsolute(contentInsetLeft, contentInsetRight);

    if (contentInsetStart != RtlSpacingHelper::UNDEFINED ||
            contentInsetEnd != RtlSpacingHelper::UNDEFINED) {
        mContentInsets->setRelative(contentInsetStart, contentInsetEnd);
    }

    mContentInsetStartWithNavigation = atts.getDimensionPixelOffset("contentInsetStartWithNavigation", RtlSpacingHelper::UNDEFINED);
    mContentInsetEndWithActions = atts.getDimensionPixelOffset("contentInsetEndWithActions", RtlSpacingHelper::UNDEFINED);

    mCollapseIcon = atts.getDrawable("collapseIcon");
    mCollapseDescription = atts.getString("collapseContentDescription");

    std::string title = atts.getString("title");
    if (!title.empty()){
        setTitle(title);
    }

    std::string subtitle = atts.getString("subtitle");
    if (!subtitle.empty()) {
        setSubtitle(subtitle);
    }

    // Set the default context, since setPopupTheme() may be a no-op.
    mPopupContext = mContext;
    //setPopupTheme(atts.getResourceId(R.styleable.Toolbar_popupTheme, 0));

    Drawable* navIcon = atts.getDrawable("navigationIcon");
    if (navIcon != nullptr) {
        setNavigationIcon(navIcon);
    }

    std::string navDesc = atts.getString("navigationContentDescription");
    if (!navDesc.empty()) {
        setNavigationContentDescription(navDesc);
    }

    Drawable* logo = atts.getDrawable("logo");
    if (logo != nullptr) {
        setLogo(logo);
    }

    std::string logoDesc = atts.getString("logoDescription");
    if (!logoDesc.empty()) {
        setLogoDescription(logoDesc);
    }

    if (atts.hasAttribute("titleTextColor")) {
        setTitleTextColor(atts.getColor("titleTextColor", 0xffffffff));
    }

    if (atts.hasAttribute("subtitleTextColor")) {
        setSubtitleTextColor(atts.getColor("subtitleTextColor", 0xffffffff));
    }
}

void Toolbar::initToolbar(){
    mGravity = Gravity::START | Gravity::CENTER_VERTICAL;
    mCollapsible = false;
    mTitleTextColor = 0xFFFFFFFF;
    mSubtitleTextColor =0xFFFFFFFF;
    mMenuView = nullptr;
    mTitleTextView = nullptr;
    mSubtitleTextView = nullptr;
    mNavButtonView = nullptr;
    mLogoView = nullptr;

    mPopupContext  = nullptr;
    mContentInsets = nullptr;
    mCollapseIcon = nullptr;
    mCollapseButtonView = nullptr;
    mExpandedActionView = nullptr;
    mExpandedMenuPresenter = nullptr;
    mOuterActionMenuPresenter = nullptr;
}

Toolbar::~Toolbar(){
    delete mContentInsets;
}

void Toolbar::onAttachedToWindow(){
    ViewGroup::onAttachedToWindow();

    // If the container is a cluster, unmark itself as a cluster to avoid having nested
    // clusters.
    ViewGroup* parent = getParent();
    while (parent) {
        if (parent->isKeyboardNavigationCluster()) {
            setKeyboardNavigationCluster(false);
            if (parent->getTouchscreenBlocksFocus()) {
                setTouchscreenBlocksFocus(false);
            }
            break;
        }
        parent = parent->getParent();
    }
}

void Toolbar::setTitleMargin(int start, int top, int end, int bottom){
    mTitleMarginStart = start;
    mTitleMarginTop = top;
    mTitleMarginEnd = end;
    mTitleMarginBottom = bottom;

    requestLayout();
}

int Toolbar::getTitleMarginStart()const{
    return mTitleMarginStart;
}

void Toolbar::setTitleMarginStart(int margin) {
    mTitleMarginStart = margin;

    requestLayout();
}

int Toolbar::getTitleMarginTop()const {
    return mTitleMarginTop;
}

void Toolbar::setTitleMarginTop(int margin) {
    mTitleMarginTop = margin;

    requestLayout();
}


int Toolbar::getTitleMarginEnd()const{
    return mTitleMarginEnd;
}


void Toolbar::setTitleMarginEnd(int margin) {
    mTitleMarginEnd = margin;

    requestLayout();
}


int Toolbar::getTitleMarginBottom() const{
    return mTitleMarginBottom;
}

void Toolbar::setTitleMarginBottom(int margin) {
    mTitleMarginBottom = margin;
    requestLayout();
}

void Toolbar::onRtlPropertiesChanged(int layoutDirection){
    ViewGroup::onRtlPropertiesChanged(layoutDirection);
    ensureContentInsets();
    mContentInsets->setDirection(layoutDirection == LAYOUT_DIRECTION_RTL);
}

bool Toolbar::canShowOverflowMenu()const{
    return (getVisibility() == VISIBLE) && (mMenuView != nullptr) && mMenuView->isOverflowReserved();
}

bool Toolbar::isOverflowMenuShowing()const{
    return (mMenuView != nullptr) && mMenuView->isOverflowMenuShowing();
}

bool Toolbar::isOverflowMenuShowPending()const{
    return (mMenuView != nullptr) && mMenuView->isOverflowMenuShowPending();
}

bool Toolbar::showOverflowMenu(){
    return (mMenuView != nullptr) && mMenuView->showOverflowMenu();
}

bool Toolbar::hideOverflowMenu(){
    return (mMenuView != nullptr) && mMenuView->hideOverflowMenu();
}

void Toolbar::setMenu(MenuBuilder* menu, ActionMenuPresenter& outerPresenter){
    if ((menu == nullptr) && (mMenuView == nullptr)) {
        return;
    }
    ensureMenuView();
    MenuBuilder* oldMenu = mMenuView->peekMenu();
    if (oldMenu == menu) {
        return;
    }

    if (oldMenu != nullptr) {
        oldMenu->removeMenuPresenter(mOuterActionMenuPresenter);
        oldMenu->removeMenuPresenter(mExpandedMenuPresenter);
    }

    if (mExpandedMenuPresenter == nullptr) {
        mExpandedMenuPresenter = new ExpandedActionViewMenuPresenter(this);
    }

    outerPresenter.setExpandedActionViewsExclusive(true);
    if (menu != nullptr) {
        menu->addMenuPresenter(&outerPresenter, mPopupContext);
        menu->addMenuPresenter(mExpandedMenuPresenter, mPopupContext);
    } else {
        outerPresenter.initForMenu(mPopupContext, nullptr);
        mExpandedMenuPresenter->initForMenu(mPopupContext, nullptr);
        outerPresenter.updateMenuView(true);
        mExpandedMenuPresenter->updateMenuView(true);
    }
    mMenuView->setPopupTheme(mPopupTheme);
    mMenuView->setPresenter(&outerPresenter);
    mOuterActionMenuPresenter = &outerPresenter;
}

void Toolbar::dismisssPopupMenus(){
    if(mMenuView){
        mMenuView->dismissPopupMenus();
    }
}

bool Toolbar::isTitleTruncated()const{
    if (mTitleTextView == nullptr) {
        return false;
    }

    Layout* titleLayout = mTitleTextView->getLayout();
    if (titleLayout == nullptr) {
        return false;
    }

    const int lineCount = titleLayout->getLineCount();
    for (int i = 0; i < lineCount; i++) {
        if (titleLayout->getEllipsisCount(i) > 0) {
            return true;
        }
    }
    return false;
}

void Toolbar::setLogo(const std::string& resId){
    setLogo(getContext()->getDrawable(resId));
}

void Toolbar::setLogo(Drawable* drawable){
    if (drawable) {
        ensureLogoView();
        if (!isChildOrHidden(mLogoView)) {
            addSystemView(mLogoView, true);
        }
    } else if (mLogoView && isChildOrHidden(mLogoView)) {
        removeView(mLogoView);
        auto it = std::find(mHiddenViews.begin(),mHiddenViews.end(),mLogoView);
        mHiddenViews.erase(it);//mHiddenViews.remove(mLogoView)
    }
    if (mLogoView) {
        mLogoView->setImageDrawable(drawable);
    }
}

Drawable* Toolbar::getLogo()const{
    return mLogoView != nullptr ? mLogoView->getDrawable() : nullptr;
}

void Toolbar::setLogoDescription(const std::string& description){
    if (!description.empty()) {
        ensureLogoView();
    }
    if (mLogoView) {
        mLogoView->setContentDescription(description);
    }    
}

std::string Toolbar::getLogoDescription()const{
    return mLogoView ? mLogoView->getContentDescription() : "";
}

void Toolbar::ensureLogoView() {
    if (mLogoView == nullptr) {
        mLogoView = new ImageView(getContext(),AttributeSet(mContext,"cdroid"));
    }
}

bool Toolbar::hasExpandedActionView()const{
   return mExpandedMenuPresenter && mExpandedMenuPresenter->mCurrentExpandedItem;
}

void Toolbar::collapseActionView(){
    MenuItemImpl* item = (mExpandedMenuPresenter==nullptr)?nullptr
        : mExpandedMenuPresenter->mCurrentExpandedItem;
    if (item != nullptr) {
        item->collapseActionView();
    }
}

std::string Toolbar::getTitle()const{
    return mTitleText;
}

void Toolbar::setTitle(const std::string&title){
    if (!title.empty()) {
        if (mTitleTextView == nullptr) {
            Context* context = getContext();
            mTitleTextView = new TextView(context,AttributeSet(mContext,"cdroid"));
            mTitleTextView->setSingleLine(true);
            mTitleTextView->setEllipsize(Layout::ELLIPSIS_END);
            if (!mTitleTextAppearance.empty()) {
                mTitleTextView->setTextAppearance(mTitleTextAppearance);
            }
            if (mTitleTextColor != 0) {
                 mTitleTextView->setTextColor(mTitleTextColor);
            }
        }
        if (!isChildOrHidden(mTitleTextView)) {
            addSystemView(mTitleTextView, true);
        }
    } else if (mTitleTextView  && isChildOrHidden(mTitleTextView)) {
        removeView(mTitleTextView);
        auto it = std::find(mHiddenViews.begin(),mHiddenViews.end(),mTitleTextView);
        mHiddenViews.erase(it);
    }
    if (mTitleTextView) {
        mTitleTextView->setText(title);
    }
    mTitleText=title;
}

std::string Toolbar::getSubtitle()const{
    return mSubtitleText;
}

void Toolbar::setSubtitle(const std::string&subtitle){
    if (!subtitle.empty()) {
        if (mSubtitleTextView == nullptr) {
            mSubtitleTextView = new TextView(mContext,AttributeSet(mContext,"cdroid"));
            mSubtitleTextView->setSingleLine(true);
            mSubtitleTextView->setEllipsize(Layout::ELLIPSIS_END);
            if (!mSubtitleTextAppearance.empty()) {
                mSubtitleTextView->setTextAppearance(mSubtitleTextAppearance);
            }
            if (mSubtitleTextColor != 0) {
                mSubtitleTextView->setTextColor(mSubtitleTextColor);
            }
        }
        if (!isChildOrHidden(mSubtitleTextView)) {
            addSystemView(mSubtitleTextView, true);
        }
    } else if (mSubtitleTextView && isChildOrHidden(mSubtitleTextView)) {
        removeView(mSubtitleTextView);
        auto it = std::find(mHiddenViews.begin(),mHiddenViews.end(),mSubtitleTextView);
        mHiddenViews.erase(it);
        //mHiddenViews.remove(mSubtitleTextView);
    }
    if (mSubtitleTextView != nullptr) {
        mSubtitleTextView->setText(subtitle);
    }
    mSubtitleText = subtitle;
}

void Toolbar::setTitleTextColor(int color){
    mTitleTextColor = color;
    if (mTitleTextView != nullptr) {
         mTitleTextView->setTextColor(color);
    }
}

void Toolbar::setSubtitleTextColor(int color){
    mSubtitleTextColor = color;
    if (mSubtitleTextView != nullptr) {
         mSubtitleTextView->setTextColor(color);
    }
}

View*Toolbar::getNavigationView()const{
    return mNavButtonView;
}

std::string Toolbar::getNavigationContentDescription()const{
    return mNavButtonView?mNavButtonView->getContentDescription():"";
}

std::string Toolbar::getCollapseContentDescription()const{
    return mCollapseButtonView != nullptr ? mCollapseButtonView->getContentDescription() : std::string();
}

void Toolbar::setCollapseContentDescription(const std::string&description){
    if (!TextUtils::isEmpty(description)) {
        ensureCollapseButtonView();
    }
    if (mCollapseButtonView != nullptr) {
        mCollapseButtonView->setContentDescription(description);
    }
}

Drawable* Toolbar::getCollapseIcon() const{
    return mCollapseButtonView != nullptr ? mCollapseButtonView->getDrawable() : nullptr;
}

void Toolbar::setCollapseIcon(Drawable* icon){
    if (icon != nullptr) {
        ensureCollapseButtonView();
        mCollapseButtonView->setImageDrawable(icon);
    } else if (mCollapseButtonView != nullptr) {
        mCollapseButtonView->setImageDrawable(mCollapseIcon);
    }
}

Menu*Toolbar::getMenu() {
    ensureMenu();
#if ENABLE(MENU)
    return mMenuView->getMenu();
#else
    return nullptr;
#endif
}

void Toolbar::setOverflowIcon(Drawable* icon) {
    ensureMenu();
#if ENABLE(MENU)
    mMenuView->setOverflowIcon(icon);
#endif
}

Drawable* Toolbar::getOverflowIcon(){
    ensureMenu();
#if ENABLE(MENU)
    return mMenuView->getOverflowIcon();
#else
    return nullptr;
#endif
}

void Toolbar::ensureMenu(){
    ensureMenuView();
    if (mMenuView->peekMenu() == nullptr) {
        // Initialize a new menu for the first time.
        MenuBuilder* menu = dynamic_cast<MenuBuilder*>(mMenuView->getMenu());
        if (mExpandedMenuPresenter == nullptr) {
            mExpandedMenuPresenter = new ExpandedActionViewMenuPresenter(this);
        }
        mMenuView->setExpandedActionViewsExclusive(true);
        menu->addMenuPresenter(mExpandedMenuPresenter, mPopupContext);
    }
}

void Toolbar::ensureMenuView(){
    if (mMenuView == nullptr) {
        mMenuView = new ActionMenuView(getContext(),AttributeSet(getContext(),"cdroid"));
        mMenuView->setPopupTheme(mPopupTheme);
        mMenuView->setOnMenuItemClickListener([this](MenuItem&item){
            return (mOnMenuItemClickListener!=nullptr)&&mOnMenuItemClickListener(item);
        });//mMenuViewItemClickListener);
        mMenuView->setMenuCallbacks(mActionMenuPresenterCallback, mMenuBuilderCallback);
        LayoutParams* lp = generateDefaultLayoutParams();
        lp->gravity = Gravity::END | (mButtonGravity & Gravity::VERTICAL_GRAVITY_MASK);
        mMenuView->setLayoutParams(lp);
        addSystemView(mMenuView, false);
    }
}

void Toolbar::setNavigationContentDescription(const std::string&content){
    if(content.length())
        ensureNavButtonView();
    if(mNavButtonView)
        mNavButtonView->setContentDescription(content);
}

void Toolbar::setNavigationIcon(Drawable*icon){
    if (icon != nullptr) {
        ensureNavButtonView();
        if (!isChildOrHidden(mNavButtonView)) {
            addSystemView(mNavButtonView, true);
        }
    } else if (mNavButtonView != nullptr && isChildOrHidden(mNavButtonView)) {
        removeView(mNavButtonView);
        auto itr = std::find(mHiddenViews.begin(),mHiddenViews.end(),mNavButtonView);
        mHiddenViews.erase(itr);//remove(mNavButtonView);
    }
    if (mNavButtonView != nullptr) {
        mNavButtonView->setImageDrawable(icon);
    }
}

Drawable*Toolbar::getNavigationIcon()const{
    return mNavButtonView?mNavButtonView->getDrawable():nullptr;
}

void Toolbar::setNavigationOnClickListener(const View::OnClickListener& ls){
    ensureNavButtonView();
    mNavButtonView->setOnClickListener(ls);  
}

void Toolbar::setOnMenuItemClickListener(const MenuItem::OnMenuItemClickListener& listener){
    mOnMenuItemClickListener = listener;
}

void Toolbar::setContentInsetsRelative(int contentInsetStart, int contentInsetEnd) {
    ensureContentInsets();
    mContentInsets->setRelative(contentInsetStart, contentInsetEnd);
}

int Toolbar::getContentInsetStart()const{
    return mContentInsets ? mContentInsets->getStart() : 0; 
}

int Toolbar::getContentInsetEnd()const{
    return mContentInsets ? mContentInsets->getEnd() : 0;
}

void Toolbar::setContentInsetsAbsolute(int contentInsetLeft, int contentInsetRight){
     ensureContentInsets();
     mContentInsets->setAbsolute(contentInsetLeft, contentInsetRight);
}

int Toolbar::getContentInsetLeft()const{
    return mContentInsets ? mContentInsets->getLeft() : 0;
}

int Toolbar::getContentInsetRight()const{
    return mContentInsets ? mContentInsets->getRight() : 0;
}

int Toolbar::getContentInsetStartWithNavigation()const{
    return mContentInsetStartWithNavigation != RtlSpacingHelper::UNDEFINED
              ? mContentInsetStartWithNavigation : getContentInsetStart();
}

void Toolbar::setContentInsetStartWithNavigation(int insetStartWithNavigation){
    if (insetStartWithNavigation < 0) {
        insetStartWithNavigation = RtlSpacingHelper::UNDEFINED;
    }
    if (insetStartWithNavigation != mContentInsetStartWithNavigation) {
        mContentInsetStartWithNavigation = insetStartWithNavigation;
        if (getNavigationIcon() != nullptr) {
            requestLayout();
        }
    }
}

int Toolbar::getContentInsetEndWithActions()const{
    return mContentInsetEndWithActions != RtlSpacingHelper::UNDEFINED
                ? mContentInsetEndWithActions
                : getContentInsetEnd();
}

void Toolbar::setContentInsetEndWithActions(int insetEndWithActions){
    if (insetEndWithActions < 0) {
        insetEndWithActions = RtlSpacingHelper::UNDEFINED;
    }
    if (insetEndWithActions != mContentInsetEndWithActions) {
        mContentInsetEndWithActions = insetEndWithActions;
        if (getNavigationIcon() != nullptr) {
            requestLayout();
        }
    }
}

int Toolbar::getCurrentContentInsetStart()const{
    return getNavigationIcon() ? std::max(getContentInsetStart(), std::max(mContentInsetStartWithNavigation, 0))
              : getContentInsetStart();
}

int Toolbar::getCurrentContentInsetEnd()const{
     bool hasActions = false;
     if (mMenuView != nullptr) {
         MenuBuilder* mb = mMenuView->peekMenu();
         hasActions = (mb != nullptr) && mb->hasVisibleItems();
     }
     return hasActions
          ? std::max(getContentInsetEnd(), std::max(mContentInsetEndWithActions, 0))
          : getContentInsetEnd();
}

int Toolbar::getCurrentContentInsetLeft()const{
    return isLayoutRtl()
                ? getCurrentContentInsetEnd()
                : getCurrentContentInsetStart();
}

int Toolbar::getCurrentContentInsetRight()const{
    return isLayoutRtl()
            ? getCurrentContentInsetStart()
            : getCurrentContentInsetEnd();
}

void Toolbar::ensureNavButtonView(){
    if (mNavButtonView == nullptr) {
        AttributeSet attrs = mContext->obtainStyledAttributes(mNavButtonStyle);
        mNavButtonView = new ImageButton(getContext(),attrs);
        LayoutParams* lp = (LayoutParams*)generateDefaultLayoutParams();
        lp->gravity = Gravity::START | (mButtonGravity & Gravity::VERTICAL_GRAVITY_MASK);
        mNavButtonView->setLayoutParams(lp);
    }
}

void Toolbar::ensureCollapseButtonView(){
    if (mCollapseButtonView == nullptr) {
        AttributeSet attrs = mContext->obtainStyledAttributes(mNavButtonStyle);
        mCollapseButtonView = new ImageButton(getContext(),attrs);
        mCollapseButtonView->setImageDrawable(mCollapseIcon);
        mCollapseButtonView->setContentDescription(mCollapseDescription);
        LayoutParams* lp = (LayoutParams*)generateDefaultLayoutParams();
        lp->gravity = Gravity::START | (mButtonGravity & Gravity::VERTICAL_GRAVITY_MASK);
        lp->mViewType = LayoutParams::EXPANDED;
        mCollapseButtonView->setLayoutParams(lp);
        mCollapseButtonView->setOnClickListener([this](View&v){
             collapseActionView();
        });
    }
}

void Toolbar::addSystemView(View* v, bool allowHide){
    ViewGroup::LayoutParams* vlp = v->getLayoutParams();
    LayoutParams* lp = nullptr;
    if (vlp == nullptr) {
        lp = (LayoutParams*)generateDefaultLayoutParams();
    } else if (!checkLayoutParams(vlp)) {
        lp = (LayoutParams*)generateLayoutParams(vlp);
    } else {
        lp = (LayoutParams*)vlp;
    }
    lp->mViewType = LayoutParams::SYSTEM;
    if (allowHide && mExpandedActionView) {
        v->setLayoutParams(lp);
        mHiddenViews.push_back(v);
    } else {
        addView(v,lp);
    }
}

void Toolbar::postShowOverflowMenu(){
    removeCallbacks(mShowOverflowMenuRunnable);
    post(mShowOverflowMenuRunnable);
}

void Toolbar::onDetachedFromWindow(){
    ViewGroup::onDetachedFromWindow();
    removeCallbacks(mShowOverflowMenuRunnable);
}

bool Toolbar::onTouchEvent(MotionEvent&ev){
    const int action = ev.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN) {
        mEatingTouch = false;
    }

    if (!mEatingTouch) {
        const bool handled = ViewGroup::onTouchEvent(ev);
        if (action == MotionEvent::ACTION_DOWN && !handled) {
            mEatingTouch = true;
        }
    }

    if ((action == MotionEvent::ACTION_UP) || (action == MotionEvent::ACTION_CANCEL)) {
        mEatingTouch = false;
    }

    return true;
}

void Toolbar::onSetLayoutParams(View* child, ViewGroup::LayoutParams* lp){
    if(checkLayoutParams(lp))
        child->setLayoutParams(generateLayoutParams(lp));
}

void Toolbar::measureChildConstrained(View* child, int parentWidthSpec, int widthUsed,
        int parentHeightSpec, int heightUsed, int heightConstraint){
    const MarginLayoutParams* lp = (const MarginLayoutParams*) child->getLayoutParams();

    int childWidthSpec = getChildMeasureSpec(parentWidthSpec,
            mPaddingLeft + mPaddingRight + lp->leftMargin + lp->rightMargin
                    + widthUsed, lp->width);
    int childHeightSpec = getChildMeasureSpec(parentHeightSpec,
            mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin
                    + heightUsed, lp->height);

    const int childHeightMode = MeasureSpec::getMode(childHeightSpec);
    if (childHeightMode != MeasureSpec::EXACTLY && heightConstraint >= 0) {
        const int size = childHeightMode != MeasureSpec::UNSPECIFIED ?
                std::min(MeasureSpec::getSize(childHeightSpec), heightConstraint) :
                heightConstraint;
        childHeightSpec = MeasureSpec::makeMeasureSpec(size, MeasureSpec::EXACTLY);
    }
    child->measure(childWidthSpec, childHeightSpec);
}

int Toolbar::measureChildCollapseMargins(View* child,int parentWidthMeasureSpec,int widthUsed,
        int parentHeightMeasureSpec, int heightUsed, int*collapsingMargins){
    const MarginLayoutParams* lp = (const MarginLayoutParams*) child->getLayoutParams();

    const int leftDiff = lp->leftMargin - collapsingMargins[0];
    const int rightDiff = lp->rightMargin - collapsingMargins[1];
    const int leftMargin = std::max(0, leftDiff);
    const int rightMargin = std::max(0, rightDiff);
    const int hMargins = leftMargin + rightMargin;
    collapsingMargins[0] = std::max(0, -leftDiff);
    collapsingMargins[1] = std::max(0, -rightDiff);

    const int childWidthMeasureSpec = getChildMeasureSpec(parentWidthMeasureSpec,
            mPaddingLeft + mPaddingRight + hMargins + widthUsed, lp->width);
    const int childHeightMeasureSpec = getChildMeasureSpec(parentHeightMeasureSpec,
            mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin
                    + heightUsed, lp->height);

    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
    return child->getMeasuredWidth() + hMargins;
}

bool Toolbar::shouldCollapse(){
    if (!mCollapsible) return false;
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        if (shouldLayout(child) && child->getMeasuredWidth() > 0 &&
           child->getMeasuredHeight() > 0) {
           return false;
        }
    }
    return true;
}

void Toolbar::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int width = 0;
    int height = 0;
    int childState = 0;

    int collapsingMargins[2];
    int marginStartIndex;
    int marginEndIndex;
    if (isLayoutRtl()) {
        marginStartIndex = 1;
        marginEndIndex = 0;
    } else {
        marginStartIndex = 0;
        marginEndIndex = 1;
    }

    // System views measure first.

    int navWidth = 0;
    if (shouldLayout(mNavButtonView)) {
        measureChildConstrained(mNavButtonView, widthMeasureSpec, width, heightMeasureSpec, 0, mMaxButtonHeight);
        navWidth = mNavButtonView->getMeasuredWidth() + getHorizontalMargins(mNavButtonView);
        height = std::max(height, mNavButtonView->getMeasuredHeight() + getVerticalMargins(mNavButtonView));
        childState = combineMeasuredStates(childState, mNavButtonView->getMeasuredState());
    }

    if (shouldLayout(mCollapseButtonView)) {
        measureChildConstrained(mCollapseButtonView, widthMeasureSpec, width, heightMeasureSpec, 0, mMaxButtonHeight);
        navWidth = mCollapseButtonView->getMeasuredWidth() + getHorizontalMargins(mCollapseButtonView);
        height = std::max(height, mCollapseButtonView->getMeasuredHeight() + getVerticalMargins(mCollapseButtonView));
        childState = combineMeasuredStates(childState, mCollapseButtonView->getMeasuredState());
    }

    const int contentInsetStart = getCurrentContentInsetStart();
    width += std::max(contentInsetStart, navWidth);
    collapsingMargins[marginStartIndex] = std::max(0, contentInsetStart - navWidth);

    int menuWidth = 0;
    if (shouldLayout(mMenuView)) {
        measureChildConstrained(mMenuView, widthMeasureSpec, width, heightMeasureSpec, 0, mMaxButtonHeight);
        menuWidth = mMenuView->getMeasuredWidth() + getHorizontalMargins(mMenuView);
        height = std::max(height, mMenuView->getMeasuredHeight() + getVerticalMargins(mMenuView));
        childState = combineMeasuredStates(childState, mMenuView->getMeasuredState());
    }

    const int contentInsetEnd = getCurrentContentInsetEnd();
    width += std::max(contentInsetEnd, menuWidth);
    collapsingMargins[marginEndIndex] = std::max(0, contentInsetEnd - menuWidth);

    if (shouldLayout(mExpandedActionView)) {
        width += measureChildCollapseMargins(mExpandedActionView, widthMeasureSpec, width, heightMeasureSpec, 0, collapsingMargins);
        height = std::max(height, mExpandedActionView->getMeasuredHeight() + getVerticalMargins(mExpandedActionView));
        childState = combineMeasuredStates(childState, mExpandedActionView->getMeasuredState());
    }

    if (shouldLayout(mLogoView)) {
        width += measureChildCollapseMargins(mLogoView, widthMeasureSpec, width, heightMeasureSpec, 0, collapsingMargins);
        height = std::max(height, mLogoView->getMeasuredHeight() + getVerticalMargins(mLogoView));
        childState = combineMeasuredStates(childState, mLogoView->getMeasuredState());
    }

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if ((lp->mViewType != LayoutParams::CUSTOM) || !shouldLayout(child)) {
            // We already got all system views above. Skip them and GONE views.
            continue;
        }

        width += measureChildCollapseMargins(child, widthMeasureSpec, width, heightMeasureSpec, 0, collapsingMargins);
        height = std::max(height, child->getMeasuredHeight() + getVerticalMargins(child));
        childState = combineMeasuredStates(childState, child->getMeasuredState());
    }

    int titleWidth = 0;
    int titleHeight = 0;
    int titleVertMargins = mTitleMarginTop + mTitleMarginBottom;
    int titleHorizMargins = mTitleMarginStart + mTitleMarginEnd;
    if (shouldLayout(mTitleTextView)) {
        titleWidth = measureChildCollapseMargins(mTitleTextView, widthMeasureSpec, width + titleHorizMargins,
                heightMeasureSpec, titleVertMargins, collapsingMargins);
        titleWidth = mTitleTextView->getMeasuredWidth() + getHorizontalMargins(mTitleTextView);
        titleHeight = mTitleTextView->getMeasuredHeight() + getVerticalMargins(mTitleTextView);
        childState = combineMeasuredStates(childState, mTitleTextView->getMeasuredState());
    }
    if (shouldLayout(mSubtitleTextView)) {
        titleWidth = std::max(titleWidth, measureChildCollapseMargins(mSubtitleTextView,
                widthMeasureSpec, width + titleHorizMargins, heightMeasureSpec,
                titleHeight + titleVertMargins, collapsingMargins));
        titleHeight += mSubtitleTextView->getMeasuredHeight() + getVerticalMargins(mSubtitleTextView);
        childState = combineMeasuredStates(childState, mSubtitleTextView->getMeasuredState());
    }

    width += titleWidth;
    height = std::max(height, titleHeight);

    // Measurement already took padding into account for available space for the children,
    // add it in for the size.
    width += getPaddingLeft() + getPaddingRight();
    height += getPaddingTop() + getPaddingBottom();

    const int measuredWidth = resolveSizeAndState(std::max(width, getSuggestedMinimumWidth()),
            widthMeasureSpec, childState & MEASURED_STATE_MASK);
    const int measuredHeight = resolveSizeAndState( std::max(height, getSuggestedMinimumHeight()),
            heightMeasureSpec, childState << MEASURED_HEIGHT_STATE_SHIFT);

    setMeasuredDimension(measuredWidth, shouldCollapse() ? 0 : measuredHeight);
}

void Toolbar::onLayout(bool changed, int l, int t, int w, int h){
    const bool isRtl = getLayoutDirection() == LAYOUT_DIRECTION_RTL;
    int width = getWidth();
    int height = getHeight();
    int paddingLeft = getPaddingLeft();
    int paddingRight = getPaddingRight();
    int paddingTop = getPaddingTop();
    int paddingBottom = getPaddingBottom();
    int left = paddingLeft;
    int right = width - paddingRight;

    int collapsingMargins[2]={0,0};

    // Align views within the minimum toolbar height, if set.
    int minHeight = getMinimumHeight();
    int alignmentHeight = minHeight >= 0 ? std::min(minHeight, h) : 0;

    if (shouldLayout(mNavButtonView)) {
        if (isRtl) {
            right = layoutChildRight(mNavButtonView, right, collapsingMargins,alignmentHeight);
        } else {
            left = layoutChildLeft(mNavButtonView, left, collapsingMargins, alignmentHeight);
        }
    }

    if (shouldLayout(mCollapseButtonView)) {
        if (isRtl) {
            right = layoutChildRight(mCollapseButtonView, right, collapsingMargins,alignmentHeight);
        } else {
            left = layoutChildLeft(mCollapseButtonView, left, collapsingMargins, alignmentHeight);
        }
    }

    if (shouldLayout(mMenuView)) {
        if (isRtl) {
            left = layoutChildLeft(mMenuView, left, collapsingMargins, alignmentHeight);
        } else {
            right = layoutChildRight(mMenuView, right, collapsingMargins, alignmentHeight);
        }
    }

    int contentInsetLeft = getCurrentContentInsetLeft();
    int contentInsetRight= getCurrentContentInsetRight();
    collapsingMargins[0] = std::max(0, contentInsetLeft - left);
    collapsingMargins[1] = std::max(0, contentInsetRight - (width - paddingRight - right));
    left  = std::max(left, contentInsetLeft);
    right = std::min(right, width - paddingRight - contentInsetRight);
    if (shouldLayout(mExpandedActionView)) {
        if (isRtl) {
            right = layoutChildRight(mExpandedActionView, right, collapsingMargins, alignmentHeight);
        } else {
            left = layoutChildLeft(mExpandedActionView, left, collapsingMargins, alignmentHeight);
        }
    }

    if (shouldLayout(mLogoView)) {
        if (isRtl) {
            right = layoutChildRight(mLogoView, right, collapsingMargins,alignmentHeight);
        } else {
            left = layoutChildLeft(mLogoView, left, collapsingMargins, alignmentHeight);
        }
    }

    const bool layoutTitle = shouldLayout(mTitleTextView);
    const bool layoutSubtitle = shouldLayout(mSubtitleTextView);
    int titleHeight = 0;
    if (layoutTitle) {
        LayoutParams* lp = (LayoutParams*) mTitleTextView->getLayoutParams();
        titleHeight += lp->topMargin + mTitleTextView->getMeasuredHeight() + lp->bottomMargin;
    }
    if (layoutSubtitle) {
        LayoutParams* lp = (LayoutParams*) mSubtitleTextView->getLayoutParams();
        titleHeight += lp->topMargin + mSubtitleTextView->getMeasuredHeight() + lp->bottomMargin;
    }

    if (layoutTitle || layoutSubtitle) {
        int titleTop=0,space=0,spaceAbove=0;
        View* topChild = layoutTitle ? mTitleTextView : mSubtitleTextView;
        View* bottomChild = layoutSubtitle ? mSubtitleTextView : mTitleTextView;
        LayoutParams* toplp = (LayoutParams*) topChild->getLayoutParams();
        LayoutParams* bottomlp = (LayoutParams*) bottomChild->getLayoutParams();
        bool titleHasWidth = layoutTitle && mTitleTextView->getMeasuredWidth() > 0
                    || layoutSubtitle && mSubtitleTextView->getMeasuredWidth() > 0;
        
        switch (mGravity & Gravity::VERTICAL_GRAVITY_MASK) {
        case Gravity::TOP:
            titleTop = getPaddingTop() + toplp->topMargin + mTitleMarginTop;
            break;
        default:
        case Gravity::CENTER_VERTICAL:
            space = height - paddingTop - paddingBottom;
            spaceAbove = (space - titleHeight) / 2;
            if (spaceAbove < toplp->topMargin + mTitleMarginTop) {
                spaceAbove = toplp->topMargin + mTitleMarginTop;
            } else {
                int spaceBelow = height - paddingBottom - titleHeight -spaceAbove - paddingTop;
                if (spaceBelow < toplp->bottomMargin + mTitleMarginBottom) {
                    spaceAbove = std::max(0, spaceAbove -(bottomlp->bottomMargin + mTitleMarginBottom - spaceBelow));
                }
            }
            titleTop = paddingTop + spaceAbove;
            break;
        case Gravity::BOTTOM:
            titleTop = height - paddingBottom - bottomlp->bottomMargin - mTitleMarginBottom -titleHeight;
            break;
        }
        if (isRtl) {
            int rd = (titleHasWidth ? mTitleMarginStart : 0) - collapsingMargins[1];
            right -= std::max(0, rd);
            collapsingMargins[1] = std::max(0, -rd);
            int titleRight = right;
            int subtitleRight = right;
            if (layoutTitle) {
                LayoutParams* lp = (LayoutParams*) mTitleTextView->getLayoutParams();
                int titleLeft = titleRight - mTitleTextView->getMeasuredWidth();
                int titleBottom = titleTop + mTitleTextView->getMeasuredHeight();
                mTitleTextView->layout(titleLeft, titleTop, titleRight, titleBottom);
                titleRight = titleLeft - mTitleMarginEnd;
                titleTop = titleBottom + lp->bottomMargin;
            }
            if (layoutSubtitle) {
                LayoutParams* lp = (LayoutParams*) mSubtitleTextView->getLayoutParams();
                titleTop += lp->topMargin;
                int subtitleLeft = subtitleRight - mSubtitleTextView->getMeasuredWidth();
                int subtitleBottom = titleTop + mSubtitleTextView->getMeasuredHeight();
                mSubtitleTextView->layout(subtitleLeft, titleTop, subtitleRight, subtitleBottom);
                subtitleRight = subtitleRight - mTitleMarginEnd;
                titleTop = subtitleBottom + lp->bottomMargin;
            }
            if (titleHasWidth) {
                right = std::min(titleRight, subtitleRight);
            }
        } else {
            int ld = (titleHasWidth ? mTitleMarginStart : 0) - collapsingMargins[0];
            left += std::max(0, ld);
            collapsingMargins[0] = std::max(0, -ld);
            int titleLeft = left;
            int subtitleLeft = left;

            if (layoutTitle) {
                LayoutParams* lp = (LayoutParams*) mTitleTextView->getLayoutParams();
                int titleRight = titleLeft + mTitleTextView->getMeasuredWidth();
                int titleBottom = titleTop + mTitleTextView->getMeasuredHeight();
                mTitleTextView->layout(titleLeft, titleTop, titleRight, titleBottom);
                titleLeft = titleRight + mTitleMarginEnd;
                titleTop = titleBottom + lp->bottomMargin;
            }
            if (layoutSubtitle) {
                LayoutParams* lp = (LayoutParams*) mSubtitleTextView->getLayoutParams();
                titleTop += lp->topMargin;
                int subtitleRight = subtitleLeft + mSubtitleTextView->getMeasuredWidth();
                int subtitleBottom = titleTop + mSubtitleTextView->getMeasuredHeight();
                mSubtitleTextView->layout(subtitleLeft, titleTop, subtitleRight, subtitleBottom);
                subtitleLeft = subtitleRight + mTitleMarginEnd;
                titleTop = subtitleBottom + lp->bottomMargin;
            }
            if (titleHasWidth) {
                left = std::max(titleLeft, subtitleLeft);
            }
        }
    }

    // Get all remaining children sorted for layout. This is all prepared
    // such that absolute layout direction can be used below.
    std::vector<View*>mTempViews;
    addCustomViewsWithGravity(mTempViews, Gravity::LEFT);
    const size_t leftViewsCount = mTempViews.size();
    for (int i = 0; i < leftViewsCount; i++) {
        left = layoutChildLeft(mTempViews.at(i), left, collapsingMargins,alignmentHeight);
    }

    addCustomViewsWithGravity(mTempViews, Gravity::RIGHT);
    const size_t rightViewsCount = mTempViews.size();
    for (int i = 0; i < rightViewsCount; i++) {
        right = layoutChildRight(mTempViews.at(i), right, collapsingMargins,alignmentHeight);
    }

    // Centered views try to center with respect to the whole bar, but views pinned
    // to the left or right can push the mass of centered views to one side or the other.
    addCustomViewsWithGravity(mTempViews, Gravity::CENTER_HORIZONTAL);
    int centerViewsWidth = getViewListMeasuredWidth(mTempViews, collapsingMargins);
    int parentCenter = paddingLeft + (width - paddingLeft - paddingRight) / 2;
    int halfCenterViewsWidth = centerViewsWidth / 2;
    int centerLeft = parentCenter - halfCenterViewsWidth;
    int centerRight = centerLeft + centerViewsWidth;
    if (centerLeft < left) {
        centerLeft = left;
    } else if (centerRight > right) {
        centerLeft -= centerRight - right;
    }
    const size_t centerViewsCount = mTempViews.size();
    for (int i = 0; i < centerViewsCount; i++) {
        centerLeft = layoutChildLeft(mTempViews.at(i), centerLeft, collapsingMargins,alignmentHeight);
    }

    mTempViews.clear();
}

int Toolbar::getViewListMeasuredWidth(const std::vector<View*>& views, int*collapsingMargins){
    int collapseLeft = collapsingMargins[0];
    int collapseRight = collapsingMargins[1];
    int width = 0;
    const size_t count = views.size();
    for (size_t i = 0; i < count; i++) {
        View* v = views.at(i);
        LayoutParams* lp = (LayoutParams*) v->getLayoutParams();
        const int l = lp->leftMargin - collapseLeft;
        const int r = lp->rightMargin - collapseRight;
        const int leftMargin = std::max(0, l);
        const int rightMargin = std::max(0, r);
        collapseLeft = std::max(0, -l);
        collapseRight = std::max(0, -r);
        width += leftMargin + v->getMeasuredWidth() + rightMargin;
    }
   return width;
}

int Toolbar::layoutChildLeft(View* child, int left, int*collapsingMargins,int alignmentHeight){
    const LayoutParams* lp = (const LayoutParams*) child->getLayoutParams();
    const int l = lp->leftMargin - collapsingMargins[0];
    left += std::max(0, l);
    collapsingMargins[0] = std::max(0, -l);
    const int top = getChildTop(child, alignmentHeight);
    int childWidth = child->getMeasuredWidth();
    child->layout(left, top, childWidth, child->getMeasuredHeight());
    left += childWidth + lp->rightMargin;
    return left;
}

int Toolbar::layoutChildRight(View* child, int right, int*collapsingMargins,int alignmentHeight){
    const LayoutParams* lp = (const LayoutParams*) child->getLayoutParams();
    const int r = lp->rightMargin - collapsingMargins[1];
    right -= std::max(0, r);
    collapsingMargins[1] = std::max(0, -r);
    const int top = getChildTop(child, alignmentHeight);
    const int childWidth = child->getMeasuredWidth();
    child->layout(right - childWidth, top, childWidth, child->getMeasuredHeight());
    right -= childWidth + lp->leftMargin;
    return right;
}

int Toolbar::getChildTop(View* child, int alignmentHeight){
    const LayoutParams* lp = (const LayoutParams*) child->getLayoutParams();
    int childHeight = child->getMeasuredHeight();
    int alignmentOffset = alignmentHeight > 0 ? (childHeight - alignmentHeight) / 2 : 0;
    switch (getChildVerticalGravity(lp->gravity)) {
    case Gravity::TOP:   return getPaddingTop() - alignmentOffset;

    case Gravity::BOTTOM:
        return getHeight() - getPaddingBottom() - childHeight - lp->bottomMargin - alignmentOffset;
    default:
    case Gravity::CENTER_VERTICAL:
        int space = getHeight() - getPaddingTop()-getPaddingBottom();
        int spaceAbove = (space - childHeight) / 2;
        if (spaceAbove < lp->topMargin) {
            spaceAbove = lp->topMargin;
        } else {
            int spaceBelow = getHeight() - -getPaddingBottom() - childHeight -spaceAbove - getPaddingTop();
            if (spaceBelow < lp->bottomMargin) {
                spaceAbove = std::max(0, spaceAbove - (lp->bottomMargin - spaceBelow));
            }
        }
        return getPaddingTop() + spaceAbove;
    }
}

int Toolbar::getChildVerticalGravity(int gravity){
    const int vgrav = gravity & Gravity::VERTICAL_GRAVITY_MASK;
    switch (vgrav) {
    case Gravity::TOP:
    case Gravity::BOTTOM:
    case Gravity::CENTER_VERTICAL:
        return vgrav;
    default:
        return mGravity & Gravity::VERTICAL_GRAVITY_MASK;
    }
}

void Toolbar::addCustomViewsWithGravity(std::vector<View*>& views, int gravity){
    const bool isRtl = getLayoutDirection() == LAYOUT_DIRECTION_RTL;
    const int childCount = getChildCount();
    int absGrav = Gravity::getAbsoluteGravity(gravity, getLayoutDirection());

    views.clear();

    if (isRtl) {
        for (int i = childCount - 1; i >= 0; i--) {
            View* child = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            if (lp->mViewType == LayoutParams::CUSTOM && shouldLayout(child) &&
                    getChildHorizontalGravity(lp->gravity) == absGrav) {
                views.push_back(child);
            }
        }
    } else {
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            if (lp->mViewType == LayoutParams::CUSTOM && shouldLayout(child) &&
                    getChildHorizontalGravity(lp->gravity) == absGrav) {
                views.push_back(child);
            }
        }
    }
}

int Toolbar::getChildHorizontalGravity(int gravity){
    const int ld = getLayoutDirection();
    const int absGrav = Gravity::getAbsoluteGravity(gravity, ld);
    const int hGrav = absGrav & Gravity::HORIZONTAL_GRAVITY_MASK;
    switch (hGrav) {
    case Gravity::LEFT:
    case Gravity::RIGHT:
    case Gravity::CENTER_HORIZONTAL:
        return hGrav;
    default:
        return ld == LAYOUT_DIRECTION_RTL ? Gravity::RIGHT : Gravity::LEFT;
    }
}

bool Toolbar::shouldLayout(View* view){
    return view && view->getParent() == this && view->getVisibility() != View::GONE;
}

int Toolbar::getHorizontalMargins(View* v){
    MarginLayoutParams* mlp = (MarginLayoutParams*) v->getLayoutParams();
    return mlp->getMarginStart() + mlp->getMarginEnd();
}

int Toolbar::getVerticalMargins(View* v){
    const MarginLayoutParams* mlp = (const MarginLayoutParams*) v->getLayoutParams();
    return mlp->topMargin + mlp->bottomMargin;
}

Toolbar::LayoutParams* Toolbar::generateLayoutParams(const AttributeSet& attrs)const{
    return new LayoutParams(getContext(), attrs);
}

Toolbar::LayoutParams* Toolbar::generateLayoutParams(const ViewGroup::LayoutParams* p)const{
    if (dynamic_cast<const LayoutParams*>(p)) {
         return new LayoutParams((const LayoutParams)*p);
    } else if (dynamic_cast<const ActionBar::LayoutParams*>(p)) {
        return new LayoutParams((const ActionBar::LayoutParams&)*p);
    } else if (dynamic_cast<const MarginLayoutParams*>(p)) {
        return new LayoutParams((const MarginLayoutParams&)*p);
    } else {
        return new LayoutParams(*p);
    }
}

Toolbar::LayoutParams* Toolbar::generateDefaultLayoutParams()const{
    return new LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
}

bool Toolbar::checkLayoutParams(const ViewGroup::LayoutParams* p)const{
     return ViewGroup::checkLayoutParams(p) && dynamic_cast<const LayoutParams*>(p);
}

bool Toolbar::isCustomView(View* child) {
    return ((LayoutParams*) child->getLayoutParams())->mViewType == LayoutParams::CUSTOM;
}

void Toolbar::removeChildrenForExpandedActionView() {
    const int childCount = getChildCount();
    // Go backwards since we're removing from the list
    for (int i = childCount - 1; i >= 0; i--) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if (lp->mViewType != LayoutParams::EXPANDED && child != mMenuView) {
            removeViewAt(i);
            mHiddenViews.push_back(child);
        }
    }
}

void Toolbar::addChildrenForExpandedActionView() {
    const int count = (int)mHiddenViews.size();
    // Re-add in reverse order since we removed in reverse order
    for (int i = count - 1; i >= 0; i--) {
        addView(mHiddenViews.at(i));
    }
    mHiddenViews.clear();
}

bool Toolbar::isChildOrHidden(View* child) const{
    auto itr = std::find(mHiddenViews.begin(),mHiddenViews.end(),child);
    return (child->getParent() == this) || (itr!=mHiddenViews.end());
}

void Toolbar::setCollapsible(bool collapsible) {
    mCollapsible = collapsible;
    requestLayout();
}

void Toolbar::setMenuCallbacks(const MenuPresenter::Callback& pcb,const MenuBuilder::Callback& mcb){
    mActionMenuPresenterCallback = pcb;
    mMenuBuilderCallback = mcb;
    if (mMenuView != nullptr) {
        mMenuView->setMenuCallbacks(pcb, mcb);
    }
}

void Toolbar::ensureContentInsets() {
    if (mContentInsets == nullptr) {
        mContentInsets = new RtlSpacingHelper();
    }
}

ActionMenuPresenter* Toolbar::getOuterActionMenuPresenter() {
     return mOuterActionMenuPresenter;
}

Context* Toolbar::getPopupContext() {
     return mPopupContext;
}

//////////////////////////////////////////////////////////////////////////////////////////////

Toolbar::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs)
   :ActionBar::LayoutParams(c, attrs){
}

Toolbar::LayoutParams::LayoutParams(int width, int height)
   :ActionBar::LayoutParams(width, height){
    this->gravity = Gravity::CENTER_VERTICAL | Gravity::START;
}

Toolbar::LayoutParams::LayoutParams(int width, int height, int gravity)
  :ActionBar::LayoutParams(width, height){
    this->gravity = gravity;
}

Toolbar::LayoutParams::LayoutParams(int gravity)
   :LayoutParams(WRAP_CONTENT, MATCH_PARENT, gravity){
}

Toolbar::LayoutParams::LayoutParams(const LayoutParams& source)
   :ActionBar::LayoutParams(source){
    mViewType = source.mViewType;
}

Toolbar::LayoutParams::LayoutParams(const ActionBar::LayoutParams& source)
  :ActionBar::LayoutParams(source){
}

Toolbar::LayoutParams::LayoutParams(const MarginLayoutParams& source)
  :ActionBar::LayoutParams(source){
    // ActionBar::LayoutParams doesn't have a MarginLayoutParams constructor.
    // Fake it here and copy over the relevant data.
    copyMarginsFrom(source);
}

Toolbar::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
  :ActionBar::LayoutParams(source){
}

////////////////////////////////////////////////////////////////////////////////////////////////
Toolbar::ExpandedActionViewMenuPresenter::ExpandedActionViewMenuPresenter(Toolbar*tb)
    :MenuPresenter(),mToolbar(tb){
    mMenu = nullptr;
    mCurrentExpandedItem = nullptr;
}

void Toolbar::ExpandedActionViewMenuPresenter::initForMenu(Context* context,MenuBuilder* menu) {
    // Clear the expanded action view when menus change.
    if (mMenu != nullptr && mCurrentExpandedItem != nullptr) {
        mMenu->collapseItemActionView(mCurrentExpandedItem);
    }
    mMenu = menu;
}

MenuView* Toolbar::ExpandedActionViewMenuPresenter::getMenuView(ViewGroup* root) {
    return nullptr;
}

void Toolbar::ExpandedActionViewMenuPresenter::updateMenuView(bool cleared) {
    // Make sure the expanded item we have is still there.
    if (mCurrentExpandedItem != nullptr) {
        bool found = false;

        if (mMenu != nullptr) {
            const int count = mMenu->size();
            for (int i = 0; i < count; i++) {
                MenuItem* item = mMenu->getItem(i);
                if (item == mCurrentExpandedItem) {
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            // The item we had expanded disappeared. Collapse.
            collapseItemActionView(*mMenu, *mCurrentExpandedItem);
        }
    }
}

void Toolbar::ExpandedActionViewMenuPresenter::setCallback(const Callback& cb) {
}

bool Toolbar::ExpandedActionViewMenuPresenter::onSubMenuSelected(SubMenuBuilder* subMenu) {
    return false;
}

void Toolbar::ExpandedActionViewMenuPresenter::onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing) {
}

bool Toolbar::ExpandedActionViewMenuPresenter::flagActionItems() {
    return false;
}

bool Toolbar::ExpandedActionViewMenuPresenter::expandItemActionView(MenuBuilder& menu, MenuItemImpl& item) {
    mToolbar->ensureCollapseButtonView();
    if (mToolbar->mCollapseButtonView->getParent() != mToolbar) {
        mToolbar->addView(mToolbar->mCollapseButtonView);
    }
    mToolbar->mExpandedActionView = item.getActionView();
    mCurrentExpandedItem = &item;
    if ( mToolbar->mExpandedActionView->getParent() != mToolbar) {
        LayoutParams* lp = mToolbar->generateDefaultLayoutParams();
        lp->gravity = Gravity::START | (mToolbar->mButtonGravity & Gravity::VERTICAL_GRAVITY_MASK);
        lp->mViewType = LayoutParams::EXPANDED;
        mToolbar->mExpandedActionView->setLayoutParams(lp);
        mToolbar->addView(mToolbar->mExpandedActionView);
    }

    mToolbar->removeChildrenForExpandedActionView();
    mToolbar->requestLayout();
    item.setActionViewExpanded(true);

    /*if (mToolbar->mExpandedActionView instanceof CollapsibleActionView) {
        ((CollapsibleActionView) mExpandedActionView).onActionViewExpanded();
    }*/

    return true;
}

bool Toolbar::ExpandedActionViewMenuPresenter::collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item) {
    // Do this before detaching the actionview from the hierarchy, in case
    // it needs to dismiss the soft keyboard, etc.
    /*if (mToolbar->mExpandedActionView instanceof CollapsibleActionView) {
        ((CollapsibleActionView) mExpandedActionView).onActionViewCollapsed();
    }*/

    mToolbar->removeView(mToolbar->mExpandedActionView);
    mToolbar->removeView(mToolbar->mCollapseButtonView);
    mToolbar->mExpandedActionView = nullptr;

    mToolbar->addChildrenForExpandedActionView();
    mCurrentExpandedItem = nullptr;
    mToolbar->requestLayout();
    item.setActionViewExpanded(false);
    return true;
}

int Toolbar::ExpandedActionViewMenuPresenter::getId() const{
    return 0;
}

Parcelable* Toolbar::ExpandedActionViewMenuPresenter::onSaveInstanceState() {
    return nullptr;
}

void Toolbar::ExpandedActionViewMenuPresenter::onRestoreInstanceState(Parcelable& state) {
}
}//namespace
