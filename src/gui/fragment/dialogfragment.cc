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
#include <fragment/dialogfragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>

namespace cdroid{
namespace fragment{

DialogFragment::DialogFragment(){
}

DialogFragment::~DialogFragment(){
    // Dialog is owned by this fragment (created in onCreateDialog via show()).
}

void DialogFragment::show(FragmentManager* manager, const std::string& tag){
    if(!manager) return;
    FragmentTransaction* t = manager->beginTransaction();
    t->add(this, tag);
    t->commit();
    if(mShowsDialog){
        mDialog = onCreateDialog(nullptr);
        if(mDialog) mDialog->show();
    }
}

void DialogFragment::dismiss(){
    if(mDialog){
        mDialog->dismiss();
    }
    if(mFragmentManager){
        FragmentTransaction* t = mFragmentManager->beginTransaction();
        t->remove(this);
        t->commit();
    }
}

}//namespace fragment
}//namespace cdroid
