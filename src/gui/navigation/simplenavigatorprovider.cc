#include <navigation/simplenavigatorprovider.h>

namespace cdroid{

Navigator*SimpleNavigatorProvider::getNavigator(const std::string& name) {
    if (!validateName(name)) {
        throw ("navigator name cannot be an empty string");
    }

    Navigator* /*<? extends NavDestination>*/navigator = mNavigators.find(name)->second;
    if (navigator == nullptr) {
        /*throw new IllegalStateException("Could not find Navigator with name \"" + name
                + "\". You must call NavController.addNavigator() for each navigation type.");*/
    }
    return  navigator;
}

Navigator*SimpleNavigatorProvider::addNavigator(Navigator*navigator) {
    std::string name = "";//getNavigator("");//getNameForNavigator(navigator->getClass());
    return addNavigator(name, navigator);
}

Navigator*SimpleNavigatorProvider::addNavigator(const std::string& name,Navigator*navigator) {
    if (!validateName(name)) {
        //throw new IllegalArgumentException("navigator name cannot be an empty string");
    }
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

