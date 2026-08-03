/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __NAVIGATOR_PROVIDER_H__
#define __NAVIGATOR_PROVIDER_H__
#include <navigation/navigator.h>
namespace cdroid{
class NavigatorProvider {
public:
    // Virtual so that `delete mNavigatorProvider` (NavController owns a SimpleNavigatorProvider via
    // a NavigatorProvider*) runs the derived dtor — otherwise SimpleNavigatorProvider's mNavigators
    // (the owned Navigator* map) would leak.
    virtual ~NavigatorProvider() = default;
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
