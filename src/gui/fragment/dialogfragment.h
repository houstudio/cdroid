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
#ifndef __DIALOGFRAGMENT_H__
#define __DIALOGFRAGMENT_H__
/*********************************************************************************
 * Port of androidx.fragment.app.DialogFragment. A Fragment that displays a Dialog;
 * subclasses override onCreateDialog to supply the Dialog, then call show()/dismiss().
 *********************************************************************************/
#include <string>
#include <fragment/fragment.h>
#include <app/dialog.h>
namespace cdroid{
namespace fragment{

class FragmentManager;

class DialogFragment : public Fragment{
public:
    DialogFragment();
    ~DialogFragment() override;

    // Override to create the Dialog for this fragment.
    virtual cdroid::Dialog* onCreateDialog(cdroid::Bundle* savedInstanceState){ (void)savedInstanceState; return nullptr; }

    // Show the dialog (adds the fragment + shows the dialog).
    void show(FragmentManager* manager, const std::string& tag);
    // Dismiss the dialog and the fragment.
    void dismiss();

    void setShowsDialog(bool shows){ mShowsDialog = shows; }
    cdroid::Dialog* getDialog() const { return mDialog; }
private:
    cdroid::Dialog* mDialog = nullptr;
    bool mShowsDialog = true;
};

}//namespace fragment
}//namespace cdroid
#endif
