# androidx.fragment.app port + CDROID host bridge (FragmentActivity).
if(ENABLE_FRAGMENT)
SET(FRAGMENT_SOURCES
    fragment/fragmentfactory.cc
    fragment/fragmentmanagerviewmodel.cc
    fragment/fragment.cc
    fragment/fragmentmanager.cc
    fragment/fragmentstatemanager.cc
    fragment/fragmenttransaction.cc
    fragment/backstackrecord.cc
    fragment/fragmentactivity.cc
    fragment/listfragment.cc
    fragment/dialogfragment.cc
    fragment/fragmentanim.cc
    fragment/fragmenttransitionimpl.cc
    fragment/fragmentviewlifecycleowner.cc
    # Next (2b-5): apps/samples fragment_demo run verification
)
endif()
