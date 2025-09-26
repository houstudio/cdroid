
include(core/core.cmake)
include(view/view.cmake)
include(widget/widget.cmake)
include(widgetEx/widgetex.cmake)

list(APPEND CDROID_SOURCES
    private/inputeventlabels.cc
)
list(APPEND CDROID_SOURCES ${CORE_SOURCES})
list(APPEND CDROID_SOURCES ${VIEW_SOURCES})
list(APPEND CDROID_SOURCES ${WIDGET_SOURCES})
list(APPEND CDROID_SOURCES ${WIDGETEX_SOURCES})
if(ENABLE_AUDIO)
    list(APPEND CDROID_SOURCES
        media/audiomanager.cc
        media/soundpool.cc
        media/audiorecord.cc
        )
endif()

if(ENABLE_MENU)
    list(APPEND CDROID_SOURCES
        menu/basemenupresenter.cc
        menu/iconmenuitemview.cc
        menu/iconmenuview.cc
        menu/listmenupresenter.cc
        menu/menudialoghelper.cc
        menu/menupopuphelper.cc
        menu/menuitem.cc
        menu/actionmenuitem.cc
        menu/expandedmenuview.cc
        menu/iconmenupresenter.cc
        menu/listmenuitemview.cc
        menu/actionmenuview.cc
        menu/actionmenuitemview.cc
        menu/actionmenupresenter.cc
        menu/menubuilder.cc
        menu/contextmenubuilder.cc
        menu/menuinflater.cc
        menu/menuitemimpl.cc
        menu/menupopup.cc
        menu/popupmenu.cc
        menu/actionmenu.cc
        menu/menuadapter.cc
        menu/standardmenupopup.cc
        menu/cascadingmenupopup.cc
        widget/menupopupwindow.cc
    )
endif(ENABLE_MENU)

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

if(ENABLE_WEAR OR TRUE)
    include(widgetEx/wear/wear.cmake)
    list(APPEND CDROID_SOURCES ${WEAR_SOURCES})
endif()
