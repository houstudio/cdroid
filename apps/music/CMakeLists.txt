cmake_minimum_required (VERSION 3.8)
project(music)

set(CMAKE_CXX_STANDARD 14)

# CDROID includes/libraries come from the parent scope (all apps/ projects
# configure under the cdroid tree; no find_package needed — see preferencedemo).
include_directories(
    ./
    src
    ${CDROID_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/porting
)

set(MUSIC_SRCS
    main.cc
    src/musicdb.cc
    src/mediaplaybackservice.cc
    src/musicutils.cc
    src/repeatingimagebutton.cc
    src/touchinterceptor.cc
    src/musicbrowseractivity.cc
    src/artistalbumbrowseractivity.cc
    src/albumbrowseractivity.cc
    src/trackbrowseractivity.cc
    src/playlistbrowseractivity.cc
    src/mediaplaybackactivity.cc
)

add_executable(music ${MUSIC_SRCS})

# App pak: the Music res subset (layouts/drawables/strings from AOSP
# packages/apps/Music/kotlin/res), compiled to binary AXML/arsc and loaded at
# runtime as music.pak beside the binary.
CreatePAK(music ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/music.pak ${PROJECT_SOURCE_DIR}/R.h)

target_link_libraries(music PRIVATE cdroid)
install(TARGETS music DESTINATION bin)
