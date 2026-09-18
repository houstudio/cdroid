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
#ifndef __APPBARCONFIGURATION_H__
#define __APPBARCONFIGURATION_H__
/*********************************************************************************
 * Port of androidx.navigation.ui.AppBarConfiguration. Determines which destinations
 * are "top level" (no Up arrow, drawer button instead) and the optional drawer layout.
 * Uses route strings (modern navigation model).
 *********************************************************************************/
#include <set>
#include <string>
#include <functional>
namespace cdroid{
class DrawerLayout;
class Openable;

class AppBarConfiguration{
public:
    // androidx AppBarConfiguration.OnNavigateUpListener: called when navigateUp
    // can't pop any further (fallback for the Up button).
    using OnNavigateUpListener = std::function<bool()>;

    class Builder;
    AppBarConfiguration() = default;

    const std::set<std::string>& getTopLevelDestinationRoutes() const { return mTopLevelRoutes; }
    // androidx getOpenableLayout(): the Openable (DrawerLayout) that should open
    // from the Up button on a top-level destination.
    Openable* getOpenableLayout() const { return mOpenableLayout; }
    // Legacy CDROID face (pre-Openable): the same layout typed as DrawerLayout
    // (defined out-of-line in appbarconfiguration.cc — needs the full type).
    DrawerLayout* getDrawerLayout() const;  // dynamic_cast<DrawerLayout*>(mOpenableLayout)
    const OnNavigateUpListener& getFallbackOnNavigateUpListener() const { return mFallbackOnNavigateUpListener; }
    bool isTopLevelDestination(const std::string& route) const {
        return mTopLevelRoutes.find(route) != mTopLevelRoutes.end();
    }
private:
    AppBarConfiguration(std::set<std::string> routes, Openable* openable, OnNavigateUpListener fallback)
        : mTopLevelRoutes(std::move(routes)), mOpenableLayout(openable),
          mFallbackOnNavigateUpListener(std::move(fallback)){}
    std::set<std::string> mTopLevelRoutes;
    Openable* mOpenableLayout = nullptr;
    OnNavigateUpListener mFallbackOnNavigateUpListener;
};

class AppBarConfiguration::Builder{
public:
    Builder& addTopLevelRoute(const std::string& route){ mTopLevelRoutes.insert(route); return *this; }
    Builder& setTopLevelRoutes(const std::set<std::string>& routes){ mTopLevelRoutes = routes; return *this; }
    // androidx setOpenableLayout(Openable) — DrawerLayout implements Openable.
    Builder& setOpenableLayout(Openable* openable){ mOpenableLayout = openable; return *this; }
    Builder& setDrawerLayout(DrawerLayout* drawer);
    Builder& setFallbackOnNavigateUpListener(OnNavigateUpListener fallback){
        mFallbackOnNavigateUpListener = std::move(fallback); return *this; }
    AppBarConfiguration* build(){ return new AppBarConfiguration(mTopLevelRoutes, mOpenableLayout,
                                                                 mFallbackOnNavigateUpListener); }
private:
    std::set<std::string> mTopLevelRoutes;
    Openable* mOpenableLayout = nullptr;
    OnNavigateUpListener mFallbackOnNavigateUpListener;
};

}//namespace cdroid
#endif
