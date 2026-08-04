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
namespace cdroid{
class DrawerLayout;

class AppBarConfiguration{
public:
    class Builder;
    AppBarConfiguration() = default;

    const std::set<std::string>& getTopLevelDestinationRoutes() const { return mTopLevelRoutes; }
    DrawerLayout* getDrawerLayout() const { return mDrawerLayout; }
    bool isTopLevelDestination(const std::string& route) const {
        return mTopLevelRoutes.find(route) != mTopLevelRoutes.end();
    }
private:
    AppBarConfiguration(std::set<std::string> routes, DrawerLayout* drawer)
        : mTopLevelRoutes(std::move(routes)), mDrawerLayout(drawer){}
    std::set<std::string> mTopLevelRoutes;
    DrawerLayout* mDrawerLayout = nullptr;
};

class AppBarConfiguration::Builder{
public:
    Builder& addTopLevelRoute(const std::string& route){ mTopLevelRoutes.insert(route); return *this; }
    Builder& setTopLevelRoutes(const std::set<std::string>& routes){ mTopLevelRoutes = routes; return *this; }
    Builder& setDrawerLayout(DrawerLayout* drawer){ mDrawerLayout = drawer; return *this; }
    AppBarConfiguration* build(){ return new AppBarConfiguration(mTopLevelRoutes, mDrawerLayout); }
private:
    std::set<std::string> mTopLevelRoutes;
    DrawerLayout* mDrawerLayout = nullptr;
};

}//namespace cdroid
#endif
