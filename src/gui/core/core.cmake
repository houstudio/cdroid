SET(CORE_SOURCES
    core/app.cc
    core/assets.cc
    core/attributeset.cc
    #core/basebundle.cc
    #core/bundle.cc
    core/calendar.cc
    core/canvas.cc
    core/color.cc
    core/display.cc
    core/displaymetrics.cc
    core/epollwrapper.cc
    core/graphdevice.cc
    core/handler.cc
    core/inputdevice.cc
    core/inputeventsource.cc
    core/inputmethod.cc
    core/inputmethodmanager.cc
    core/insets.cc
    core/intent.cc
    core/uri.cc
    core/iostreams.cc
    core/keyboard.cc
    core/keycharactermap.cc
    core/keylayoutmap.cc
    core/layout.cc
    core/looper.cc
    core/parcel.cc
    core/path.cc
    core/pathmeasure.cc
    core/outline.cc
    core/porterduff.cc
    core/preferences.cc
    core/process.cc
    core/countdowntimer.cc
    core/scheduler.cc
    core/systemclock.cc
    core/tokenizer.cc
    core/xmlpullparser.cc
    #core/transform.cc
    core/typedvalue.cc
    core/typeface.cc
    core/uieventsource.cc
    core/windowmanager.cc
    core/ziparchive.cc
)

list(APPEND CORE_SOURCES
    core/wifimanager.cc
    core/wifissid.cc
)

if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message("Building on Windows with MSVC compiler")
    #list(APPEND CDROID_SOURCES core/wepoll.cc)
endif()

