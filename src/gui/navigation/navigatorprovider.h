#ifndef __NAVIGATOR_PROVIDER_H__
#define __NAVIGATOR_PROVIDER_H__
#include <navigation/navigator.h>
namespace cdroid{
class NavigatorProvider {
public:
    /**
     * Retrieves a registered {@link Navigator} by name.
     *
     * @param name name of the navigator to return
     * @return the registered navigator with the given name
     *
     * @throws IllegalStateException if the Navigator has not been added
     *
     * @see #addNavigator(String, Navigator)
     */
    virtual Navigator* getNavigator(const std::string& name)=0;

    /**
     * Register a navigator using the name provided by the
     * {@link Navigator.Name Navigator.Name annotation}. {@link NavDestination destinations} may
     * refer to any registered navigator by name for inflation. If a navigator by this name is
     * already registered, this new navigator will replace it.
     *
     * @param navigator navigator to add
     * @return the previously added Navigator for the name provided by the
     * {@link Navigator.Name Navigator.Name annotation}, if any
     */
    virtual Navigator*addNavigator(Navigator*navigator)=0;

    /**
     * Register a navigator by name. {@link NavDestination destinations} may refer to any
     * registered navigator by name for inflation. If a navigator by this name is already
     * registered, this new navigator will replace it.
     *
     * @param name name for this navigator
     * @param navigator navigator to add
     * @return the previously added Navigator for the given name, if any
     */
    virtual Navigator*addNavigator(const std::string& name,Navigator*navigator)=0;
};
}/*endof namespace*/
#endif /*__NAVIGATOR_PROVIDER_H__*/
