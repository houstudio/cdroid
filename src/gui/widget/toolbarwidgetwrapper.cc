#include <widget/toolbarwidgetwrapper.h>
#include <widget/toolbar.h>
#include <widget/cdwindow.h>
#include <widget/actionbar.h>
#include <widget/R.h>
#include <menu/menu.h>
#include <menu/menubuilder.h>
#include <menu/menupresenter.h>
#include <menu/actionmenupresenter.h>
#include <menu/actionmenuitem.h>
#include <view/view.h>
#include <porting/cdlog.h>

namespace cdroid{

ToolbarWidgetWrapper::ToolbarWidgetWrapper(Toolbar* toolbar, bool style)
  : mToolbar(toolbar){
    // Mirrors androidx ToolbarWidgetWrapper(toolbar, style, R.string.abc_action_bar_up_description,
    // R.drawable.abc_ic_ab_back_material). CDROID seams: no theme homeAsUpIndicator attr resolution
    // and no Context::getText(int), so the default Up indicator comes from the built-in asset and the
    // default content-description int is left unresolved.
    mTitle = mToolbar->getTitle();
    mSubtitle = mToolbar->getSubtitle();
    mTitleSet = !mTitle.empty();
    mNavIcon = mToolbar->getNavigationIcon();
    mDefaultNavigationIcon = mToolbar->getContext()->getDrawable("cdroid:drawable/ic_ab_back_holo_dark");
    // style==true reads the ActionBar style theme attrs; it is intentionally not ported (the only
    // call site, ToolbarActionBar, passes false). For style==false upstream runs detectDisplayOptions().
    if(!style){
        mDisplayOpts = detectDisplayOptions();
    }
    mHomeDescription = mToolbar->getNavigationContentDescription();

    // Home/up click: synthesize an R::id::home ActionMenuItem and route through the WindowCallback,
    // gated on mMenuPrepared (ToolbarWidgetWrapper.java:181-190). Built at click time so its title
    // tracks the current window title (upstream captures a final field at construction — a minor
    // improvement, not a behavioral divergence for the home affordance).
    mToolbar->setNavigationOnClickListener([this](View&){
        if(mWindowCallback && mMenuPrepared){
            ActionMenuItem home(mToolbar->getContext(), 0, R::id::home, 0, 0, mTitle);
            mWindowCallback->onMenuItemSelected(Window::FEATURE_OPTIONS_PANEL, home);
        }
    });
}

ToolbarWidgetWrapper::~ToolbarWidgetWrapper(){
    delete mActionMenuPresenter;
}

int ToolbarWidgetWrapper::detectDisplayOptions(){
    int opts = ActionBar::DISPLAY_SHOW_TITLE | ActionBar::DISPLAY_SHOW_HOME | ActionBar::DISPLAY_USE_LOGO;
    if(mToolbar->getNavigationIcon() != nullptr){
        opts |= ActionBar::DISPLAY_HOME_AS_UP;
        mDefaultNavigationIcon = mToolbar->getNavigationIcon();
    }
    return opts;
}

ViewGroup* ToolbarWidgetWrapper::getViewGroup(){ return mToolbar; }
Context* ToolbarWidgetWrapper::getContext(){ return mToolbar->getContext(); }

bool ToolbarWidgetWrapper::hasExpandedActionView(){ return mToolbar->hasExpandedActionView(); }
void ToolbarWidgetWrapper::collapseActionView(){ mToolbar->collapseActionView(); }

void ToolbarWidgetWrapper::setWindowCallback(WindowCallback* cb){ mWindowCallback = cb; }

void ToolbarWidgetWrapper::setWindowTitle(const std::string& title){
    // "Real" title always trumps window title.
    if(!mTitleSet) setTitleInt(title);
}

void ToolbarWidgetWrapper::setTitle(const std::string& title){
    mTitleSet = true;
    setTitleInt(title);
}

void ToolbarWidgetWrapper::setTitleInt(const std::string& title){
    mTitle = title;
    if((mDisplayOpts & ActionBar::DISPLAY_SHOW_TITLE) != 0){
        mToolbar->setTitle(title);
    }
}

std::string ToolbarWidgetWrapper::getTitle(){ return mToolbar->getTitle(); }

std::string ToolbarWidgetWrapper::getSubtitle(){ return mToolbar->getSubtitle(); }

void ToolbarWidgetWrapper::setSubtitle(const std::string& subtitle){
    mSubtitle = subtitle;
    if((mDisplayOpts & ActionBar::DISPLAY_SHOW_TITLE) != 0){
        mToolbar->setSubtitle(subtitle);
    }
}

void ToolbarWidgetWrapper::setIcon(Drawable* d){ mIcon = d; updateToolbarLogo(); }
void ToolbarWidgetWrapper::setLogo(Drawable* d){ mLogo = d; updateToolbarLogo(); }

void ToolbarWidgetWrapper::updateToolbarLogo(){
    Drawable* logo = nullptr;
    if((mDisplayOpts & ActionBar::DISPLAY_SHOW_HOME) != 0){
        if((mDisplayOpts & ActionBar::DISPLAY_USE_LOGO) != 0) logo = mLogo != nullptr ? mLogo : mIcon;
        else logo = mIcon;
    }
    mToolbar->setLogo(logo);
}

bool ToolbarWidgetWrapper::canShowOverflowMenu(){ return mToolbar->canShowOverflowMenu(); }
bool ToolbarWidgetWrapper::isOverflowMenuShowing(){ return mToolbar->isOverflowMenuShowing(); }
bool ToolbarWidgetWrapper::isOverflowMenuShowPending(){ return mToolbar->isOverflowMenuShowPending(); }
bool ToolbarWidgetWrapper::showOverflowMenu(){ return mToolbar->showOverflowMenu(); }
bool ToolbarWidgetWrapper::hideOverflowMenu(){ return mToolbar->hideOverflowMenu(); }

void ToolbarWidgetWrapper::setMenuPrepared(){ mMenuPrepared = true; }

void ToolbarWidgetWrapper::setMenu(Menu* menu, const MenuPresenter::Callback& cb){
    // Dormant in the Toolbar-as-ActionBar path (ToolbarActionBar uses setMenuCallbacks + getMenu);
    // ported for DecorToolbar completeness. androidx also calls mActionMenuPresenter.setId(...);
    // CDROID's MenuPresenter has no setId, so that line is omitted.
    if(mActionMenuPresenter == nullptr){
        mActionMenuPresenter = new ActionMenuPresenter(mToolbar->getContext());
    }
    mActionMenuPresenter->setCallback(cb);
    mToolbar->setMenu(dynamic_cast<MenuBuilder*>(menu), *mActionMenuPresenter);
}

void ToolbarWidgetWrapper::dismissPopupMenus(){ mToolbar->dismisssPopupMenus(); }

int ToolbarWidgetWrapper::getDisplayOptions(){ return mDisplayOpts; }

void ToolbarWidgetWrapper::setDisplayOptions(int newOpts){
    const int oldOpts = mDisplayOpts;
    const int changed = oldOpts ^ newOpts;
    mDisplayOpts = newOpts;
    if(changed == 0) return;

    if((changed & ActionBar::DISPLAY_HOME_AS_UP) != 0){
        if((newOpts & ActionBar::DISPLAY_HOME_AS_UP) != 0) updateHomeAccessibility();
        updateNavigationIcon();
    }
    if((changed & AFFECTS_LOGO_MASK) != 0){
        updateToolbarLogo();
    }
    if((changed & ActionBar::DISPLAY_SHOW_TITLE) != 0){
        if((newOpts & ActionBar::DISPLAY_SHOW_TITLE) != 0){
            mToolbar->setTitle(mTitle);
            mToolbar->setSubtitle(mSubtitle);
        }else{
            mToolbar->setTitle(std::string());
            mToolbar->setSubtitle(std::string());
        }
    }
    if((changed & ActionBar::DISPLAY_SHOW_CUSTOM) != 0 && mCustomView != nullptr){
        if((newOpts & ActionBar::DISPLAY_SHOW_CUSTOM) != 0) mToolbar->addView(mCustomView);
        else mToolbar->removeView(mCustomView);
    }
}

bool ToolbarWidgetWrapper::isTitleTruncated(){ return mToolbar->isTitleTruncated(); }
void ToolbarWidgetWrapper::setCollapsible(bool collapsible){ mToolbar->setCollapsible(collapsible); }
void ToolbarWidgetWrapper::setHomeButtonEnabled(bool /*enable*/){ /* ignore */ }

void ToolbarWidgetWrapper::setCustomView(View* view){
    if(mCustomView != nullptr && (mDisplayOpts & ActionBar::DISPLAY_SHOW_CUSTOM) != 0){
        mToolbar->removeView(mCustomView);
    }
    mCustomView = view;
    if(view != nullptr && (mDisplayOpts & ActionBar::DISPLAY_SHOW_CUSTOM) != 0){
        mToolbar->addView(view);
    }
}

View* ToolbarWidgetWrapper::getCustomView(){ return mCustomView; }

void ToolbarWidgetWrapper::setNavigationIcon(Drawable* icon){
    mNavIcon = icon;
    updateNavigationIcon();
}

void ToolbarWidgetWrapper::setNavigationContentDescription(const std::string& description){
    mHomeDescription = description;
    updateHomeAccessibility();
}

void ToolbarWidgetWrapper::updateNavigationIcon(){
    if((mDisplayOpts & ActionBar::DISPLAY_HOME_AS_UP) != 0){
        mToolbar->setNavigationIcon(mNavIcon != nullptr ? mNavIcon : mDefaultNavigationIcon);
    }else{
        mToolbar->setNavigationIcon(nullptr);
    }
}

void ToolbarWidgetWrapper::updateHomeAccessibility(){
    if((mDisplayOpts & ActionBar::DISPLAY_HOME_AS_UP) != 0){
        if(!mHomeDescription.empty()){
            mToolbar->setNavigationContentDescription(mHomeDescription);
        }
        // else: upstream applies mDefaultNavigationContentDescription (an int res); CDROID has no
        // int->string resolution, so the default description is left inert.
    }
}

void ToolbarWidgetWrapper::setDefaultNavigationIcon(Drawable* icon){
    if(mDefaultNavigationIcon != icon){
        mDefaultNavigationIcon = icon;
        updateNavigationIcon();
    }
}

void ToolbarWidgetWrapper::setBackgroundDrawable(Drawable* d){ mToolbar->setBackground(d); }
int  ToolbarWidgetWrapper::getHeight(){ return mToolbar->getHeight(); }
void ToolbarWidgetWrapper::setVisibility(int visible){ mToolbar->setVisibility(visible); }
int  ToolbarWidgetWrapper::getVisibility(){ return mToolbar->getVisibility(); }

void ToolbarWidgetWrapper::setMenuCallbacks(const MenuPresenter::Callback& presenterCallback,
                                            const MenuBuilder::Callback& menuBuilderCallback){
    mToolbar->setMenuCallbacks(presenterCallback, menuBuilderCallback);
}

Menu* ToolbarWidgetWrapper::getMenu(){ return mToolbar->getMenu(); }

int  ToolbarWidgetWrapper::getNavigationMode(){ return mNavigationMode; }
void ToolbarWidgetWrapper::setNavigationMode(int mode){
    // Deprecated list navigation (Spinner/Tab views) is not ported; just remember the mode. The
    // throw-on-TABS guard lives in ActionBar/ToolbarActionBar::setNavigationMode.
    mNavigationMode = mode;
}

}//namespace
