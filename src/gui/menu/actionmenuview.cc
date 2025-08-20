#include <core/bitset.h>
#include <menu/actionmenuview.h>
#include <menu/actionmenuitemview.h>
#include <menu/actionmenupresenter.h>
namespace cdroid{

ActionMenuView::ActionMenuView(Context* context,const AttributeSet& attrs)
  :LinearLayout(context, attrs){
    setBaselineAligned(false);
    const float density = context->getDisplayMetrics().density;
    mMinCellSize = int(MIN_CELL_SIZE * density);
    mGeneratedItemPadding = int(GENERATED_ITEM_PADDING * density);
    mPopupContext = context;
    mPopupTheme = 0;
}

void ActionMenuView::setPopupTheme(int resId) {
    if (mPopupTheme != resId) {
        mPopupTheme = resId;
        if (resId == 0) {
            mPopupContext = mContext;
        } else {
            mPopupContext = mContext;//new ContextThemeWrapper(mContext, resId);
        }
    }
}

int ActionMenuView::getPopupTheme() {
    return mPopupTheme;
}

void ActionMenuView::setPresenter(ActionMenuPresenter* presenter) {
    mPresenter = presenter;
    mPresenter->setMenuView(this);
}

/*void ActionMenuView::onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);

    if (mPresenter != null) {
        mPresenter.updateMenuView(false);

        if (mPresenter.isOverflowMenuShowing()) {
            mPresenter.hideOverflowMenu();
            mPresenter.showOverflowMenu();
        }
    }
}*/

void ActionMenuView::setOnMenuItemClickListener(const OnMenuItemClickListener& listener) {
    mOnMenuItemClickListener = listener;
}

void ActionMenuView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    // If we've been given an exact size to match, apply special formatting during layout.
    const bool wasFormatted = mFormatItems;
    mFormatItems = MeasureSpec::getMode(widthMeasureSpec) == MeasureSpec::EXACTLY;

    if (wasFormatted != mFormatItems) {
        mFormatItemsWidth = 0; // Reset this when switching modes
    }

    // Special formatting can change whether items can fit as action buttons.
    // Kick the menu and update presenters when this changes.
    const int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    if (mFormatItems && mMenu != nullptr && widthSize != mFormatItemsWidth) {
        mFormatItemsWidth = widthSize;
        mMenu->onItemsChanged(true);
    }

    const int childCount = getChildCount();
    if (mFormatItems && childCount > 0) {
        onMeasureExactFormat(widthMeasureSpec, heightMeasureSpec);
    } else {
        // Previous measurement at exact format may have set margins - reset them.
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            lp->leftMargin = lp->rightMargin = 0;
        }
        LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    }
}

void ActionMenuView::onMeasureExactFormat(int widthMeasureSpec, int heightMeasureSpec) {
    // We already know the width mode is EXACTLY if we're here.
    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    int heightSize = MeasureSpec::getSize(heightMeasureSpec);

    const int widthPadding = getPaddingLeft() + getPaddingRight();
    const int heightPadding = getPaddingTop() + getPaddingBottom();

    const int itemHeightSpec = getChildMeasureSpec(heightMeasureSpec, heightPadding,
            ViewGroup::LayoutParams::WRAP_CONTENT);

    widthSize -= widthPadding;

    // Divide the view into cells.
    const int cellCount = widthSize / mMinCellSize;
    const int cellSizeRemaining = widthSize % mMinCellSize;

    if (cellCount == 0) {
        // Give up, nothing fits.
        setMeasuredDimension(widthSize, 0);
        return;
    }

    const int cellSize = mMinCellSize + cellSizeRemaining / cellCount;

    int cellsRemaining = cellCount;
    int maxChildHeight = 0;
    int maxCellsUsed = 0;
    int expandableItemCount = 0;
    int visibleItemCount = 0;
    bool hasOverflow = false;

    // This is used as a bitfield to locate the smallest items present. Assumes childCount < 64.
    int64_t smallestItemsAt = 0;

    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        if (child->getVisibility() == GONE) continue;

        const bool isGeneratedItem = dynamic_cast<ActionMenuItemView*>(child);
        visibleItemCount++;

        if (isGeneratedItem) {
            // Reset padding for generated menu item views; it may change below
            // and views are recycled.
            child->setPadding(mGeneratedItemPadding, 0, mGeneratedItemPadding, 0);
        }

        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        lp->expanded = false;
        lp->extraPixels = 0;
        lp->cellsUsed = 0;
        lp->expandable = false;
        lp->leftMargin = 0;
        lp->rightMargin = 0;
        lp->preventEdgeOffset = isGeneratedItem && ((ActionMenuItemView*) child)->hasText();

        // Overflow always gets 1 cell. No more, no less.
        const int cellsAvailable = lp->isOverflowButton ? 1 : cellsRemaining;

        const int cellsUsed = measureChildForCells(child, cellSize, cellsAvailable,
                itemHeightSpec, heightPadding);

        maxCellsUsed = std::max(maxCellsUsed, cellsUsed);
        if (lp->expandable) expandableItemCount++;
        if (lp->isOverflowButton) hasOverflow = true;

        cellsRemaining -= cellsUsed;
        maxChildHeight = std::max(maxChildHeight, child->getMeasuredHeight());
        if (cellsUsed == 1) smallestItemsAt |= (1 << i);
    }

    // When we have overflow and a single expanded (text) item, we want to try centering it
    // visually in the available space even though overflow consumes some of it.
    const bool centerSingleExpandedItem = hasOverflow && visibleItemCount == 2;

    // Divide space for remaining cells if we have items that can expand.
    // Try distributing whole leftover cells to smaller items first.

    bool needsExpansion = false;
    while (expandableItemCount > 0 && cellsRemaining > 0) {
        int minCells = INT_MAX;
        long minCellsAt = 0; // Bit locations are indices of relevant child views
        int minCellsItemCount = 0;
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

            // Don't try to expand items that shouldn't.
            if (!lp->expandable) continue;

            // Mark indices of children that can receive an extra cell.
            if (lp->cellsUsed < minCells) {
                minCells = lp->cellsUsed;
                minCellsAt = 1 << i;
                minCellsItemCount = 1;
            } else if (lp->cellsUsed == minCells) {
                minCellsAt |= 1 << i;
                minCellsItemCount++;
            }
        }

        // Items that get expanded will always be in the set of smallest items when we're done.
        smallestItemsAt |= minCellsAt;

        if (minCellsItemCount > cellsRemaining) break; // Couldn't expand anything evenly. Stop.

        // We have enough cells, all minimum size items will be incremented.
        minCells++;

        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            if ((minCellsAt & (1 << i)) == 0) {
                // If this item is already at our small item count, mark it for later.
                if (lp->cellsUsed == minCells) smallestItemsAt |= 1 << i;
                continue;
            }

            if (centerSingleExpandedItem && lp->preventEdgeOffset && cellsRemaining == 1) {
                // Add padding to this item such that it centers.
                child->setPadding(mGeneratedItemPadding + cellSize, 0, mGeneratedItemPadding, 0);
            }
            lp->cellsUsed++;
            lp->expanded = true;
            cellsRemaining--;
        }

        needsExpansion = true;
    }

    // Divide any space left that wouldn't divide along cell boundaries
    // evenly among the smallest items

    const bool singleItem = !hasOverflow && visibleItemCount == 1;
    if (cellsRemaining > 0 && smallestItemsAt != 0 &&
            (cellsRemaining < visibleItemCount - 1 || singleItem || maxCellsUsed > 1)) {
        float expandCount = BitSet64::count(smallestItemsAt);

        if (!singleItem) {
            // The items at the far edges may only expand by half in order to pin to either side.
            if ((smallestItemsAt & 1) != 0) {
                LayoutParams* lp = (LayoutParams*) getChildAt(0)->getLayoutParams();
                if (!lp->preventEdgeOffset) expandCount -= 0.5f;
            }
            if ((smallestItemsAt & (1 << (childCount - 1))) != 0) {
                LayoutParams* lp = ((LayoutParams*) getChildAt(childCount - 1)->getLayoutParams());
                if (!lp->preventEdgeOffset) expandCount -= 0.5f;
            }
        }

        const int extraPixels = expandCount > 0 ? (int) (cellsRemaining * cellSize / expandCount) : 0;

        for (int i = 0; i < childCount; i++) {
            if ((smallestItemsAt & (1 << i)) == 0) continue;

            View* child = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            if (dynamic_cast<ActionMenuItemView*>(child)) {
                // If this is one of our views, expand and measure at the larger size.
                lp->extraPixels = extraPixels;
                lp->expanded = true;
                if (i == 0 && !lp->preventEdgeOffset) {
                    // First item gets part of its new padding pushed out of sight.
                    // The last item will get this implicitly from layout.
                    lp->leftMargin = -extraPixels / 2;
                }
                needsExpansion = true;
            } else if (lp->isOverflowButton) {
                lp->extraPixels = extraPixels;
                lp->expanded = true;
                lp->rightMargin = -extraPixels / 2;
                needsExpansion = true;
            } else {
                // If we don't know what it is, give it some margins instead
                // and let it center within its space. We still want to pin
                // against the edges.
                if (i != 0) {
                    lp->leftMargin = extraPixels / 2;
                }
                if (i != childCount - 1) {
                    lp->rightMargin = extraPixels / 2;
                }
            }
        }

        cellsRemaining = 0;
    }

    // Remeasure any items that have had extra space allocated to them.
    if (needsExpansion) {
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

            if (!lp->expanded) continue;

            const int width = lp->cellsUsed * cellSize + lp->extraPixels;
            child->measure(MeasureSpec::makeMeasureSpec(width, MeasureSpec::EXACTLY),
                    itemHeightSpec);
        }
    }

    if (heightMode != MeasureSpec::EXACTLY) {
        heightSize = maxChildHeight;
    }

    setMeasuredDimension(widthSize, heightSize);
}

int ActionMenuView::measureChildForCells(View* child, int cellSize, int cellsRemaining,
        int parentHeightMeasureSpec, int parentHeightPadding) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

    const int childHeightSize = MeasureSpec::getSize(parentHeightMeasureSpec) - parentHeightPadding;
    const int childHeightMode = MeasureSpec::getMode(parentHeightMeasureSpec);
    const int childHeightSpec = MeasureSpec::makeMeasureSpec(childHeightSize, childHeightMode);

    const ActionMenuItemView* itemView = dynamic_cast<ActionMenuItemView*>(child) ?
            (ActionMenuItemView*) child : nullptr;
    const bool hasText = itemView != nullptr && itemView->hasText();

    int cellsUsed = 0;
    if (cellsRemaining > 0 && (!hasText || cellsRemaining >= 2)) {
        const int childWidthSpec = MeasureSpec::makeMeasureSpec(
                cellSize * cellsRemaining, MeasureSpec::AT_MOST);
        child->measure(childWidthSpec, childHeightSpec);

        const int measuredWidth = child->getMeasuredWidth();
        cellsUsed = measuredWidth / cellSize;
        if (measuredWidth % cellSize != 0) cellsUsed++;
        if (hasText && cellsUsed < 2) cellsUsed = 2;
    }

    const bool expandable = !lp->isOverflowButton && hasText;
    lp->expandable = expandable;

    lp->cellsUsed = cellsUsed;
    const int targetWidth = cellsUsed * cellSize;
    child->measure(MeasureSpec::makeMeasureSpec(targetWidth, MeasureSpec::EXACTLY),
            childHeightSpec);
    return cellsUsed;
}

void ActionMenuView::onLayout(bool changed, int left, int top, int layoutWidth, int layoutHeight) {
    if (!mFormatItems) {
        LinearLayout::onLayout(changed, left, top, layoutWidth, layoutHeight);
        return;
    }

    const int childCount = getChildCount();
    const int midVertical = layoutHeight / 2;
    const int dividerWidth = getDividerWidth();
    int overflowWidth = 0;
    int nonOverflowWidth = 0;
    int nonOverflowCount = 0;
    int widthRemaining = layoutWidth - getPaddingRight() - getPaddingLeft();
    bool hasOverflow = false;
    const bool bLayoutRtl = isLayoutRtl();
    for (int i = 0; i < childCount; i++) {
        View* v = getChildAt(i);
        if (v->getVisibility() == GONE) {
            continue;
        }

        LayoutParams* p = (LayoutParams*) v->getLayoutParams();
        if (p->isOverflowButton) {
            overflowWidth = v->getMeasuredWidth();
            if (hasDividerBeforeChildAt(i)) {
                overflowWidth += dividerWidth;
            }

            int height = v->getMeasuredHeight();
            int r;
            int l;
            if (bLayoutRtl) {
                l = getPaddingLeft() + p->leftMargin;
                r = l + overflowWidth;
            } else {
                r = getWidth() - getPaddingRight() - p->rightMargin;
                l = r - overflowWidth;
            }
            int t = midVertical - (height / 2);
            int b = t + height;
            v->layout(l, t, r, b);

            widthRemaining -= overflowWidth;
            hasOverflow = true;
        } else {
            const int size = v->getMeasuredWidth() + p->leftMargin + p->rightMargin;
            nonOverflowWidth += size;
            widthRemaining -= size;
            if (hasDividerBeforeChildAt(i)) {
                nonOverflowWidth += dividerWidth;
            }
            nonOverflowCount++;
        }
    }

    if (childCount == 1 && !hasOverflow) {
        // Center a single child
        View* v = getChildAt(0);
        const int width = v->getMeasuredWidth();
        const int height = v->getMeasuredHeight();
        const int midHorizontal = layoutWidth / 2;
        const int l = midHorizontal - width / 2;
        const int t = midVertical - height / 2;
        v->layout(l, t, width, height);
        return;
    }

    const int spacerCount = nonOverflowCount - (hasOverflow ? 0 : 1);
    const int spacerSize = std::max(0, spacerCount > 0 ? widthRemaining / spacerCount : 0);

    if (bLayoutRtl) {
        int startRight = getWidth() - getPaddingRight();
        for (int i = 0; i < childCount; i++) {
            View* v = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) v->getLayoutParams();
            if (v->getVisibility() == GONE || lp->isOverflowButton) {
                continue;
            }

            startRight -= lp->rightMargin;
            int width = v->getMeasuredWidth();
            int height = v->getMeasuredHeight();
            int t = midVertical - height / 2;
            v->layout(startRight - width, t, width, height);
            startRight -= width + lp->leftMargin + spacerSize;
        }
    } else {
        int startLeft = getPaddingLeft();
        for (int i = 0; i < childCount; i++) {
            View* v = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) v->getLayoutParams();
            if (v->getVisibility() == GONE || lp->isOverflowButton) {
                continue;
            }

            startLeft += lp->leftMargin;
            int width = v->getMeasuredWidth();
            int height = v->getMeasuredHeight();
            int t = midVertical - height / 2;
            v->layout(startLeft, t, width, height);
            startLeft += width + lp->rightMargin + spacerSize;
        }
    }
}

void ActionMenuView::onDetachedFromWindow() {
    LinearLayout::onDetachedFromWindow();
    dismissPopupMenus();
}

void ActionMenuView::setOverflowIcon(Drawable* icon) {
    getMenu();
    mPresenter->setOverflowIcon(icon);
}

Drawable* ActionMenuView::getOverflowIcon() {
    getMenu();
    return mPresenter->getOverflowIcon();
}

bool ActionMenuView::isOverflowReserved() {
    return mReserveOverflow;
}

void ActionMenuView::setOverflowReserved(bool reserveOverflow) {
    mReserveOverflow = reserveOverflow;
}

ActionMenuView::LayoutParams* ActionMenuView::generateDefaultLayoutParams() const{
    LayoutParams* params = new LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
    params->gravity = Gravity::CENTER_VERTICAL;
    return params;
}

ActionMenuView::LayoutParams* ActionMenuView::generateLayoutParams(const AttributeSet& attrs) const{
    return new LayoutParams(getContext(), attrs);
}

ActionMenuView::LayoutParams* ActionMenuView::generateLayoutParams(const ViewGroup::LayoutParams* p) const{
    if (p != nullptr) {
        LayoutParams* result = dynamic_cast<const LayoutParams*>(p)
                ? new LayoutParams((const LayoutParams&)* p)
                : new LayoutParams(*p);
        if (result->gravity <= Gravity::NO_GRAVITY) {
            result->gravity = Gravity::CENTER_VERTICAL;
        }
        return result;
    }
    return generateDefaultLayoutParams();
}

bool ActionMenuView::checkLayoutParams(const ViewGroup::LayoutParams* p) const{
    return (p != nullptr) &&dynamic_cast<const LayoutParams*>(p);
}

ActionMenuView::LayoutParams* ActionMenuView::generateOverflowButtonLayoutParams() const{
    LayoutParams* result = generateDefaultLayoutParams();
    result->isOverflowButton = true;
    return result;
}

bool ActionMenuView::invokeItem(MenuItemImpl& item) {
    return mMenu->performItemAction((MenuItem*)&item, 0);
}

int ActionMenuView::getWindowAnimations() {
    return 0;
}

void ActionMenuView::initialize(MenuBuilder* menu) {
    mMenu = menu;
}

Menu* ActionMenuView::getMenu() {
    if (mMenu == nullptr) {
        Context* context = getContext();
        mMenu = new MenuBuilder(context);
        MenuBuilder::Callback menuCallback;
        menuCallback.onMenuItemSelected=[this](MenuBuilder& menu, MenuItem& item){
            return  (mOnMenuItemClickListener!=nullptr)&&mOnMenuItemClickListener(item);
        };
        menuCallback.onMenuModeChange=[this](MenuBuilder& menu){
             if(mMenuBuilderCallback.onMenuModeChange){
                 mMenuBuilderCallback.onMenuModeChange(menu);
             }
        };
        mMenu->setCallback(menuCallback);
        mPresenter = new ActionMenuPresenter(context);
        mPresenter->setReserveOverflow(true);
        if((mActionMenuPresenterCallback.onOpenSubMenu==nullptr)&&(mActionMenuPresenterCallback.onCloseMenu==nullptr)){
            mActionMenuPresenterCallback.onCloseMenu=[](MenuBuilder& menu,bool){};
            mActionMenuPresenterCallback.onOpenSubMenu=[](MenuBuilder&){return false;};
        }
        mPresenter->setCallback(mActionMenuPresenterCallback);
        mMenu->addMenuPresenter(mPresenter, mPopupContext);
        mPresenter->setMenuView(this);
    }

    return mMenu;
}

void ActionMenuView::setMenuCallbacks(const MenuPresenter::Callback& pcb, const MenuBuilder::Callback& mcb) {
    mActionMenuPresenterCallback = pcb;
    mMenuBuilderCallback = mcb;
}

MenuBuilder* ActionMenuView::peekMenu() {
    return mMenu;
}

bool ActionMenuView::showOverflowMenu() {
    return mPresenter != nullptr && mPresenter->showOverflowMenu();
}

bool ActionMenuView::hideOverflowMenu() {
    return mPresenter != nullptr && mPresenter->hideOverflowMenu();
}

bool ActionMenuView::isOverflowMenuShowing() {
    return mPresenter != nullptr && mPresenter->isOverflowMenuShowing();
}

bool ActionMenuView::isOverflowMenuShowPending() {
    return mPresenter != nullptr && mPresenter->isOverflowMenuShowPending();
}

void ActionMenuView::dismissPopupMenus() {
    if (mPresenter != nullptr) {
        mPresenter->dismissPopupMenus();
    }
}

bool ActionMenuView::hasDividerBeforeChildAt(int childIndex) {
    if (childIndex == 0) {
        return false;
    }
    View* childBefore = getChildAt(childIndex - 1);
    View* child = getChildAt(childIndex);
    bool result = false;
    if ((childIndex < getChildCount()) && dynamic_cast<ActionMenuChildView*>(childBefore)) {
        result |= ((ActionMenuChildView*) childBefore)->needsDividerAfter();
    }
    if ((childIndex > 0) && dynamic_cast<ActionMenuChildView*>(child)) {
        result |= ((ActionMenuChildView*) child)->needsDividerBefore();
    }
    return result;
}

bool ActionMenuView::dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event) {
    return false;
}

void ActionMenuView::setExpandedActionViewsExclusive(bool exclusive) {
    mPresenter->setExpandedActionViewsExclusive(exclusive);
}

////////////////////////////////////////////////////////////////////////////////////////////
#if 0
/**
 * Interface responsible for receiving menu item click events if the items themselves
 * do not have individual item click listeners.
 */
public interface OnMenuItemClickListener {
    /**
     * This method will be invoked when a menu item is clicked if the item itself did
     * not already handle the event.
     *
     * @param item {@link MenuItem} that was clicked
     * @return <code>true</code> if the event was handled, <code>false</code> otherwise.
     */
    public bool onMenuItemClick(MenuItem item);
}

private class MenuBuilderCallback implements MenuBuilder.Callback {
    @Override
    public bool onMenuItemSelected(MenuBuilder menu, MenuItem item) {
        return mOnMenuItemClickListener != null &&
                mOnMenuItemClickListener.onMenuItemClick(item);
    }

    @Override
    public void onMenuModeChange(MenuBuilder menu) {
        if (mMenuBuilderCallback != null) {
            mMenuBuilderCallback.onMenuModeChange(menu);
        }
    }
}

private class ActionMenuPresenterCallback implements ActionMenuPresenter.Callback {
    @Override
    public void onCloseMenu(MenuBuilder menu, bool allMenusAreClosing) {
    }

    @Override
    public bool onOpenSubMenu(MenuBuilder subMenu) {
        return false;
    }
}

public interface ActionMenuChildView {
    public bool needsDividerBefore();
    public bool needsDividerAfter();
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////

//public static class LayoutParams:public LinearLayout::LayoutParams {
ActionMenuView::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs):LinearLayout::LayoutParams(c, attrs){
}

ActionMenuView::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& other):LinearLayout::LayoutParams(other){
}

ActionMenuView::LayoutParams::LayoutParams(const LayoutParams& other)
    :LinearLayout::LayoutParams((LinearLayout::LayoutParams&) other){
    isOverflowButton = other.isOverflowButton;
}

ActionMenuView::LayoutParams::LayoutParams(int width, int height):LinearLayout::LayoutParams(width, height){
    isOverflowButton = false;
}

ActionMenuView::LayoutParams::LayoutParams(int width, int height, bool isOverflowButton):LinearLayout::LayoutParams(width, height){
    this->isOverflowButton = isOverflowButton;
}

}/*endof namespace*/
