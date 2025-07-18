#include <view/menuitem.h>
#include <view/view.h>
namespace cdroid{

MenuItem& MenuItem::setIconTintList(const ColorStateList* tint){
    return *this;
}

const ColorStateList* MenuItem::getIconTintList() {
    return nullptr;
}

MenuItem& MenuItem::setIconTintMode(int tintMode) {
    return *this;
}

int MenuItem::getIconTintMode() {
    return 0;
}

MenuItem& MenuItem::setIntent(Intent* intent){
    return *this;
}

Intent* MenuItem::getIntent(){
    return nullptr;
}

MenuItem& MenuItem::setShortcut(int numericChar, int alphaChar){
    return *this;
}

MenuItem& MenuItem::setShortcut(int numericChar, int alphaChar, int numericModifiers, int alphaModifiers) {
    if ((alphaModifiers & Menu::SUPPORTED_MODIFIERS_MASK) == KeyEvent::META_CTRL_ON
            && (numericModifiers & Menu::SUPPORTED_MODIFIERS_MASK) == KeyEvent::META_CTRL_ON) {
        return setShortcut(numericChar, alphaChar);
    } else {
        return *this;
    }
}

MenuItem& MenuItem::setNumericShortcut(int numericChar){
    return *this;
}

MenuItem& MenuItem::setNumericShortcut(int numericChar, int numericModifiers) {
    if ((numericModifiers & Menu::SUPPORTED_MODIFIERS_MASK) == KeyEvent::META_CTRL_ON) {
        return setNumericShortcut(numericChar);
    } else {
        return *this;
    }
}

int MenuItem::getNumericShortcut(){
    return 0;
}

int MenuItem::getNumericModifiers() {
    return KeyEvent::META_CTRL_ON;
}

MenuItem& MenuItem::setAlphabeticShortcut(int alphaChar){
    return *this;
}

MenuItem& MenuItem::setAlphabeticShortcut(int alphaChar, int alphaModifiers) {
    if ((alphaModifiers & Menu::SUPPORTED_MODIFIERS_MASK) == KeyEvent::META_CTRL_ON) {
        return setAlphabeticShortcut(alphaChar);
    } else {
        return *this;
    }
}

int MenuItem::getAlphabeticShortcut(){
    return 0;
}

int MenuItem::getAlphabeticModifiers() {
    return KeyEvent::META_CTRL_ON;
}
#if 0
MenuItem& MenuItem::setCheckable(bool checkable);
bool isCheckable();
MenuItem& setChecked(bool checked);
bool isChecked();

MenuItem setVisible(bool visible);
bool isVisible();

MenuItem& setEnabled(bool enabled);
bool isEnabled();

bool hasSubMenu();
SubMenu* getSubMenu();

MenuItem& setOnMenuItemClickListener(const OnMenuItemClickListener& menuItemClickListener);
ContextMenuInfo* getMenuInfo();

void setShowAsAction(int actionEnum);

MenuItem& setShowAsActionFlags(int actionEnum);
MenuItem& setActionView(View* view);
MenuItem& setActionView(const std::string& resId);
View* getActionView();

MenuItem& setActionProvider(ActionProvider* actionProvider);
ActionProvider* getActionProvider();

bool expandActionView();
bool collapseActionView();
bool isActionViewExpanded();

MenuItem& setOnActionExpandListener(OnActionExpandListener listener);
#endif

MenuItem& MenuItem::setContentDescription(const std::string& contentDescription) {
    return *this;
}

std::string MenuItem::getContentDescription() {
    return std::string();
}

MenuItem& MenuItem::setTooltipText(const std::string& tooltipText) {
    return *this;
}

std::string MenuItem::getTooltipText() {
    return std::string();
}

bool MenuItem::requiresActionButton() {
    return false;
}

bool MenuItem::requiresOverflow() {
    return true;
}
}/*endof namespace*/

