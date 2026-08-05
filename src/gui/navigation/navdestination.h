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
#ifndef __NAV_DESTINATION_H__
#define __NAV_DESTINATION_H__
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <core/context.h>
#include <core/sparsearray.h>
#include <core/bundle.h>
#include <navigation/navargument.h>
namespace cdroid{
class NavAction;
class Navigator;
class NavGraph;
class NavOptions;
class NavDeepLink;

class NavDestination {
public:
    static std::string getDisplayName(Context* context, int id);
private:
    Navigator* mNavigator;
    NavGraph* mParent = nullptr;
    int mId = 0;
    std::string mLabel;
    std::string mRoute;            // modern route identifier
    std::string mNavigatorName;    // navigator name (e.g. "fragment"/"navigation")
    Bundle mDefaultArgs;
    std::vector<NavDeepLink*> mDeepLinks;
    SparseArray<NavAction*> mActions;
    std::map<std::string, NavArgument*> mArguments;
public:
    NavDestination(Navigator* navigator);
    NavDestination(const std::string& navigatorName);
    // CDROID has no GC: NavDestination owns its NavDeepLinks / NavActions / NavArguments. Virtual so
    // that deleting a NavDestination* which actually points at a NavGraph recurses into ~NavGraph
    // (which owns its child destinations) — nested <navigation> graphs are common.
    virtual ~NavDestination();

    virtual void onInflate(Context* context, const AttributeSet& attrs);
    void setParent(NavGraph* parent);
    NavGraph* getParent();
    // Ancestor chain self -> root (androidx NavDestination.hierarchy).
    std::vector<NavDestination*> hierarchy();
    int getId() const;
    void setId(int id);
    void setLabel(const std::string& label);
    const std::string getLabel() const;
    Navigator& getNavigator();

    // Modern route API.
    const std::string& getRoute() const { return mRoute; }
    void setRoute(const std::string& route) { mRoute = route; }
    const std::string& getNavigatorName() const { return mNavigatorName; }
    void setNavigatorName(const std::string& name) { mNavigatorName = name; }
    bool hasRoute(const std::string& route) const { return !mRoute.empty() && mRoute == route; }
    // Matches a (possibly argument-filled) route against this destination's route pattern, e.g.
    // "home/42" matches the "home/{id}" pattern. Exact match short-circuits; otherwise the {arg}
    // placeholder machinery from NavDeepLink is reused (androidx NavDestination.matchRoute).
    bool matchRoute(const std::string& route) const;
    void addArgument(const std::string& name, NavArgument* argument);
    void removeArgument(const std::string& name);
    const std::map<std::string, NavArgument*>& getArguments() const { return mArguments; }

    Bundle getDefaultArguments();
    void setDefaultArguments(Bundle& args);
    void addDefaultArguments(Bundle& args);
    void addDeepLink(const std::string& uriPattern);
    virtual std::pair<NavDestination*, Bundle*>* matchDeepLink(const std::string& uri);
    std::vector<int> buildDeepLinkIds();
    NavAction* getAction(int id);
    void putAction(int actionId, int destId);
    void putAction(int actionId, NavAction* action);
    void removeAction(int actionId);
    void navigate(Bundle* args, NavOptions* navOptions);
    // androidx NavDestination.addInDefaultArgs: merge caller args over this destination's default
    // arguments (from <argument android:defaultValue>). Returns a new Bundle, or `args` as-is if
    // there are no defaults.
    Bundle* addInDefaultArgs(Bundle* args);
    // Extract path-param arguments from a filled route (e.g. "detail/42" against pattern
    // "detail/{id}" → {id:"42"}) via NavDeepLink. Returns nullptr if the route has no params.
    Bundle* matchRouteArgs(const std::string& route) const;
};
}/*endof namespace */
#endif /*__NAV_DESTINATION_H__*/
