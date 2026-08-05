#include <widget/toolbaractionbar.h>
#include <widget/toolbarwidgetwrapper.h>
#include <widget/toolbar.h>
#include <widget/cdwindow.h>
#include <widget/actionbar.h>
#include <widget/R.h>
#include <menu/menu.h>
#include <menu/menubuilder.h>
#include <menu/menupresenter.h>
#include <menu/menuitem.h>
#include <view/view.h>
#include <stdexcept>

namespace cdroid{

ToolbarActionBar::ToolbarActionBar(Toolbar* toolbar, const std::string& title,
                                   WindowCallback* windowCallback)
  : mWindowCallback(windowCallback){
    // Mirrors androidx ToolbarActionBar ctor: wrap the toolbar, install the window callback, wire the
    // action-menu item click -> host onMenuItemSelected, and seed the window title.
    mDecorToolbar = new ToolbarWidgetWrapper(toolbar, false);
    mDecorToolbar->setWindowCallback(windowCallback);
    toolbar->setOnMenuItemClickListener([this](MenuItem& item)->bool{
        return mWindowCallback->onMenuItemSelected(Window::FEATURE_OPTIONS_PANEL, item);
    });
    mDecorToolbar->setWindowTitle(title);
}

ToolbarActionBar::~ToolbarActionBar(){ delete mDecorToolbar; }

void ToolbarActionBar::setDisplayOptions(int options){ setDisplayOptions(options, 0xffffffff); }

void ToolbarActionBar::setDisplayOptions(int options, int mask){
    const int current = mDecorToolbar->getDisplayOptions();
    mDecorToolbar->setDisplayOptions((options & mask) | (current & ~mask));
}

int ToolbarActionBar::getDisplayOptions() const{ return mDecorToolbar->getDisplayOptions(); }

void ToolbarActionBar::setTitle(const std::string& title){ mDecorToolbar->setTitle(title); }
void ToolbarActionBar::setSubtitle(const std::string& subtitle){ mDecorToolbar->setSubtitle(subtitle); }

void ToolbarActionBar::show(){ mDecorToolbar->setVisibility(View::VISIBLE); }
void ToolbarActionBar::hide(){ mDecorToolbar->setVisibility(View::GONE); }
bool ToolbarActionBar::isShowing() const{ return mDecorToolbar->getVisibility() == View::VISIBLE; }
int  ToolbarActionBar::getHeight() const{ return mDecorToolbar->getHeight(); }
Context* ToolbarActionBar::getThemedContext(){ return mDecorToolbar->getContext(); }

void ToolbarActionBar::setCustomView(View* view){ mDecorToolbar->setCustomView(view); }
void ToolbarActionBar::setIcon(Drawable* icon){ mDecorToolbar->setIcon(icon); }
void ToolbarActionBar::setLogo(Drawable* logo){ mDecorToolbar->setLogo(logo); }
void ToolbarActionBar::setHomeAsUpIndicator(Drawable* indicator){
    // androidx setHomeAsUpIndicator -> DecorToolbar.setNavigationIcon.
    mDecorToolbar->setNavigationIcon(indicator);
}
void ToolbarActionBar::setHomeActionContentDescription(const std::string& description){
    // androidx setHomeActionContentDescription -> DecorToolbar.setNavigationContentDescription.
    mDecorToolbar->setNavigationContentDescription(description);
}
void ToolbarActionBar::setHomeButtonEnabled(bool enabled){
    // If the nav button on a Toolbar is present, it's enabled. No-op (matches androidx).
    mDecorToolbar->setHomeButtonEnabled(enabled);
}

bool ToolbarActionBar::openOptionsMenu(){ return mDecorToolbar->showOverflowMenu(); }
bool ToolbarActionBar::closeOptionsMenu(){ return mDecorToolbar->hideOverflowMenu(); }

int  ToolbarActionBar::getNavigationMode() const{ return mDecorToolbar->getNavigationMode(); }
void ToolbarActionBar::setNavigationMode(int mode){
    if(mode == ActionBar::NAVIGATION_MODE_TABS){
        throw std::runtime_error("Tabs are not supported in toolbar action bars");
    }
    mDecorToolbar->setNavigationMode(mode);
}
void* ToolbarActionBar::newTab(){
    throw std::runtime_error("Tabs are not supported in toolbar action bars");
}

void ToolbarActionBar::addOnMenuVisibilityListener(const OnMenuVisibilityListener& listener){
    mMenuVisibilityListeners.push_back(listener);
}

void ToolbarActionBar::removeOnMenuVisibilityListener(const OnMenuVisibilityListener& listener){
    // CDROID listeners are structs holding a std::function; compare by callable target (works for
    // function pointers / stateful targets; stateless lambdas share a null target, so removal among
    // several identical lambdas is best-effort — a known limit of the std::function callback model).
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

void ToolbarActionBar::invalidateOptionsMenu(){
    // androidx posts a coalescing mMenuInvalidator; CDROID rebuilds synchronously (plan 接缝4) — there
    // is no PhoneWindow panel race to guard against.
    populateOptionsMenu();
}

void ToolbarActionBar::populateOptionsMenu(){
    // Mirrors androidx ToolbarActionBar.populateOptionsMenu: freeze item-change dispatching, rebuild
    // the menu via the host's panel callbacks, then resume dispatching.
    Menu* menu = getMenu();
    MenuBuilder* mb = dynamic_cast<MenuBuilder*>(menu);
    if(mb) mb->stopDispatchingItemsChanged();
    menu->clear();
    if(!mWindowCallback->onCreatePanelMenu(Window::FEATURE_OPTIONS_PANEL, *menu) ||
       !mWindowCallback->onPreparePanel(Window::FEATURE_OPTIONS_PANEL, nullptr, *menu)){
        menu->clear();
    }
    if(mb) mb->startDispatchingItemsChanged();
    // CDROID seam (接缝5): no PhoneWindow/AppCompatDelegate panel driver to call ToolbarMenuCallback
    // .onPreparePanel -> setMenuPrepared(); drive it here so the home-gate flips after the first menu
    // build (Window::setActionBar calls invalidateOptionsMenu -> this).
    if(!mToolbarMenuPrepared){
        mDecorToolbar->setMenuPrepared();
        mToolbarMenuPrepared = true;
    }
}

Menu* ToolbarActionBar::getMenu(){
    if(!mMenuCallbackSet){
        // androidx ActionMenuPresenterCallback / MenuBuilderCallback, ported as struct lambdas.
        MenuPresenter::Callback pcb;
        pcb.onCloseMenu = [this](MenuBuilder& menu, bool){
            if(mClosingActionMenu) return;
            mClosingActionMenu = true;
            mDecorToolbar->dismissPopupMenus();
            mWindowCallback->onPanelClosed(Window::FEATURE_SUPPORT_ACTION_BAR, menu);
            mClosingActionMenu = false;
        };
        pcb.onOpenSubMenu = [this](MenuBuilder& subMenu)->bool{
            mWindowCallback->onMenuOpened(Window::FEATURE_SUPPORT_ACTION_BAR, subMenu);
            return true;
        };
        MenuBuilder::Callback mcb;
        mcb.onMenuItemSelected = [](MenuBuilder&, MenuItem&)->bool{ return false; };
        mcb.onMenuModeChange = [this](MenuBuilder& menu){
            if(mDecorToolbar->isOverflowMenuShowing()){
                mWindowCallback->onPanelClosed(Window::FEATURE_SUPPORT_ACTION_BAR, menu);
            }else if(mWindowCallback->onPreparePanel(Window::FEATURE_OPTIONS_PANEL, nullptr, menu)){
                mWindowCallback->onMenuOpened(Window::FEATURE_SUPPORT_ACTION_BAR, menu);
            }
        };
        mDecorToolbar->setMenuCallbacks(pcb, mcb);
        mMenuCallbackSet = true;
    }
    return mDecorToolbar->getMenu();
}

}//namespace
