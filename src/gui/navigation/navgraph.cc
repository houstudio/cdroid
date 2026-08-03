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
#include <navigation/navgraph.h>
#include <navigation/navgraphnavigator.h>
#include <navigation/navigatorprovider.h>
namespace cdroid{

NavGraph::NavGraph(/*@NonNull*/ NavigatorProvider* navigatorProvider)
    :NavGraph((NavGraphNavigator*)navigatorProvider->getNavigator("navigation")) {
}

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
NavGraph::NavGraph(/*@NonNull*/ NavGraphNavigator* navGraphNavigator)
   :NavDestination(navGraphNavigator){
}

NavGraph::~NavGraph(){
    // Owns every NavDestination in mNodes (added via addDestination). Delete each; a child that is
    // itself a NavGraph recurses through its own ~NavGraph via NavDestination's virtual dtor.
    // mNodesByRoute holds the SAME pointers (a route index) — clear it without deleting.
    for(int i = 0; i < mNodes.size(); i++) delete mNodes.valueAt(i);
    mNodes.clear();
    mNodesByRoute.clear();
}

void NavGraph::onInflate(Context* context, const AttributeSet& attrs){
    NavDestination::onInflate(context, attrs);
    const std::string startRoute = attrs.getString("startDestination");
    if(!startRoute.empty()) setStartDestination(startRoute);
    else setStartDestination(attrs.getResourceId("startDestination", 0));
}

std::pair<NavDestination*, Bundle*>* NavGraph::matchDeepLink(/*@NonNull Uri*/const std::string& uri) {
    // First search through any deep links directly added to this NavGraph
    std::pair<NavDestination*, Bundle*>*result = NavDestination::matchDeepLink(uri);
    if (result != nullptr) {
        return result;
    }
    const size_t  size = mNodes.size();
    for(int i=0;i<size;i++){
        NavDestination*child = mNodes.valueAt(i);
        std::pair<NavDestination*, Bundle*>* childResult = child->matchDeepLink(uri);
        if(childResult!=nullptr)return childResult;
    }
    return nullptr;
}

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
void NavGraph::addDestination(/*@NonNull*/ NavDestination* node) {
    if (node->getId() == 0 && node->getRoute().empty()) {
        throw std::runtime_error("Destinations must have an id or a route."
                " Call setId()/setRoute() or include android:id/app:route in your navigation XML.");
    }
    NavDestination* existingDestination = mNodes.get(node->getId());
    if (existingDestination == node) {
        return;
    }
    if (node->getParent() != nullptr) {
        throw std::runtime_error("Destination already has a parent set."
                " Call NavGraph.remove() to remove the previous parent.");
    }
    if (existingDestination != nullptr) {
        existingDestination->setParent(nullptr);
    }
    node->setParent(this);
    mNodes.put(node->getId(), node);
    if(!node->getRoute().empty()) mNodesByRoute[node->getRoute()] = node;
}

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
void NavGraph::addDestinations(const std::vector<NavDestination*>& nodes) {
    for (NavDestination* node : nodes) {
        if (node == nullptr) {
            continue;
        }
        addDestination(node);
    }
}

/**
 * Finds a destination in the collection by ID. This will recursively check the
 * {@link #getParent() parent} of this navigation graph if node is not found in
 * this navigation graph.
 *
 * @param resid ID to locate
 * @return the node with ID resid
 */
NavDestination* NavGraph::findNode(int resid) {
    return findNode(resid, true);
}

NavDestination* NavGraph::findNode(int resid, bool searchParents) {
    NavDestination* destination = mNodes.get(resid);
    // Search the parent for the NavDestination if it is not a child of this navigation graph
    // and searchParents is true
    return destination ? destination
            : searchParents && getParent() ? getParent()->findNode(resid) : nullptr;
}

NavGraph::Iterator NavGraph::begin() const {
    return Iterator((NavGraph*)this,0);
}

NavGraph::Iterator NavGraph::end() const {
    return Iterator((NavGraph*)this,mNodes.size());
};

/**
 * Add all destinations from another collection to this one. As each destination has at most
 * one parent, the destinations will be removed from the given NavGraph.
 *
 * @param other collection of destinations to add. All destinations will be removed from this
 * graph after being added to this graph.
 */
void NavGraph::addAll(NavGraph* other) {
    // androidx: "each destination has at most one parent, the destinations will be removed from the
    // given NavGraph." Transfer ownership from `other` to this. addDestination() throws if a node
    // still carries a parent, so detach every node from `other` first, then re-add here. The
    // destinations are NOT deleted during the detach — they move to this. (Previously this left both
    // graphs pointing at the same NavDestination*, so freeing either would double-free.)
    std::vector<NavDestination*> moved;
    moved.reserve(other->mNodes.size());
    for(int i = 0; i < other->mNodes.size(); i++) moved.push_back(other->mNodes.valueAt(i));
    for(NavDestination* d : moved){
        if(d->getParent() == other) d->setParent(nullptr);
    }
    other->mNodes.clear();
    other->mNodesByRoute.clear();
    for(NavDestination* d : moved) addDestination(d);  // setParent(this) + insert into this
}

/**
 * Remove a given destination from this NavGraph
 *
 * @param node the destination to remove.
 */
void NavGraph::remove(NavDestination* node) {
    int index = mNodes.indexOfKey(node->getId());
    if (index >= 0) {
        mNodes.valueAt(index)->setParent(nullptr);
        mNodes.removeAt(index);
    }
}

/**
 * Clear all destinations from this navigation graph.
 */
void NavGraph::clear() {
    // Detach every child WITHOUT deleting — this is the ownership-transfer / generic-remove path
    // (used by addAll). ~NavGraph is what deletes the destinations this graph still owns. The old
    // body was a no-op stub (iterated but never removed), which is why clear() leaked.
    for(int i = 0; i < mNodes.size(); i++){
        NavDestination* d = mNodes.valueAt(i);
        if(d->getParent() == this) d->setParent(nullptr);
    }
    mNodes.clear();
    mNodesByRoute.clear();
}

/**
 * Returns the starting destination for this NavGraph. When navigating to the NavGraph, this
 * destination is the one the user will initially see.
 * @return
 */
int NavGraph::getStartDestination() const{
    return mStartDestId;
}

/**
 * Sets the starting destination for this NavGraph.
 *
 * @param startDestId The id of the destination to be shown when navigating to this NavGraph.
 */
void NavGraph::setStartDestination(int startDestId) {
    mStartDestId = startDestId;
}

NavDestination* NavGraph::findNode(const std::string& route) {
    return findNode(route, true);
}

NavDestination* NavGraph::findNode(const std::string& route, bool searchParents) {
    auto it = mNodesByRoute.find(route);
    NavDestination* destination = (it != mNodesByRoute.end()) ? it->second : nullptr;
    if(!destination){
        // An argument-filled route (e.g. "home/42") is not a key in mNodesByRoute (keyed by the
        // "home/{id}" pattern); fall back to pattern matching across children (androidx).
        for(auto i = begin(); i != end(); ++i){
            NavDestination* d = (*i).second;
            if(d && d->matchRoute(route)){ destination = d; break; }
        }
    }
    return destination ? destination
        : (searchParents && getParent() ? getParent()->findNode(route) : nullptr);
}

std::vector<NavDestination*> NavGraph::childHierarchy(){
    // graph -> graph.startDest -> startDest.startDest -> ... until a non-graph leaf.
    std::vector<NavDestination*> chain;
    NavGraph* g = this;
    chain.push_back(g);
    while(g){
        const std::string& route = g->getStartDestinationRoute();
        NavDestination* start = route.empty() ? nullptr : g->findNode(route);
        if(!start) break;
        chain.push_back(start);
        g = dynamic_cast<NavGraph*>(start);
    }
    return chain;
}

NavGraph::Iterator::Iterator(NavGraph*g,int iter):mGraph(g){
    mIter=iter;
}

NavGraph::Iterator& NavGraph::Iterator::operator++() {
    ++mIter;
    return *this;
}

NavGraph::Iterator NavGraph::Iterator::operator++(int) {
    Iterator temp(mGraph,mIter);
    ++(*this);
    return temp;
}

std::pair<int, NavDestination*> NavGraph::Iterator::operator*() const {
    // mIter is a positional index (0..size), NOT a key — use valueAt/keyAt, not get(key).
    return {mGraph->mNodes.keyAt(mIter), mGraph->mNodes.valueAt(mIter)};
}

bool NavGraph::Iterator::operator!=(const Iterator& other) const {
    return mIter != other.mIter;
}

}/*endof namesapce*/

