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
#ifndef __FRAGMENTFACTORY_H__
#define __FRAGMENTFACTORY_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentFactory. C++ has no reflection, so the
 * ClassLoader/Class.forName instantiate() is replaced by an explicit registry of
 * class name -> factory lambda. Register a Fragment via REGISTER_FRAGMENT(T) in
 * its .cc, or call FragmentFactory::registerFragment directly.
 *********************************************************************************/
#include <string>
#include <functional>
namespace cdroid{
namespace fragment{

class Fragment;

class FragmentFactory{
public:
    virtual ~FragmentFactory() = default;

    // Registers a constructor under the given (fully-qualified) class name.
    static void registerFragment(const std::string& className, std::function<Fragment*()> ctor);
    // Returns true if className is a registered Fragment class.
    static bool isFragmentClass(const std::string& className);
    // Creates a new Fragment instance by class name (throws-equivalent: null + log).
    virtual Fragment* instantiate(const std::string& className);
};

// Self-registration helper. Place `REGISTER_FRAGMENT(MyFragment)` in the Fragment's
// .cc to register it under its class name with the default constructor.
#define REGISTER_FRAGMENT(ClassName) \
    static const int _cdroid_frag_reg_##ClassName = \
        (::cdroid::fragment::FragmentFactory::registerFragment( \
             #ClassName, []() -> ::cdroid::fragment::Fragment* { return new ClassName(); }), 0)

}//namespace fragment
}//namespace cdroid
#endif
