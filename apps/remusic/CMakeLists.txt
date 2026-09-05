cmake_minimum_required (VERSION 3.8)
project(remusic)

set(CMAKE_CXX_STANDARD 14)

# CDROID includes/libraries come from the parent scope (all apps/ projects
# configure under the cdroid tree; no find_package needed — see deskclock).
include_directories(
    ./
    src
    ${CDROID_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/porting
)

# Real audio backend: FFmpeg decode + RtAudio output from the vcpkg triplet
# the CURRENT toolchain uses (VCPKG_TARGET_TRIPLET), falling back to probing
# every installed triplet. Absent everywhere -> wall-clock driver (silent).
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
    if(NOT REMUSIC_FFMPEG AND _VCPKG_ROOT
            AND EXISTS "${_VCPKG_ROOT}/installed/${_t}/include/libavcodec/avcodec.h"
            AND EXISTS "${_VCPKG_ROOT}/installed/${_t}/include/rtaudio")
        set(_VCPKG_TRIPLET "${_VCPKG_ROOT}/installed/${_t}")
        set(REMUSIC_FFMPEG ON)
        set(REMUSIC_TRIPLET_USED "${_t}")
    endif()
endforeach()
if(REMUSIC_FFMPEG)
    message(STATUS "remusic: real audio backend ON (triplet ${REMUSIC_TRIPLET_USED})")
endif()

# Online radio (radio-browser.info, key-free): libcurl fetch + JsonCpp parse.
# Plain file probe + raw link (find_package CONFIG needs the vcpkg toolchain
# in scope; the ffmpeg block above already set _VCPKG_TRIPLET + include/link dirs).
if(_VCPKG_TRIPLET
        AND EXISTS "${_VCPKG_TRIPLET}/include/curl/curl.h"
        AND EXISTS "${_VCPKG_TRIPLET}/include/json/json.h")
    set(REMUSIC_ONLINE ON)
    message(STATUS "remusic: online radio ON (curl+jsoncpp)")
endif()

set(REMUSIC_SRCS
    main.cc
    src/mainactivity.cc
    src/mediaplaybackservice.cc
    src/musicplayer.cc
    src/musicprovider.cc
    src/supportcompat.cc
    src/mainfragment.cc
    src/musicfragment.cc
    src/tabactivity.cc
    src/quickcontrols.cc
    src/lrcview.cc
    src/playingactivity.cc
    src/navactivities.cc
    src/playliststore.cc
    src/localsearchactivity.cc
    src/themestore.cc
)
if(REMUSIC_FFMPEG)
    list(APPEND REMUSIC_SRCS src/ffmpegbackend.cc)
endif()
if(REMUSIC_ONLINE)
    list(APPEND REMUSIC_SRCS src/radiobrowser.cc src/archiveorg.cc src/httputil.cc)
    add_definitions(-DREMUSIC_ONLINE)
endif()

if(REMUSIC_FFMPEG)
    add_definitions(-DREMUSIC_ENABLE_FFMPEG)
    option(REMUSIC_WALLCLOCK_ONLY "force the wall-clock driver" OFF)
    if(REMUSIC_WALLCLOCK_ONLY)
        add_definitions(-DREMUSIC_WALLCLOCK_ONLY)
    endif()
    include_directories(${_VCPKG_TRIPLET}/include)
    # debug/ first (matches the Debug flavor of the framework libs); release
    # lib/ backs the ports that ship no debug build naming (-lcurl etc.).
    link_directories(${_VCPKG_TRIPLET}/debug/lib ${_VCPKG_TRIPLET}/lib)
endif()

add_executable(remusic ${REMUSIC_SRCS})
if(REMUSIC_FFMPEG)
    target_link_libraries(remusic PRIVATE cdroid avformat avcodec avutil swresample rtaudio)
else()
    target_link_libraries(remusic PRIVATE cdroid)
endif()
if(REMUSIC_ONLINE)
    target_link_libraries(remusic PRIVATE curl jsoncpp)
endif()

CreatePAK(remusic ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/remusic.pak ${PROJECT_SOURCE_DIR}/R.h)
