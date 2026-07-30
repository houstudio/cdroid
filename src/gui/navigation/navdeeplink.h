#ifndef __NAV_DEEPLINK_H__
#define __NAV_DEEPLINK_H__
/*********************************************************************************
 * Port of androidx.navigation.NavDeepLink (rewritten). Compiles a uriPattern
 * with {arg} placeholders into a regex; matches() tests a uri, getMatchingArguments()
 * extracts placeholder values (typed via NavArgument when provided).
 * Keeps the legacy (string)-only overloads so existing navdestination.cc compiles.
 *********************************************************************************/
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <core/bundle.h>
namespace cdroid{

class NavArgument;

class NavDeepLink{
public:
    explicit NavDeepLink(const std::string& uriPattern);
    const std::string& getUriPattern() const { return mUriPattern; }
    bool isExactDeepLink() const;
    bool matches(const std::string& deepLink) const;
    // Legacy: no type map; placeholders returned as strings.
    Bundle* getMatchingArguments(const std::string& deepLink);
    // Modern: typed extraction via the destination's arguments map.
    Bundle* getMatchingArguments(const std::string& deepLink,
                                 const std::map<std::string, NavArgument*>& arguments) const;
private:
    void buildRegex(const std::string& pattern);
    std::string mUriPattern;
    std::regex mRegex;
    std::vector<std::string> mArgNames;
};

}//namespace cdroid
#endif/*__NAV_DEEPLINK_H__*/
