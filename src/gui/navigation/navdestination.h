#ifndef __NAV_DESTINATION_H__
#define __NAV_DESTINATION_H__
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <core/context.h>
#include <core/sparsearray.h>
#include <core/any.h>
namespace cdroid{
class NavAction;
class Navigator;
class NavGraph;
class NavOptions;
class NavDeepLink;
//typedef std::map<const std::string,nonstd::any> Bundle;

class NavDestination {
public:
    static std::string getDisplayName(Context* context, int id);
private:
    Navigator* mNavigator;
    NavGraph* mParent;
    int mId;
    std::string mLabel;
    Bundle mDefaultArgs;
    std::vector<NavDeepLink*> mDeepLinks;
    SparseArray<NavAction*> mActions;
public:
    NavDestination(Navigator*navigator);

    virtual void onInflate(Context* context, const AttributeSet& attrs);
    void setParent(NavGraph* parent);

    NavGraph* getParent();
    int getId() const;
    void setId(int id);
    void setLabel(const std::string&label);

    const std::string getLabel() const;

    Navigator& getNavigator();

    Bundle getDefaultArguments();
    void setDefaultArguments(Bundle& args);

    void addDefaultArguments(Bundle& args);

    void addDeepLink(/*@NonNull*/const std::string& uriPattern);
    virtual std::pair<NavDestination*, Bundle>* matchDeepLink(/*Uri*/const std::string& uri);
    std::vector<int> buildDeepLinkIds();
    NavAction* getAction(int id);
    void putAction(int actionId, int destId);
    void putAction(int actionId, NavAction* action);
    void removeAction(int actionId);
    void navigate(Bundle* args, NavOptions* navOptions);
};
}/*endof namespace */
#endif /*__NAV_DESTINATION_H__*/
