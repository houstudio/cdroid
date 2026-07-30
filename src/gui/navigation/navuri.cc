#include <navigation/navuri.h>

namespace cdroid{

static std::vector<std::string> split(const std::string& s, char delim){
    std::vector<std::string> out;
    std::string cur;
    for(char c : s){
        if(c == delim){ out.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    out.push_back(cur);
    return out;
}

NavUri::NavUri(const std::string& uriString) : mUri(uriString){
    // Split off fragment (#).
    std::string rest = uriString;
    size_t hash = rest.find('#');
    if(hash != std::string::npos){
        mFragment = rest.substr(hash + 1);
        rest = rest.substr(0, hash);
    }
    // Split off query (?).
    size_t q = rest.find('?');
    if(q != std::string::npos){
        mQuery = rest.substr(q + 1);
        rest = rest.substr(0, q);
    }
    mPath = rest;
    // Path segments: strip scheme://host if present, then split on '/'.
    std::string p = mPath;
    size_t scheme = p.find("://");
    if(scheme != std::string::npos){
        p = p.substr(scheme + 3);
        size_t slash = p.find('/');
        p = (slash == std::string::npos) ? std::string() : p.substr(slash);
    }
    for(const std::string& seg : split(p, '/')){
        if(!seg.empty()) mPathSegments.push_back(seg);
    }
    // Query parameters.
    if(!mQuery.empty()){
        for(const std::string& pair : split(mQuery, '&')){
            size_t eq = pair.find('=');
            if(eq != std::string::npos){
                std::string k = pair.substr(0, eq);
                std::string v = pair.substr(eq + 1);
                mQueryParams[k].push_back(v);
            }
        }
    }
}

std::vector<std::string> NavUri::getQueryParameters(const std::string& key) const{
    auto it = mQueryParams.find(key);
    if(it == mQueryParams.end()) return {};
    return it->second;
}

}//namespace cdroid
