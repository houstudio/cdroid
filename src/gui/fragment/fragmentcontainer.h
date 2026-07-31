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
#ifndef __FRAGMENTCONTAINER_H__
#define __FRAGMENTCONTAINER_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentContainer. Callbacks to a Fragment's
 * container (locate views). The deprecated instantiate(Context,String,Bundle) is
 * omitted; CDROID instantiates Fragments via FragmentFactory's registry.
 *********************************************************************************/
#include <view/view.h>
namespace cdroid{
namespace fragment{

class FragmentContainer{
public:
    virtual ~FragmentContainer() = default;
    // Returns the view with the given id, or null if not a child of this container.
    virtual cdroid::View* onFindViewById(int id) = 0;
    // Returns true if the container holds any view.
    virtual bool onHasView() = 0;
};

}//namespace fragment
}//namespace cdroid
#endif
