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
#include <navigation/navdestination.h>
#include <navigation/navaction.h>
#include <navigation/navgraph.h>
#include <navigation/navigator.h>
#include <navigation/navdeeplink.h>
#include <navigation/navargument.h>
#include <porting/cdlog.h>
namespace cdroid{

std::string NavDestination::getDisplayName(Context* context, int id) {
    return "";
}

NavDestination::NavDestination(Navigator* navigator) {
    mNavigator = navigator;
}

NavDestination::NavDestination(const std::string& navigatorName) {
    mNavigator = nullptr;
    mNavigatorName = navigatorName;
}

NavDestination::~NavDestination(){
    // Owns its deep links, actions and arguments (added via addDeepLink / putAction / addArgument).
    for(NavDeepLink* dl : mDeepLinks) delete dl;
    for(int i = 0; i < mActions.size(); i++) delete mActions.valueAt(i);
    for(auto& kv : mArguments) delete kv.second;
}

void NavDestination::addArgument(const std::string& name, NavArgument* argument) {
    mArguments[name] = argument;
}

void NavDestination::removeArgument(const std::string& name) {
    mArguments.erase(name);
}

void NavDestination::onInflate(Context* context, const AttributeSet& attrs) {
    setId(attrs.getResourceId("id", 0));
    setLabel(attrs.getString("label"));
    const std::string route = attrs.getString("route");
    if(!route.empty()) setRoute(route);
}

void NavDestination::setParent(NavGraph* parent) {
    mParent = parent;
}

NavGraph* NavDestination::getParent() {
    return mParent;
}

int NavDestination::getId() const{
    return mId;
}

void NavDestination::setId(int id) {
    mId = id;
}

void NavDestination::setLabel(const std::string&label) {
    mLabel = label;
}

const std::string NavDestination::getLabel() const{
    return mLabel;
}

Navigator& NavDestination::getNavigator() {
    return *mNavigator;
}

/*@NonNull*/ Bundle NavDestination::getDefaultArguments() {
    return mDefaultArgs;
}

void NavDestination::setDefaultArguments(Bundle& args) {
    mDefaultArgs = args;
}

void NavDestination::addDefaultArguments(Bundle& args) {
    //getDefaultArguments().putAll(args);
}

void NavDestination::addDeepLink(const std::string& uriPattern) {
    mDeepLinks.push_back(new NavDeepLink(uriPattern));
}

std::pair<NavDestination*, Bundle*>* NavDestination::matchDeepLink(/*@NonNull Uri*/const std::string& uri) {
    if (mDeepLinks.empty()){// == nullptr) {
        return nullptr;
    }
    for (NavDeepLink* deepLink : mDeepLinks) {
        Bundle* matchingArguments = deepLink->getMatchingArguments(uri);
        if (matchingArguments != nullptr) {
            return new std::pair<NavDestination*, Bundle*>{this, matchingArguments};
        }
    }
    return nullptr;
}

bool NavDestination::matchRoute(const std::string& route) const {
    if(mRoute.empty()) return false;
    if(mRoute == route) return true; // exact match covers no-argument routes
    if(mRoute.find('{') == std::string::npos) return false; // no placeholder -> cannot match
    NavDeepLink dl(mRoute); // reuse the {arg} -> regex machinery from NavDeepLink
    return dl.matches(route);
}

std::vector<NavDestination*> NavDestination::hierarchy(){
    std::vector<NavDestination*> chain;
    NavDestination* current = this;
    while(current){
        chain.push_back(current);
        current = current->getParent(); // NavGraph* -> NavDestination*
    }
    return chain; // { this, parent, ..., root }
}

std::vector<int> NavDestination::buildDeepLinkIds() {
    std::vector<NavDestination*> hierarchy;
    NavDestination* current = this;
    do {
        NavGraph* parent = current->getParent();
        if (parent == nullptr || parent->getStartDestination() != current->getId()) {
            hierarchy.insert(hierarchy.begin(),current);//addFirst(current);
        }
        current = parent;
    } while (current != nullptr);
    std::vector<int>deepLinkIds;// = new int[hierarchy.size()];
    int index = 0;
    for (NavDestination* destination : hierarchy) {
        deepLinkIds.push_back(destination->getId());//[index++] = destination->getId();
    }
    return deepLinkIds;
}


NavAction* NavDestination::getAction(int id) {
    NavAction* destination = mActions.size()==0 ? nullptr : mActions.get(id);
    // Search the parent for the given action if it is not found in this destination
    return destination != nullptr ? destination
            : getParent() != nullptr ? getParent()->getAction(id) : nullptr;
}

void NavDestination::putAction(int actionId, int destId) {
    putAction(actionId, new NavAction(destId));
}

void NavDestination::putAction(int actionId, NavAction* action) {
    if (actionId == 0) {
        throw ("Cannot have an action with actionId 0");
    }
    mActions.put(actionId, action);
}

void NavDestination::removeAction(int actionId) {
    if (mActions.size() == 0) {
        return;
    }
    mActions.remove(actionId);//delete(actionId);
}

void NavDestination::navigate(/*@Nullable*/ Bundle* args, /*@Nullable*/ NavOptions* navOptions) {
    LOGD("NavDestination.navigate route='%s' mNavigator=%p navigatorName='%s'",
         getRoute().c_str(), mNavigator, getNavigatorName().c_str());
    mNavigator->navigate(this, args, navOptions);
}

Bundle* NavDestination::addInDefaultArgs(Bundle* args){
    // androidx NavDestination.addInDefaultArgs: merge caller args over default args (defaults from
    // <argument android:defaultValue>). Caller overrides defaults. Returns args as-is if no defaults.
    if(mDefaultArgs.isEmpty()) return args;
    Bundle* result = new Bundle();
    result->putAll(mDefaultArgs);
    if(args) result->putAll(*args);
    return result;
}

Bundle* NavDestination::matchRouteArgs(const std::string& route) const{
    // Extract path-param arguments from a filled route (e.g. "detail/42" vs pattern "detail/{id}").
    // Returns nullptr if this destination's route has no {param} placeholders.
    if(mRoute.empty() || mRoute.find('{') == std::string::npos) return nullptr;
    NavDeepLink link(mRoute);
    return link.getMatchingArguments(route);
}
}/*endof namespace*/
