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
#include <widget/internal_R.h>  // cdroid::internal::R (framework res ids)
#include <navigation/navigationui.h>
#include <navigation/appbarconfiguration.h>
#include <navigation/navdestination.h>
#include <navigation/navgraph.h>
#include <navigation/navoptions.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <widget/openable.h>
#include <widgetEx/navigationview/navigationview.h>
#include <menu/menuitem.h>
#include <widget/actionbar.h>
#include <widget/toolbar.h>
#include <view/view.h>
#include <porting/cdlog.h>

namespace cdroid{

// androidx navigateUp(NavController, Openable): delegate to the configuration
// form with an on-the-fly configuration (its sole openable is the given layout).
bool NavigationUI::navigateUp(NavController* navController, Openable* openableLayout){
    if (!navController) return false;
    std::unique_ptr<AppBarConfiguration> configuration(
            AppBarConfiguration::Builder().setOpenableLayout(openableLayout).build());
    return navigateUp(navController, configuration.get());
}

bool NavigationUI::navigateUp(NavController* navController, AppBarConfiguration* configuration){
    if (!navController) return false;
    Openable* openableLayout = configuration ? configuration->getOpenableLayout() : nullptr;
    NavDestination* currentDestination = navController->getCurrentDestination();
    if (openableLayout != nullptr && currentDestination != nullptr
            && configuration->isTopLevelDestination(currentDestination->getRoute())) {
        openableLayout->open();
        return true;
    }
    if (navController->navigateUp()) {
        return true;
    }
    return configuration && configuration->getFallbackOnNavigateUpListener()
        ? configuration->getFallbackOnNavigateUpListener()() : false;
}

// --- androidx listener classes -------------------------------------------------
// Abstract: title + Up affordance. androidx animates a DrawerArrowDrawable;
// CDROID has none, so the built-in Up indicator asset (ic_ab_back) stands in,
// and with an openable layout configured top-level destinations show no icon
// (the drawer toggle is the app chrome's job) — same net behavior as before.
AbstractAppBarOnDestinationChangedListener::AbstractAppBarOnDestinationChangedListener(
        Context* context, AppBarConfiguration* configuration)
    : mContext(context), mConfiguration(configuration){}

void AbstractAppBarOnDestinationChangedListener::attach(NavController* controller){
    controller->addOnDestinationChangedListener(
        [this](NavController* c, NavDestination* d, Bundle* b){ onDestinationChanged(c, d, b); });
}

void AbstractAppBarOnDestinationChangedListener::onDestinationChanged(
        NavController* /*controller*/, NavDestination* destination, Bundle* /*arguments*/){
    if (destination == nullptr) return;
    // androidx skips FloatingWindow destinations (dialogs); CDROID has no
    // FloatingWindow marker yet.
    const std::string& label = destination->getLabel();
    if (!label.empty()) {
        setTitle(label);
    }
    const bool isTopLevel = mConfiguration
        && mConfiguration->isTopLevelDestination(destination->getRoute());
    if (isTopLevel) {
        // top-level: no Up icon (with an openable, the drawer toggle owns it)
        setNavigationIcon(nullptr);
    } else {
        setNavigationIcon(mContext->getDrawable(cdroid::internal::R::drawable::ic_ab_back_holo_dark));
    }
}

ToolbarOnDestinationChangedListener::ToolbarOnDestinationChangedListener(
        Toolbar* toolbar, AppBarConfiguration* configuration)
    : AbstractAppBarOnDestinationChangedListener(toolbar ? toolbar->getContext() : nullptr, configuration),
      mToolbar(toolbar){}

void ToolbarOnDestinationChangedListener::onDestinationChanged(
        NavController* controller, NavDestination* destination, Bundle* arguments){
    if (mToolbar == nullptr) {  // androidx: WeakReference gone -> stop
        return;
    }
    AbstractAppBarOnDestinationChangedListener::onDestinationChanged(controller, destination, arguments);
}

void ToolbarOnDestinationChangedListener::setTitle(const std::string& title){
    if (mToolbar) mToolbar->setTitle(title);
}

void ToolbarOnDestinationChangedListener::setNavigationIcon(Drawable* icon){
    if (mToolbar) mToolbar->setNavigationIcon(icon);
}

ActionBarOnDestinationChangedListener::ActionBarOnDestinationChangedListener(
        Context* context, ActionBar* actionBar, AppBarConfiguration* configuration)
    : AbstractAppBarOnDestinationChangedListener(context, configuration), mActionBar(actionBar){}

void ActionBarOnDestinationChangedListener::onDestinationChanged(
        NavController* controller, NavDestination* destination, Bundle* arguments){
    if (mActionBar == nullptr) return;
    AbstractAppBarOnDestinationChangedListener::onDestinationChanged(controller, destination, arguments);
}

void ActionBarOnDestinationChangedListener::setTitle(const std::string& title){
    if (mActionBar) mActionBar->setTitle(title);
}

void ActionBarOnDestinationChangedListener::setNavigationIcon(Drawable* icon){
    // androidx: setDisplayHomeAsUpEnabled(icon != null) + drawer-toggle delegate.
    if (mActionBar) mActionBar->setDisplayHomeAsUpEnabled(icon != nullptr);
}

void NavigationUI::setupActionBarWithNavController(ActionBar* actionBar,
                                                   NavController* navController,
                                                   AppBarConfiguration* configuration){
    if(!actionBar || !navController) return;
    // The listener functor (value-owned by the controller) captures `this`, so
    // the listener object must outlive it — kept in an app-lifetime arena, the
    // same ownership model as the previous raw-pointer lambda captures.
    static std::vector<std::unique_ptr<ActionBarOnDestinationChangedListener>> sListeners;
    sListeners.push_back(std::make_unique<ActionBarOnDestinationChangedListener>(
            nullptr, actionBar, configuration));
    sListeners.back()->attach(navController);
}

void NavigationUI::setupWithNavController(Toolbar* toolbar, NavController* navController,
                                          AppBarConfiguration* configuration){
    if(!toolbar || !navController) return;
    // androidx NavigationUI.setupWithNavController(Toolbar, NavController, AppBarConfiguration)
    // attaches a ToolbarOnDestinationChangedListener (title + Up indicator, via
    // AbstractAppBarOnDestinationChangedListener) and wires the navigation click UNCONDITIONALLY
    // to navigateUp(navController, configuration). With no Openable/drawer configured (CDROID
    // wires no drawer here) the icon logic clears the nav icon on top-level destinations and
    // shows the Up indicator otherwise — androidx uses a DrawerArrowDrawable at progress 1; CDROID
    // has no DrawerArrowDrawable, so the built-in homeAsUpIndicator asset stands in.
    // The 2-arg overload passes no AppBarConfiguration; androidx then builds a default whose sole
    // top-level destination is the graph's start destination. Mirror that so the start screen
    // shows no Up arrow (and the existing 2-arg callers don't regress to an arrow on home).
    // androidx attaches a ToolbarOnDestinationChangedListener; with no explicit
    // configuration the default's sole top-level destination is the graph's start
    // destination (so home shows no Up arrow) — build that config here.
    std::unique_ptr<AppBarConfiguration> defaultConfig;
    if (configuration == nullptr && navController->getGraph() != nullptr) {
        const std::string& startRoute = navController->getGraph()->getStartDestinationRoute();
        if (!startRoute.empty()) {
            defaultConfig.reset(AppBarConfiguration::Builder()
                    .addTopLevelRoute(startRoute).build());
            configuration = defaultConfig.get();
        }
    }
    static std::vector<std::unique_ptr<ToolbarOnDestinationChangedListener>> sListeners;
    sListeners.push_back(std::make_unique<ToolbarOnDestinationChangedListener>(toolbar, configuration));
    sListeners.back()->attach(navController);
    // Wired once, unconditionally — navigateUp itself decides drawer-vs-pop from the configuration.
    toolbar->setNavigationOnClickListener([navController, configuration](View&){
        navigateUp(navController, configuration);
    });
}

// androidx NavigationUI.matchDestination (internal): the destination's id or
// any ANCESTOR graph's id matches (hierarchy = self -> root).
static bool matchDestination(NavDestination* destination, int destId) {
    for (NavDestination* node : destination->hierarchy()) {
        if (node->getId() == destId) return true;
    }
    return false;
}

bool NavigationUI::onNavDestinationSelected(MenuItem* item, NavController* navController){
    if (item == nullptr || navController == nullptr) return false;

    NavOptions::Builder builder;
    builder.setLaunchSingleTop(true).setRestoreState(true);
    // androidx picks view animations for ActivityNavigator destinations and
    // animator resources otherwise; CDROID ships neither nav_default_* set, so
    // no explicit animations are set here (the nav transition keeps defaults).
    if ((item->getOrder() & Menu::CATEGORY_SECONDARY) == 0) {
        // Primary menu items pop back to the start destination, saving its state.
        NavDestination* start = navController->getGraph()
                ? NavGraph::findStartDestination(navController->getGraph()) : nullptr;
        if (start) {
            builder.setPopUpTo(start->getId(), /*inclusive*/ false, /*saveState*/ true);
        }
    }
    NavOptions* options = builder.build();

    // androidx throws/catches IllegalArgumentException when the id cannot be
    // resolved from the current destination; CDROID's navigate(int) returns
    // silently, and matchDestination decides success below.
    navController->navigate(item->getItemId(), nullptr, options);
    // Return true only if the destination we've navigated to matches the MenuItem.
    NavDestination* current = navController->getCurrentDestination();
    const bool matched = current != nullptr && matchDestination(current, item->getItemId());
    if (!matched) {
        LOGD("NavigationUI: ignoring onNavDestinationSelected for MenuItem 0x%x "
             "as it cannot be found from the current destination", item->getItemId());
    }
    return matched;
}


// androidx NavigationUI.setupWithNavController(NavigationView, NavController):
// item clicks navigate (and close the Openable parent — a drawer), destination
// changes check the matching menu items (matchDestination over the hierarchy).
void NavigationUI::setupWithNavController(NavigationView* navigationView, NavController* navController){
    if (navigationView == nullptr || navController == nullptr) return;

    class ItemListener : public NavigationView::OnNavigationItemSelectedListener {
    public:
        NavController* mController;
        NavigationView* mView;
        explicit ItemListener(NavController* c, NavigationView* v)
            : mController(c), mView(v) {}
        bool onNavigationItemSelected(MenuItem* item) override {
            const bool handled = NavigationUI::onNavDestinationSelected(item, mController);
            if (handled) {
                // androidx: close the Openable parent (a DrawerLayout); the
                // bottom-sheet branch is not ported.
                Openable* openable = dynamic_cast<Openable*>(mView->getParent());
                if (openable != nullptr) {
                    openable->close();
                }
            }
            return handled;
        }
    };
    // The listener must outlive the view's clicks; app-lifetime arena like the
    // app-bar listeners above.
    static std::vector<std::unique_ptr<ItemListener>> sItemListeners;
    sItemListeners.push_back(std::make_unique<ItemListener>(navController, navigationView));
    navigationView->setNavigationItemSelectedListener(sItemListeners.back().get());

    navController->addOnDestinationChangedListener(
        [navigationView](NavController*, NavDestination* destination, Bundle*){
            if (destination == nullptr) return;
            // androidx skips FloatingWindow destinations (not ported).
            Menu* menu = navigationView->getMenu();
            for (int i = 0; i < menu->size(); i++) {
                MenuItem* item = menu->getItem(i);
                item->setChecked(matchDestination(destination, item->getItemId()));
            }
        });
}

}//namespace cdroid
