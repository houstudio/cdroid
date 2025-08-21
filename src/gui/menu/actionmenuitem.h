#ifndef __ACTION_MENUITEM_H__
#define __ACTION_MENUITEM_H__
#include <view/menuitem.h>
namespace cdroid{
class ActionMenuItem:public MenuItem {
private:
    static constexpr int NO_ICON = 0;
    static constexpr int CHECKABLE      = 0x00000001;
    static constexpr int CHECKED        = 0x00000002;
    static constexpr int EXCLUSIVE      = 0x00000004;
    static constexpr int HIDDEN         = 0x00000008;
    static constexpr int ENABLED        = 0x00000010;
private:
    int mId;
    int mGroup;
    int mCategoryOrder;
    int mOrdering;

    std::string mTitle;
    std::string mTitleCondensed;
    Intent* mIntent;
    int mShortcutNumericChar;
    int mShortcutNumericModifiers = KeyEvent::META_CTRL_ON;
    int mShortcutAlphabeticChar;
    int mShortcutAlphabeticModifiers = KeyEvent::META_CTRL_ON;
    int mIconResId = NO_ICON;
    int mFlags = ENABLED;
    int mIconTintMode;
    Drawable* mIconDrawable;
    const ColorStateList* mIconTintList = nullptr;
    bool mHasIconTint = false;
    bool mHasIconTintMode = false;

    Context* mContext;
    OnMenuItemClickListener mClickListener;

    std::string mContentDescription;
    std::string mTooltipText;
private:
    void applyIconTint() {
        if (mIconDrawable != nullptr && (mHasIconTint || mHasIconTintMode)) {
            mIconDrawable = mIconDrawable->mutate();

            if (mHasIconTint) {
                mIconDrawable->setTintList(mIconTintList);
            }

            if (mHasIconTintMode) {
                mIconDrawable->setTintMode(mIconTintMode);
            }
        }
    }

public:
    ActionMenuItem(Context* context, int group, int id, int categoryOrder, int ordering,const std::string& title) {
        mContext = context;
        mId = id;
        mGroup = group;
        mCategoryOrder = categoryOrder;
        mOrdering = ordering;
        mTitle = title;
    }

    int getAlphabeticShortcut() override{
        return mShortcutAlphabeticChar;
    }

    int getAlphabeticModifiers() override{
        return mShortcutAlphabeticModifiers;
    }

    int getGroupId()const override{
        return mGroup;
    }

    Drawable* getIcon() override{
        return mIconDrawable;
    }

    Intent* getIntent() override{
        return mIntent;
    }

    int getItemId()const override{
        return mId;
    }

    ContextMenuInfo* getMenuInfo() override{
        return nullptr;
    }

    int getNumericShortcut() const override{
        return mShortcutNumericChar;
    }

    int getNumericModifiers() const override{
        return mShortcutNumericModifiers;
    }

    int getOrder()const override{
        return mOrdering;
    }

    SubMenu* getSubMenu() override{
        return nullptr;
    }

    std::string getTitle() override{
        return mTitle;
    }

    std::string getTitleCondensed() override{
        return !mTitleCondensed.empty() ? mTitleCondensed : mTitle;
    }

    bool hasSubMenu() override{
        return false;
    }

    bool isCheckable() const override{
        return (mFlags & CHECKABLE) != 0;
    }

    bool isChecked() const override{
        return (mFlags & CHECKED) != 0;
    }

    bool isEnabled() const override{
        return (mFlags & ENABLED) != 0;
    }

    bool isVisible() override{
        return (mFlags & HIDDEN) == 0;
    }

    MenuItem& setAlphabeticShortcut(int alphaChar) override{
        mShortcutAlphabeticChar = std::tolower(alphaChar);
        return *this;
    }

    MenuItem& setAlphabeticShortcut(int alphachar, int alphaModifiers) override{
        mShortcutAlphabeticChar = std::tolower(alphachar);
        mShortcutAlphabeticModifiers = KeyEvent::normalizeMetaState(alphaModifiers);
        return *this;
    }

    MenuItem& setCheckable(bool checkable) override{
        mFlags = (mFlags & ~CHECKABLE) | (checkable ? CHECKABLE : 0);
        return *this;
    }

    ActionMenuItem& setExclusiveCheckable(bool exclusive) override{
        mFlags = (mFlags & ~EXCLUSIVE) | (exclusive ? EXCLUSIVE : 0);
        return *this;
    }

    MenuItem& setChecked(bool checked) {
        mFlags = (mFlags & ~CHECKED) | (checked ? CHECKED : 0);
        return *this;
    }

    MenuItem& setEnabled(bool enabled) override{
        mFlags = (mFlags & ~ENABLED) | (enabled ? ENABLED : 0);
        return *this;
    }

    MenuItem& setIcon(Drawable* icon) override{
        mIconDrawable = icon;
        mIconResId = NO_ICON;
        applyIconTint();
        return *this;
    }

    MenuItem& setIcon(const std::string& iconRes) override{
        mIconResId = iconRes;
        mIconDrawable = mContext->getDrawable(iconRes);
        applyIconTint();
        return *this;
    }

    MenuItem& setIconTintList(const ColorStateList* iconTintList) override{
        mIconTintList = iconTintList;
        mHasIconTint = true;
        applyIconTint();
        return *this;
    }

    const ColorStateList* getIconTintList() override{
        return mIconTintList;
    }

    MenuItem& setIconTintMode(int iconTintMode) override{
        mIconTintMode = iconTintMode;
        mHasIconTintMode = true;
        applyIconTint();
        return *this;
    }

    int getIconTintMode() override{
        return mIconTintMode;
    }

    MenuItem& setIntent(Intent* intent) override{
        mIntent = intent;
        return *this;
    }

    MenuItem& setNumericShortcut(int numericChar) override{
        mShortcutNumericChar = numericChar;
        return *this;
    }

    MenuItem& setNumericShortcut(int numericChar, int numericModifiers) override{
        mShortcutNumericChar = numericChar;
        mShortcutNumericModifiers = KeyEvent::normalizeMetaState(numericModifiers);
        return *this;
    }

    MenuItem& setOnMenuItemClickListener(OnMenuItemClickListener menuItemClickListener) override{
        mClickListener = menuItemClickListener;
        return *this;
    }

    MenuItem& setShortcut(int numericChar, int alphaChar) override{
        mShortcutNumericChar = numericChar;
        mShortcutAlphabeticChar = std::tolower(alphaChar);
        return *this;
    }

    MenuItem& setShortcut(int numericChar, int alphaChar, int numericModifiers, int alphaModifiers) override{
        mShortcutNumericChar = numericChar;
        mShortcutNumericModifiers = KeyEvent::normalizeMetaState(numericModifiers);
        mShortcutAlphabeticChar = std::tolower(alphaChar);
        mShortcutAlphabeticModifiers = KeyEvent::normalizeMetaState(alphaModifiers);
        return *this;
    }

    MenuItem& setTitle(const std::string& title) override{
        mTitle = title;
        return *this;
    }

    MenuItem& setTitle(int title) override{
        //mTitle = mContext.getResources().getString(title);
        return *this;
    }

    MenuItem& setTitleCondensed(const std::string& title) override{
        mTitleCondensed = title;
        return *this;
    }

    MenuItem& setVisible(bool visible) override{
        mFlags = (mFlags & HIDDEN) | (visible ? 0 : HIDDEN);
        return *this;
    }

    bool invoke() override{
        if (mClickListener != nullptr && mClickListener(*this)) {
            return true;
        }
        if (mIntent != nullptr) {
            //mContext->startActivity(mIntent);
            return true;
        }
        return false;
    }

    void setShowAsAction(int show) override{
        // Do nothing. ActionMenuItems always show as action buttons.
    }

    MenuItem& setActionView(View* actionView) override{
        throw std::logic_error("UnsupportedOperationException");
    }

    View* getActionView() override{
        return nullptr;
    }

    MenuItem& setActionView(int resId) override{
        throw std::logic_error("UnsupportedOperationException");
    }

    ActionProvider* getActionProvider() override{
        return nullptr;
    }

    MenuItem& setActionProvider(ActionProvider* actionProvider) override{
        throw std::logic_error("UnsupportedOperationException");
    }

    MenuItem& setShowAsActionFlags(int actionEnum) override{
        setShowAsAction(actionEnum);
        return *this;
    }

    bool expandActionView() override{
        return false;
    }

    bool collapseActionView() override{
        return false;
    }

    bool isActionViewExpanded() override{
        return false;
    }

    MenuItem& setOnActionExpandListener(OnActionExpandListener listener) override{
        // No need to save the listener; ActionMenuItem does not support collapsing items.
        return *this;
    }

    MenuItem& setContentDescription(const std::string& contentDescription) override{
        mContentDescription = contentDescription;
        return *this;
    }

    std::string getContentDescription() override{
        return mContentDescription;
    }

    MenuItem& setTooltipText(const std::string& tooltipText) override{
        mTooltipText = tooltipText;
        return *this;
    }

    std::string getTooltipText() override{
        return mTooltipText;
    }
};
}/*endof namespace*/
#endif/*__ACTION_MENUITEM_H__*/

