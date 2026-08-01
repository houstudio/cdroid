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
#ifndef __FRAGMENTHOSTCALLBACK_H__
#define __FRAGMENTHOSTCALLBACK_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentHostCallback<H>. CDROID host type is
 * fixed to Window (typedef Window Activity), so this is non-template: onGetHost()
 * returns cdroid::Window*. Subclassed by the host (FragmentActivity::HostCallbacks).
 *********************************************************************************/
#include <fragment/fragmentcontainer.h>
namespace cdroid{
class Context;
class Handler;
class LayoutInflater;
class Window;
namespace fragment{

class FragmentHostCallback : public FragmentContainer{
public:
    ~FragmentHostCallback() override = default;
    virtual cdroid::Context* getContext() = 0;
    virtual cdroid::Handler* getHandler() = 0;
    virtual cdroid::LayoutInflater* onGetLayoutInflater() = 0;
    virtual cdroid::Window* onGetHost() = 0;
    virtual int onGetWindowAnimations() { return 0; }
    // Host refreshes the options menu (Fragment.setHasOptionsMenu/setMenuVisibility hook here).
    virtual void onSupportInvalidateOptionsMenu() {}
    // FragmentContainer
    cdroid::View* onFindViewById(int id) override = 0;
    bool onHasView() override = 0;
};

}//namespace fragment
}//namespace cdroid
#endif
