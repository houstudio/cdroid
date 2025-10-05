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
#include <widget/fastscroller.h>
#include <widget/listview.h>
#include <widget/headerviewlistadapter.h>
#include <utils/mathutils.h>
#include <utils/textutils.h>
#include <float.h>
#include <cdlog.h>

namespace cdroid{

FastScroller::FastScroller(AbsListView*listView,const std::string& styleResId){
    mList = listView;
    mDecorAnimation  = nullptr;
    mPreviewAnimation= nullptr;
    mOldItemCount  = listView->getCount();
    mOldChildCount = listView->getChildCount();
    mHeaderCount   = 0;
    mFirstVisibleItem = 0;

    Context* context = listView->getContext();
    mScaledTouchSlop = ViewConfiguration::get(context).getScaledTouchSlop();
    mScrollBarStyle  = listView->getScrollBarStyle();

    mEnabled  = false;
    mLongList = false;
    mAlwaysShow = false;
    mUpdatingLayout  = false;
    mLayoutFromRight = false;
    mScrollCompleted = true;
    mState = STATE_VISIBLE;
    mMatchDragPosition =true;// context.getApplicationInfo().targetSdkVersion >= Build.VERSION_CODES.HONEYCOMB;
    mCurrentSection =-1;
    mScrollbarPosition=-1;

    AttributeSet atts;
    atts.setContext(listView->getContext(),"");
    mTrackImage = new ImageView(context,atts);
    mTrackImage->setScaleType(ScaleType::FIT_XY);
    mThumbImage = new ImageView(context,atts);
    mThumbImage->setScaleType(ScaleType::FIT_XY);
    mPreviewImage = new View(context,atts);
    mPreviewImage->setAlpha(.0f);

    mPrimaryText = createPreviewTextView(context);
    mSecondaryText = createPreviewTextView(context);

    mMinimumTouchTarget = context->getDimension("cdroid:dimen/fast_scroller_minimum_touch_target");

    setStyle(styleResId);

    mOverlay = (ViewGroupOverlay*)listView->getOverlay();
    if(mOverlay){
        mOverlay->add(mTrackImage);
        mOverlay->add(mThumbImage);
        mOverlay->add(mPreviewImage);
        mOverlay->add(mPrimaryText);
        mOverlay->add(mSecondaryText);
    }
    ViewGroup::OnHierarchyChangeListener hls;
    hls.onChildViewRemoved=[](View&container,View *view){
        delete view;
    };
    mOverlay->getOverlayView()->setOnHierarchyChangeListener(hls);
    getSectionsFromIndexer();
    updateLongList(mOldChildCount, mOldItemCount);
    setScrollbarPosition(listView->getVerticalScrollbarPosition());
    postAutoHide();
}

FastScroller::~FastScroller(){
    delete mDecorAnimation;
    delete mPreviewAnimation;
    //do not delete mOverlay and its children
    //they are created/freed by View/ViewGroup,
}

void FastScroller::updateAppearance() {
    int width = 0;

    // Add track to overlay if it has an image.
    mTrackImage->setImageDrawable(mTrackDrawable);
    if (mTrackDrawable != nullptr) {
        width = std::max(width, mTrackDrawable->getIntrinsicWidth());
    }

    // Add thumb to overlay if it has an image.
    mThumbImage->setImageDrawable(mThumbDrawable);
    mThumbImage->setMinimumWidth(mThumbMinWidth);
    mThumbImage->setMinimumHeight(mThumbMinHeight);
    if (mThumbDrawable != nullptr) {
        width = std::max(width, mThumbDrawable->getIntrinsicWidth());
    }

    // Account for minimum thumb width.
    mWidth = std::max(width, mThumbMinWidth);

    if (!mTextAppearance.empty()) {
        mPrimaryText->setTextAppearance(mTextAppearance);
        mSecondaryText->setTextAppearance(mTextAppearance);
    }

    if (mTextColor != nullptr) {
        mPrimaryText->setTextColor(mTextColor);
        mSecondaryText->setTextColor(mTextColor);
    }

    if (mTextSize > 0) {
        mPrimaryText->setTextSize(TypedValue::COMPLEX_UNIT_PX, mTextSize);
        mSecondaryText->setTextSize(TypedValue::COMPLEX_UNIT_PX, mTextSize);
    }

    int padding = mPreviewPadding;
    mPrimaryText->setIncludeFontPadding(false);
    mPrimaryText->setPadding(padding, padding, padding, padding);
    mSecondaryText->setIncludeFontPadding(false);
    mSecondaryText->setPadding(padding, padding, padding, padding);

    refreshDrawablePressedState();
    mDeferHide=[this](){setState(STATE_NONE);};
}

void FastScroller::setStyle(const std::string&styleResId){
    Context* context = mList->getContext();
    AttributeSet ta = context->obtainStyledAttributes(styleResId);//R.styleable.FastScroll, R.attr.fastScrollStyle, resId);
   
    mOverlayPosition = ta.getInt("position", OVERLAY_FLOATING);
    mPreviewResId[PREVIEW_LEFT] = ta.getString("backgroundLeft");
    mPreviewResId[PREVIEW_RIGHT] = ta.getString("backgroundRight");
    mThumbDrawable = ta.getDrawable("thumbDrawable");
    mTrackDrawable = ta.getDrawable("trackDrawable");
    mTextAppearance = ta.getString("textAppearance");//R.styleable.FastScroll_textAppearance
    mTextColor = ta.getColorStateList("textColor");
    mTextSize  = ta.getDimensionPixelSize("textSize", 0);
    mPreviewMinWidth = ta.getDimensionPixelSize("minWidth", 0);
    mPreviewMinHeight= ta.getDimensionPixelSize("minHeight", 0);
    mThumbMinWidth  = ta.getDimensionPixelSize("thumbMinWidth", 0);
    mThumbMinHeight = ta.getDimensionPixelSize("thumbMinHeight", 0);
    mPreviewPadding = ta.getDimensionPixelSize("padding", 0);
    mThumbPosition  = ta.getInt("thumbPosition", THUMB_POSITION_MIDPOINT);
    updateAppearance();
}

void FastScroller::remove() {
    mOverlay->remove(mTrackImage);
    mOverlay->remove(mThumbImage);
    mOverlay->remove(mPreviewImage);
    mOverlay->remove(mPrimaryText);
    mOverlay->remove(mSecondaryText);
}

void FastScroller::setEnabled(bool enabled){
    if (mEnabled != enabled) {
        mEnabled = enabled;
        onStateDependencyChanged(true);
    }
}

bool FastScroller::isEnabled()const{
    return mEnabled && (mLongList || mAlwaysShow);
}

void FastScroller::setAlwaysShow(bool alwaysShow){
    if (mAlwaysShow != alwaysShow) {
        mAlwaysShow = alwaysShow;
        onStateDependencyChanged(false);
    }
}

bool FastScroller::isAlwaysShowEnabled()const{
    return mAlwaysShow;
}

void FastScroller::onStateDependencyChanged(bool peekIfEnabled) {
    if (isEnabled()) {
        if (isAlwaysShowEnabled()) {
            setState(STATE_VISIBLE);
        } else if (mState == STATE_VISIBLE) {
            postAutoHide();
        } else if (peekIfEnabled) {
            setState(STATE_VISIBLE);
            postAutoHide();
        }
    } else {
        stop();
    }
    mList->resolvePadding();
}

void FastScroller::setScrollBarStyle(int style){
    if (mScrollBarStyle != style) {
        mScrollBarStyle = style;
        updateLayout();
    }
}

void FastScroller::stop() {
    setState(STATE_NONE);
}

void FastScroller::setScrollbarPosition(int position){
    if (position == View::SCROLLBAR_POSITION_DEFAULT) {
        position = mList->isLayoutRtl() ? View::SCROLLBAR_POSITION_LEFT : View::SCROLLBAR_POSITION_RIGHT;
    }

    if (mScrollbarPosition != position) {
        mScrollbarPosition = position;
        mLayoutFromRight = position != View::SCROLLBAR_POSITION_LEFT;

        const std::string previewResId = mPreviewResId[mLayoutFromRight ? PREVIEW_RIGHT : PREVIEW_LEFT];
        mPreviewImage->setBackgroundResource(previewResId);

        // Propagate padding to text min width/height.
        const int textMinWidth = std::max(0, mPreviewMinWidth - mPreviewImage->getPaddingLeft()
                - mPreviewImage->getPaddingRight());
        mPrimaryText->setMinimumWidth(textMinWidth);
        mSecondaryText->setMinimumWidth(textMinWidth);

        const int textMinHeight = std::max(0, mPreviewMinHeight - mPreviewImage->getPaddingTop()
                    - mPreviewImage->getPaddingBottom());
        mPrimaryText->setMinimumHeight(textMinHeight);
        mSecondaryText->setMinimumHeight(textMinHeight);

        // Requires re-layout.
        updateLayout();
    }
}

int FastScroller::getWidth()const{
    return mWidth;
}

void FastScroller::onSizeChanged(int w, int h, int oldw, int oldh) {
    updateLayout();
}

void FastScroller::onItemCountChanged(int childCount, int itemCount) {
    if (mOldItemCount != itemCount || mOldChildCount != childCount) {
        mOldItemCount = itemCount;
        mOldChildCount = childCount;

        const bool hasMoreItems = itemCount - childCount > 0;
        if (hasMoreItems && mState != STATE_DRAGGING) {
            const int firstVisibleItem = mList->getFirstVisiblePosition();
            setThumbPos(getPosFromItemCount(firstVisibleItem, childCount, itemCount));
        }

        updateLongList(childCount, itemCount);
    }
}

void FastScroller::updateLongList(int childCount, int itemCount) {
    const bool longList = childCount > 0 && itemCount / childCount >= MIN_PAGES;
    if (mLongList != longList) {
        mLongList = longList;

        onStateDependencyChanged(false);
    }
}

TextView* FastScroller::createPreviewTextView(Context* context) {
    LayoutParams* params = new LayoutParams( LayoutParams::WRAP_CONTENT, LayoutParams::WRAP_CONTENT);
    AttributeSet atts(context,"");
    TextView* textView = new TextView(context,atts);
    textView->setLayoutParams(params);
    textView->setSingleLine(true);
    textView->setEllipsize(Layout::ELLIPSIS_MIDDLE);
    textView->setGravity(Gravity::CENTER);
    textView->setAlpha(.0f);

    // Manually propagate inherited layout direction.
    textView->setLayoutDirection(mList->getLayoutDirection());

    return textView;
}

void FastScroller::updateLayout(){
    // Prevent re-entry when RTL properties change as a side-effect of
    // resolving padding.
    if (mUpdatingLayout) {
        return;
    }

    mUpdatingLayout = true;

    updateContainerRect();

    layoutThumb();
    layoutTrack();

    updateOffsetAndRange();

    Rect bounds;
    measurePreview(mPrimaryText, bounds);
    applyLayout(mPrimaryText, bounds);
    measurePreview(mSecondaryText, bounds);
    applyLayout(mSecondaryText, bounds);

    if (mPreviewImage != nullptr) {
        // Apply preview image padding.
        bounds.left -= mPreviewImage->getPaddingLeft();
        bounds.top  -= mPreviewImage->getPaddingTop();
        bounds.width += (mPreviewImage->getPaddingLeft()+ mPreviewImage->getPaddingRight());
        bounds.height+= (mPreviewImage->getPaddingTop() + mPreviewImage->getPaddingBottom());
        applyLayout(mPreviewImage, bounds);
    }

    mUpdatingLayout = false;
}

void FastScroller::applyLayout(View* view,const Rect& bounds) {
    view->layout(bounds.left, bounds.top, bounds.width, bounds.height);
    view->setPivotX(mLayoutFromRight ? bounds.width : 0);
}

void FastScroller::measurePreview(View* v, Rect& out) {
    // Apply the preview image's padding as layout margins.
    Rect margins;
    margins.left = mPreviewImage->getPaddingLeft();
    margins.top  = mPreviewImage->getPaddingTop();
    margins.width = mPreviewImage->getPaddingRight();
    margins.height= mPreviewImage->getPaddingBottom();

    if (mOverlayPosition == OVERLAY_FLOATING) {
        measureFloating(v, &margins, out);
    } else {
        measureViewToSide(v, mThumbImage, &margins, out);
    }
}

void FastScroller::measureViewToSide(View* view, View* adjacent,const Rect* margins, Rect& out) {
    int marginLeft;
    int marginTop;
    int marginRight;
    if (margins == nullptr) {
        marginLeft = 0;
        marginTop = 0;
        marginRight = 0;
    } else {
        marginLeft = margins->left;
        marginTop = margins->top;
        marginRight = margins->width;
    }

    Rect& container = mContainerRect;
    int containerWidth = container.width;
    int maxWidth;
    if (adjacent == nullptr) {
        maxWidth = containerWidth;
    } else if (mLayoutFromRight) {
        maxWidth = adjacent->getLeft();
    } else {
        maxWidth = containerWidth - adjacent->getRight();
    }

    const int adjMaxHeight= std::max(0, container.height);
    const int adjMaxWidth = std::max(0, maxWidth - marginLeft - marginRight);
    const int widthMeasureSpec = MeasureSpec::makeMeasureSpec(adjMaxWidth, MeasureSpec::AT_MOST);
    const int heightMeasureSpec = MeasureSpec::makeSafeMeasureSpec(adjMaxHeight, MeasureSpec::UNSPECIFIED);
    view->measure(widthMeasureSpec, heightMeasureSpec);

    // Align to the left or right.
    const int width = std::min(adjMaxWidth, view->getMeasuredWidth());
    int left, right;
    if (mLayoutFromRight) {
        right = (adjacent == nullptr ? container.right() : adjacent->getLeft()) - marginRight;
        left = right - width;
    } else {
        left = (adjacent == nullptr ? container.left : adjacent->getRight()) + marginLeft;
        right = left + width;
    }

    // Don't adjust the vertical position.
    const int top = marginTop;
    const int bottom = top + view->getMeasuredHeight();
    out.set(left, top, right-left, bottom-top);
}

void FastScroller::measureFloating(View* preview,const Rect* margins, Rect& out) {
    int marginLeft;
    int marginTop;
    int marginRight;
    if (margins == nullptr) {
        marginLeft = 0;
        marginTop = 0;
        marginRight = 0;
    } else {
        marginLeft = margins->left;
        marginTop = margins->top;
        marginRight = margins->width;
    }

    Rect& container = mContainerRect;
    const int containerWidth = container.width;
    const int adjMaxHeight = std::max(0, container.height);
    const int adjMaxWidth = std::max(0, containerWidth - marginLeft - marginRight);
    const int widthMeasureSpec = MeasureSpec::makeMeasureSpec(adjMaxWidth, MeasureSpec::AT_MOST);
    const int heightMeasureSpec = MeasureSpec::makeSafeMeasureSpec(adjMaxHeight, MeasureSpec::UNSPECIFIED);
    preview->measure(widthMeasureSpec, heightMeasureSpec);

    // Align at the vertical center, 10% from the top.
    const int containerHeight = container.height;
    const int width = preview->getMeasuredWidth();
    const int top = containerHeight / 10 + marginTop + container.top;
    const int bottom = top + preview->getMeasuredHeight();
    const int left = (containerWidth - width) / 2 + container.left;
    const int right = left + width;
    out.set(left, top, right-left, bottom-top);
}

void FastScroller::updateContainerRect() {
    mList->resolvePadding();

    Rect& container = mContainerRect;
    container.left = 0;
    container.top = 0;
    container.width = mList->getWidth();
    container.height= mList->getHeight();

    if ((mScrollBarStyle == View::SCROLLBARS_INSIDE_INSET)
            || (mScrollBarStyle == View::SCROLLBARS_INSIDE_OVERLAY)) {
        container.left += mList->getPaddingLeft();
        container.top  += mList->getPaddingTop();
        container.width -= (mList->getPaddingLeft()+mList->getPaddingRight());
        container.height-= (mList->getPaddingTop()+mList->getPaddingBottom());

        // In inset mode, we need to adjust for padded scrollbar width.
        if (mScrollBarStyle == View::SCROLLBARS_INSIDE_INSET) {
            const int width = getWidth();
            if (mScrollbarPosition == View::SCROLLBAR_POSITION_RIGHT) {
                container.width += width;
            } else {
                container.left -= width;
            }
        }
    }
}

void FastScroller::layoutThumb() {
    Rect bounds;
    measureViewToSide(mThumbImage, nullptr, nullptr, bounds);
    applyLayout(mThumbImage, bounds);
}

void FastScroller::layoutTrack() {
    Rect& container = mContainerRect;
    const int maxWidth = std::max(0, container.width);
    const int maxHeight = std::max(0, container.height);
    const int widthMeasureSpec = MeasureSpec::makeMeasureSpec(maxWidth, MeasureSpec::AT_MOST);
    const int heightMeasureSpec = MeasureSpec::makeSafeMeasureSpec(maxHeight, MeasureSpec::UNSPECIFIED);
    mTrackImage->measure(widthMeasureSpec, heightMeasureSpec);

    int top;
    int bottom;
    if (mThumbPosition == THUMB_POSITION_INSIDE) {
        top = container.top;
        bottom = container.bottom();
    } else {
        const int thumbHalfHeight = mThumbImage->getHeight() / 2;
        top = container.top + thumbHalfHeight;
        bottom = container.bottom() - thumbHalfHeight;
    }

    const int trackWidth = mTrackImage->getMeasuredWidth();
    const int left = mThumbImage->getLeft() + (mThumbImage->getWidth() - trackWidth) / 2;
    mTrackImage->layout(left, top, trackWidth, bottom-top);
}

void FastScroller::updateOffsetAndRange() {
    float min;
    float max;
    if (mThumbPosition == THUMB_POSITION_INSIDE) {
        const float halfThumbHeight = mThumbImage->getHeight() / 2.f;
        min = mTrackImage->getTop() + halfThumbHeight;
        max = mTrackImage->getBottom() - halfThumbHeight;
    } else{
        min = mTrackImage->getTop();
        max = mTrackImage->getBottom();
    }

    mThumbOffset = min;
    mThumbRange = max - min;
}

void FastScroller::setState(int state) {
    mList->removeCallbacks(mDeferHide);

    if (mAlwaysShow && state == STATE_NONE) {
        state = STATE_VISIBLE;
    }

    if (state == mState) {
        return;
    }
    switch (state) {
    case STATE_NONE: transitionToHidden();  break;
    case STATE_VISIBLE: transitionToVisible();break;
    case STATE_DRAGGING:
         if (transitionPreviewLayout(mCurrentSection)) {
             transitionToDragging();
         } else {
             transitionToVisible();
         }
         break;
    }
    mState = state;
    refreshDrawablePressedState();
}

void FastScroller::refreshDrawablePressedState() {
    const bool isPressed = mState == STATE_DRAGGING;
    mThumbImage->setPressed(isPressed);
    mTrackImage->setPressed(isPressed);
}

void FastScroller::transitionToHidden() {
    if (mDecorAnimation != nullptr) {
        mDecorAnimation->cancel();
        delete mDecorAnimation;
    }

    Animator* fadeOut = groupAnimatorOfFloat("alpha", 0.f, {mThumbImage, mTrackImage,
            mPreviewImage, mPrimaryText, mSecondaryText});
    fadeOut->setDuration(DURATION_FADE_OUT);

    // Push the thumb and track outside the list bounds.
    const float offset = mLayoutFromRight ? mThumbImage->getWidth() : -mThumbImage->getWidth();
    Animator* slideOut = groupAnimatorOfFloat("translationX", offset, {mThumbImage, mTrackImage});
    slideOut->setDuration(DURATION_FADE_OUT);

    mDecorAnimation = new AnimatorSet();
    mDecorAnimation->playTogether({fadeOut, slideOut});
    mDecorAnimation->start();

    mShowingPreview = false;
}

void FastScroller::transitionToVisible() {
    if (mDecorAnimation != nullptr) {
        mDecorAnimation->cancel();
        delete mDecorAnimation;
    }

    Animator* fadeIn = groupAnimatorOfFloat("alpha", 1.f, {mThumbImage, mTrackImage});
    fadeIn->setDuration(DURATION_FADE_IN);

    Animator* fadeOut = groupAnimatorOfFloat("alpha", 0.f, {mPreviewImage, mPrimaryText, mSecondaryText});
    fadeOut->setDuration(DURATION_FADE_OUT);

    Animator* slideIn = groupAnimatorOfFloat("translationX", 0.f, {mThumbImage, mTrackImage});
    slideIn->setDuration(DURATION_FADE_IN);

    mDecorAnimation = new AnimatorSet();
    mDecorAnimation->playTogether({fadeIn, fadeOut, slideIn});
    mDecorAnimation->start();

    mShowingPreview = false;
}

void FastScroller::transitionToDragging() {
    if (mDecorAnimation != nullptr) {
        mDecorAnimation->cancel();
        delete mDecorAnimation;
    }

    Animator* fadeIn = groupAnimatorOfFloat("alpha", 1.f, {mThumbImage, mTrackImage, mPreviewImage});
    fadeIn->setDuration(DURATION_FADE_IN);

    Animator* slideIn = groupAnimatorOfFloat("translationX", 0.f, {mThumbImage, mTrackImage});
    slideIn->setDuration(DURATION_FADE_IN);

    mDecorAnimation = new AnimatorSet();
    mDecorAnimation->playTogether({fadeIn, slideIn});
    mDecorAnimation->start();

    mShowingPreview = true;
}

void FastScroller::postAutoHide() {
    mList->removeCallbacks(mDeferHide);
    mList->postDelayed(mDeferHide, FADE_TIMEOUT);
}

void FastScroller::onScroll(int firstVisibleItem, int visibleItemCount, int totalItemCount) {
    if (!isEnabled()) {
        setState(STATE_NONE);
        return;
    }

    const bool hasMoreItems = (totalItemCount - visibleItemCount > 0);
    if (hasMoreItems && mState != STATE_DRAGGING) {
        setThumbPos(getPosFromItemCount(firstVisibleItem, visibleItemCount, totalItemCount));
    }

    mScrollCompleted = true;

    if (mFirstVisibleItem != firstVisibleItem) {
        mFirstVisibleItem = firstVisibleItem;

        // Show the thumb, if necessary, and set up auto-fade.
        if (mState != STATE_DRAGGING) {
            setState(STATE_VISIBLE);
            postAutoHide();
        }
    }
}

void FastScroller::getSectionsFromIndexer() {
    mSectionIndexer = {nullptr,nullptr,nullptr};

    Adapter* adapter = mList->getAdapter();
    if (dynamic_cast<HeaderViewListAdapter*>(adapter)) {
        mHeaderCount = ((HeaderViewListAdapter*) adapter)->getHeadersCount();
        adapter = ((HeaderViewListAdapter*) adapter)->getWrappedAdapter();
    }

    /*if (dynamic_cast<ExpandableListConnector*>(adapter)){
        ExpandableListAdapter* expAdapter = ((ExpandableListConnector*) adapter)->getAdapter();
        if (expAdapter instanceof SectionIndexer) {
            mSectionIndexer = (SectionIndexer) expAdapter;
            mListAdapter = adapter;
            mSections = mSectionIndexer.getSections();
        }
    } else if (adapter instanceof SectionIndexer) {
        mListAdapter = adapter;
        mSectionIndexer = (SectionIndexer) adapter;
        mSections = mSectionIndexer.getSections();
    } else*/ {
        mListAdapter = adapter;
        mSections.clear();// = nullptr;
    }
}

void FastScroller::onSectionsChanged() {
    mListAdapter = nullptr;
}

void FastScroller::scrollTo(float position) {
    mScrollCompleted = false;

    int count = mList->getCount();
    int sectionIndex = -1;
    int sectionCount = mSections.size();

    if (mSections.size()) {
        int exactSection = MathUtils::constrain(
                (int) (position * sectionCount), 0, sectionCount - 1);
        int targetSection = exactSection;
        int targetIndex = mSectionIndexer.getPositionForSection(targetSection);
        sectionIndex = targetSection;

        // Given the expected section and index, the following code will
        // try to account for missing sections (no names starting with..)
        // It will compute the scroll space of surrounding empty sections
        // and interpolate the currently visible letter's range across the
        // available space, so that there is always some list movement while
        // the user moves the thumb.
        int nextIndex = count;
        int prevIndex = targetIndex;
        int prevSection = targetSection;
        int nextSection = targetSection + 1;

        // Assume the next section is unique
        if (targetSection < sectionCount - 1) {
            nextIndex = mSectionIndexer.getPositionForSection(targetSection + 1);
        }

        // Find the previous index if we're slicing the previous section
        if (nextIndex == targetIndex) {
            // Non-existent letter
            while (targetSection > 0) {
                targetSection--;
                prevIndex = mSectionIndexer.getPositionForSection(targetSection);
                if (prevIndex != targetIndex) {
                    prevSection = targetSection;
                    sectionIndex = targetSection;
                    break;
                } else if (targetSection == 0) {
                    // When section reaches 0 here, sectionIndex must follow it.
                    // Assuming mSectionIndexer.getPositionForSection(0) == 0.
                    sectionIndex = 0;
                    break;
                }
            }
        }

        // Find the next index, in case the assumed next index is not
        // unique. For instance, if there is no P, then request for P's
        // position actually returns Q's. So we need to look ahead to make
        // sure that there is really a Q at Q's position. If not, move
        // further down...
        int nextNextSection = nextSection + 1;
        while (nextNextSection < sectionCount &&
                mSectionIndexer.getPositionForSection(nextNextSection) == nextIndex) {
            nextNextSection++;
            nextSection++;
        }

        // Compute the beginning and ending scroll range percentage of the
        // currently visible section. This could be equal to or greater than
        // (1 / nSections). If the target position is near the previous
        // position, snap to the previous position.
        float prevPosition = (float) prevSection / sectionCount;
        float nextPosition = (float) nextSection / sectionCount;
        float snapThreshold = (count == 0) ? FLT_MAX : .125f / count;
        if (prevSection == exactSection && position - prevPosition < snapThreshold) {
            targetIndex = prevIndex;
        } else {
            targetIndex = prevIndex + (int) ((nextIndex - prevIndex) * (position - prevPosition)
                / (nextPosition - prevPosition));
        }

        // Clamp to valid positions.
        targetIndex = MathUtils::constrain(targetIndex, 0, count - 1);

        /*if (dynamic_cast<ExpandableListView*>(mList)) {
            ExpandableListView expList = (ExpandableListView) mList;
            expList.setSelectionFromTop(expList.getFlatListPosition(
                    ExpandableListView.getPackedPositionForGroup(targetIndex + mHeaderCount)),0);
        } else */if (dynamic_cast<ListView*>(mList)) {
            ((ListView*)mList)->setSelectionFromTop(targetIndex + mHeaderCount, 0);
        } else {
            mList->setSelection(targetIndex + mHeaderCount);
        }
    } else {
        int index = MathUtils::constrain((int) (position * count), 0, count - 1);
        /*if (mList instanceof ExpandableListView) {
            ExpandableListView expList = (ExpandableListView) mList;
            expList.setSelectionFromTop(expList.getFlatListPosition(
                    ExpandableListView.getPackedPositionForGroup(index + mHeaderCount)), 0);
        } else */if (dynamic_cast<ListView*>(mList)) {
            ((ListView*)mList)->setSelectionFromTop(index + mHeaderCount, 0);
        } else {
            mList->setSelection(index + mHeaderCount);
        }

        sectionIndex = -1;
    }

    if (mCurrentSection != sectionIndex) {
        mCurrentSection = sectionIndex;
        bool hasPreview = transitionPreviewLayout(sectionIndex);
        if (!mShowingPreview && hasPreview) {
            transitionToDragging();
        } else if (mShowingPreview && !hasPreview) {
            transitionToVisible();
        }
    }
}

bool FastScroller::transitionPreviewLayout(int sectionIndex) {
    std::string text;
    /*auto& sections = mSections;
    if (sections.size() && sectionIndex >= 0 && sectionIndex < sections.size()) {
        Object section = sections[sectionIndex];
        if (section != null) {
            text = section.toString();
        }
    }*/

    Rect bounds;
    View* preview = mPreviewImage;
    TextView* showing;
    TextView* target;
    if (mShowingPrimary) {
        showing = mPrimaryText;
        target = mSecondaryText;
    } else {
        showing = mSecondaryText;
        target = mPrimaryText;
    }

    // Set and layout target immediately.
    target->setText(text);
    measurePreview(target, bounds);
    applyLayout(target, bounds);

    if (mPreviewAnimation != nullptr) {
        mPreviewAnimation->cancel();
        delete mPreviewAnimation;
    }

    // Cross-fade preview text.
    Animator* showTarget = animateAlpha(target, 1.f);
    Animator* hideShowing = animateAlpha(showing, 0.f);
    showTarget->setDuration(DURATION_CROSS_FADE);
    hideShowing->setDuration(DURATION_CROSS_FADE);
    Animator::AnimatorListener mSwitchPrimaryListener;
    mSwitchPrimaryListener.onAnimationEnd=[this](Animator& animation,bool isReverse){
        mShowingPrimary = !mShowingPrimary;
    };
    hideShowing->addListener(mSwitchPrimaryListener);

    // Apply preview image padding and animate bounds, if necessary.
    bounds.left-= preview->getPaddingLeft();
    bounds.top -= preview->getPaddingTop();
    bounds.width  += preview->getPaddingRight() + preview->getPaddingLeft();
    bounds.height += preview->getPaddingBottom()+ preview->getPaddingTop();
    Animator* resizePreview = animateBounds(preview, bounds);
    resizePreview->setDuration(DURATION_RESIZE);

    mPreviewAnimation = new AnimatorSet();
    AnimatorSet::Builder* builder = mPreviewAnimation->play(hideShowing);
    builder->with(showTarget).with(resizePreview);

    // The current preview size is unaffected by hidden or showing. It's
    // used to set starting scales for things that need to be scaled down.
    const int previewWidth = preview->getWidth() - preview->getPaddingLeft()
            - preview->getPaddingRight();

    // If target is too large, shrink it immediately to fit and expand to
    // target size. Otherwise, start at target size.
    const int targetWidth = target->getWidth();
    if (targetWidth > previewWidth) {
        target->setScaleX((float) previewWidth / targetWidth);
        Animator* scaleAnim = animateScaleX(target, 1.f);
        scaleAnim->setDuration(DURATION_RESIZE);
        builder->with(scaleAnim);
    } else {
        target->setScaleX(1.f);
    }

    // If showing is larger than target, shrink to target size.
    const int showingWidth = showing->getWidth();
    if (showingWidth > targetWidth) {
        float scale = (float) targetWidth / showingWidth;
        Animator* scaleAnim = animateScaleX(showing, scale);
        scaleAnim->setDuration(DURATION_RESIZE);
        builder->with(scaleAnim);
    }
    delete builder;
    mPreviewAnimation->start();
    return TextUtils::isEmpty(text);
}

void FastScroller::setThumbPos(float position) {
    float thumbMiddle = position * mThumbRange + mThumbOffset;
    mThumbImage->setTranslationY(thumbMiddle - mThumbImage->getHeight() / 2.f);

    float previewHalfHeight = mPreviewImage->getHeight() / 2.f;
    float previewPos;
    switch (mOverlayPosition) {
    case OVERLAY_AT_THUMB:
        previewPos = thumbMiddle;
        break;
    case OVERLAY_ABOVE_THUMB:
        previewPos = thumbMiddle - previewHalfHeight;
        break;
    case OVERLAY_FLOATING:
    default:
        previewPos = 0;
        break;
    }

    // Center the preview on the thumb, constrained to the list bounds.
    Rect& container = mContainerRect;
    int top = container.top;
    int bottom = container.bottom();
    float minP = top + previewHalfHeight;
    float maxP = bottom - previewHalfHeight;
    float previewMiddle = MathUtils::constrain(previewPos, minP, maxP);
    float previewTop = previewMiddle - previewHalfHeight;
    mPreviewImage->setTranslationY(previewTop);

    mPrimaryText->setTranslationY(previewTop);
    mSecondaryText->setTranslationY(previewTop);
}

float FastScroller::getPosFromMotionEvent(float y) {
    // If the list is the same height as the thumbnail or shorter,
    // effectively disable scrolling.
    if (mThumbRange <= 0) {
        return .0f;
    }

    return MathUtils::constrain((y - mThumbOffset) / mThumbRange, .0f, 1.f);
}

float FastScroller::getPosFromItemCount(int firstVisibleItem, int visibleItemCount, int totalItemCount) {
    SectionIndexer& sectionIndexer = mSectionIndexer;
    if (sectionIndexer.getSectionForPosition==nullptr||sectionIndexer.getPositionForSection==nullptr
        ||sectionIndexer.getSections== nullptr || mListAdapter == nullptr) {
        getSectionsFromIndexer();
    }

    if (visibleItemCount == 0 || totalItemCount == 0) {
        // No items are visible.
        return 0;
    }

    bool hasSections = sectionIndexer.getSectionForPosition && mSections.size();
    if (!hasSections || !mMatchDragPosition) {
        if (visibleItemCount == totalItemCount) {
            // All items are visible.
            return 0;
        } else {
            return (float) firstVisibleItem / (totalItemCount - visibleItemCount);
        }
    }

    // Ignore headers.
    firstVisibleItem -= mHeaderCount;
    if (firstVisibleItem < 0) {
        return 0;
    }
    totalItemCount -= mHeaderCount;

    // Hidden portion of the first visible row.
    View* child = mList->getChildAt(0);
    float incrementalPos;
    if (child == nullptr || child->getHeight() == 0) {
        incrementalPos = 0;
    } else {
        incrementalPos = (float) (mList->getPaddingTop() - child->getTop()) / child->getHeight();
    }

    // Number of rows in this section.
    int section = sectionIndexer.getSectionForPosition(firstVisibleItem);
    int sectionPos = sectionIndexer.getPositionForSection(section);
    int sectionCount = mSections.size();
    int positionsInSection;
    if (section < sectionCount - 1) {
        int nextSectionPos;
        if (section + 1 < sectionCount) {
            nextSectionPos = sectionIndexer.getPositionForSection(section + 1);
        } else {
            nextSectionPos = totalItemCount - 1;
        }
        positionsInSection = nextSectionPos - sectionPos;
    } else {
        positionsInSection = totalItemCount - sectionPos;
    }

    // Position within this section.
    float posWithinSection;
    if (positionsInSection == 0) {
        posWithinSection = 0;
    } else {
        posWithinSection = (firstVisibleItem + incrementalPos - sectionPos)
                / positionsInSection;
    }

    float result = (section + posWithinSection) / sectionCount;
    // Fake out the scroll bar for the last item. Since the section indexer
    // won't ever actually move the list in this end space, make scrolling
    // across the last item account for whatever space is remaining.
    if (firstVisibleItem > 0 && firstVisibleItem + visibleItemCount == totalItemCount) {
        View* lastChild = mList->getChildAt(visibleItemCount - 1);
        int bottomPadding = mList->getPaddingBottom();
        int maxSize;
        int currentVisibleSize;
        if (mList->getClipToPadding()) {
            maxSize = lastChild->getHeight();
            currentVisibleSize = mList->getHeight() - bottomPadding - lastChild->getTop();
        } else {
            maxSize = lastChild->getHeight() + bottomPadding;
            currentVisibleSize = mList->getHeight() - lastChild->getTop();
        }
        if (currentVisibleSize > 0 && maxSize > 0) {
            result += (1 - result) * ((float) currentVisibleSize / maxSize );
        }
    }
    return result;
}

void FastScroller::cancelFling() {
    MotionEvent* cancelFling = MotionEvent::obtain(
            0, 0, MotionEvent::ACTION_CANCEL, 0, 0, 0);
    mList->onTouchEvent(*cancelFling);
    cancelFling->recycle();
}

void FastScroller::cancelPendingDrag() {
    mPendingDrag = -1;
}

/**
  * Delays dragging until after the framework has determined that the user is
  * scrolling, rather than tapping.
  */
void FastScroller::startPendingDrag() {
    mPendingDrag = SystemClock::uptimeMillis() + TAP_TIMEOUT;
}

void FastScroller::beginDrag() {
    mPendingDrag = -1;

    setState(STATE_DRAGGING);

    if (mListAdapter == nullptr && mList != nullptr) {
        getSectionsFromIndexer();
    }

    if (mList != nullptr) {
        mList->requestDisallowInterceptTouchEvent(true);
        mList->reportScrollStateChange(AbsListView::OnScrollListener::SCROLL_STATE_TOUCH_SCROLL);
    }

    cancelFling();
}

bool FastScroller::onInterceptTouchEvent(MotionEvent& ev) {
    if (!isEnabled()) {
        return false;
    }
    switch (ev.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        if (isPointInside(ev.getX(), ev.getY())) {
            // If the parent has requested that its children delay
            // pressed state (e.g. is a scrolling container) then we
            // need to allow the parent time to decide whether it wants
            // to intercept events. If it does, we will receive a CANCEL event.
            if (!mList->isInScrollingContainer()) {
                // This will get dispatched to onTouchEvent(). Start
                // dragging there.
                return true;
            }

            mInitialTouchY = ev.getY();
            startPendingDrag();
        }
        break;
    case MotionEvent::ACTION_MOVE:
        if (!isPointInside(ev.getX(), ev.getY())) {
            cancelPendingDrag();
        } else if (mPendingDrag >= 0 && mPendingDrag <= SystemClock::uptimeMillis()) {
            beginDrag();

            float pos = getPosFromMotionEvent(mInitialTouchY);
            LOGD("mInitialTouchY=%f pos=%f",mInitialTouchY,pos);
            scrollTo(pos);
            // This may get dispatched to onTouchEvent(), but it
            // doesn't really matter since we'll already be in a drag.
            return onTouchEvent(ev);
        }
        break;
    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_CANCEL:
        cancelPendingDrag();
        break;
    }

    return false;
}

bool FastScroller::onInterceptHoverEvent(MotionEvent& ev) {
    if (!isEnabled()) {
        return false;
    }

    const int actionMasked = ev.getActionMasked();
    if ((actionMasked == MotionEvent::ACTION_HOVER_ENTER || actionMasked == MotionEvent::ACTION_HOVER_MOVE) 
           && mState == STATE_NONE && isPointInside(ev.getX(), ev.getY())) {
        setState(STATE_VISIBLE);
        postAutoHide();
    }

    return false;
}


PointerIcon* FastScroller::onResolvePointerIcon(MotionEvent& event, int pointerIndex) {
    if ((mState == STATE_DRAGGING) || isPointInside(event.getX(), event.getY())) {
        return PointerIcon::getSystemIcon(mList->getContext(), PointerIcon::TYPE_ARROW);
    }
    return nullptr;
}

bool FastScroller::onTouchEvent(MotionEvent& me) {
    if (!isEnabled()) {
        return false;
    }

    switch (me.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        if (isPointInside(me.getX(), me.getY())) {
            if (!mList->isInScrollingContainer()) {
                beginDrag();
                return true;
            }
        }
        break;

    case MotionEvent::ACTION_UP:
        if (mPendingDrag >= 0) {
            // Allow a tap to scroll.
            beginDrag();

            const float pos = getPosFromMotionEvent(me.getY());
            setThumbPos(pos);
            scrollTo(pos);
            // Will hit the STATE_DRAGGING check below
        }

        if (mState == STATE_DRAGGING) {
            if (mList != nullptr) {
                // ViewGroup does the right thing already, but there might
                // be other classes that don't properly reset on touch-up,
                // so do this explicitly just in case.
                mList->requestDisallowInterceptTouchEvent(false);
                mList->reportScrollStateChange(AbsListView::OnScrollListener::SCROLL_STATE_IDLE);
            }

            setState(STATE_VISIBLE);
            postAutoHide();
            return true;
        }
        break;

    case MotionEvent::ACTION_MOVE:
        if (mPendingDrag >= 0 && abs(me.getY() - mInitialTouchY) > mScaledTouchSlop) {
            beginDrag();
            // Will hit the STATE_DRAGGING check below
        }

        if (mState == STATE_DRAGGING) {
            // TODO: Ignore jitter.
            const float pos = getPosFromMotionEvent(me.getY());
            setThumbPos(pos);
            // If the previous scrollTo is still pending
            if (mScrollCompleted) scrollTo(pos);
            return true;
        }
        break;
    case MotionEvent::ACTION_CANCEL:   cancelPendingDrag();    break;
    }

    return false;
}

bool FastScroller::isPointInside(float x, float y) {
    return isPointInsideX(x) && (mTrackDrawable || isPointInsideY(y));
}

bool FastScroller::isPointInsideX(float x) {
    const float offset = mThumbImage->getTranslationX();
    const float left = mThumbImage->getLeft() + offset;
    const float right = mThumbImage->getRight() + offset;

    // Apply the minimum touch target size.
    const float targetSizeDiff = mMinimumTouchTarget - (right - left);
    const float adjust = targetSizeDiff > 0 ? targetSizeDiff : 0;

    if (mLayoutFromRight) {
        return x >= mThumbImage->getLeft() - adjust;
    } else {
        return x <= mThumbImage->getRight() + adjust;
    }
}

bool FastScroller::isPointInsideY(float y) {
   const float offset = mThumbImage->getTranslationY();
   const float top = mThumbImage->getTop() + offset;
   const float bottom = mThumbImage->getBottom() + offset;

   // Apply the minimum touch target size.
   const float targetSizeDiff = mMinimumTouchTarget - (bottom - top);
   const float adjust = (targetSizeDiff > 0) ? targetSizeDiff / 2.f : 0;

   return (y >= (top - adjust)) && (y <= (bottom + adjust));
}

Animator* FastScroller::groupAnimatorOfFloat(const std::string&propName, float value,const std::vector<View*>&views){
    AnimatorSet* animSet = new AnimatorSet();
    AnimatorSet::Builder* builder = nullptr;

    for (int i = int(views.size() - 1); i >= 0; i--) {
        Animator* anim = ObjectAnimator::ofFloat(views[i],propName, {value});
        if (builder == nullptr) {
            builder = animSet->play(anim);
        } else {
            builder->with(anim);
        }
    }
    delete builder;
    return animSet;
}

Animator* FastScroller::animateScaleX(View* v, float target){
    return ObjectAnimator::ofFloat(v,"scaleX", {target});
}

Animator* FastScroller::animateAlpha(View* v, float alpha){
    return ObjectAnimator::ofFloat(v,"alpha", {alpha});
}

Animator* FastScroller::animateBounds(View* v,const Rect& bounds){
    PropertyValuesHolder* left = PropertyValuesHolder::ofInt("left", {bounds.left});
    PropertyValuesHolder* top = PropertyValuesHolder::ofInt("top", {bounds.top});
    PropertyValuesHolder* right = PropertyValuesHolder::ofInt("right", {bounds.width});
    PropertyValuesHolder* bottom = PropertyValuesHolder::ofInt("bottom", {bounds.height});
    return ObjectAnimator::ofPropertyValuesHolder(v,{left, top, right, bottom});
}
}//endof namespace



