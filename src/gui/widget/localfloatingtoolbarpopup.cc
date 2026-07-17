/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
 *
 * Line-by-line port of AOSP android-36
 *   com.android.internal.widget.floatingtoolbar.LocalFloatingToolbarPopup.
 *
 * Phase 3a: PopupWindow + content + main panel + positioning core + show/dismiss/
 * hide/updateCoordinates + layoutMainPanelItems + MenuItemRepr.
 * Phase 3b: overflow panel (custom ListView) + overflow button + adapter +
 *   OverflowPanelViewHelper + layoutOverflowPanelItems + sizing helpers + full
 *   setPanelsStatesAtRestingPosition + open/close overflow (instant + AVD morph).
 *   The open/close width/height/button tween, show/dismiss/hide alpha, and the
 *   LogAccelerateInterpolator are TODO(3b-anim). Touchable-region insets and
 *   app-bounds clamping are TODO(3c).
 *
 * CDROID-specific divergence (dependency reason, behavior preserved):
 *   CDROID's ImageView TAKES OWNERSHIP of the Drawable passed to setImageDrawable
 *   (it deletes the old one on replace and in its dtor) -- unlike AOSP, whose
 *   ImageView borrows drawables. Therefore overflow-button icons are fetched fresh
 *   per setImageDrawable call (the Assets cache hands out new instances), and menu
 *   item icons are cloned (when a ConstantState is available) before being handed
 *   to a button, so ImageView never deletes MenuItemImpl's owned mIconDrawable.
 *********************************************************************************/
#include <widget/localfloatingtoolbarpopup.h>
#include <widget/floatingtoolbar.h>
#include <widget/floatingtoolbarpopup.h>
#include <widget/linearlayout.h>
#include <widget/popupwindow.h>
#include <widget/textview.h>
#include <widget/imageview.h>
#include <widget/R.h>
#include <view/layoutinflater.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <menu/menuitem.h>
#include <drawable/colordrawable.h>
#include <drawable/drawable.h>
#include <core/context.h>
#include <core/rect.h>
#include <porting/cdlog.h>
#include <algorithm>
#include <cmath>

namespace cdroid{

// Out-of-line definitions for the odr-used static constexpr members (C++14:
// std::min/max take them by const reference).
constexpr int LocalFloatingToolbarPopup::MIN_OVERFLOW_SIZE;
constexpr int LocalFloatingToolbarPopup::MAX_OVERFLOW_SIZE;

// =====================================================================================
//  MenuItemRepr  (AOSP public static final class MenuItemRepr)
// =====================================================================================
LocalFloatingToolbarPopup::MenuItemRepr LocalFloatingToolbarPopup::MenuItemRepr::of(MenuItem* menuItem) {
    MenuItemRepr r;
    r.itemId = menuItem->getItemId();
    r.groupId = menuItem->getGroupId();
    r.title = menuItem->getTitle();
    r.hasIcon = menuItem->getIcon() != nullptr;
    return r;
}

bool LocalFloatingToolbarPopup::MenuItemRepr::equals(const MenuItemRepr& o) const {
    return itemId == o.itemId && groupId == o.groupId && title == o.title;
}

bool LocalFloatingToolbarPopup::MenuItemRepr::operator<(const MenuItemRepr& o) const {
    if (itemId != o.itemId) return itemId < o.itemId;
    if (groupId != o.groupId) return groupId < o.groupId;
    return title < o.title;
}

bool LocalFloatingToolbarPopup::MenuItemRepr::reprEquals(const std::vector<MenuItem*>& a, const std::vector<MenuItem*>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) {
        if (!MenuItemRepr::of(a[i]).equals(MenuItemRepr::of(b[i]))) return false;
    }
    return true;
}

// =====================================================================================
//  LocalFloatingToolbarPopup
// =====================================================================================
LocalFloatingToolbarPopup::LocalFloatingToolbarPopup(Context* context, View* parent)
    : mContext(LocalFloatingToolbarPopup::applyDefaultTheme(context))
    , mParent(parent)
    , mPopupWindow(nullptr)
    , mMarginHorizontal(0), mMarginVertical(0), mLineHeight(0), mIconTextSpacing(0)
    , mContentContainer(nullptr)
    , mMainPanel(nullptr)
    , mOverflowPanel(nullptr)
    , mOverflowButton(nullptr)
    , mOverflowPanelAdapter(nullptr)
    , mOverflowPanelViewHelper(nullptr) {
    mCoordsOnWindow[0] = mCoordsOnWindow[1] = 0;
    mTmpCoords[0] = mTmpCoords[1] = 0;

    mContentContainer = createContentContainer(mContext);
    mPopupWindow = createPopupWindow(mContentContainer);

    mMarginHorizontal = mContext->getDimensionPixelSize("cdroid:dimen/floating_toolbar_horizontal_margin");
    mMarginVertical   = mContext->getDimensionPixelSize("cdroid:dimen/floating_toolbar_vertical_margin");
    mLineHeight       = mContext->getDimensionPixelSize("cdroid:dimen/floating_toolbar_height");
    mIconTextSpacing  = mContext->getDimensionPixelSize("cdroid:dimen/floating_toolbar_icon_text_spacing");

    // Views. Drawables are fetched fresh per setImageDrawable (see file header note).
    mOverflowButton = createOverflowButton();
    mOverflowButtonSize = measure(mOverflowButton);
    mMainPanel = createMainPanel();
    mOverflowPanelViewHelper = new OverflowPanelViewHelper(mContext, mIconTextSpacing);
    mOverflowPanelAdapter = new OverflowItemAdapter(this);
    mOverflowPanel = createOverflowPanel();

    mMenuItemButtonOnClickListener = [this](View& v) {
        // The button's tag is the MenuItem* it represents (AOSP stores a MenuItemRepr and
        // looks up mMenuItems; functionally equivalent since buttons are recreated on relayout).
        if (!mOnMenuItemClickListener) return;
        MenuItem* menuItem = static_cast<MenuItem*>(v.getTag());
        if (menuItem == nullptr) return;
        mOnMenuItemClickListener(*menuItem);
    };
    mOverflowPanelItemClickListener = [this](AdapterView& /*parent*/, View& /*view*/, int position, long /*id*/) {
        if (!mOnMenuItemClickListener) return;
        MenuItem* menuItem = static_cast<MenuItem*>(mOverflowPanelAdapter->getItem(position));
        if (menuItem == nullptr) return;
        mOnMenuItemClickListener(*menuItem);
    };
}

LocalFloatingToolbarPopup::~LocalFloatingToolbarPopup() {
    // AOSP relies on GC; CDROID's PopupWindow dtor frees only its background drawables, and
    // ViewGroup never deletes children. So this popup owns its whole content tree and must
    // free it. The popup window was dismissed (finish->dismiss) before destruction; its
    // decorView Window is async-removed by WindowManager and does not reference these views.
    ViewGroup* holder = (mPopupWindow != nullptr)
            ? (ViewGroup*) mPopupWindow->getContentView()   // popupContentHolder
            : nullptr;
    auto detach = [](View* v){
        if (v != nullptr) {
            ViewGroup* p = v->getParent();
            if (p != nullptr) p->removeView(v);   // detach only (no deletion)
        }
    };
    // Detach children before their containers so no parent holds a dangling pointer.
    detach(mMainPanel);
    detach(mOverflowPanel);
    detach(mOverflowButton);
    detach(mContentContainer);   // child of popupContentHolder
    if (holder != nullptr) {
        ViewGroup* hp = holder->getParent();
        if (hp != nullptr) hp->removeView(holder);   // detach from the (dismissed) decor tree
    }
    delete mPopupWindow;             // PopupWindow object (dtor frees its background drawables only)
    delete holder;                   // popupContentHolder (the PopupWindow's contentView)
    delete mOverflowPanelViewHelper; // owns its mCalculator button
    delete mMainPanel;
    delete mOverflowPanel;           // dtor may reference its adapter (still valid below)
    delete mOverflowButton;
    delete mContentContainer;
    delete mOverflowPanelAdapter;    // AdapterView keeps a raw pointer only (no delete)
}

FloatingToolbarPopup* FloatingToolbarPopup::createInstance(Context* context, View* parent) {
    return new LocalFloatingToolbarPopup(context, parent);
}

bool LocalFloatingToolbarPopup::setOutsideTouchable(
        bool outsideTouchable, const std::function<void()>& onDismiss) {
    bool ret = false;
    if ((mPopupWindow->isOutsideTouchable() ? true : false) != outsideTouchable) {
        mPopupWindow->setOutsideTouchable(outsideTouchable);
        mPopupWindow->setFocusable(!outsideTouchable);
        mPopupWindow->update();
        ret = true;
    }
    mPopupWindow->setOnDismissListener(onDismiss);
    return ret;
}

void LocalFloatingToolbarPopup::setWidthChanged(bool widthChanged) {
    mWidthChanged = widthChanged;
}

void LocalFloatingToolbarPopup::setSuggestedWidth(int suggestedWidth) {
    // Check if there's been a substantial width spec change.
    int difference = std::abs(suggestedWidth - mSuggestedWidth);
    mWidthChanged = difference > (mSuggestedWidth * 0.2);
    mSuggestedWidth = suggestedWidth;
}

void LocalFloatingToolbarPopup::show(const std::vector<MenuItem*>& menuItems,
        const MenuItem::OnMenuItemClickListener& menuItemClickListener, const Rect& contentRect) {
    if (isLayoutRequired(menuItems) || mWidthChanged) {
        dismiss();
        layoutMenuItems(menuItems, menuItemClickListener, mSuggestedWidth);
    } else {
        updateMenuItems(menuItems, menuItemClickListener);
    }
    if (!isShowing()) {
        show(contentRect);
    } else if (!(mPreviousContentRect == contentRect)) {
        updateCoordinates(contentRect);
    }
    mWidthChanged = false;
    mPreviousContentRect = contentRect;
}

void LocalFloatingToolbarPopup::show(const Rect& contentRectOnScreen) {
    if (isShowing()) {
        return;
    }
    mHidden = false;
    mDismissed = false;
    cancelDismissAndHideAnimations();
    cancelOverflowAnimations();

    refreshCoordinatesAndOverflowDirection(contentRectOnScreen);
    preparePopupContent();
    // The position is specified in window coordinates.
    mPopupWindow->showAtLocation(mParent, Gravity::NO_GRAVITY,
                                 mCoordsOnWindow[0], mCoordsOnWindow[1]);
    // Force a layout + draw on the freshly-attached popup: addWindow/showAtLocation/moveWindow
    // don't invalidate the new window itself (moveWindow only flips + dirties windows below it),
    // so the first frame can be blank when the previous popup's teardown races this show.
    if (View* root = mPopupWindow->getContentView()) {
        root->requestLayout();
        root->invalidate();
    }
    setTouchableSurfaceInsetsComputer();
    runShowAnimation();
}

void LocalFloatingToolbarPopup::dismiss() {
    if (mDismissed) {
        return;
    }
    mHidden = false;
    mDismissed = true;
    // TODO(3b-anim): cancel hide animation, run dismiss animation; 3a/3b do the cleanup directly.
    mPopupWindow->dismiss();
    mContentContainer->removeAllViews();
    setZeroTouchableSurface();
}

void LocalFloatingToolbarPopup::hide() {
    if (!isShowing()) {
        return;
    }
    mHidden = true;
    // TODO(3b-anim): run hide animation; 3a/3b dismiss the window directly (isShowing()->false).
    mPopupWindow->dismiss();
    setZeroTouchableSurface();
}

bool LocalFloatingToolbarPopup::isShowing() {
    return !mDismissed && !mHidden;
}

bool LocalFloatingToolbarPopup::isHidden() {
    return mHidden;
}

void LocalFloatingToolbarPopup::updateCoordinates(const Rect& contentRectOnScreen) {
    if (!isShowing() || !mPopupWindow->isShowing()) {
        return;
    }
    cancelOverflowAnimations();
    refreshCoordinatesAndOverflowDirection(contentRectOnScreen);
    preparePopupContent();
    mPopupWindow->update(mCoordsOnWindow[0], mCoordsOnWindow[1],
                         mPopupWindow->getWidth(), mPopupWindow->getHeight());
}

void LocalFloatingToolbarPopup::refreshCoordinatesAndOverflowDirection(const Rect& contentRectOnScreen) {
    refreshViewPort();

    int xVal = 0;
    if (mPopupWindow->getWidth() > mViewPortOnScreen.width) {
        // Not enough space - prefer to position as far left as possible.
        xVal = mViewPortOnScreen.left;
    } else {
        // Initialize x ensuring the toolbar isn't rendered behind the system bar insets.
        int lo = mViewPortOnScreen.left;
        int hi = mViewPortOnScreen.right() - mPopupWindow->getWidth();
        int v = contentRectOnScreen.centerX() - mPopupWindow->getWidth() / 2;
        xVal = std::min(std::max(v, lo), hi);   // Math.clamp(v, lo, hi)
    }

    int yVal = 0;
    const int availableHeightAboveContent = contentRectOnScreen.top - mViewPortOnScreen.top;
    const int availableHeightBelowContent = mViewPortOnScreen.bottom() - contentRectOnScreen.bottom();
    const int margin = 2 * mMarginVertical;
    const int toolbarHeightWithVerticalMargin = mLineHeight + margin;

    if (!hasOverflow()) {
        if (availableHeightAboveContent >= toolbarHeightWithVerticalMargin) {
            // Enough space at the top of the content.
            yVal = contentRectOnScreen.top - toolbarHeightWithVerticalMargin;
        } else if (availableHeightBelowContent >= toolbarHeightWithVerticalMargin) {
            // Enough space at the bottom of the content.
            yVal = contentRectOnScreen.bottom();
        } else if (availableHeightBelowContent >= mLineHeight) {
            // Just enough space to fit the toolbar with no vertical margins.
            yVal = contentRectOnScreen.bottom() - mMarginVertical;
        } else {
            // Not enough space. Prefer to position as high as possible.
            yVal = std::max(mViewPortOnScreen.top,
                            contentRectOnScreen.top - toolbarHeightWithVerticalMargin);
        }
    } else {
        // Has an overflow.
        const int minimumOverflowHeightWithMargin =
                calculateOverflowHeight(MIN_OVERFLOW_SIZE) + margin;
        const int availableHeightThroughContentDown =
                mViewPortOnScreen.bottom() - contentRectOnScreen.top
                        + toolbarHeightWithVerticalMargin;
        const int availableHeightThroughContentUp =
                contentRectOnScreen.bottom() - mViewPortOnScreen.top
                        + toolbarHeightWithVerticalMargin;

        if (availableHeightAboveContent >= minimumOverflowHeightWithMargin) {
            // Enough space at the top of the content rect for the overflow.
            updateOverflowHeight(availableHeightAboveContent - margin);
            yVal = contentRectOnScreen.top - mPopupWindow->getHeight();
            mOpenOverflowUpwards = true;
        } else if (availableHeightAboveContent >= toolbarHeightWithVerticalMargin
                && availableHeightThroughContentDown >= minimumOverflowHeightWithMargin) {
            // Enough space at the top for the main panel but not the overflow. Open downwards.
            updateOverflowHeight(availableHeightThroughContentDown - margin);
            yVal = contentRectOnScreen.top - toolbarHeightWithVerticalMargin;
            mOpenOverflowUpwards = false;
        } else if (availableHeightBelowContent >= minimumOverflowHeightWithMargin) {
            // Enough space at the bottom of the content rect for the overflow. Open downwards.
            updateOverflowHeight(availableHeightBelowContent - margin);
            yVal = contentRectOnScreen.bottom();
            mOpenOverflowUpwards = false;
        } else if (availableHeightBelowContent >= toolbarHeightWithVerticalMargin
                && mViewPortOnScreen.height >= minimumOverflowHeightWithMargin) {
            // Enough space at the bottom for the main panel but not the overflow. Open upwards.
            updateOverflowHeight(availableHeightThroughContentUp - margin);
            yVal = contentRectOnScreen.bottom() + toolbarHeightWithVerticalMargin
                    - mPopupWindow->getHeight();
            mOpenOverflowUpwards = true;
        } else {
            // Not enough space. Position at the top of the view port and open downwards.
            updateOverflowHeight(mViewPortOnScreen.height - margin);
            yVal = mViewPortOnScreen.top;
            mOpenOverflowUpwards = false;
        }
    }

    // Convert screen coords -> window coords via the root view's screen/window offset.
    mParent->getRootView()->getLocationOnScreen(mTmpCoords);
    int rootViewLeftOnScreen = mTmpCoords[0];
    int rootViewTopOnScreen = mTmpCoords[1];
    mParent->getRootView()->getLocationInWindow(mTmpCoords);
    int rootViewLeftOnWindow = mTmpCoords[0];
    int rootViewTopOnWindow = mTmpCoords[1];
    int windowLeftOnScreen = rootViewLeftOnScreen - rootViewLeftOnWindow;
    int windowTopOnScreen = rootViewTopOnScreen - rootViewTopOnWindow;
    // TODO(3c): clamp to Configuration.windowConfiguration.getAppBounds() (not ported yet);
    //           AOSP clamps the popup coords to the app bounds here. For now no app-bounds clamp.
    mCoordsOnWindow[0] = xVal - windowLeftOnScreen;
    mCoordsOnWindow[1] = yVal - windowTopOnScreen;
}

void LocalFloatingToolbarPopup::refreshViewPort() {
    mParent->getWindowVisibleDisplayFrame(mViewPortOnScreen);
}

int LocalFloatingToolbarPopup::calculateOverflowHeight(int maxItemSize) const {
    // Maximum of 4 items, minimum of 2 if the overflow has to scroll.
    int actualSize = std::min(MAX_OVERFLOW_SIZE,
            std::min(std::max(MIN_OVERFLOW_SIZE, maxItemSize), mOverflowPanel->getCount()));
    int extension = 0;
    if (actualSize < mOverflowPanel->getCount()) {
        // The overflow will require scrolling to get to all the items.
        // Extend the height so that part of the hidden items is displayed.
        extension = (int) (mLineHeight * 0.5f);
    }
    return actualSize * mLineHeight + mOverflowButtonSize.height() + extension;
}

void LocalFloatingToolbarPopup::updateOverflowHeight(int suggestedHeight) {
    if (hasOverflow()) {
        const int maxItemSize = (suggestedHeight - mOverflowButtonSize.height()) / mLineHeight;
        const int newHeight = calculateOverflowHeight(maxItemSize);
        if (mOverflowPanelSize.height() != newHeight) {
            mOverflowPanelSize = Size::Make(mOverflowPanelSize.width(), newHeight);
        }
        setSize(mOverflowPanel, mOverflowPanelSize);
        if (mIsOverflowOpen) {
            setSize(mContentContainer, mOverflowPanelSize);
            if (mOpenOverflowUpwards) {
                const int deltaHeight = mOverflowPanelSize.height() - newHeight;
                mContentContainer->setY(mContentContainer->getY() + deltaHeight);
                mOverflowButton->setY(mOverflowButton->getY() - deltaHeight);
            }
        } else {
            setSize(mContentContainer, mMainPanelSize);
        }
        updatePopupSize();
    }
}

void LocalFloatingToolbarPopup::updatePopupSize() {
    int width = 0;
    int height = 0;
    width = std::max(width, mMainPanelSize.width());
    height = std::max(height, mMainPanelSize.height());
    width = std::max(width, mOverflowPanelSize.width());
    height = std::max(height, mOverflowPanelSize.height());
    mPopupWindow->setWidth(width + mMarginHorizontal * 2);
    mPopupWindow->setHeight(height + mMarginVertical * 2);
    maybeComputeTransitionDurationScale();
}

int LocalFloatingToolbarPopup::getAdjustedToolbarWidth(int suggestedWidth) {
    int width = suggestedWidth;
    refreshViewPort();
    int maxWidth = mViewPortOnScreen.width - 2 * mMarginHorizontal;
    if (width <= 0) {
        // No suggested width; use the preferred width dimen.
        width = mContext->getDimensionPixelSize("cdroid:dimen/floating_toolbar_preferred_width");
    }
    return std::min(width, maxWidth);
}

int LocalFloatingToolbarPopup::getOverflowWidth() const {
    int overflowWidth = 0;
    const int count = mOverflowPanelAdapter->getCount();
    for (int i = 0; i < count; i++) {
        MenuItem* menuItem = static_cast<MenuItem*>(mOverflowPanelAdapter->getItem(i));
        overflowWidth = std::max(mOverflowPanelViewHelper->calculateWidth(menuItem), overflowWidth);
    }
    return overflowWidth;
}

void LocalFloatingToolbarPopup::layoutMenuItems(const std::vector<MenuItem*>& menuItems,
        const MenuItem::OnMenuItemClickListener& menuItemClickListener, int suggestedWidth) {
    cancelOverflowAnimations();
    clearPanels();
    updateMenuItems(menuItems, menuItemClickListener);
    std::vector<MenuItem*> remaining = layoutMainPanelItems(menuItems, getAdjustedToolbarWidth(suggestedWidth));
    if (!remaining.empty()) {
        // Add remaining items to the overflow.
        layoutOverflowPanelItems(remaining);
    }
    updatePopupSize();
}

void LocalFloatingToolbarPopup::updateMenuItems(const std::vector<MenuItem*>& menuItems,
        const MenuItem::OnMenuItemClickListener& menuItemClickListener) {
    mMenuItems.clear();
    for (MenuItem* menuItem : menuItems) {
        mMenuItems[MenuItemRepr::of(menuItem)] = menuItem;
    }
    mOnMenuItemClickListener = menuItemClickListener;
}

bool LocalFloatingToolbarPopup::isLayoutRequired(const std::vector<MenuItem*>& menuItems) const {
    std::vector<MenuItem*> current;
    for (auto& kv : mMenuItems) current.push_back(kv.second);
    return !MenuItemRepr::reprEquals(menuItems, current);
}

std::vector<MenuItem*> LocalFloatingToolbarPopup::layoutMainPanelItems(
        const std::vector<MenuItem*>& menuItems, int toolbarWidth) {
    int availableWidth = toolbarWidth;

    std::vector<MenuItem*> remainingMenuItems;
    // Add the overflow menu items to the end of the remainingMenuItems list.
    std::vector<MenuItem*> overflowMenuItems;
    for (MenuItem* menuItem : menuItems) {
        if (menuItem->getItemId() != R::id::textAssist && menuItem->requiresOverflow()) {
            overflowMenuItems.push_back(menuItem);
        } else {
            remainingMenuItems.push_back(menuItem);
        }
    }
    remainingMenuItems.insert(remainingMenuItems.end(), overflowMenuItems.begin(), overflowMenuItems.end());

    mMainPanel->removeAllViews();
    mMainPanel->setPaddingRelative(0, 0, 0, 0);

    bool isFirstItem = true;
    while (!remainingMenuItems.empty()) {
        MenuItem* menuItem = remainingMenuItems.front();

        // If this is not the first item and it requires overflow, stop (rest go to overflow).
        if (!isFirstItem && menuItem->requiresOverflow()) {
            break;
        }

        const bool showIcon = isFirstItem && menuItem->getItemId() == R::id::textAssist;
        View* menuItemButton = createMenuItemButton(mContext, menuItem, mIconTextSpacing, showIcon);
        if (!showIcon) {
            // AOSP: ((LinearLayout) menuItemButton).setGravity(Gravity.CENTER).
            if (dynamic_cast<LinearLayout*>(menuItemButton)) {
                ((LinearLayout*) menuItemButton)->setGravity(Gravity::CENTER);
            }
        }

        // Additional start padding for the first button to even out spacing.
        if (isFirstItem) {
            menuItemButton->setPaddingRelative(
                    (int) (1.5 * menuItemButton->getPaddingStart()),
                    menuItemButton->getPaddingTop(),
                    menuItemButton->getPaddingEnd(),
                    menuItemButton->getPaddingBottom());
        }
        // Additional end padding for the last button to even out spacing.
        bool isLastItem = remainingMenuItems.size() == 1;
        if (isLastItem) {
            menuItemButton->setPaddingRelative(
                    menuItemButton->getPaddingStart(),
                    menuItemButton->getPaddingTop(),
                    (int) (1.5 * menuItemButton->getPaddingEnd()),
                    menuItemButton->getPaddingBottom());
        }

        menuItemButton->measure(View::MeasureSpec::UNSPECIFIED, View::MeasureSpec::UNSPECIFIED);
        const int menuItemButtonWidth = std::min(menuItemButton->getMeasuredWidth(), toolbarWidth);

        // Check if we can fit an item while reserving space for the overflowButton.
        const bool canFitWithOverflow = menuItemButtonWidth <= availableWidth - mOverflowButtonSize.width();
        const bool canFitNoOverflow = isLastItem && menuItemButtonWidth <= availableWidth;
        if (canFitWithOverflow || canFitNoOverflow) {
            // setButtonTagAndClickListener.
            menuItemButton->setTag(static_cast<void*>(menuItem));
            menuItemButton->setOnClickListener(mMenuItemButtonOnClickListener);
            menuItemButton->setTooltipText(menuItem->getTooltipText());
            mMainPanel->addView(menuItemButton);
            ViewGroup::LayoutParams* params = menuItemButton->getLayoutParams();
            params->width = menuItemButtonWidth;
            menuItemButton->setLayoutParams(params);
            availableWidth -= menuItemButtonWidth;
            remainingMenuItems.erase(remainingMenuItems.begin());
        } else {
            break;
        }
        isFirstItem = false;
    }

    if (!remainingMenuItems.empty()) {
        // Reserve space for the overflow button.
        mMainPanel->setPaddingRelative(0, 0, mOverflowButtonSize.width(), 0);
    }

    mMainPanelSize = measure(mMainPanel);
    return remainingMenuItems;
}

void LocalFloatingToolbarPopup::layoutOverflowPanelItems(const std::vector<MenuItem*>& menuItems) {
    mOverflowPanelAdapter->clear();
    const int size = menuItems.size();
    for (int i = 0; i < size; i++) {
        mOverflowPanelAdapter->add(menuItems[i]);
    }
    mOverflowPanel->setAdapter(mOverflowPanelAdapter);
    if (mOpenOverflowUpwards) {
        mOverflowPanel->setY(0);
    } else {
        mOverflowPanel->setY(mOverflowButtonSize.height());
    }

    int width = std::max(getOverflowWidth(), mOverflowButtonSize.width());
    int height = calculateOverflowHeight(MAX_OVERFLOW_SIZE);
    mOverflowPanelSize = Size::Make(width, height);
    setSize(mOverflowPanel, mOverflowPanelSize);
    mHasOverflow = true;
}

void LocalFloatingToolbarPopup::preparePopupContent() {
    mContentContainer->removeAllViews();
    // Add views in the specified order so they stack up as expected.
    // Order: overflowPanel, mainPanel, overflowButton.
    if (hasOverflow()) {
        mContentContainer->addView(mOverflowPanel);
    }
    mContentContainer->addView(mMainPanel);
    if (hasOverflow()) {
        mContentContainer->addView(mOverflowButton);
    }
    setPanelsStatesAtRestingPosition();
    setContentAreaAsTouchableSurface();
    // TODO(3b): RTL recalculation hack (hide + post reposition).
}

void LocalFloatingToolbarPopup::clearPanels() {
    mOverflowPanelSize = Size::MakeEmpty();
    mMainPanelSize = Size::MakeEmpty();
    mHasOverflow = false;
    mIsOverflowOpen = false;
    mMainPanel->removeAllViews();
    mOverflowPanelAdapter->clear();
    mOverflowPanel->setAdapter(mOverflowPanelAdapter);
    mContentContainer->removeAllViews();
}

void LocalFloatingToolbarPopup::setPanelsStatesAtRestingPosition() {
    mOverflowButton->setEnabled(true);
    mOverflowPanel->awakenScrollBars();

    if (mIsOverflowOpen) {
        // Set open state.
        const Size containerSize = mOverflowPanelSize;
        setSize(mContentContainer, containerSize);
        mMainPanel->setAlpha(0);
        mMainPanel->setVisibility(View::INVISIBLE);
        mOverflowPanel->setAlpha(1);
        mOverflowPanel->setVisibility(View::VISIBLE);
        // Overflow button shows the back arrow (AOSP mArrow == ft_avd_tooverflow).
        // Fresh instance: CDROID ImageView owns+deletes it (see file header note).
        Drawable* arrowIcon = mContext->getDrawable("cdroid:drawable/ft_avd_tooverflow");
        if (arrowIcon) arrowIcon->setAutoMirrored(true);
        mOverflowButton->setImageDrawable(arrowIcon);
        mOverflowButton->setContentDescription(
                mContext->getString("cdroid:string/floating_toolbar_close_overflow_description"));

        // Update x-coordinates. (TODO 3b-anim: RTL branch.)
        // LTR: align container right; main panel aligns right; overflow button + panel align left.
        mContentContainer->setX(mPopupWindow->getWidth() - containerSize.width() - mMarginHorizontal);
        mMainPanel->setX(-mContentContainer->getX());
        mOverflowButton->setX(0);
        mOverflowPanel->setX(0);

        // Update y-coordinates depending on overflow's open direction.
        if (mOpenOverflowUpwards) {
            mContentContainer->setY(mMarginVertical);  // align top
            mMainPanel->setY(containerSize.height() - mContentContainer->getHeight());
            mOverflowButton->setY(containerSize.height() - mOverflowButtonSize.height());
            mOverflowPanel->setY(0);  // align top
        } else {
            // opens downwards.
            mContentContainer->setY(mMarginVertical);  // align top
            mMainPanel->setY(0);
            mOverflowButton->setY(0);
            mOverflowPanel->setY(mOverflowButtonSize.height());
        }
    } else {
        // Overflow not open. Set closed state.
        const Size containerSize = mMainPanelSize;
        setSize(mContentContainer, containerSize);
        mMainPanel->setAlpha(1);
        mMainPanel->setVisibility(View::VISIBLE);
        mOverflowPanel->setAlpha(0);
        mOverflowPanel->setVisibility(View::INVISIBLE);
        // Overflow button shows the more icon (AOSP mOverflow == ft_avd_toarrow).
        Drawable* overflowIcon = mContext->getDrawable("cdroid:drawable/ft_avd_toarrow");
        if (overflowIcon) overflowIcon->setAutoMirrored(true);
        mOverflowButton->setImageDrawable(overflowIcon);
        mOverflowButton->setContentDescription(
                mContext->getString("cdroid:string/floating_toolbar_open_overflow_description"));

        if (hasOverflow()) {
            // Update x-coordinates. (TODO 3b-anim: RTL branch.)
            mContentContainer->setX(mPopupWindow->getWidth() - containerSize.width() - mMarginHorizontal);
            mMainPanel->setX(0);
            mOverflowButton->setX(containerSize.width() - mOverflowButtonSize.width());
            mOverflowPanel->setX(containerSize.width() - mOverflowPanelSize.width());

            // Update y-coordinates depending on overflow's open direction.
            if (mOpenOverflowUpwards) {
                mContentContainer->setY(
                        mMarginVertical + mOverflowPanelSize.height() - containerSize.height());
                mMainPanel->setY(0);
                mOverflowButton->setY(0);
                mOverflowPanel->setY(containerSize.height() - mOverflowPanelSize.height());
            } else {
                // opens downwards.
                mContentContainer->setY(mMarginVertical);
                mMainPanel->setY(0);
                mOverflowButton->setY(0);
                mOverflowPanel->setY(mOverflowButtonSize.height());
            }
        } else {
            // No overflow: position the container at (margin, margin) so the toolbar has
            // symmetric margins within the popup window (panel + 2*margin).
            mContentContainer->setX(mMarginHorizontal);  // align left
            mContentContainer->setY(mMarginVertical);    // align top
            mMainPanel->setX(0);
            mMainPanel->setY(0);
        }
    }
}

void LocalFloatingToolbarPopup::setContentAreaAsTouchableSurface() {
    // TODO(3c): set the touchable region (ViewTreeObserver.OnComputeInternalInsetsListener)
    //           to the content area. Not ported yet; the popup is touch-modal by default.
}

void LocalFloatingToolbarPopup::setZeroTouchableSurface() {
    // TODO(3c): zero out the touchable region.
}

void LocalFloatingToolbarPopup::setTouchableSurfaceInsetsComputer() {
    // TODO(3c): register ViewTreeObserver.OnComputeInternalInsetsListener (not ported yet).
}

void LocalFloatingToolbarPopup::positionContentYCoordinatesIfOpeningOverflowUpwards() {
    if (mOpenOverflowUpwards) {
        mMainPanel->setY(mContentContainer->getHeight() - mMainPanelSize.height());
        mOverflowButton->setY(mContentContainer->getHeight() - mOverflowButton->getHeight());
        mOverflowPanel->setY(mContentContainer->getHeight() - mOverflowPanelSize.height());
    }
}

bool LocalFloatingToolbarPopup::isInRTLMode() const {
    return false;   // TODO(3b-anim): honor layout direction + RTL support flag.
}

void LocalFloatingToolbarPopup::openOverflow() {
    mIsOverflowOpen = true;
    // TODO(3b-anim): tween container width/height + overflow-button X + panel alpha + AVD morph.
    // 3b: snap to the open resting state immediately.
    setPanelsStatesAtRestingPosition();
    setContentAreaAsTouchableSurface();
}

void LocalFloatingToolbarPopup::closeOverflow() {
    mIsOverflowOpen = false;
    // TODO(3b-anim): tween back + AVD morph.
    setPanelsStatesAtRestingPosition();
    setContentAreaAsTouchableSurface();
}

bool LocalFloatingToolbarPopup::isOverflowAnimating() const {
    // TODO(3b-anim): mOpenOverflowAnimation.hasStarted() && !hasEnded() || close...
    return false;
}

void LocalFloatingToolbarPopup::maybeComputeTransitionDurationScale() {
    // TODO(3b-anim): scale = euclidean(mainPanel - overflowPanel size) / density.
    //                Unused until the open/close tween is ported.
    mTransitionDurationScale = 0;
}

int LocalFloatingToolbarPopup::getAdjustedDuration(int originalDuration) const {
    // TODO(3b-anim): scale with mTransitionDurationScale + ValueAnimator::getDurationScale().
    return originalDuration;
}

ViewGroup* LocalFloatingToolbarPopup::createMainPanel() {
    // TODO(3b-anim): anonymous LinearLayout subclass overriding onMeasure (clamp to
    //                mMainPanelSize during overflow animation) + onInterceptTouchEvent.
    // 3b: plain horizontal LinearLayout (instant open/close => isOverflowAnimating()==false).
    LinearLayout* panel = new LinearLayout(mContext, AttributeSet(mContext, "cdroid"));
    panel->setOrientation(LinearLayout::HORIZONTAL);
    return panel;
}

ViewGroup* LocalFloatingToolbarPopup::createContentContainer(Context* context) {
    ViewGroup* contentContainer = (ViewGroup*) LayoutInflater::from(context)
            ->inflate("cdroid:layout/floating_popup_container", nullptr);
    ViewGroup::LayoutParams* lp = new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT);
    contentContainer->setLayoutParams(lp);
    contentContainer->setTag((void*)FloatingToolbar::FLOATING_TOOLBAR_TAG);
    contentContainer->setClipToOutline(true);
    return contentContainer;
}

PopupWindow* LocalFloatingToolbarPopup::createPopupWindow(ViewGroup* content) {
    LinearLayout* popupContentHolder = new LinearLayout(content->getContext(), AttributeSet(content->getContext(), "cdroid"));
    PopupWindow* popupWindow = new PopupWindow(popupContentHolder,
            ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT);
    popupWindow->setClippingEnabled(false);
    // TODO: popupWindow->setWindowLayoutType(TYPE_APPLICATION_ABOVE_SUB_PANEL) — CDROID has no
    //       such window type; left at default.
    popupWindow->setBackgroundDrawable(new ColorDrawable(0));   // Color.TRANSPARENT
    ViewGroup::LayoutParams* lp = new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT);
    content->setLayoutParams(lp);
    popupContentHolder->addView(content);
    return popupWindow;
}

ImageButton* LocalFloatingToolbarPopup::createOverflowButton() {
    ImageButton* overflowButton = (ImageButton*) LayoutInflater::from(mContext)
            ->inflate("cdroid:layout/floating_popup_overflow_button", nullptr);
    // Closed-state icon (AOSP mOverflow == ft_avd_toarrow). Fresh: ImageView owns+deletes it.
    Drawable* overflowIcon = mContext->getDrawable("cdroid:drawable/ft_avd_toarrow");
    if (overflowIcon) overflowIcon->setAutoMirrored(true);
    overflowButton->setImageDrawable(overflowIcon);
    overflowButton->setOnClickListener([this](View& /*v*/) {
        if (mIsOverflowOpen) {
            // TODO(3b-anim): overflowButton.setImageDrawable(mToOverflow); mToOverflow.start();
            closeOverflow();
        } else {
            // TODO(3b-anim): overflowButton.setImageDrawable(mToArrow); mToArrow.start();
            openOverflow();
        }
    });
    return overflowButton;
}

LocalFloatingToolbarPopup::OverflowPanel* LocalFloatingToolbarPopup::createOverflowPanel() {
    OverflowPanel* overflowPanel = new OverflowPanel(this);
    ViewGroup::LayoutParams* lp = new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT);
    overflowPanel->setLayoutParams(lp);
    overflowPanel->setDivider(nullptr);
    overflowPanel->setDividerHeight(0);
    overflowPanel->setAdapter(mOverflowPanelAdapter);
    overflowPanel->setOnItemClickListener(mOverflowPanelItemClickListener);
    return overflowPanel;
}

Size LocalFloatingToolbarPopup::measure(View* view) {
    view->measure(View::MeasureSpec::UNSPECIFIED, View::MeasureSpec::UNSPECIFIED);
    return Size::Make(view->getMeasuredWidth(), view->getMeasuredHeight());
}

void LocalFloatingToolbarPopup::setSize(View* view, int width, int height) {
    view->setMinimumWidth(width);
    view->setMinimumHeight(height);
    ViewGroup::LayoutParams* params = view->getLayoutParams();
    if (params == nullptr) params = new ViewGroup::LayoutParams(0, 0);
    params->width = width;
    params->height = height;
    view->setLayoutParams(params);
}

void LocalFloatingToolbarPopup::setSize(View* view, const Size& size) {
    setSize(view, size.width(), size.height());
}

void LocalFloatingToolbarPopup::setWidth(View* view, int width) {
    ViewGroup::LayoutParams* params = view->getLayoutParams();
    setSize(view, width, (params != nullptr) ? params->height : 0);
}

void LocalFloatingToolbarPopup::setHeight(View* view, int height) {
    ViewGroup::LayoutParams* params = view->getLayoutParams();
    setSize(view, (params != nullptr) ? params->width : 0, height);
}

View* LocalFloatingToolbarPopup::createMenuItemButton(
        Context* context, MenuItem* menuItem, int iconTextSpacing, bool showIcon) {
    View* menuItemButton = LayoutInflater::from(context)
            ->inflate("cdroid:layout/floating_popup_menu_button", nullptr);
    if (menuItem != nullptr) {
        updateMenuItemButton(menuItemButton, menuItem, iconTextSpacing, showIcon);
    }
    return menuItemButton;
}

void LocalFloatingToolbarPopup::updateMenuItemButton(
        View* menuItemButton, MenuItem* menuItem, int iconTextSpacing, bool showIcon) {
    TextView* buttonText = (TextView*) menuItemButton->findViewById(R::id::floating_toolbar_menu_item_text);
    if (buttonText) buttonText->setEllipsize(TextUtils::TruncateAt::NONE);
    if (menuItem->getTitle().empty()) {
        if (buttonText) buttonText->setVisibility(View::GONE);
    } else {
        if (buttonText) {
            buttonText->setVisibility(View::VISIBLE);
            buttonText->setText(menuItem->getTitle());
            // The layout's textColor ?attr/floatingToolbarForegroundColor may not resolve in the
            // popup's theme; force a visible color (toolbar background is dark). TODO(3c): theme tint.
            buttonText->setTextColor(0xFFFFFFFF);
        }
    }
    ImageView* buttonIcon = (ImageView*) menuItemButton->findViewById(R::id::floating_toolbar_menu_item_image);
    if (menuItem->getIcon() == nullptr || !showIcon) {
        if (buttonIcon) buttonIcon->setVisibility(View::GONE);
        if (buttonText) buttonText->setPaddingRelative(0, 0, 0, 0);
    } else {
        // Clone the icon so ImageView (which owns+deletes the drawable it shows, unlike AOSP)
        // does not free MenuItemImpl's owned mIconDrawable. Cloning needs a ConstantState; if the
        // icon has none, fall back to the borrowed pointer (only icon-bearing textAssist items
        // reach here, which CDROID's toolbar currently does not produce). TODO(3c).
        Drawable* icon = menuItem->getIcon();
        if (icon) {
            std::shared_ptr<Drawable::ConstantState> cs = icon->getConstantState();
            if (cs) {
                Drawable* cloned = cs->newDrawable();
                if (cloned) icon = cloned;
            }
        }
        if (buttonIcon) {
            buttonIcon->setVisibility(View::VISIBLE);
            buttonIcon->setImageDrawable(icon);
        }
        if (buttonText) buttonText->setPaddingRelative(iconTextSpacing, 0, 0, 0);
    }
    const std::string contentDescription = menuItem->getContentDescription();
    if (contentDescription.empty()) {
        menuItemButton->setContentDescription(menuItem->getTitle());
    } else {
        menuItemButton->setContentDescription(contentDescription);
    }
}

Context* LocalFloatingToolbarPopup::applyDefaultTheme(Context* originalContext) {
    // TODO(3c): wrap in Theme_DeviceDefault/_Light based on ?attr/isLightTheme. CDROID has no
    //           DeviceDefault theme; return as-is for 3b.
    return originalContext;
}

void LocalFloatingToolbarPopup::runShowAnimation() {
    // TODO(3b-anim): mShowAnimation.start() (ObjectAnimator ALPHA 0->1, 150ms).
}

void LocalFloatingToolbarPopup::runDismissAnimation() {
    // TODO(3b-anim).
}

void LocalFloatingToolbarPopup::runHideAnimation() {
    // TODO(3b-anim).
}

void LocalFloatingToolbarPopup::cancelDismissAndHideAnimations() {
    // TODO(3b-anim): mDismissAnimation.cancel(); mHideAnimation.cancel().
}

void LocalFloatingToolbarPopup::cancelOverflowAnimations() {
    mContentContainer->clearAnimation();
    // TODO(3b-anim): mMainPanel->animate().cancel(); mOverflowPanel->animate().cancel();
    //                mToArrow->stop(); mToOverflow->stop().
}

// =====================================================================================
//  OverflowPanel  (AOSP private static final class OverflowPanel extends ListView)
// =====================================================================================
LocalFloatingToolbarPopup::OverflowPanel::OverflowPanel(LocalFloatingToolbarPopup* popup)
    : ListView(popup->mContext, AttributeSet(popup->mContext, "cdroid"))
    , mPopup(popup) {
    // AOSP: setScrollBarDefaultDelayBeforeFade(ViewConfiguration.getScrollDefaultDelay() * 3);
    //       setScrollIndicators(SCROLL_INDICATOR_TOP | SCROLL_INDICATOR_BOTTOM).
    setScrollBarDefaultDelayBeforeFade(0);   // TODO(3c): ViewConfiguration scroll default delay.
    setScrollIndicators(View::SCROLL_INDICATOR_TOP | View::SCROLL_INDICATOR_BOTTOM);
}

void LocalFloatingToolbarPopup::OverflowPanel::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // Update heightMeasureSpec so the list is not clipped as its coordinates are offset
    // during the open/close animation. Clamped to >= 0 (AOSP assumes the size is set first).
    int height = std::max(0, mPopup->mOverflowPanelSize.height() - mPopup->mOverflowButtonSize.height());
    heightMeasureSpec = View::MeasureSpec::makeMeasureSpec(height, View::MeasureSpec::EXACTLY);
    ListView::onMeasure(widthMeasureSpec, heightMeasureSpec);
}

// =====================================================================================
//  OverflowItemAdapter  (AOSP anonymous ArrayAdapter<MenuItem>)
// =====================================================================================
int LocalFloatingToolbarPopup::OverflowItemAdapter::getCount() const {
    return (int) mItems.size();
}

void* LocalFloatingToolbarPopup::OverflowItemAdapter::getItem(int position) const {
    if (position < 0 || position >= (int) mItems.size()) return nullptr;
    return mItems[position];
}

long LocalFloatingToolbarPopup::OverflowItemAdapter::getItemId(int position) const {
    return position;
}

View* LocalFloatingToolbarPopup::OverflowItemAdapter::getView(
        int position, View* convertView, ViewGroup* /*parent*/) {
    MenuItem* menuItem = static_cast<MenuItem*>(getItem(position));
    return mPopup->mOverflowPanelViewHelper->getView(
            menuItem, mPopup->mOverflowPanelSize.width(), convertView);
}

void LocalFloatingToolbarPopup::OverflowItemAdapter::add(MenuItem* menuItem) {
    mItems.push_back(menuItem);
    notifyDataSetChanged();
}

void LocalFloatingToolbarPopup::OverflowItemAdapter::clear() {
    mItems.clear();
    notifyDataSetInvalidated();
}

// =====================================================================================
//  OverflowPanelViewHelper  (AOSP private static final class OverflowPanelViewHelper)
// =====================================================================================
LocalFloatingToolbarPopup::OverflowPanelViewHelper::OverflowPanelViewHelper(Context* context, int iconTextSpacing)
    : mContext(context)
    , mIconTextSpacing(iconTextSpacing)
    , mSidePadding(context->getDimensionPixelSize("cdroid:dimen/floating_toolbar_overflow_side_padding"))
    , mCalculator(nullptr) {
    mCalculator = createMenuButton(nullptr);
}

LocalFloatingToolbarPopup::OverflowPanelViewHelper::~OverflowPanelViewHelper() {
    delete mCalculator;
}

View* LocalFloatingToolbarPopup::OverflowPanelViewHelper::getView(MenuItem* menuItem, int minimumWidth, View* convertView) {
    if (convertView != nullptr) {
        LocalFloatingToolbarPopup::updateMenuItemButton(
                convertView, menuItem, mIconTextSpacing, shouldShowIcon(menuItem));
    } else {
        convertView = createMenuButton(menuItem);
    }
    convertView->setMinimumWidth(minimumWidth);
    return convertView;
}

int LocalFloatingToolbarPopup::OverflowPanelViewHelper::calculateWidth(MenuItem* menuItem) {
    LocalFloatingToolbarPopup::updateMenuItemButton(
            mCalculator, menuItem, mIconTextSpacing, shouldShowIcon(menuItem));
    mCalculator->measure(View::MeasureSpec::UNSPECIFIED, View::MeasureSpec::UNSPECIFIED);
    return mCalculator->getMeasuredWidth();
}

View* LocalFloatingToolbarPopup::OverflowPanelViewHelper::createMenuButton(MenuItem* menuItem) {
    View* button = LocalFloatingToolbarPopup::createMenuItemButton(
            mContext, menuItem, mIconTextSpacing, shouldShowIcon(menuItem));
    button->setPadding(mSidePadding, 0, mSidePadding, 0);
    return button;
}

bool LocalFloatingToolbarPopup::OverflowPanelViewHelper::shouldShowIcon(MenuItem* menuItem) const {
    if (menuItem != nullptr) {
        return menuItem->getGroupId() == R::id::textAssist;
    }
    return false;
}

}//namespace cdroid
