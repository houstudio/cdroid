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
    src/asyncringtoneplayer.cc
    src/alarmklaxon.cc
    src/ringtonepreviewklaxon.cc
    src/ringtonebackend.cc
    src/alarmactivity.cc
    src/customringtone.cc
    src/customringtonedao.cc
    src/ringtonemodel.cc
    src/headerholder.cc
    src/ringtoneholder.cc
    src/systemringtoneholder.cc
    src/customringtoneholder.cc
    src/addcustomringtoneholder.cc
    src/headerviewholder.cc
    src/ringtoneviewholder.cc
    src/addcustomringtoneviewholder.cc
    src/ringtoneloader.cc
    src/ringtonepickeractivity.cc
)

# Real audio backend for the ringtone stack (AsyncRingtonePlayer -> FFmpeg
# decode + RtAudio output), detected exactly like remusic's: the vcpkg triplet
# the CURRENT toolchain uses, falling back to probing every installed triplet.
# Absent everywhere -> the player degrades to its logging stub.
# Board bring-up: vcpkg install ffmpeg "rtaudio[alsa]" --triplet <board>.
set(_VCPKG_ROOT "$ENV{VCPKG_ROOT}")
if(NOT _VCPKG_ROOT AND DEFINED ENV{VCPKG_INSTALLATION_ROOT})
    set(_VCPKG_ROOT "$ENV{VCPKG_INSTALLATION_ROOT}")
endif()
if(NOT _VCPKG_ROOT AND EXISTS "/home/houzh/vcpkg/installed")
    set(_VCPKG_ROOT "/home/houzh/vcpkg")
endif()
set(_CAND_TRIPLETS "")
if(DEFINED ENV{VCPKG_TARGET_TRIPLET})
    list(APPEND _CAND_TRIPLETS "$ENV{VCPKG_TARGET_TRIPLET}")
endif()
if(_VCPKG_ROOT AND EXISTS "${_VCPKG_ROOT}/installed")
    file(GLOB _INSTALLED_TRIPLETS RELATIVE "${_VCPKG_ROOT}/installed" "${_VCPKG_ROOT}/installed/*-linux*")
    list(APPEND _CAND_TRIPLETS ${_INSTALLED_TRIPLETS})
endif()
foreach(_t ${_CAND_TRIPLETS})
    if(NOT DESKCLOCK_FFMPEG AND _VCPKG_ROOT
            AND EXISTS "${_VCPKG_ROOT}/installed/${_t}/include/libavcodec/avcodec.h"
            AND EXISTS "${_VCPKG_ROOT}/installed/${_t}/include/rtaudio")
        set(_VCPKG_TRIPLET "${_VCPKG_ROOT}/installed/${_t}")
        set(DESKCLOCK_FFMPEG ON)
        set(DESKCLOCK_TRIPLET_USED "${_t}")
    endif()
endforeach()
if(DESKCLOCK_FFMPEG)
    message(STATUS "deskclock: ringtone audio backend ON (triplet ${DESKCLOCK_TRIPLET_USED})")
    add_definitions(-DDESKCLOCK_ENABLE_FFMPEG)
    include_directories("${_VCPKG_TRIPLET}/include")
    link_directories("${_VCPKG_TRIPLET}/lib")
endif()

add_executable(deskclock ${DESKCLOCK_SRCS})

# App pak: the DeskClock res subset (layouts/drawables/strings from AOSP
# packages/apps/DeskClock/res), compiled to binary AXML/arsc and loaded at
# runtime as deskclock.pak beside the binary.
CreatePAK(deskclock ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/deskclock.pak ${PROJECT_SOURCE_DIR}/R.h)

target_link_libraries(deskclock PRIVATE cdroid)
if(DESKCLOCK_FFMPEG)
    target_link_libraries(deskclock PRIVATE avformat avcodec avutil swresample rtaudio)
endif()
install(TARGETS deskclock DESTINATION bin)
