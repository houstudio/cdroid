#ifndef __NAVURI_H__
#define __NAVURI_H__
/*********************************************************************************
 * Port of androidx.navigation.NavUri. Platform-agnostic URI (path?query#fragment)
 * with segment/query-parameter access. Used by NavDeepLink matching.
 *********************************************************************************/
#include <string>
#include <vector>
#include <map>
namespace cdroid{

class NavUri{
public:
    explicit NavUri(const std::string& uriString);
    const std::string& getFragment() const { return mFragment; }
    const std::string& getQuery() const { return mQuery; }
    const std::vector<std::string>& getPathSegments() const { return mPathSegments; }
    std::vector<std::string> getQueryParameters(const std::string& key) const;
    const std::string& toString() const { return mUri; }
    static NavUri* parse(const std::string& s){ return new NavUri(s); }
private:
    std::string mUri;
    std::string mPath;
    std::string mQuery;
    std::string mFragment;
    std::vector<std::string> mPathSegments;
    std::map<std::string, std::vector<std::string>> mQueryParams;
};

}//namespace cdroid
#endif
