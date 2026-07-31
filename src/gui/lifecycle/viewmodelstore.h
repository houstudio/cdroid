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
#ifndef __VIEWMODELSTORE_H__
#define __VIEWMODELSTORE_H__
/*********************************************************************************
 * Port of androidx.lifecycle.ViewModelStore. Owns the ViewModel instances put
 * into it and clears them on destruction/clear().
 *********************************************************************************/
#include <string>
#include <unordered_map>
#include <lifecycle/viewmodel.h>
namespace cdroid{
namespace lifecycle{

class ViewModelStore{
public:
    ViewModelStore() = default;
    ~ViewModelStore();

    // Stores viewModel under key (ownership transferred); clears any previous.
    void put(const std::string& key, ViewModel* viewModel);
    // Returns the ViewModel under key (borrowed), or null.
    ViewModel* get(const std::string& key);
    // Clears and deletes all stored ViewModels.
    void clear();

private:
    std::unordered_map<std::string, ViewModel*> mMap;
};

}//namespace lifecycle
}//namespace cdroid
#endif
