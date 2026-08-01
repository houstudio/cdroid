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
#include <fragment/fragmentfactory.h>
#include <fragment/fragment.h>
#include <porting/cdlog.h>
#include <unordered_map>

namespace cdroid{
namespace fragment{

namespace{
// Lazily-initialized global registry (single UI thread in CDROID).
std::unordered_map<std::string, std::function<Fragment*()>>& registry(){
    static std::unordered_map<std::string, std::function<Fragment*()>> sRegistry;
    return sRegistry;
}
}//anonymous

void FragmentFactory::registerFragment(const std::string& className, std::function<Fragment*()> ctor){
    registry()[className] = std::move(ctor);
}

bool FragmentFactory::isFragmentClass(const std::string& className){
    return registry().count(className) > 0;
}

Fragment* FragmentFactory::instantiate(const std::string& className){
    auto it = registry().find(className);
    if(it == registry().end()){
        LOGE("FragmentFactory: no Fragment registered for class %s "
             "(use REGISTER_FRAGMENT or FragmentFactory::registerFragment)", className.c_str());
        return nullptr;
    }
    Fragment* f = it->second();
    if(f) f->mClassName = className; // stamp for FragmentState (androidx Fragment.getClass().getName())
    return f;
}

}//namespace fragment
}//namespace cdroid
