# androidx.navigation port (modern NavBackStackEntry + route + NavType model).
# NOTE: navcontroller.cc / navdeeplinkbuilder.cc are currently non-compiling
# Java pastes (#if-guarded); they are rewritten under Stage 3. Kept here for
# source-list parity until then; ENABLE_NAVIGATION stays OFF until Stage 3.
if(ENABLE_NAVIGATION)
SET(NAVIGATION_SOURCES
    navigation/navaction.cc
    navigation/navtype.cc
    navigation/navargument.cc
    navigation/navuri.cc
    navigation/navbackstackentry.cc
    navigation/navigatorstate.cc
    navigation/noopnavigator.cc
    navigation/navcontroller.cc         # rewritten (modern route model) in stage 3-5
    navigation/fragmentnavigator.cc     # stage 4: navigation-fragment
    navigation/navhostfragment.cc       # stage 4: navigation-fragment
    navigation/navdeeplink.cc              # stub body, rewritten in stage 3-2
    #navigation/navdeeplinkbuilder.cc      # dead (#if 0 Java paste), rewritten in stage 3-6
    navigation/navdestination.cc
    navigation/navgraph.cc
    navigation/navgraphnavigator.cc
    navigation/navigation.cc
    navigation/navigator.cc
    navigation/navinflater.cc
    navigation/navoptions.cc
    navigation/simplenavigatorprovider.cc
)
endif()
