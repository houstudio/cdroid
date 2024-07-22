#include <navigation/simplenavigatorprovider.h>

namespace cdroid{

Navigator*SimpleNavigatorProvider::getNavigator(const std::string& name) {
    LOGE_IF(!validateName(name),"navigator name cannot be an empty string");
    Navigator* /*<? extends NavDestination>*/navigator = mNavigators.find(name)->second;
    FATAL_IF(navigator == nullptr,"Could not find Navigator with name "
        "You must call NavController.addNavigator() for each navigation type.",name.c_str());
    return  navigator;
}

Navigator*SimpleNavigatorProvider::addNavigator(Navigator*navigator) {
    std::string name = "";//getNavigator("");//getNameForNavigator(navigator->getClass());
    FATAL("do not call this function");
    return addNavigator(name, navigator);
}

Navigator*SimpleNavigatorProvider::addNavigator(const std::string& name,Navigator*navigator) {
    FATAL_IF(!validateName(name),"navigator name cannot be an empty string");
    mNavigators.insert({name, navigator});
    return navigator;
}

const std::map<const std::string, Navigator*>& SimpleNavigatorProvider::getNavigators() {
    return mNavigators;
}

bool SimpleNavigatorProvider::validateName(const std::string& name) {
    return !name.empty();
}

}/*endof namespace*/

