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
