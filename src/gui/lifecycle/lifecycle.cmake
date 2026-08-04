# androidx.lifecycle + androidx.lifecycle.viewmodel foundation.
# Ported to C++ to provide the Lifecycle/ViewModel substrate required by
# Fragment and modern Navigation. Kotlin coroutines/Flow/serialization and
# reflective @OnLifecycleEvent are intentionally omitted (see plan Stage 1).
if(ENABLE_LIFECYCLE)
SET(LIFECYCLE_SOURCES
    lifecycle/lifecycle.cc
    lifecycle/lifecycling.cc
    lifecycle/lifecycleregistry.cc
    lifecycle/viewmodel.cc
    lifecycle/viewmodelstore.cc
    lifecycle/creationextras.cc
    lifecycle/viewmodelprovider.cc
    lifecycle/hasdefaultviewmodelproviderfactory.cc
)
endif()
