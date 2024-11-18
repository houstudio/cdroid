#include <navigation/navgraph.h>
#include <navigation/navgraphnavigator.h>
#include <navigation/navigatorprovider.h>
namespace cdroid{

NavGraph::NavGraph(/*@NonNull*/ NavigatorProvider* navigatorProvider)
    :NavGraph((NavGraphNavigator*)navigatorProvider->getNavigator("")) {
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

void NavGraph::onInflate(Context* context, const AttributeSet& attrs){
    NavDestination::onInflate(context, attrs);
    setStartDestination(attrs.getResourceId("startDestination", 0));
}

std::pair<NavDestination*, Bundle>* NavGraph::matchDeepLink(/*@NonNull Uri*/const std::string& uri) {
    // First search through any deep links directly added to this NavGraph
    std::pair<NavDestination*, Bundle>*result = NavDestination::matchDeepLink(uri);
    if (result != nullptr) {
        return result;
    }
    // Then search through all child destinations for a matching deep link
#if 0
    for (NavDestination* child : this) {
        std::pair<NavDestination*, Bundle>* childResult = child->matchDeepLink(uri);
        if (childResult != nullptr) {
            return childResult;
        }
    }
#else
    const size_t  size = mNodes.size();
    for(int i=0;i<size;i++){
        NavDestination*child = mNodes.valueAt(i);
	std::pair<NavDestination*, Bundle>* childResult = child->matchDeepLink(uri);
	if(childResult)return childResult;
    }
#endif
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
    if (node->getId() == 0) {
        throw std::runtime_error("Destinations must have an id."
                " Call setId() or include an android:id in your navigation XML.");
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
    const size_t size = other->mNodes.size();
    for(int i=0;i<size;i++){
        NavDestination* destination = other->mNodes.valueAt(i);
        addDestination(destination);
    }
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
    Iterator iterator = begin();
    while (iterator!=end()) {
        //iterator.next();
        //iterator.remove();
	iterator++;
    }
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
    return {mIter,mGraph->mNodes.get(mIter)};
}

bool NavGraph::Iterator::operator!=(const Iterator& other) const {
    return mIter != other.mIter;
}

}/*endof namesapce*/

