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
#include <menu/menuitemimpl.h>
#include <menu/actionmenuitemview.h>
namespace cdroid{

DECLARE_WIDGET(ActionMenuItemView)
ActionMenuItemView::ActionMenuItemView(Context* context,const AttributeSet& attrs)
    :TextView(context, attrs){//, defStyleAttr, defStyleRes){
    mIcon = nullptr;
    mItemData = nullptr;
    mExpandedFormat = false;
    mForwardingListener = nullptr;
    mAllowTextWithIcon = shouldAllowTextWithIcon();
    mMinWidth = attrs.getDimensionPixelSize("minWidth", 0);

    const float density = context->getDisplayMetrics().density;
    mMaxIconSize = (int) (MAX_ICON_SIZE * density + 0.5f);
    setOnClickListener([this](View&v){onClick(v);});
    mSavedPaddingLeft = -1;
    //setSaveEnabled(false);
}

ActionMenuItemView::~ActionMenuItemView(){
    delete mForwardingListener;
}

/*void ActionMenuItemView::onConfigurationChanged(Configuration newConfig) {
    TextView::onConfigurationChanged(newConfig);

    mAllowTextWithIcon = shouldAllowTextWithIcon();
    updateTextButtonVisibility();
}*/

std::string ActionMenuItemView::getAccessibilityClassName() const{
    return "android.widget.Button";
}

bool ActionMenuItemView::shouldAllowTextWithIcon() {
#if 0
    final Configuration configuration = getContext().getResources().getConfiguration();
    const int width = configuration.screenWidthDp;
    const int height = configuration.screenHeightDp;
    return  width >= 480 || (width >= 640 && height >= 480)
            || configuration.orientation == Configuration.ORIENTATION_LANDSCAPE;
#else
    return false;
#endif
}

void ActionMenuItemView::setPadding(int l, int t, int r, int b) {
    mSavedPaddingLeft = l;
    TextView::setPadding(l, t, r, b);
}

MenuItemImpl* ActionMenuItemView::getItemData() {
    return mItemData;
}

void ActionMenuItemView::initialize(MenuItemImpl* itemData, int menuType) {
    mItemData = itemData;

    setIcon(itemData->getIcon());
    setTitle(itemData->getTitleForItemView(this)); // Title is only displayed if there is no icon
    setId(itemData->getItemId());

    setVisibility(itemData->isVisible() ? VISIBLE : GONE);
    setEnabled(itemData->isEnabled());

    if (itemData->hasSubMenu()) {
        if (mForwardingListener == nullptr) {
            mForwardingListener = new ActionMenuItemForwardingListener(this);
        }
    }
}

bool ActionMenuItemView::onTouchEvent(MotionEvent& e) {
    if (mItemData->hasSubMenu() && (mForwardingListener != nullptr)
            && mForwardingListener->onTouch(*this, e)) {
        return true;
    }
    return TextView::onTouchEvent(e);
}

void ActionMenuItemView::onClick(View& v) {
    if (mItemInvoker != nullptr) {
        mItemInvoker(*mItemData);//.invokeItem(mItemData);
    }
}

void ActionMenuItemView::setItemInvoker(const MenuBuilder::ItemInvoker& invoker) {
    mItemInvoker = invoker;
}

void ActionMenuItemView::setPopupCallback(const PopupCallback& popupCallback) {
    mPopupCallback = popupCallback;
}

bool ActionMenuItemView::prefersCondensedTitle()const{
    return true;
}

void ActionMenuItemView::setCheckable(bool checkable) {
    // TODO Support checkable action items
}

void ActionMenuItemView::setChecked(bool checked) {
    // TODO Support checkable action items
}

void ActionMenuItemView::setEnabled(bool enabled){
    TextView::setEnabled(enabled);
}

void ActionMenuItemView::setExpandedFormat(bool expandedFormat) {
    if (mExpandedFormat != expandedFormat) {
        mExpandedFormat = expandedFormat;
        if (mItemData != nullptr) {
            mItemData->actionFormatChanged();
        }
    }
}

void ActionMenuItemView::updateTextButtonVisibility() {
    bool visible = !TextUtils::isEmpty(mTitle);
    visible &= (mIcon == nullptr) || (mItemData->showsTextAsAction() && (mAllowTextWithIcon || mExpandedFormat));

    setText(visible ? mTitle : std::string());

    const std::string contentDescription = mItemData->getContentDescription();
    if (TextUtils::isEmpty(contentDescription)) {
        // Use the uncondensed title for content description, but only if the title is not
        // shown already.
        setContentDescription(visible ? std::string() : mItemData->getTitle());
    } else {
        setContentDescription(contentDescription);
    }

    const std::string tooltipText = mItemData->getTooltipText();
    if (TextUtils::isEmpty(tooltipText)) {
        // Use the uncondensed title for tooltip, but only if the title is not shown already.
        setTooltipText(visible ? std::string() : mItemData->getTitle());
    } else {
        setTooltipText(tooltipText);
    }
}

void ActionMenuItemView::setIcon(Drawable* icon) {
    mIcon = icon;
    if (icon != nullptr) {
        int width = icon->getIntrinsicWidth();
        int height = icon->getIntrinsicHeight();
        if (width > mMaxIconSize) {
            const float scale = (float) mMaxIconSize / width;
            width = mMaxIconSize;
            height *= scale;
        }
        if (height > mMaxIconSize) {
            const float scale = (float) mMaxIconSize / height;
            height = mMaxIconSize;
            width *= scale;
        }
        icon->setBounds(0, 0, width, height);
    }
    setCompoundDrawables(icon, nullptr, nullptr, nullptr);
    updateTextButtonVisibility();
}

bool ActionMenuItemView::hasText()const {
    return !TextUtils::isEmpty(getText());
}

void ActionMenuItemView::setShortcut(bool showShortcut, int shortcutKey) {
    // Action buttons don't show text for shortcut keys.
}

void ActionMenuItemView::setTitle(const std::string& title) {
    mTitle = title;
    updateTextButtonVisibility();
}

bool ActionMenuItemView::dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event) {
    onPopulateAccessibilityEvent(event);
    return true;
}

void ActionMenuItemView::onPopulateAccessibilityEventInternal(AccessibilityEvent& event) {
    TextView::onPopulateAccessibilityEventInternal(event);
    const std::string cdesc = getContentDescription();
    if (!TextUtils::isEmpty(cdesc)) {
        //event.getText().add(cdesc);
    }
}

bool ActionMenuItemView::dispatchHoverEvent(MotionEvent& event) {
    // Don't allow children to hover; we want this to be treated as a single component.
    return onHoverEvent(event);
}

bool ActionMenuItemView::showsIcon() {
    return true;
}

bool ActionMenuItemView::needsDividerBefore() {
    return hasText() && mItemData->getIcon() == nullptr;
}

bool ActionMenuItemView::needsDividerAfter() {
    return hasText();
}

void ActionMenuItemView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    const bool textVisible = hasText();
    if (textVisible && mSavedPaddingLeft >= 0) {
        TextView::setPadding(mSavedPaddingLeft, getPaddingTop(),
                getPaddingRight(), getPaddingBottom());
    }

    TextView::onMeasure(widthMeasureSpec, heightMeasureSpec);

    const int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    const int widthSize = MeasureSpec::getSize(widthMeasureSpec);
    const int oldMeasuredWidth = getMeasuredWidth();
    const int targetWidth = widthMode == MeasureSpec::AT_MOST ? std::min(widthSize, mMinWidth)
            : mMinWidth;

    if ((widthMode != MeasureSpec::EXACTLY) && (mMinWidth > 0) && (oldMeasuredWidth < targetWidth)) {
        // Remeasure at exactly the minimum width.
        TextView::onMeasure(MeasureSpec::makeMeasureSpec(targetWidth, MeasureSpec::EXACTLY), heightMeasureSpec);
    }

    if (!textVisible && (mIcon != nullptr)) {
        // TextView won't center compound drawables in both dimensions without
        // a little coercion. Pad in to center the icon after we've measured.
        const int w = getMeasuredWidth();
        const int dw = mIcon->getBounds().width;
        TextView::setPadding((w - dw) / 2, getPaddingTop(), getPaddingRight(), getPaddingBottom());
    }
}

void ActionMenuItemView::onRestoreInstanceState(Parcelable& state) {
    // This might get called with the state of ActionView since it shares the same ID with
    // ActionMenuItemView. Do not restore this state as ActionMenuItemView never saved it.
    //TextView::onRestoreInstanceState(nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ActionMenuItemView::ActionMenuItemForwardingListener::ActionMenuItemForwardingListener(View*v):ForwardingListener(v){
}

ShowableListMenu ActionMenuItemView::ActionMenuItemForwardingListener::getPopup(){
    if (((ActionMenuItemView*)mSrc)->mPopupCallback != nullptr) {
        return ((ActionMenuItemView*)mSrc)->mPopupCallback();//.getPopup();
    }
    return ShowableListMenu();
}

bool ActionMenuItemView::ActionMenuItemForwardingListener::onForwardingStarted(){
    // Call the invoker, then check if the expected popup is showing.
    ActionMenuItemView*iv = (ActionMenuItemView*)mSrc;
    if ((iv->mItemInvoker != nullptr) && iv->mItemInvoker(*iv->mItemData)) {
        ShowableListMenu popup = getPopup();
        return popup.isShowing && popup.isShowing();
    }
    return false;
}

}/*endof namespace*/

