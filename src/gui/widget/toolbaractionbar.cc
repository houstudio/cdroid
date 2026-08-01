#include <widget/toolbaractionbar.h>
#include <widget/toolbar.h>
#include <widget/cdwindow.h>
#include <widget/R.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <menu/menubuilder.h>
#include <menu/menupresenter.h>
#include <menu/actionmenuitem.h>
#include <view/view.h>
#include <stdexcept>
namespace cdroid{

ToolbarActionBar::ToolbarActionBar(Toolbar* toolbar, const std::string& title, Window* host)
  : mToolbar(toolbar), mHost(host), mTitle(title){
    // Seed remembered title/subtitle and current navigation icon (ToolbarWidgetWrapper ctor).
    mTitleSet = !mTitle.empty();
    mSubtitle = mToolbar->getSubtitle();
    mNavIcon = mToolbar->getNavigationIcon();
    mDisplayOpts = detectDisplayOptions();

    // Navigation icon click -> synthesize an R::id::home item -> host dispatch.
    mToolbar->setNavigationOnClickListener([this](View&){ fireHomePressed(); });
    // Action-menu item click -> host onOptionsItemSelected.
    mToolbar->setOnMenuItemClickListener(
        [this](MenuItem& item)->bool{ return mHost->onOptionsItemSelected(&item); });

    // Apply the default display options so the seeded title shows.
    applyDisplayOptions(mDisplayOpts);
}

ToolbarActionBar::~ToolbarActionBar() = default;

int ToolbarActionBar::detectDisplayOptions(){
    int opts = DISPLAY_SHOW_TITLE | DISPLAY_SHOW_HOME | DISPLAY_USE_LOGO;
    if(mToolbar->getNavigationIcon() != nullptr){
        opts |= DISPLAY_HOME_AS_UP;
        mDefaultNavigationIcon = mToolbar->getNavigationIcon();
    }
    return opts;
}

void ToolbarActionBar::setDisplayOptions(int options){
    setDisplayOptions(options, 0xffffffff);
}

void ToolbarActionBar::setDisplayOptions(int options, int mask){
    const int current = mDisplayOpts;
    applyDisplayOptions((options & mask) | (current & ~mask));
}

void ToolbarActionBar::applyDisplayOptions(int newOpts){
    const int oldOpts = mDisplayOpts;
    const int changed = oldOpts ^ newOpts;
    mDisplayOpts = newOpts;
    if(changed == 0) return;

    if((changed & DISPLAY_HOME_AS_UP) != 0){
        updateNavigationIcon();
    }
    if((changed & AFFECTS_LOGO_MASK) != 0){
        updateToolbarLogo();
    }
    if((changed & DISPLAY_SHOW_TITLE) != 0){
        if((newOpts & DISPLAY_SHOW_TITLE) != 0){
            mToolbar->setTitle(mTitle);
            mToolbar->setSubtitle(mSubtitle);
        }else{
            mToolbar->setTitle(std::string());
            mToolbar->setSubtitle(std::string());
        }
    }
    if((changed & DISPLAY_SHOW_CUSTOM) != 0 && mCustomView != nullptr){
        if((newOpts & DISPLAY_SHOW_CUSTOM) != 0) mToolbar->addView(mCustomView);
        else mToolbar->removeView(mCustomView);
    }
}

int ToolbarActionBar::getDisplayOptions() const{
    return mDisplayOpts;
}

void ToolbarActionBar::updateNavigationIcon(){
    if((mDisplayOpts & DISPLAY_HOME_AS_UP) != 0){
        mToolbar->setNavigationIcon(mNavIcon != nullptr ? mNavIcon : mDefaultNavigationIcon);
    }else{
        mToolbar->setNavigationIcon(nullptr);
    }
}

void ToolbarActionBar::updateToolbarLogo(){
    Drawable* logo = nullptr;
    if((mDisplayOpts & DISPLAY_SHOW_HOME) != 0){
        if((mDisplayOpts & DISPLAY_USE_LOGO) != 0) logo = mLogo != nullptr ? mLogo : mIcon;
        else logo = mIcon;
    }
    mToolbar->setLogo(logo);
}

void ToolbarActionBar::setTitle(const std::string& title){
    mTitleSet = true;
    mTitle = title;
    if((mDisplayOpts & DISPLAY_SHOW_TITLE) != 0) mToolbar->setTitle(mTitle);
}

void ToolbarActionBar::setSubtitle(const std::string& subtitle){
    mSubtitle = subtitle;
    if((mDisplayOpts & DISPLAY_SHOW_TITLE) != 0) mToolbar->setSubtitle(mSubtitle);
}

void ToolbarActionBar::setCustomView(View* view){
    // Visibility of the custom view is governed by DISPLAY_SHOW_CUSTOM, applied when that
    // bit toggles; here we just swap the remembered view (adding/removing as appropriate).
    if(mCustomView != nullptr && (mDisplayOpts & DISPLAY_SHOW_CUSTOM) != 0){
        mToolbar->removeView(mCustomView);
    }
    mCustomView = view;
    if(view != nullptr && (mDisplayOpts & DISPLAY_SHOW_CUSTOM) != 0){
        mToolbar->addView(view);
    }
}

void ToolbarActionBar::setIcon(Drawable* icon){ mIcon = icon; updateToolbarLogo(); }
void ToolbarActionBar::setLogo(Drawable* logo){ mLogo = logo; updateToolbarLogo(); }

void ToolbarActionBar::setHomeButtonEnabled(bool /*enabled*/){
    // If the nav button on a Toolbar is present, it's enabled. No-op.
}

void ToolbarActionBar::show(){
    mToolbar->setVisibility(View::VISIBLE);
}

void ToolbarActionBar::hide(){
    mToolbar->setVisibility(View::GONE);
}

bool ToolbarActionBar::isShowing() const{
    return mToolbar->getVisibility() == View::VISIBLE;
}

int  ToolbarActionBar::getHeight() const{
    return mToolbar->getHeight();
}

Context* ToolbarActionBar::getThemedContext(){
    return mToolbar->getContext();
}

void ToolbarActionBar::addOnMenuVisibilityListener(const OnMenuVisibilityListener& listener){
    mMenuVisibilityListeners.push_back(listener);
}

void ToolbarActionBar::removeOnMenuVisibilityListener(const OnMenuVisibilityListener& listener){
    // CDROID listeners are structs holding a std::function; compare by callable target
    // (works for function pointers / stateful targets; stateless lambdas share a null
    // target, so removal among several identical lambdas is best-effort — a known limit
    // of the std::function callback model).
    auto target = listener.onMenuVisibilityChanged.target<void(bool)>();
    for(auto it = mMenuVisibilityListeners.begin(); it != mMenuVisibilityListeners.end(); ++it){
        if(it->onMenuVisibilityChanged.target<void(bool)>() == target){
            mMenuVisibilityListeners.erase(it);
            return;
        }
    }
}

void ToolbarActionBar::dispatchMenuVisibilityChanged(bool isVisible){
    if(isVisible == mLastMenuVisibility) return;
    mLastMenuVisibility = isVisible;
    for(auto& l : mMenuVisibilityListeners){
        if(l.onMenuVisibilityChanged) l.onMenuVisibilityChanged(isVisible);
    }
}

Menu* ToolbarActionBar::getMenu(){
    if(!mMenuCallbackSet){
        MenuPresenter::Callback pcb;
        pcb.onCloseMenu = [this](MenuBuilder&, bool){
            mToolbar->dismisssPopupMenus();
            dispatchMenuVisibilityChanged(false);
        };
        pcb.onOpenSubMenu = [](MenuBuilder&)->bool{ return false; };
        MenuBuilder::Callback mcb;
        mcb.onMenuItemSelected = [](MenuBuilder&, MenuItem&)->bool{ return false; };
        mcb.onMenuModeChange = [this](MenuBuilder&){
            if(mToolbar->isOverflowMenuShowing()){
                dispatchMenuVisibilityChanged(false);
            }else{
                dispatchMenuVisibilityChanged(true);
            }
        };
        mToolbar->setMenuCallbacks(pcb, mcb);
        mMenuCallbackSet = true;
    }
    return mToolbar->getMenu();
}

void ToolbarActionBar::populateOptionsMenu(){
    Menu* menu = getMenu();
    menu->clear();
    // Mirrors androidx: if onCreatePanelMenu or onPreparePanel declines, drop the menu.
    if(!mHost->onCreateOptionsMenu(menu)){
        menu->clear();
    }else if(!mHost->onPrepareOptionsMenu(menu)){
        menu->clear();
    }
    mMenuPrepared = true;
}

void ToolbarActionBar::invalidateOptionsMenu(){
    // androidx posts a coalescing mMenuInvalidator; CDROID simplifies to a synchronous
    // rebuild (no PhoneWindow panel race to guard against).
    populateOptionsMenu();
}

void ToolbarActionBar::fireHomePressed(){
    // Mirror androidx ToolbarWidgetWrapper: synthesize an R::id::home ActionMenuItem and
    // dispatch it. Activity::onOptionsItemSelected folds home -> onNavigateUp when
    // DISPLAY_HOME_AS_UP is set.
    if(!mMenuPrepared) return;
    ActionMenuItem home(mToolbar->getContext(), 0, R::id::home, 0, 0, mTitle);
    mHost->onOptionsItemSelected(&home);
}

int ToolbarActionBar::getNavigationMode() const{
    return NAVIGATION_MODE_STANDARD;
}

void ToolbarActionBar::setNavigationMode(int mode){
    if(mode == NAVIGATION_MODE_TABS){
        throw std::runtime_error("Tabs are not supported in toolbar action bars");
    }
}

void* ToolbarActionBar::newTab(){
    throw std::runtime_error("Tabs are not supported in toolbar action bars");
}

}//namespace
