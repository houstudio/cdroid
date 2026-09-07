cmake_minimum_required (VERSION 3.8)
project(deskclock)

set(CMAKE_CXX_STANDARD 14)

# CDROID includes/libraries come from the parent scope (all apps/ projects
# configure under the cdroid tree; no find_package needed — see music).
include_directories(
    ./
    src
    ${CDROID_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/porting
)

set(DESKCLOCK_SRCS
    main.cc
    src/deskclock.cc
    src/logutils.cc
    src/animatorutils.cc
    src/itemanimator.cc
    src/timezone.cc
    src/utils.cc
    src/screensaversettingsactivity.cc
    src/circleview.cc
    src/circlebuttonslayout.cc
    src/analogclock.cc
    src/floatingactionbutton.cc
    src/uidata.cc
    src/timer.cc
    src/stopwatch.cc
    src/timerdao.cc
    src/stopwatchdao.cc
    src/city.cc
    src/weekdays.cc
    src/settingsdao.cc
    src/timermodel.cc
    src/stopwatchmodel.cc
    src/citymodel.cc
    src/datamodel.cc
    src/texttime.cc
    src/deskclockfragment.cc
    src/actionbarmenu.cc
    src/dropshadowcontroller.cc
    src/fragmenttabpageradapter.cc
    src/tabfragments.cc
    src/clockfragment.cc
    src/autosizingviews.cc
    src/ellipsizelayout.cc
    src/stopwatchcircleview.cc
    src/stopwatchtextcontroller.cc
    src/stopwatchlandscapelayout.cc
    src/lapsadapter.cc
    src/stopwatchfragment.cc
    src/verticalviewpager.cc
    src/timercircleframelayout.cc
    src/timercircleview.cc
    src/timertextcontroller.cc
    src/timerstringformatter.cc
    src/timeritem.cc
    src/timersetupview.cc
    src/timerklaxon.cc
    src/timeritemfragment.cc
    src/expiredtimersactivity.cc
    src/timerpageradapter.cc
    src/timerfragment.cc
    src/labeldialogfragment.cc
    src/searchmenuitemcontroller.cc
    src/cityselectionactivity.cc
    src/alarm.cc
    src/alarminstance.cc
    src/alarmstatemanager.cc
    src/alarmupdatehandler.cc
    src/alarmtimeclickhandler.cc
    src/alarmutils.cc
    src/timepickerdialogfragment.cc
    src/alarmitemholder.cc
    src/alarmitemviewholder.cc
    src/collapsedalarmviewholder.cc
    src/expandedalarmviewholder.cc
    src/alarmclockfragment.cc
    src/alarmrecyclerview.cc
    src/settingsactivity.cc
    src/screensaveractivity.cc
)

add_executable(deskclock ${DESKCLOCK_SRCS})

# App pak: the DeskClock res subset (layouts/drawables/strings from AOSP
# packages/apps/DeskClock/res), compiled to binary AXML/arsc and loaded at
# runtime as deskclock.pak beside the binary.
CreatePAK(deskclock ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/deskclock.pak ${PROJECT_SOURCE_DIR}/R.h)

target_link_libraries(deskclock PRIVATE cdroid)
install(TARGETS deskclock DESTINATION bin)
