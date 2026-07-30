#include <navigation/navdeeplink.h>
#include <navigation/navargument.h>
#include <navigation/navtype.h>

namespace cdroid{

NavDeepLink::NavDeepLink(const std::string& uriPattern) : mUriPattern(uriPattern){
    buildRegex(uriPattern);
}

void NavDeepLink::buildRegex(const std::string& pattern){
    // Convert {arg} placeholders into capture groups; escape regex metacharacters.
    static const std::string kSpecial = "\\^$.|?*+()[]";
    std::string regexStr = "^";
    for(size_t i = 0; i < pattern.size();){
        char c = pattern[i];
        if(c == '{'){
            size_t end = pattern.find('}', i);
            if(end != std::string::npos){
                mArgNames.push_back(pattern.substr(i + 1, end - i - 1));
                regexStr += "([^/#?]+)";
                i = end + 1;
                continue;
            }
        }
        if(kSpecial.find(c) != std::string::npos) regexStr += '\\';
        regexStr += c;
        i++;
    }
    regexStr += "$";
    mRegex = std::regex(regexStr);
}

bool NavDeepLink::isExactDeepLink() const{
    return mUriPattern.find('{') == std::string::npos;
}

bool NavDeepLink::matches(const std::string& deepLink) const{
    try{
        return std::regex_match(deepLink, mRegex);
    }catch(...){
        return false;
    }
}

Bundle* NavDeepLink::getMatchingArguments(const std::string& deepLink){
    return getMatchingArguments(deepLink, std::map<std::string, NavArgument*>());
}

Bundle* NavDeepLink::getMatchingArguments(const std::string& deepLink,
                                          const std::map<std::string, NavArgument*>& arguments) const{
    std::smatch sm;
    if(!std::regex_match(deepLink, sm, mRegex)) return nullptr;
    Bundle* result = new Bundle();
    for(size_t i = 0; i < mArgNames.size() && (i + 1) < sm.size(); i++){
        const std::string& name = mArgNames[i];
        const std::string value = sm[i + 1].str();
        auto it = arguments.find(name);
        if(it != arguments.end() && it->second){
            switch(it->second->getType()){
                case NavTypeKind::INT:    result->putInt(name,    IntType().parseValue(value)); break;
                case NavTypeKind::LONG:   result->putLong(name,   LongType().parseValue(value)); break;
                case NavTypeKind::FLOAT:  result->putFloat(name,  FloatType().parseValue(value)); break;
                case NavTypeKind::BOOL:   result->putBoolean(name, BoolType().parseValue(value)); break;
                default:                  result->putString(name, value); break;
            }
        }else{
            result->putString(name, value);
        }
    }
    return result;
}

}//namespace cdroid
