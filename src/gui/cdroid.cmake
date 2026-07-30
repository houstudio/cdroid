
include(core/core.cmake)
include(view/view.cmake)
include(animation/animations.cmake)
include(drawable/drawables.cmake)
include(widget/widget.cmake)
include(menu/menu.cmake)
include(transition/transition.cmake)
include(lifecycle/lifecycle.cmake)
include(savedstate/savedstate.cmake)
include(fragment/fragment.cmake)
include(widgetEx/widgetex.cmake)
include(navigation/navigation.cmake)

list(APPEND CDROID_SOURCES
    private/inputeventlabels.cc
    ${CORE_SOURCES}
    ${VIEW_SOURCES}
    ${ANIMATION_SOURCES}
    ${DRAWABLE_SOURCES}
    ${WIDGET_SOURCES}
    ${MENU_SOURCES}
    ${TRANSITION_SOURCES}
    ${LIFECYCLE_SOURCES}
    ${SAVEDSTATE_SOURCES}
    ${FRAGMENT_SOURCES}
    ${WIDGETEX_SOURCES}
    ${WEAR_SOURCES}
    ${NAVIGATION_SOURCES}
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

