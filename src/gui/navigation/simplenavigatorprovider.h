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

