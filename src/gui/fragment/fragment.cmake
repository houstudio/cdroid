# androidx.fragment.app port + CDROID host bridge (FragmentWindow).
if(ENABLE_FRAGMENT)
SET(FRAGMENT_SOURCES
    fragment/fragmentfactory.cc
    fragment/fragmentmanagerviewmodel.cc
    fragment/fragment.cc
    fragment/fragmentmanager.cc
    fragment/fragmenttransaction.cc
    fragment/backstackrecord.cc
    fragment/fragmentwindow.cc
    fragment/listfragment.cc
    fragment/dialogfragment.cc
    # Next (2b-5): apps/samples fragment_demo run verification
)
endif()
