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

    virtual void onInflate(Context* context, const AttributeSet& attrs);
    void setParent(NavGraph* parent);
    NavGraph* getParent();
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
};
}/*endof namespace */
#endif /*__NAV_DESTINATION_H__*/
