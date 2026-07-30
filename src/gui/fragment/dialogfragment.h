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
