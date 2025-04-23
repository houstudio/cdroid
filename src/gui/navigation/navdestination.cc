#include <navigation/navdestination.h>
#include <navigation/navaction.h>
#include <navigation/navgraph.h>
#include <navigation/navigator.h>
#include <navigation/navdeeplink.h>
namespace cdroid{

std::string NavDestination::getDisplayName(Context* context, int id) {
    /*try {
        return context.getResources().getResourceName(id);
    } catch (Resources.NotFoundException e) {
        return Integer.toString(id);
    }*/
    return "";
}

NavDestination::NavDestination(/*@NonNull Navigator<? extends NavDestination>*/Navigator* navigator) {
    mNavigator = navigator;
}

void NavDestination::onInflate(Context* context, const AttributeSet& attrs) {
    setId(attrs.getResourceId("id", 0));
    setLabel(attrs.getString("label"));
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
    Bundle defaultArgs = getDefaultArguments();
    Bundle *finalArgs = new Bundle();
    /*finalArgs.putAll(defaultArgs);
    if (args != nullptr) {
        finalArgs.putAll(args);
    }*/
    mNavigator->navigate(this, finalArgs, navOptions);
}
}/*endof namespace*/
