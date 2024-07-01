#ifndef __SIMPLE_NAVIGATOR_PROVIDER_H__
#define __SIMPLE_NAVIGATOR_PROVIDER_H__
#include <navigation/navigatorprovider.h>
namespace cdroid{

class SimpleNavigatorProvider :public NavigatorProvider {
private:
    std::map<const std::string, Navigator*> mNavigators;
    bool validateName(const std::string& name);
public:
    Navigator*getNavigator(const std::string& name) override;

    Navigator*addNavigator(Navigator*navigator)override;
    Navigator*addNavigator(const std::string& name,Navigator*navigator)override;
    const std::map<const std::string, Navigator*>& getNavigators();
};
}/*endof namespace*/
#endif/*__SIMPLE_NAVIGATOR_PROVIDER_H__*/

