
include(core/core.cmake)
include(view/view.cmake)
include(animation/animations.cmake)
include(drawables/drawables.cmake)
include(widget/widget.cmake)
include(menu/menu.cmake)
include(widgetEx/widgetex.cmake)
include(widgetEx/wear/wear.cmake)

list(APPEND CDROID_SOURCES
    private/inputeventlabels.cc
    ${CORE_SOURCES}
    ${VIEW_SOURCES}
    ${ANIMATION_SOURCES}
    ${DRAWABLE_SOURCES}
    ${WIDGET_SOURCES}
    ${MENU_SOURCES}
    ${WIDGETEX_SOURCES}
    ${WEAR_SOURCES}
)

if(ENABLE_AUDIO)
    list(APPEND CDROID_SOURCES
        media/audiomanager.cc
        media/soundpool.cc
        media/audiorecord.cc
        )
endif()

if(ENABLE_GESTURE)
    list(APPEND CDROID_SOURCES
        gesture/gesture.cc
        gesture/gesturelibraries.cc
        gesture/gestureoverlayview.cc
        gesture/gesturestore.cc
        gesture/gesturestroke.cc
        gesture/gestureutils.cc
        gesture/instance.cc
        gesture/instancelearner.cc
        gesture/learner.cc
        gesture/orientedboundingbox.cc
    )
endif(ENABLE_GESTURE)

if(ENABLE_DIALOGS OR ENABLE_SPINNER)
    list(APPEND CDROID_SOURCES
        app/alertcontroller.cc
        app/alertdialog.cc
        app/dialog.cc
        app/progressdialog.cc
    )
endif(ENABLE_DIALOGS)


if(ENABLE_TRANSITION)
    list(APPEND CDROID_SOURCES
        transition/scene.cc
    )
endif()

if(ENABLE_NAVIGATION)
    list(APPEND CDROID_SOURCES
        navigation/navaction.cc
        navigation/navcontroller.cc
        navigation/navdeeplink.cc
        navigation/navdeeplinkbuilder.cc
        navigation/navdestination.cc
        navigation/navgraph.cc
        navigation/navgraphnavigator.cc
        navigation/navigation.cc
        navigation/navigator.cc
        navigation/navinflater.cc
        navigation/navoptions.cc
        navigation/simplenavigatorprovider.cc
    )
endif(ENABLE_NAVIGATION)

