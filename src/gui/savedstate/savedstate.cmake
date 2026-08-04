# androidx.savedstate foundation (SavedStateRegistry + SavedStateHandle family).
# SavedState is backed by CDROID's existing Bundle (core/bundle.h : BaseBundle).
if(ENABLE_SAVEDSTATE)
SET(SAVEDSTATE_SOURCES
    savedstate/savedstateregistry.cc
    savedstate/savedstateregistrycontroller.cc
    savedstate/savedstatehandle.cc
    savedstate/savedstateviewmodelfactory.cc
    savedstate/savedstatehandlesupport.cc
    # savedstatehandle / viewmodel-savedstate family added when Fragment wires VMs
)
endif()
