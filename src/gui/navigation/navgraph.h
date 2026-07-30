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
