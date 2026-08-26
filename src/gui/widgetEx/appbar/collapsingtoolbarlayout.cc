#include <widgetEx/appbar/collapsingtoolbarlayout.h>
#include <widgetEx/appbar/appbarlayout.h>
#include <widgetEx/widgetex_styleable.h>
#include <drawable/drawable.h>
#include <drawable/colordrawable.h>
#include <content/typedarray.h>
#include <view/viewgroup.h>
#include <algorithm>

namespace cdroid {
using namespace cdroid::internal;

CollapsingToolbarLayout::LayoutParams::LayoutParams(Context* context, const AttributeSet& attrs)
    : FrameLayout::LayoutParams(context, attrs) {
    auto ta = context->obtainStyledAttributes(attrs,
                R::styleable::CollapsingToolbarLayoutLayout, 0, 0);
            collapseMode = ta->getInt(R::styleable::CollapsingToolbarLayoutLayout_layout_collapseMode,
            COLLAPSE_MODE_OFF);
    parallaxMultiplier = ta->getFloat(
                R::styleable::CollapsingToolbarLayoutLayout_layout_collapseParallaxMultiplier, 0.5f);
}

CollapsingToolbarLayout::LayoutParams::LayoutParams(int width, int height)
    : FrameLayout::LayoutParams(width, height) {}

CollapsingToolbarLayout::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
    : FrameLayout::LayoutParams(source) {}

CollapsingToolbarLayout::CollapsingToolbarLayout(Context* context, const AttributeSet* attrs)
    : CollapsingToolbarLayout(context, attrs, 0) {}

CollapsingToolbarLayout::CollapsingToolbarLayout(Context* context,
        const AttributeSet* attrs, int defStyleAttr)
    : FrameLayout(context, attrs, defStyleAttr), mCollapsingTextHelper(this) {
    auto ta = context->obtainStyledAttributes(*attrs,
            R::styleable::CollapsingToolbarLayout, defStyleAttr, 0);
    mToolbarId = ta->getResourceId(R::styleable::CollapsingToolbarLayout_toolbarId, View::NO_ID);
    mTitleCollapseMode = ta->getInt(
            R::styleable::CollapsingToolbarLayout_titleCollapseMode, TITLE_COLLAPSE_MODE_SCALE);
    const int margin = ta->getDimensionPixelSize(
            R::styleable::CollapsingToolbarLayout_expandedTitleMargin, 0);
    mExpandedMarginStart = mExpandedMarginTop = mExpandedMarginEnd = mExpandedMarginBottom = margin;
    if (ta->hasValue(R::styleable::CollapsingToolbarLayout_contentScrim)) {
        mContentScrim = ta->getDrawable(R::styleable::CollapsingToolbarLayout_contentScrim);
    }
    if (ta->hasValue(R::styleable::CollapsingToolbarLayout_statusBarScrim)) {
        mStatusBarScrim = ta->getDrawable(R::styleable::CollapsingToolbarLayout_statusBarScrim);
    }
    setWillNotDraw(false);
}

CollapsingToolbarLayout::~CollapsingToolbarLayout() {
    delete mContentScrim;
    delete mStatusBarScrim;
}

void CollapsingToolbarLayout::setTitle(const std::u16string& title) {
    if (mTitle == title) return;
    mTitle = title;
    mCollapsingTextHelper.setText(title);
    invalidate();
}

const std::u16string& CollapsingToolbarLayout::getTitle() const { return mTitle; }

void CollapsingToolbarLayout::setTitleCollapseMode(int mode) {
    if (mode != TITLE_COLLAPSE_MODE_SCALE && mode != TITLE_COLLAPSE_MODE_FADE) return;
    mTitleCollapseMode = mode;
    invalidate();
}

int CollapsingToolbarLayout::getTitleCollapseMode() const { return mTitleCollapseMode; }

void CollapsingToolbarLayout::setContentScrim(Drawable* scrim) {
    if (mContentScrim == scrim) return;
    delete mContentScrim;
    mContentScrim = scrim;
    invalidate();
}

Drawable* CollapsingToolbarLayout::getContentScrim() const { return mContentScrim; }

void CollapsingToolbarLayout::setStatusBarScrim(Drawable* scrim) {
    if (mStatusBarScrim == scrim) return;
    delete mStatusBarScrim;
    mStatusBarScrim = scrim;
    invalidate();
}

Drawable* CollapsingToolbarLayout::getStatusBarScrim() const { return mStatusBarScrim; }

void CollapsingToolbarLayout::setScrimsShown(bool shown) {
    if (mScrimsShown == shown) return;
    mScrimsShown = shown;
    mScrimAlpha = shown ? 255 : 0;
    invalidate();
}

bool CollapsingToolbarLayout::isScrimsShown() const { return mScrimsShown; }

void CollapsingToolbarLayout::setExpandedTitleMargin(int start, int top, int end, int bottom) {
    mExpandedMarginStart = start;
    mExpandedMarginTop = top;
    mExpandedMarginEnd = end;
    mExpandedMarginBottom = bottom;
    updateTitleBounds();
}

void CollapsingToolbarLayout::setToolbarId(int id) {
    mToolbarId = id;
    mRefreshToolbar = true;
    requestLayout();
}

int CollapsingToolbarLayout::getToolbarId() const { return mToolbarId; }

void CollapsingToolbarLayout::updateToolbar() {
    if (!mRefreshToolbar) return;
    mToolbar = nullptr;
    if (mToolbarId != View::NO_ID) mToolbar = findViewById(mToolbarId);
    if (mToolbar == nullptr && getChildCount() > 0) mToolbar = getChildAt(getChildCount() - 1);
    mRefreshToolbar = false;
}

void CollapsingToolbarLayout::onAttachedToWindow() {
    FrameLayout::onAttachedToWindow();
    ViewGroup* parent = getParent();
    while (parent != nullptr) {
        AppBarLayout* appBar = dynamic_cast<AppBarLayout*>(parent);
        if (appBar != nullptr) {
            appBar->addOnOffsetChangedListener([this](AppBarLayout& bar, int offset) {
                onOffsetChanged(bar, offset);
            });
            break;
        }
        parent = parent->getParent();
    }
}

void CollapsingToolbarLayout::updateTitleBounds() {
    updateToolbar();
    const int collapsedLeft = mToolbar ? mToolbar->getLeft() + mToolbar->getPaddingLeft() : getPaddingLeft();
    const int collapsedTop = mToolbar ? mToolbar->getTop() + mToolbar->getPaddingTop() : getPaddingTop();
    const int collapsedRight = mToolbar ? mToolbar->getRight() - mToolbar->getPaddingRight() : getWidth() - getPaddingRight();
    const int collapsedBottom = mToolbar ? mToolbar->getBottom() - mToolbar->getPaddingBottom() : getHeight() - getPaddingBottom();
    mCollapsingTextHelper.setExpandedBounds(mExpandedMarginStart, mExpandedMarginTop,
            getWidth() - mExpandedMarginEnd, getHeight() - mExpandedMarginBottom);
    mCollapsingTextHelper.setCollapsedBounds(collapsedLeft, collapsedTop,
            std::max(collapsedLeft, collapsedRight), std::max(collapsedTop, collapsedBottom));
}

void CollapsingToolbarLayout::updateScrimVisibility() {
    const int trigger = std::max(0, getHeight() - getMinimumHeight() * 2);
    setScrimsShown(getHeight() + mCurrentOffset < trigger);
}

void CollapsingToolbarLayout::onOffsetChanged(AppBarLayout& appBarLayout, int verticalOffset) {
    (void)appBarLayout;
    mCurrentOffset = verticalOffset;
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = dynamic_cast<LayoutParams*>(child->getLayoutParams());
        if (lp == nullptr) continue;
        if (lp->collapseMode == LayoutParams::COLLAPSE_MODE_PIN) {
            child->offsetTopAndBottom(std::max(0, -verticalOffset) - child->getTop());
        } else if (lp->collapseMode == LayoutParams::COLLAPSE_MODE_PARALLAX) {
            child->setTranslationY(-verticalOffset * lp->parallaxMultiplier);
        }
    }
    const int range = std::max(1, getHeight() - getMinimumHeight());
    mCollapsingTextHelper.setExpandedFraction(
            static_cast<float>(-verticalOffset) / static_cast<float>(range));
    updateScrimVisibility();
    invalidate();
}

void CollapsingToolbarLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    updateToolbar();
    updateTitleBounds();
}

void CollapsingToolbarLayout::onLayout(bool changed, int left, int top, int right, int bottom) {
    FrameLayout::onLayout(changed, left, top, right, bottom);
    updateToolbar();
    updateTitleBounds();
}

void CollapsingToolbarLayout::onDraw(Canvas& canvas) {
    FrameLayout::onDraw(canvas);
    if (mContentScrim && mScrimAlpha > 0) {
        mContentScrim->setBounds(0, 0, getWidth(), getHeight());
        mContentScrim->setAlpha(mScrimAlpha);
        mContentScrim->draw(canvas);
    }
    if (mTitleEnabled) mCollapsingTextHelper.draw(canvas);
    if (mStatusBarScrim && mScrimAlpha > 0) {
        mStatusBarScrim->setBounds(0, 0, getWidth(), getPaddingTop());
        mStatusBarScrim->setAlpha(mScrimAlpha);
        mStatusBarScrim->draw(canvas);
    }
}

CollapsingToolbarLayout::LayoutParams* CollapsingToolbarLayout::generateDefaultLayoutParams() const {
    return new LayoutParams(ViewGroup::LayoutParams::MATCH_PARENT,
            ViewGroup::LayoutParams::MATCH_PARENT);
}

CollapsingToolbarLayout::LayoutParams* CollapsingToolbarLayout::generateLayoutParams(
        const ViewGroup::LayoutParams* params) const {
    return new LayoutParams(*params);
}

CollapsingToolbarLayout::LayoutParams* CollapsingToolbarLayout::generateLayoutParams(
        const AttributeSet& attrs) const {
    return new LayoutParams(mContext, attrs);
}

DECLARE_WIDGET(CollapsingToolbarLayout)

}
