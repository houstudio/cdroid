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
#ifndef __HASDEFAULTVIEWMODELPROVIDERFACTORY_H__
#define __HASDEFAULTVIEWMODELPROVIDERFACTORY_H__
/*********************************************************************************
 * Port of androidx.lifecycle.HasDefaultViewModelProviderFactory. The default
 * factory cannot reflectively instantiate ViewModels in C++, so owners are
 * expected to override getDefaultViewModelProviderFactory (Fragment does this
 * with SavedStateViewModelFactory).
 *********************************************************************************/
#include <lifecycle/viewmodelprovider.h>
#include <lifecycle/creationextras.h>
namespace cdroid{
namespace lifecycle{

class HasDefaultViewModelProviderFactory{
public:
    virtual ~HasDefaultViewModelProviderFactory() = default;
    virtual ViewModelProvider::Factory& getDefaultViewModelProviderFactory();
    virtual CreationExtras& getDefaultViewModelCreationExtras();
};

}//namespace lifecycle
}//namespace cdroid
#endif
