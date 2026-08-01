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
 * The dialog lifecycle is driven by the fragment lifecycle: onStart shows, onStop hides,
 * onDestroyView dismisses. Style/theme/cancelable/showsDialog are saved+restored.
 *********************************************************************************/
#include <string>
#include <fragment/fragment.h>
#include <app/dialog.h>
#include <app/dialoginterface.h>
namespace cdroid{
namespace fragment{

class FragmentManager;
class FragmentTransaction;

class DialogFragment : public Fragment{
public:
    // androidx DialogFragment style constants (DialogFragment.java:319-339).
    static const int STYLE_NORMAL   = 0;
    static const int STYLE_NO_TITLE = 1;
    static const int STYLE_NO_FRAME = 2;
    static const int STYLE_NO_INPUT = 3;

    DialogFragment();
    ~DialogFragment() override;

    // androidx DialogFragment.setStyle (DialogFragment.java:471): call before the dialog is shown.
    void setStyle(int style, int theme);
    int getTheme() const { return mTheme; }

    // Whether to show as a dialog (true) or as an embedded fragment (false).
    void setShowsDialog(bool showsDialog);
    bool getShowsDialog() const { return mShowsDialog; }

    // Cancelable (back/cancel dismisses the dialog).
    void setCancelable(bool cancelable);
    bool isCancelable() const { return mCancelable; }

    // Override to create the Dialog for this fragment (androidx onCreateDialog :883).
    virtual cdroid::Dialog* onCreateDialog(cdroid::Bundle* savedInstanceState){
        (void)savedInstanceState;
        return new cdroid::Dialog(getContext());
    }

    // Show the dialog by adding this fragment (androidx show :496).
    void show(FragmentManager* manager, const std::string& tag);
    // Show via a caller-provided transaction (androidx show :514); returns the back-stack id.
    int show(FragmentTransaction* transaction, const std::string& tag);
    // Show synchronously (androidx showNow :534).
    void showNow(FragmentManager* manager, const std::string& tag);

    // Dismiss the dialog + fragment (androidx dismiss :549 / dismissAllowingStateLoss :568).
    void dismiss();
    void dismissAllowingStateLoss();

    cdroid::Dialog* getDialog() const { return mDialog; }
    cdroid::Dialog* requireDialog();

    // DialogInterface callbacks (androidx onCancel :891 / onDismiss :896). Subclasses override.
    virtual void onCancel(cdroid::DialogInterface* dialog){ (void)dialog; }
    virtual void onDismiss(cdroid::DialogInterface* dialog){ (void)dialog; }

    // Lifecycle overrides (androidx DialogFragment lifecycle hooks).
    void onCreate(cdroid::Bundle* savedInstanceState) override;
    void onStart() override;
    void onStop() override;
    void onDestroyView() override;
    void onSaveInstanceState(cdroid::Bundle* outState) override;

private:
    cdroid::Dialog* mDialog = nullptr;
    bool mDialogCreated = false;
    bool mCreatingDialog = false;  // recursion guard for onCreateDialog
    bool mShowsDialog = true;
    bool mCancelable = true;
    int mStyle = STYLE_NORMAL;
    int mTheme = 0;
    bool mViewDestroyed = false;
    bool mDismissed = false;
    bool mShownByMe = false;
    int mBackStackId = -1;
    cdroid::Bundle* mDialogState = nullptr;  // saved dialog state for restore (owned)

    // androidx DialogFragment.dismissInternal (:572).
    void dismissInternal(bool allowStateLoss, bool fromOnDismiss);
    // androidx DialogFragment.prepareDialog (:909): lazily create + configure the Dialog.
    void prepareDialog(cdroid::Bundle* savedInstanceState);
    // androidx DialogFragment.setupDialog (:845): apply window features per mStyle.
    void setupDialog(cdroid::Dialog* dialog, int style);
};

}//namespace fragment
}//namespace cdroid
#endif
