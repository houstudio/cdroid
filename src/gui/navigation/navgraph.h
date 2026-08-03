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
#ifndef __NAV_GRAPH_H__
#define __NAV_GRAPH_H__
#include <string>
#include <unordered_map>
#include <navigation/navdestination.h>

namespace cdroid{

class NavigatorProvider;
class NavGraphNavigator;
class NavGraph :public NavDestination {
private:
    SparseArray<NavDestination*> mNodes;
    std::unordered_map<std::string, NavDestination*> mNodesByRoute; // route -> destination
    int mStartDestId = 0;
    std::string mStartDestinationRoute;
public:
    class Iterator;
public:
    NavGraph(NavigatorProvider* navigatorProvider);
    NavGraph(NavGraphNavigator* navGraphNavigator);
    // Owns every NavDestination in mNodes (added via addDestination); nested NavGraph children
    // recurse through NavDestination's virtual dtor.
    ~NavGraph() override;

    void onInflate(Context* context,const AttributeSet& attrs) override;
    std::pair<NavDestination*, Bundle*>* matchDeepLink(const std::string& uri) override;

    void addDestination(NavDestination* node);
    void addDestinations(const std::vector<NavDestination*>& nodes);

    NavDestination* findNode(int resid);
    NavDestination* findNode(int resid, bool searchParents);
    // Modern route lookups.
    NavDestination* findNode(const std::string& route);
    NavDestination* findNode(const std::string& route, bool searchParents);

    Iterator begin()const;
    Iterator end()const;

    void addAll(NavGraph* other);
    void remove(NavDestination* node);
    void clear();

    int getStartDestination() const;
    void setStartDestination(int startDestId);
    const std::string& getStartDestinationRoute() const { return mStartDestinationRoute; }
    void setStartDestination(const std::string& route) { mStartDestinationRoute = route; }
    // graph -> graph.startDest -> startDest.startDest -> ... until non-graph (androidx childHierarchy,
    // used by singleTop when the target is itself a NavGraph).
    std::vector<NavDestination*> childHierarchy();
};

class NavGraph::Iterator {
private:
    NavGraph*mGraph;
    int mIter;
public:
    Iterator(NavGraph*g,int iter);
    Iterator& operator++();
    Iterator operator++(int);
    std::pair<int, NavDestination*> operator*() const;
    bool operator!=(const Iterator& other) const;
};

}/*endof namespace*/
#endif /*__NAV_GRAPH_H__*/
