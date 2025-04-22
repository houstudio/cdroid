#ifndef __NAV_GRAPH_H__
#define __NAV_GRAPH_H__
#include <navigation/navdestination.h>

namespace cdroid{

class NavigatorProvider;
class NavGraphNavigator;
class NavGraph :public NavDestination {
private:
    SparseArray<NavDestination*> mNodes;
    int mStartDestId;
public:
    class Iterator;
public:
    /**
     * Construct a new NavGraph. This NavGraph is not valid until you
     * {@link #addDestination(NavDestination) add a destination} and
     * {@link #setStartDestination(int) set the starting destination}.
     *
     * @param navigatorProvider The {@link NavController} which this NavGraph
     *                          will be associated with.
     */
    NavGraph(/*@NonNull*/NavigatorProvider* navigatorProvider);

    /**
     * Construct a new NavGraph. This NavGraph is not valid until you
     * {@link #addDestination(NavDestination) add a destination} and
     * {@link #setStartDestination(int) set the starting destination}.
     *
     * @param navGraphNavigator The {@link NavGraphNavigator} which this destination
     *                          will be associated with. Generally retrieved via a
     *                          {@link NavController}'s
     *                          {@link NavigatorProvider#getNavigator(Class)} method.
     */
    NavGraph(/*@NonNull Navigator<? extends NavGraph> */NavGraphNavigator*navGraphNavigator);

    void onInflate(Context* context,const AttributeSet& attrs) override;
    //@Override @Nullable
    std::pair<NavDestination*, Bundle*>* matchDeepLink(/*Uri*/const std::string& uri) override;

    /**
     * Adds a destination to this NavGraph. The destination must have an
     * {@link NavDestination#getId()} id} set.
     *
     * <p>The destination must not have a {@link NavDestination#getParent() parent} set. If
     * the destination is already part of a {@link NavGraph navigation graph}, call
     * {@link #remove(NavDestination)} before calling this method.</p>
     *
     * @param node destination to add
     */
    void addDestination(/*NonNull*/ NavDestination* node);

    /**
     * Adds multiple destinations to this NavGraph. Each destination must have an
     * {@link NavDestination#getId()} id} set.
     *
     * <p> Each destination must not have a {@link NavDestination#getParent() parent} set. If
     * any destination is already part of a {@link NavGraph navigation graph}, call
     * {@link #remove(NavDestination)} before calling this method.</p>
     *
     * @param nodes destinations to add
     */
    void addDestinations(const std::vector<NavDestination*>& nodes);

    /**
     * Finds a destination in the collection by ID. This will recursively check the
     * {@link #getParent() parent} of this navigation graph if node is not found in
     * this navigation graph.
     *
     * @param resid ID to locate
     * @return the node with ID resid
     */
    NavDestination* findNode(int resid);
    NavDestination* findNode(int resid, bool searchParents);
    //@NonNull  @Override
    Iterator begin()const;
    Iterator end()const;

    /**
     * Add all destinations from another collection to this one. As each destination has at most
     * one parent, the destinations will be removed from the given NavGraph.
     *
     * @param other collection of destinations to add. All destinations will be removed from this
     * graph after being added to this graph.
     */
    void addAll(/*@NonNull*/NavGraph* other);

    /**
     * Remove a given destination from this NavGraph
     *
     * @param node the destination to remove.
     */
    void remove(/*@NonNull */NavDestination* node);

    /**
     * Clear all destinations from this navigation graph.
     */
    void clear();
    /**
     * Returns the starting destination for this NavGraph. When navigating to the NavGraph, this
     * destination is the one the user will initially see.
     * @return
     */
    int getStartDestination() const;

    /**
     * Sets the starting destination for this NavGraph.
     *
     * @param startDestId The id of the destination to be shown when navigating to this NavGraph.
     */
    void setStartDestination(int startDestId);
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


