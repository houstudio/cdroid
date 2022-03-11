#include <widget/toolbar.h>
namespace cdroid{

//DECLARE_WIDGET(ToolBar)

ToolBar::ToolBar(Context*ctx,const AttributeSet&atts):ViewGroup(ctx,atts){

}

void ToolBar::onAttachedToWindow(){
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

void ToolBar::setTitleMargin(int start, int top, int end, int bottom){
    mTitleMarginStart = start;
    mTitleMarginTop = top;
    mTitleMarginEnd = end;
    mTitleMarginBottom = bottom;

    requestLayout();
}

int ToolBar::getTitleMarginStart()const{
    return mTitleMarginStart;
}

void ToolBar::setTitleMarginStart(int margin) {
    mTitleMarginStart = margin;

    requestLayout();
}

int ToolBar::getTitleMarginTop()const {
    return mTitleMarginTop;
}

void ToolBar::setTitleMarginTop(int margin) {
    mTitleMarginTop = margin;

    requestLayout();
}


int ToolBar::getTitleMarginEnd()const{
    return mTitleMarginEnd;
}


void ToolBar::setTitleMarginEnd(int margin) {
    mTitleMarginEnd = margin;

    requestLayout();
}


int ToolBar::getTitleMarginBottom() const{
    return mTitleMarginBottom;
}

void ToolBar::setTitleMarginBottom(int margin) {
    mTitleMarginBottom = margin;
    requestLayout();
}

void ToolBar::setLogo(const std::string& resId){
    setLogo(getContext()->getDrawable(resId));
}

void ToolBar::setLogo(Drawable* drawable){
    if (drawable) {
        ensureLogoView();
        if (!isChildOrHidden(mLogoView)) {
            addSystemView(mLogoView, true);
        }
    } else if (mLogoView && isChildOrHidden(mLogoView)) {
        removeView(mLogoView);
        //mHiddenViews.remove(mLogoView);
    }
    if (mLogoView) {
        mLogoView->setImageDrawable(drawable);
    }
}

Drawable* ToolBar::getLogo()const{
    return mLogoView != nullptr ? mLogoView->getDrawable() : nullptr;
}

void ToolBar::setLogoDescription(const std::string& description){
    if (!description.empty()) {
        ensureLogoView();
    }
    if (mLogoView) {
        mLogoView->setContentDescription(description);
    }    
}

std::string ToolBar::getLogoDescription()const{
    return mLogoView ? mLogoView->getContentDescription() : "";
}

void ToolBar::ensureLogoView() {
    if (mLogoView == nullptr) {
        mLogoView = new ImageView(getContext(),AttributeSet());
    }
}

bool ToolBar::hasExpandedActionView()const{
   return false;//mExpandedMenuPresenter && mExpandedMenuPresenter.mCurrentExpandedItem;
}

void ToolBar::collapseActionView(){

}

std::string ToolBar::getTitle()const{
    return mTitleText;
}

void ToolBar::setTitle(const std::string&title){
    #if 0
    if (!title.empty()) {
       if (mTitleTextView == nullptr) {
           Context* context = getContext();
           mTitleTextView = new TextView(context,AttributeSet());
           mTitleTextView->setSingleLine(true);
           //mTitleTextView->setEllipsize(TextUtils.TruncateAt.END);
           if (mTitleTextAppearance != 0) {
               //mTitleTextView->setTextAppearance(mTitleTextAppearance);
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
       mHiddenViews->remove(mTitleTextView);
    }
    if (mTitleTextView) {
        mTitleTextView->setText(title);
    }
    #endif
    mTitleText=title;
}

std::string ToolBar::getSubtitle()const{
    return mSubtitleText;
}

void ToolBar::setSubtitle(const std::string&){
}

void ToolBar::setTitleTextColor(int color){
    mTitleTextColor = color;
}

void ToolBar::setSubtitleTextColor(int color){
    mSubtitleTextColor = color;
}

std::string ToolBar::getNavigationContentDescription()const{
    return mNavButtonView?mNavButtonView->getContentDescription():"";
}

void ToolBar::setNavigationContentDescription(const std::string&content){
    if(content.length())
        ensureNavButtonView();
    if(mNavButtonView)
        mNavButtonView->setContentDescription(content);
}

void ToolBar::setNavigationIcon(Drawable*){
}

Drawable*ToolBar::getNavigationIcon()const{
    return mNavButtonView?mNavButtonView->getDrawable():nullptr;
}

void ToolBar::setNavigationOnClickListener(View::OnClickListener ls){
    ensureNavButtonView();
    mNavButtonView->setOnClickListener(ls);  
}

View*ToolBar::getNavigationView()const{
    return mNavButtonView;
}

//void ToolBar::setOnMenuItemClickListener(OnMenuItemClickListener listener){}

void ToolBar::setContentInsetsRelative(int contentInsetStart, int contentInsetEnd) {
    //ensureContentInsets();
    //mContentInsets.setRelative(contentInsetStart, contentInsetEnd);
}

int ToolBar::getContentInsetStart()const{
    
}

int ToolBar::getContentInsetEnd()const{
}

void ToolBar::setContentInsetsAbsolute(int contentInsetLeft, int contentInsetRight){
}

int ToolBar::getContentInsetLeft()const{
    return 0;//mContentInsets ? mContentInsets->getLeft() : 0;
}

int ToolBar::getContentInsetRight()const{
    return 0;//mContentInsets ? mContentInsets->getRight() : 0;
}

int ToolBar::getContentInsetStartWithNavigation()const{
    return 0;
}

void ToolBar::setContentInsetStartWithNavigation(int insetStartWithNavigation){
}

int ToolBar::getContentInsetEndWithActions()const{
    return 0;
}

void ToolBar::setContentInsetEndWithActions(int insetEndWithActions){
}

int ToolBar::getCurrentContentInsetStart()const{
}

int ToolBar::getCurrentContentInsetEnd()const{
}

int ToolBar::getCurrentContentInsetLeft()const{
}

int ToolBar::getCurrentContentInsetRight()const{
}

void ToolBar::ensureNavButtonView(){
}

void ToolBar::ensureCollapseButtonView(){
}

void ToolBar::addSystemView(View* v, bool allowHide){
}

void ToolBar::postShowOverflowMenu(){
}

void ToolBar::onDetachedFromWindow(){
}

bool ToolBar::onTouchEvent(MotionEvent&ev){
    int action = ev.getActionMasked();
    if (action == MotionEvent::ACTION_DOWN) {
        mEatingTouch = false;
    }

    if (!mEatingTouch) {
        const bool handled = ViewGroup::onTouchEvent(ev);
        if (action == MotionEvent::ACTION_DOWN && !handled) {
            mEatingTouch = true;
        }
    }

    if (action == MotionEvent::ACTION_UP || action == MotionEvent::ACTION_CANCEL) {
        mEatingTouch = false;
    }

    return true;
}

void ToolBar::onSetLayoutParams(View* child, ViewGroup::LayoutParams* lp){
    if(checkLayoutParams(lp))
        child->setLayoutParams(generateLayoutParams(lp));
}

void ToolBar::measureChildConstrained(View* child, int parentWidthSpec, int widthUsed,
        int parentHeightSpec, int heightUsed, int heightConstraint){
    MarginLayoutParams* lp = (MarginLayoutParams*) child->getLayoutParams();

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

int ToolBar::measureChildCollapseMargins(View* child,int parentWidthMeasureSpec,int widthUsed,
        int parentHeightMeasureSpec, int heightUsed, int*collapsingMargins){
    MarginLayoutParams* lp = (MarginLayoutParams*) child->getLayoutParams();

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

bool ToolBar::shouldCollapse(){
    if (!mCollapsible) return false;
    int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        if (shouldLayout(child) && child->getMeasuredWidth() > 0 &&
           child->getMeasuredHeight() > 0) {
           return false;
        }
    }
    return true;
}

void ToolBar::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int width = 0;
    int height = 0;
    int childState = 0;

    int collapsingMargins[2];// = mTempMargins;
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
        measureChildConstrained(mNavButtonView, widthMeasureSpec, width, heightMeasureSpec, 0,
                mMaxButtonHeight);
        navWidth = mNavButtonView->getMeasuredWidth() + getHorizontalMargins(mNavButtonView);
        height = std::max(height, mNavButtonView->getMeasuredHeight() +
                getVerticalMargins(mNavButtonView));
        childState = combineMeasuredStates(childState, mNavButtonView->getMeasuredState());
    }

    if (shouldLayout(mCollapseButtonView)) {
        measureChildConstrained(mCollapseButtonView, widthMeasureSpec, width,
                heightMeasureSpec, 0, mMaxButtonHeight);
        navWidth = mCollapseButtonView->getMeasuredWidth() +
                getHorizontalMargins(mCollapseButtonView);
        height = std::max(height, mCollapseButtonView->getMeasuredHeight() +
                getVerticalMargins(mCollapseButtonView));
        childState = combineMeasuredStates(childState, mCollapseButtonView->getMeasuredState());
    }

    int contentInsetStart = getCurrentContentInsetStart();
    width += std::max(contentInsetStart, navWidth);
    collapsingMargins[marginStartIndex] = std::max(0, contentInsetStart - navWidth);

    int menuWidth = 0;
    if (shouldLayout(mMenuView)) {
        measureChildConstrained(mMenuView, widthMeasureSpec, width, heightMeasureSpec, 0,
                mMaxButtonHeight);
        menuWidth = mMenuView->getMeasuredWidth() + getHorizontalMargins(mMenuView);
        height = std::max(height, mMenuView->getMeasuredHeight() +
                getVerticalMargins(mMenuView));
        childState = combineMeasuredStates(childState, mMenuView->getMeasuredState());
    }

    int contentInsetEnd = getCurrentContentInsetEnd();
    width += std::max(contentInsetEnd, menuWidth);
    collapsingMargins[marginEndIndex] = std::max(0, contentInsetEnd - menuWidth);

    if (shouldLayout(mExpandedActionView)) {
        width += measureChildCollapseMargins(mExpandedActionView, widthMeasureSpec, width,
                heightMeasureSpec, 0, collapsingMargins);
        height = std::max(height, mExpandedActionView->getMeasuredHeight() +
                getVerticalMargins(mExpandedActionView));
        childState = combineMeasuredStates(childState, mExpandedActionView->getMeasuredState());
    }

    if (shouldLayout(mLogoView)) {
        width += measureChildCollapseMargins(mLogoView, widthMeasureSpec, width,
                heightMeasureSpec, 0, collapsingMargins);
        height = std::max(height, mLogoView->getMeasuredHeight() +
                getVerticalMargins(mLogoView));
        childState = combineMeasuredStates(childState, mLogoView->getMeasuredState());
    }

    int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if (lp->mViewType != LayoutParams::CUSTOM || !shouldLayout(child)) {
            // We already got all system views above. Skip them and GONE views.
            continue;
        }

        width += measureChildCollapseMargins(child, widthMeasureSpec, width,
                heightMeasureSpec, 0, collapsingMargins);
        height = std::max(height, child->getMeasuredHeight() + getVerticalMargins(child));
        childState = combineMeasuredStates(childState, child->getMeasuredState());
    }

    int titleWidth = 0;
    int titleHeight = 0;
    int titleVertMargins = mTitleMarginTop + mTitleMarginBottom;
    int titleHorizMargins = mTitleMarginStart + mTitleMarginEnd;
    if (shouldLayout(mTitleTextView)) {
        titleWidth = measureChildCollapseMargins(mTitleTextView, widthMeasureSpec,
                width + titleHorizMargins, heightMeasureSpec, titleVertMargins,
                collapsingMargins);
        titleWidth = mTitleTextView->getMeasuredWidth() + getHorizontalMargins(mTitleTextView);
        titleHeight = mTitleTextView->getMeasuredHeight() + getVerticalMargins(mTitleTextView);
        childState = combineMeasuredStates(childState, mTitleTextView->getMeasuredState());
    }
    if (shouldLayout(mSubtitleTextView)) {
        titleWidth = std::max(titleWidth, measureChildCollapseMargins(mSubtitleTextView,
                widthMeasureSpec, width + titleHorizMargins,
                heightMeasureSpec, titleHeight + titleVertMargins,
                collapsingMargins));
        titleHeight += mSubtitleTextView->getMeasuredHeight() +
                getVerticalMargins(mSubtitleTextView);
        childState = combineMeasuredStates(childState, mSubtitleTextView->getMeasuredState());
    }

    width += titleWidth;
    height = std::max(height, titleHeight);

    // Measurement already took padding into account for available space for the children,
    // add it in for the size.
    width += getPaddingLeft() + getPaddingRight();
    height += getPaddingTop() + getPaddingBottom();

    int measuredWidth = resolveSizeAndState(
            std::max(width, getSuggestedMinimumWidth()),
            widthMeasureSpec, childState & MEASURED_STATE_MASK);
    int measuredHeight = resolveSizeAndState(
            std::max(height, getSuggestedMinimumHeight()),
            heightMeasureSpec, childState << MEASURED_HEIGHT_STATE_SHIFT);

    setMeasuredDimension(measuredWidth, shouldCollapse() ? 0 : measuredHeight);
}

void ToolBar::onLayout(bool changed, int l, int t, int w, int h){
    const bool isRtl = getLayoutDirection() == LAYOUT_DIRECTION_RTL;
    int width = getWidth();
    int height = getHeight();
    int paddingLeft = getPaddingLeft();
    int paddingRight = getPaddingRight();
    int paddingTop = getPaddingTop();
    int paddingBottom = getPaddingBottom();
    int left = paddingLeft;
    int right = width - paddingRight;

    int collapsingMargins[2];// = mTempMargins;
    collapsingMargins[0] = collapsingMargins[1] = 0;

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
    int contentInsetRight = getCurrentContentInsetRight();
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

    bool layoutTitle = shouldLayout(mTitleTextView);
    bool layoutSubtitle = shouldLayout(mSubtitleTextView);
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
    int leftViewsCount = mTempViews.size();
    for (int i = 0; i < leftViewsCount; i++) {
        left = layoutChildLeft(mTempViews.at(i), left, collapsingMargins,alignmentHeight);
    }

    addCustomViewsWithGravity(mTempViews, Gravity::RIGHT);
    int rightViewsCount = mTempViews.size();
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
    int centerViewsCount = mTempViews.size();
    for (int i = 0; i < centerViewsCount; i++) {
        centerLeft = layoutChildLeft(mTempViews.at(i), centerLeft, collapsingMargins,alignmentHeight);
    }

    mTempViews.clear();
}

int ToolBar::getViewListMeasuredWidth(const std::vector<View*>& views, int*collapsingMargins){
    int collapseLeft = collapsingMargins[0];
    int collapseRight = collapsingMargins[1];
    int width = 0;
    const int count = views.size();
    for (int i = 0; i < count; i++) {
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

int ToolBar::layoutChildLeft(View* child, int left, int*collapsingMargins,int alignmentHeight){
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

int ToolBar::layoutChildRight(View* child, int right, int*collapsingMargins,int alignmentHeight){
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

int ToolBar::getChildTop(View* child, int alignmentHeight){
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

int ToolBar::getChildVerticalGravity(int gravity){
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

void ToolBar::addCustomViewsWithGravity(std::vector<View*>& views, int gravity){
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

int ToolBar::getChildHorizontalGravity(int gravity){
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

bool ToolBar::shouldLayout(View* view){
    return view && view->getParent() == this && view->getVisibility() != View::GONE;
}

int ToolBar::getHorizontalMargins(View* v){
    MarginLayoutParams* mlp = (MarginLayoutParams*) v->getLayoutParams();
    return mlp->getMarginStart() + mlp->getMarginEnd();
}

int ToolBar::getVerticalMargins(View* v){
    const MarginLayoutParams* mlp = (const MarginLayoutParams*) v->getLayoutParams();
    return mlp->topMargin + mlp->bottomMargin;
}

ViewGroup::LayoutParams* ToolBar::generateLayoutParams(const AttributeSet& attrs)const{
    return new LayoutParams(getContext(), attrs);
}

ViewGroup::LayoutParams* ToolBar::generateLayoutParams(const ViewGroup::LayoutParams* p)const{
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


ViewGroup::LayoutParams* ToolBar::generateDefaultLayoutParams()const{
    return new LayoutParams(LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
}

bool ToolBar::checkLayoutParams(const ViewGroup::LayoutParams* p)const{
     return ViewGroup::checkLayoutParams(p) && dynamic_cast<const LayoutParams*>(p);
}

bool ToolBar::isCustomView(View* child) {
    return ((LayoutParams*) child->getLayoutParams())->mViewType == LayoutParams::CUSTOM;
}

void ToolBar::removeChildrenForExpandedActionView() {
    int childCount = getChildCount();
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

void ToolBar::addChildrenForExpandedActionView() {
    const int count = mHiddenViews.size();
    // Re-add in reverse order since we removed in reverse order
    for (int i = count - 1; i >= 0; i--) {
        addView(mHiddenViews.at(i));
    }
    mHiddenViews.clear();
}

bool ToolBar::isChildOrHidden(View* child) {
    return child->getParent() == this;// || mHiddenViews.contains(child);
}
//////////////////////////////////////////////////////////////////////////////////////////////
ToolBar::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs)
   :ActionBar::LayoutParams(c, attrs){
}

ToolBar::LayoutParams::LayoutParams(int width, int height)
   :ActionBar::LayoutParams(width, height){
    this->gravity = Gravity::CENTER_VERTICAL | Gravity::START;
}

ToolBar::LayoutParams::LayoutParams(int width, int height, int gravity)
  :ActionBar::LayoutParams(width, height){
    this->gravity = gravity;
}

ToolBar::LayoutParams::LayoutParams(int gravity)
   :LayoutParams(WRAP_CONTENT, MATCH_PARENT, gravity){
}

ToolBar::LayoutParams::LayoutParams(const LayoutParams& source)
   :ActionBar::LayoutParams(source){
    mViewType = source.mViewType;
}

ToolBar::LayoutParams::LayoutParams(const ActionBar::LayoutParams& source)
  :ActionBar::LayoutParams(source){
}

ToolBar::LayoutParams::LayoutParams(const MarginLayoutParams& source)
  :ActionBar::LayoutParams(source){
    // ActionBar.LayoutParams doesn't have a MarginLayoutParams constructor.
    // Fake it here and copy over the relevant data.
    copyMarginsFrom(source);
}

ToolBar::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
  :ActionBar::LayoutParams(source){
}

}//namespace
