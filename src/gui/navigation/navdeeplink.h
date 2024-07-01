#ifndef __NAV_DEEPLINK_H__
#define __NAV_DEEPLINK_H__
#include <string>
#include <vector>
#include <navigation/navdestination.h>

namespace cdroid{
class NavDeepLink {
private:
    std::vector<std::string> mArguments;
    //Pattern mPattern;
public:
    /**
     * NavDestinations should be created via {@link Navigator#createDestination}.
     */
    NavDeepLink(const std::string& uri);
    bool matches(/*@NonNull Uri*/const std::string& deepLink)const;
    Bundle getMatchingArguments(/*@NonNull Uri*/const std::string& deepLink);
};
}/*endof namespace*/
#endif/*__NAV_DEEPLINK_H__*/
