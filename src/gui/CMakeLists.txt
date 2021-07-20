project(gui C CXX)

option( LUA_BINDINGS "UI Lua script bindings" OFF)
option( GUI_STATIC "UI is static link" ON)
set(CMAKE_CXX_STANDARD 11)
aux_source_directory(core SRCS_GUICORE)
aux_source_directory(views SRCS_VIEWS)
aux_source_directory(widget SRCS_WIDGET)
aux_source_directory(drawables SRCS_DRAWABLES)
aux_source_directory(cairomm   SRCS_CAIROMM)
aux_source_directory(dialogs SRCS_DIALOG)
aux_source_directory(looper SRCS_LOOP)
aux_source_directory(animation SRCS_ANIMATION)

message("UI Lua script bindings is ${LUA_BINDINGS} guistatic=${GUI_STATIC}")

include_directories(
    ./ ./private 
    ./core 
    ./views 
    ./cairomm 
    ./core/svg ./luabinding 
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/freetype2
    ${CMAKE_BINARY_DIR}/include/pixman
    ${CMAKE_BINARY_DIR}/include/cairo
    ${CMAKE_BINARY_DIR}/include/porting
    ${CMAKE_BINARY_DIR}/include/pinyin
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libunibreak/src
    ${CMAKE_BINARY_DIR}/include/fribidi
    ${CMAKE_SOURCE_DIR}/src/modules/vold
    ${CMAKE_SOURCE_DIR}/src/3rdparty/grt
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/include
)

add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(SOURCES_GUI ${SRCS_GUICORE} ${SRCS_VIEWS} 
    ${SRCS_WIDGET} ${SRCS_DRAWABLES} 
    ${SRCS_CAIROMM} ${SRCS_DIALOG} 
    ${SRCS_LOOP} ${SRCS_ANIMATION})

add_library(gui_objs OBJECT ${SOURCES_GUI})
add_library(gui_static STATIC $<TARGET_OBJECTS:gui_objs>)
add_library(gui SHARED $<TARGET_OBJECTS:gui_objs>)

add_custom_target(systempak
   COMMAND zip -r -0 ${CMAKE_CURRENT_BINARY_DIR}/system.pak *
   BYPRODUCTS gui gui_static
   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/res
   COMMENT "package system resource"
   )

CreatePAK(${PROJECT_SOURCE_DIR}/res  ${CMAKE_CURRENT_BINARY_DIR}/cdroid.pak)
set_target_properties(gui_static PROPERTIES OUTPUT_NAME "gui")

file(GLOB MMHEADERS   ${PROJECT_SOURCE_DIR}/cairomm/*.h)
file(GLOB GUIHEADERS  ${PROJECT_SOURCE_DIR}/core/*.h)
file(GLOB DRAWABLEHEADERS  ${PROJECT_SOURCE_DIR}/drawables/*.h)
file(GLOB WIDGETHEADERS  ${PROJECT_SOURCE_DIR}/widget/*.h)
file(GLOB VIEWSHEADERS  ${PROJECT_SOURCE_DIR}/views/*.h)
file(GLOB LOOPERHEADERS  ${PROJECT_SOURCE_DIR}/looper/*.h)
file(GLOB ANIMHEADERS  ${PROJECT_SOURCE_DIR}/animation/*.h)
message("=======OPTIONAL_LIBS=${OPTIONAL_LIBS}")

target_link_libraries(gui pixman-1 png jpeg cairo unibreak zip jsoncpp tvhal ${OPTIONAL_LIBS})
target_link_libraries(gui_static pixman-1 png jpeg cairo_static unibreak  zip jsoncpp tvhal_static ${OPTIONAL_LIBS})

file(COPY ${GUIHEADERS} ${VIEWSHEADERS} windows.h drawables.h  DESTINATION ${CMAKE_BINARY_DIR}/include/gui PATTERN "*.h")
file(COPY ${MMHEADERS}     DESTINATION ${CMAKE_BINARY_DIR}/include/gui/cairomm PATTERN "*.h")
file(COPY ${WIDGETHEADERS} DESTINATION ${CMAKE_BINARY_DIR}/include/gui/widget PATTERN "*.h")
file(COPY ${DRAWABLEHEADERS} DESTINATION ${CMAKE_BINARY_DIR}/include/gui/drawables PATTERN "*.h")
file(COPY ${LOOPERHEADERS} DESTINATION ${CMAKE_BINARY_DIR}/include/looper PATTERN "*.h")

install (TARGETS gui  DESTINATION lib)
install (TARGETS gui_static  DESTINATION lib)
install (DIRECTORY ${PROJECT_SOURCE_DIR} DESTINATION include/  FILES_MATCHING PATTERN "*.h"
    PATTERN "views" EXCLUDE 
    PATTERN "core" EXCLUDE)
install (FILES ${MMHEADERS} DESTINATION include/gui/cairomm/ )
install (FILES ${GUIHEADERS} windows.h drawables.h DESTINATION include/gui/)
install (FILES ${WIDGETHEADERS} DESTINATION include/gui/widget)
install (FILES ${DRAWABLEHEADERS} DESTINATION include/gui/drawables)
install (FILES ${VIEWSHEADERS} DESTINATION include/gui/)
install (FILES ${LOOPERHEADERS} DESTINATION include/looper)

